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
#include "obs-weak-source.hpp"

namespace streamfx::obs {
	class source_active_reference {
		::streamfx::obs::weak_source _target;

		public:
		~source_active_reference()
		{
			auto v = _target.lock();
			if (v) {
				v.decrement_active();
			}
		}
		source_active_reference(::streamfx::obs::source& source) : _target(source)
		{
			source.increment_active();
		}

		public:
		static FORCE_INLINE std::shared_ptr<source_active_reference>
							add_active_reference(::streamfx::obs::source& source)
		{
			return std::make_shared<source_active_reference>(source);
		}
	};
} // namespace streamfx::obs
