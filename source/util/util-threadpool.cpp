/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2020 Michael Fabian Dirks
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#include "util-threadpool.hpp"
#include "common.hpp"
#include "util/util-logging.hpp"

#include "warning-disable.hpp"
#include <cstddef>
#include "warning-enable.hpp"

#ifdef _DEBUG
#define ST_PREFIX "<%s> "
#define D_LOG_ERROR(x, ...) P_LOG_ERROR(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_WARNING(x, ...) P_LOG_WARN(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_INFO(x, ...) P_LOG_INFO(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_DEBUG(x, ...) P_LOG_DEBUG(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#else
#define ST_PREFIX "<util::threadpool> "
#define D_LOG_ERROR(...) P_LOG_ERROR(ST_PREFIX __VA_ARGS__)
#define D_LOG_WARNING(...) P_LOG_WARN(ST_PREFIX __VA_ARGS__)
#define D_LOG_INFO(...) P_LOG_INFO(ST_PREFIX __VA_ARGS__)
#define D_LOG_DEBUG(...) P_LOG_DEBUG(ST_PREFIX __VA_ARGS__)
#endif

// Most Tasks likely wait for IO, so we can use that time for other tasks.
#define ST_CONCURRENCY_MULTIPLIER 2

streamfx::util::threadpool::threadpool()
	: _workers(), _worker_stop(false), _worker_idx(0), _tasks(), _tasks_lock(), _tasks_cv()
{
	std::size_t concurrency = static_cast<size_t>(std::thread::hardware_concurrency() * ST_CONCURRENCY_MULTIPLIER);
	for (std::size_t n = 0; n < concurrency; n++) {
		_workers.emplace_back(std::bind(&streamfx::util::threadpool::work, this));
	}
}

streamfx::util::threadpool::~threadpool()
{
	_worker_stop = true;
	_tasks_cv.notify_all();
	for (auto& thread : _workers) {
		_tasks_cv.notify_all();
		if (thread.joinable()) {
			thread.join();
		}
	}
}

std::shared_ptr<::streamfx::util::threadpool::task> streamfx::util::threadpool::push(threadpool_callback_t fn,
																					 threadpool_data_t     data)
{
	auto task = std::make_shared<streamfx::util::threadpool::task>(fn, data);

	// Append the task to the queue.
	std::unique_lock<std::mutex> lock(_tasks_lock);
	_tasks.emplace_back(task);
	_tasks_cv.notify_one();

	return task;
}

void streamfx::util::threadpool::pop(std::shared_ptr<::streamfx::util::threadpool::task> work)
{
	if (work) {
		{
			std::unique_lock<std::mutex> lock(work->_mutex);
			work->_is_dead = true;
		}
		work->_is_complete.notify_all();
	}
}

void streamfx::util::threadpool::work()
{
	std::shared_ptr<streamfx::util::threadpool::task> local_work{};
	uint32_t                                          local_number = _worker_idx.fetch_add(1);

	while (!_worker_stop) {
		// Wait for more work, or immediately continue if there is still work to do.
		{
			// Lock the tasks mutex to check for work.
			std::unique_lock<std::mutex> lock(_tasks_lock);

			// If there are currently no tasks queued, wait on the condition variable.
			// This temporarily unlocks the mutex until it is woken up.
			if (_tasks.size() == 0) {
				_tasks_cv.wait(lock, [this]() { return _worker_stop || _tasks.size() > 0; });
			}

			// If there is either no tasks or we were asked to stop, skip everything.
			if (_worker_stop || (_tasks.size() == 0)) {
				continue;
			}

			// Grab the latest task and immediately remove it from the queue.
			local_work = _tasks.front();
			_tasks.pop_front();
		}

		// If the task was killed, skip everything again.
		if (local_work->_is_dead.load()) {
			continue;
		}

		// Try to execute work, but don't crash on catchable exceptions.
		if (local_work->_callback) {
			try {
				local_work->_callback(local_work->_data);
			} catch (std::exception const& ex) {
				D_LOG_WARNING("Worker %" PRIx32 " caught exception from task (%" PRIxPTR ", %" PRIxPTR
							  ") with message: %s",
							  local_number, reinterpret_cast<ptrdiff_t>(local_work->_callback.target<void>()),
							  reinterpret_cast<ptrdiff_t>(local_work->_data.get()), ex.what());
			} catch (...) {
				D_LOG_WARNING("Worker %" PRIx32 " caught exception of unknown type from task (%" PRIxPTR ", %" PRIxPTR
							  ").",
							  local_number, reinterpret_cast<ptrdiff_t>(local_work->_callback.target<void>()),
							  reinterpret_cast<ptrdiff_t>(local_work->_data.get()));
			}
			{
				std::unique_lock<std::mutex> lock(local_work->_mutex);
				local_work->_is_dead.store(true);
			}
			local_work->_is_complete.notify_all();
		}

		// Remove our reference to the work unit.
		local_work.reset();
	}

	_worker_idx.fetch_sub(1);
}

streamfx::util::threadpool::task::task() = default;

streamfx::util::threadpool::task::task(threadpool_callback_t fn, threadpool_data_t dt)
	: _mutex(), _is_complete(), _is_dead(false), _callback(fn), _data(dt)
{}

void streamfx::util::threadpool::task::await_completion()
{
	if (!_is_dead) {
		std::unique_lock<std::mutex> lock(_mutex);
		_is_complete.wait(lock, [this]() { return this->_is_dead.load(); });
	}
}
