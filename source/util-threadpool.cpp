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
#include <cstddef>

// Most Tasks likely wait for IO, so we can use that time for other tasks.
#define CONCURRENCY_MULTIPLIER 2

util::threadpool::threadpool() : _workers(), _worker_stop(false), _tasks(), _tasks_lock(), _tasks_cv()
{
	size_t concurrency = static_cast<size_t>(std::thread::hardware_concurrency()) * CONCURRENCY_MULTIPLIER;
	for (size_t n = 0; n < concurrency; n++) {
		_workers.emplace_back(std::bind(&util::threadpool::work, this));
	}
}

util::threadpool::~threadpool()
{
	_worker_stop = true;
	_tasks_cv.notify_all();
	for (auto& thread : _workers) {
		_tasks_cv.notify_all();
		if (thread.joinable())
			thread.join();
	}
}

void util::threadpool::push(threadpool_function_t fn, std::shared_ptr<void> data)
{
	std::unique_lock<std::mutex> lock(_tasks_lock);
	_tasks.emplace_back(fn, data);
	_tasks_cv.notify_one();
}

void util::threadpool::work()
{
	std::pair<threadpool_function_t, std::shared_ptr<void>> work;

	while (!_worker_stop) {
		// Wait for more work, or immediately continue if there is still work to do.
		{
			std::unique_lock<std::mutex> lock(_tasks_lock);
			if (_tasks.size() == 0)
				_tasks_cv.wait(lock, [this]() { return _worker_stop || _tasks.size() > 0; });
			if (_tasks.size() == 0)
				continue;
			work = _tasks.front();
			_tasks.pop_front();
		}

		// Execute work.
		if (work.first)
			work.first(work.second);
	}
}
