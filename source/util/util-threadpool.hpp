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

#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>

namespace streamfx::util {
	typedef std::shared_ptr<void>                  threadpool_data_t;
	typedef std::function<void(threadpool_data_t)> threadpool_callback_t;

	class threadpool {
		public:
		class task {
			protected:
			std::atomic_bool      _is_dead;
			threadpool_callback_t _callback;
			threadpool_data_t     _data;

			public:
			task();
			task(threadpool_callback_t callback_function, threadpool_data_t data);

			friend class streamfx::util::threadpool;
		};

		private:
		std::list<std::thread>                                         _workers;
		std::atomic_bool                                               _worker_stop;
		std::atomic<uint32_t>                                          _worker_idx;
		std::list<std::shared_ptr<::streamfx::util::threadpool::task>> _tasks;
		std::mutex                                                     _tasks_lock;
		std::condition_variable                                        _tasks_cv;

		public:
		threadpool();
		~threadpool();

		std::shared_ptr<::streamfx::util::threadpool::task> push(threadpool_callback_t callback_function,
																 threadpool_data_t     data);

		void pop(std::shared_ptr<::streamfx::util::threadpool::task> work);

		private:
		void work();
	};
} // namespace streamfx::util
