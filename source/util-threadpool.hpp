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
#include <functional>
#include <list>
#include <mutex>
#include <thread>

namespace util {
	typedef std::function<void(std::shared_ptr<void>)> threadpool_function_t;

	class threadpool {
		std::list<std::thread>                                             _workers;
		std::atomic_bool                                                   _worker_stop;
		std::list<std::pair<threadpool_function_t, std::shared_ptr<void>>> _tasks;
		std::mutex                                                         _tasks_lock;
		std::condition_variable                                            _tasks_cv;

		public:
		threadpool();
		~threadpool();

		void push(threadpool_function_t fn, std::shared_ptr<void> data);

		private:
		void work();
	};
} // namespace util
