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

#include "utility-event.hpp"

utility::event::event()
{
	on.add    = std::make_shared<utility::event>();
	on.remove = std::make_shared<utility::event>();
	on.empty  = std::make_shared<utility::event>();
}

utility::event::~event()
{
	clear();
	on.add.reset();
	on.remove.reset();
	on.empty.reset();
}

size_t utility::event::add(listener_t callback)
{
	_listeners.push_back(callback);

	{
		event_args_t args;
		args.emplace("event", this);
		args.emplace("listener", &callback);
		on.add->call(args);
	}

	return _listeners.size();
}

size_t utility::event::remove(listener_t callback)
{
	_listeners.remove(callback);

	{
		event_args_t args;
		args.emplace("event", this);
		args.emplace("listener", &callback);
		on.remove->call(args);
	}

	if (_listeners.empty()) {
		event_args_t args;
		args.emplace("event", this);
		on.empty->call(args);
	}

	return _listeners.size();
}

size_t utility::event::count()
{
	return _listeners.size();
}

bool utility::event::empty()
{
	return _listeners.empty();
}

void utility::event::clear()
{
	_listeners.clear();
	event_args_t args;
	args.emplace("event", this);
	on.empty->call(args);
}

size_t utility::event::call(event_args_t& arguments)
{
	size_t idx = 0;
	for (auto const& listener : _listeners) {
		/*if (listener.first.expired()) {
			// Lifeline has expired, so remove it.
			_listeners.remove(listener);
			continue;
		}*/

		//if (listener.second(listener.first, arguments)) {
//			break;
	//	}
		idx++;
	}
	return idx;
}
