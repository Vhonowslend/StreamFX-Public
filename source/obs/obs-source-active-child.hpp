/*
 * Copyright (C) 2022 Michael Fabian Dirks
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
#include "obs-source.hpp"
#include "obs-tools.hpp"
#include "obs-weak-source.hpp"

namespace streamfx::obs {
	class source_active_child {
		::streamfx::obs::weak_source _parent;
		::streamfx::obs::weak_source _child;

		public:
		~source_active_child()
		{
			auto parent = _parent.lock();
			auto child  = _child.lock();
			if (parent && child) {
				obs_source_remove_active_child(parent, child);
			}
		}
		source_active_child(::streamfx::obs::source const& parent, ::streamfx::obs::source const& child)
			: _parent(parent), _child(child)
		{
			if (::streamfx::obs::tools::source_find_source(child, parent)) {
				throw std::runtime_error("Child contains Parent");
			} else if (!obs_source_add_active_child(parent, child)) {
				throw std::runtime_error("Child contains Parent");
			}
		}
	};
} // namespace streamfx::obs
