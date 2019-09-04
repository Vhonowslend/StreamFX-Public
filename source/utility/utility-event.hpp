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
	typedef std::weak_ptr<void>          lifeline_t;
	typedef std::map<std::string, void*> event_args_t;
#define D_EVENT_FUNC_T void(std::weak_ptr<void>, event_args_t)
	typedef std::function<D_EVENT_FUNC_T> event_cb_t;

	struct event_pair {
		lifeline_t lifeline;
		event_cb_t callback;

		event_pair(lifeline_t _lifeline, event_cb_t _callback)
		{
			lifeline = _lifeline;
			callback = _callback;
		}

		bool operator==(const event_pair& rhs)
		{
			return (!lifeline.owner_before(rhs.lifeline) && !rhs.lifeline.owner_before(lifeline))
				   && (lifeline.lock() == rhs.lifeline.lock())
				   && (callback.target<D_EVENT_FUNC_T>() == rhs.callback.target<D_EVENT_FUNC_T>());
		}
	};

	typedef event_pair listener_t;

	class event {
		std::list<listener_t> _listeners;

		public:
		event();
		virtual ~event();

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

		size_t call(event_args_t& arguments);

		inline size_t operator()(event_args_t& arguments)
		{
			return call(arguments);
		}

		inline size_t call()
		{
			event_args_t args{};
			return call(args);
		}

		inline size_t operator()()
		{
			return call();
		}

		public:
		struct {
			std::shared_ptr<utility::event> add;
			std::shared_ptr<utility::event> remove;
			std::shared_ptr<utility::event> empty;
		} on;
	};
} // namespace utility

#endif OBS_STREAM_EFFECTS_UTILITY_EVENT_CPP
