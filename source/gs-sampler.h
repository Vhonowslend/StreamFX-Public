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
#include <inttypes.h>
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/graphics/graphics.h>
	#pragma warning( pop )
}

namespace gs {
	class sampler {
		public:
		sampler();
		~sampler();

		void set_filter(gs_sample_filter v);
		gs_sample_filter get_filter();

		void set_address_mode_u(gs_address_mode v);
		gs_address_mode get_address_mode_u();

		void set_address_mode_v(gs_address_mode v);
		gs_address_mode get_address_mode_v();

		void set_address_mode_w(gs_address_mode v);
		gs_address_mode get_address_mode_w();

		void set_max_anisotropy(int v);
		int get_max_anisotropy();

		void set_border_color(uint32_t v);
		void set_border_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
		uint32_t get_border_color();
		uint8_t get_border_color(bool r, bool g, bool b, bool a);

		gs_sampler_state* refresh();

		gs_sampler_state* get_object();

		private:
		bool m_dirty;
		gs_sampler_info m_samplerInfo;
		gs_sampler_state* m_samplerState;
	};
}
