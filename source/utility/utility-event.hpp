/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2018 Michael Fabian Dirks
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

#ifndef OBS_STREAM_EFFECTS_UTILITY_EVENT_CPP
#define OBS_STREAM_EFFECTS_UTILITY_EVENT_CPP
#pragma once

#include <functional>
#include <list>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

namespace utility {
	typedef std::map<std::string, void*>                  arguments_t;
	typedef std::shared_ptr<void>                         lifeline_t;
	typedef std::function<bool(lifeline_t, arguments_t&)> callback_t;
	typedef std::pair<lifeline_t, callback_t>             listener_t;

	class event {
		std::list<listener_t> _listeners;

		public:
		size_t add(listener_t listener);

		inline event operator+=(listener_t listener)
		{
			add(listener);
			return *this;
		}

		size_t remove(listener_t listener);

		inline event operator-=(listener_t listener)
		{
			remove(listener);
			return *this;
		}

		size_t count();

		bool empty();

		inline operator bool()
		{
			return !this->empty();
		}

		void clear();

		size_t call(arguments_t& arguments);

		inline size_t operator()(arguments_t& arguments)
		{
			return call(arguments);
		}

		inline size_t call()
		{
			return call(arguments_t{});
		}

		inline size_t operator()()
		{
			return call(arguments_t{});
		}

		public:
		struct {
			utility::event add;
			utility::event remove;
			utility::event empty;
		} on;
	};
} // namespace utility

#endif OBS_STREAM_EFFECTS_UTILITY_EVENT_CPP
