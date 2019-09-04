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

size_t utility::event::add(listener_t callback)
{
	_listeners.push_back(callback);

	{
		arguments_t args;
		args.emplace("event", this);
		args.emplace("listener", &callback);
		on.add(args);
	}

	return _listeners.size();
}

size_t utility::event::remove(listener_t callback)
{
	_listeners.remove(callback);

	{
		arguments_t args;
		args.emplace("event", this);
		args.emplace("listener", &callback);
		on.remove(args);
	}
	
	if (_listeners.empty()) {
		arguments_t args;
		args.emplace("event", this);
		on.empty(args);
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
	return _listeners.clear();
}

size_t utility::event::call(arguments_t& arguments)
{
	size_t idx = 0;
	for (auto const& listener : _listeners) {
		if (listener.second(listener.first, arguments)) {
			break;
		}
		idx++;
	}
	return idx;
}

size_t utility::event::operator()(arguments_t& arguments)
{
	return call(arguments);
}
