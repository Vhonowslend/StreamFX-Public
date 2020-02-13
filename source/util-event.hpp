/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2017 Michael Fabian Dirks
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
#include <functional>
#include <list>

namespace util {
	template<typename... _args>
	class event {
		std::list<std::function<void(_args...)>> listeners;

		std::function<void()> listen_cb;
		std::function<void()> silence_cb;

		public /* functions */:

		// Destructor
		virtual ~event()
		{
			this->clear();
		}

		// Add new listener.
		inline void add(std::function<void(_args...)> listener)
		{
			if (listeners.size() == 0) {
				if (listen_cb) {
					listen_cb();
				}
			}
			listeners.push_back(listener);
		}

		// Remove existing listener.
		inline void remove(std::function<void(_args...)> listener)
		{
			listeners.remove(listener);
			if (listeners.size() == 0) {
				if (silence_cb) {
					silence_cb();
				}
			}
		}

		// Check if empty / no listeners.
		inline bool empty()
		{
			return listeners.empty();
		}

		// Remove all listeners.
		inline void clear()
		{
			listeners.clear();
			if (silence_cb) {
				silence_cb();
			}
		}

		public /* operators */:
		// Call Listeners with arguments.
		/// Not valid without the extra template.
		template<typename... _largs>
		inline void operator()(_args... args)
		{
			for (auto& l : listeners) {
				l(args...);
			}
		}

		// Convert to bool (true if not empty, false if empty).
		inline operator bool()
		{
			return !this->empty();
		}

		// Add new listener.
		inline event<_args...>& operator+=(std::function<void(_args...)> listener)
		{
			this->add(listener);
			return *this;
		}

		// Remove existing listener.
		inline event<_args...>& operator-=(std::function<void(_args...)> listener)
		{
			this->remove(listener);
			return *this;
		}

		public /* events */:
		void set_listen_callback(std::function<void()> cb)
		{
			this->listen_cb = cb;
		}

		void set_silence_callback(std::function<void()> cb)
		{
			this->silence_cb = cb;
		}
	};
} // namespace util
