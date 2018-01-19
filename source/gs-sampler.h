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

namespace GS {
	class Sampler {
		public:
		Sampler();
		~Sampler();

		void SetSampleFilter(gs_sample_filter v);
		gs_sample_filter GetSampleFilter();

		void SetAddressModeU(gs_address_mode v);
		gs_address_mode GetAddressModeU();

		void SetAddressModeV(gs_address_mode v);
		gs_address_mode GetAddressModeV();

		void SetAddressModeW(gs_address_mode v);
		gs_address_mode GetAddressModeW();

		void SetMaxAnisotropy(int v);
		int GetMaxAnisotropy();

		void SetBorderColor(uint32_t v);
		void SetBorderColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
		uint32_t GetBorderColor();
		uint8_t GetBorderColor(bool r, bool g, bool b, bool a);

		gs_sampler_state* Refresh();

		gs_sampler_state* GetObject();

		private:
		bool m_dirty;
		gs_sampler_info m_samplerInfo;
		gs_sampler_state* m_samplerState;
	};
}
