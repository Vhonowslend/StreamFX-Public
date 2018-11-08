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

#ifndef OBS_STREAM_EFFECTS_OBS_SOURCE_HPP
#define OBS_STREAM_EFFECTS_OBS_SOURCE_HPP
#pragma once

#include <cinttypes>
#include <string>

extern "C" {
#include "obs.h"
}

namespace obs {
	class source {
		obs_source_t* self;
		bool          track_ownership = false;

		static void handle_destroy(void* p, calldata_t* calldata);

		public:
		virtual ~source();

		source(std::string name, bool track_ownership = true, bool add_reference = true);

		source(obs_source_t* source, bool track_ownership = true, bool add_reference = false);

		source& operator=(const source& ref);

		source& operator=(source&& ref) noexcept;

		obs_source_type type();

		void* type_data();

		uint32_t width();
		uint32_t height();

		bool destroyed();

		public: // Unsafe Methods
		void clear();

		obs_source_t* get();
	};
} // namespace obs

#endif
