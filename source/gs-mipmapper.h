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
#include "gs-vertexbuffer.h"
#include "gs-rendertarget.h"
#include "gs-texture.h"

extern "C" {
	#pragma warning (push)
	#pragma warning (disable: 4201)
	#include "libobs/graphics/graphics.h"
	#pragma warning (pop)
}

namespace GS {
	class MipMapper {
		public:
		MipMapper();
		~MipMapper();

		gs_texture_t* Render(gs_texture_t*);

		private:
		GS::VertexBuffer m_vb;
		GS::RenderTarget m_rt;
		GS::Texture m_buf;
	};
}
