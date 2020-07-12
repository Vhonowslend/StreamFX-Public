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
#include "common.hpp"
#include <chrono>
#include <map>
#include <mutex>

namespace util {
	class profiler : public std::enable_shared_from_this<util::profiler> {
		std::map<std::chrono::nanoseconds, size_t> _timings;
		std::mutex                                 _timings_lock;

		public:
		class instance {
			std::shared_ptr<profiler>                      _parent;
			std::chrono::high_resolution_clock::time_point _start;

			public:
			instance(std::shared_ptr<profiler> parent);

			~instance();

			void cancel();

			void reparent(std::shared_ptr<profiler> parent);
		};

		private:
		profiler();

		public:
		~profiler();

		std::shared_ptr<class util::profiler::instance> track();

		void track(std::chrono::nanoseconds duration);

		std::uint64_t count();

		std::chrono::nanoseconds total_duration();

		double_t average_duration();

		std::chrono::nanoseconds percentile(double_t percentile, bool by_time = false);

		public:
		static std::shared_ptr<util::profiler> create()
		{
			return std::shared_ptr<util::profiler>{new profiler()};
		}
	};
} // namespace util
