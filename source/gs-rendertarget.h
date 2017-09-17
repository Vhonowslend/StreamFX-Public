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
	class RenderTarget {
		friend class RenderTargetOp;

		public:
		RenderTarget(gs_color_format colorFormat, gs_zstencil_format zsFormat);
		virtual ~RenderTarget();

		gs_texture_t* GetTextureObject();
		GS::RenderTargetOp Render(uint32_t width, uint32_t height);

		protected:
		gs_texrender_t* m_renderTarget;
		bool m_isBeingRendered;
	};

	class RenderTargetOp {
		public:
		RenderTargetOp(GS::RenderTarget* rt, uint32_t width, uint32_t height);
		virtual ~RenderTargetOp();

		// Move Constructor
		RenderTargetOp(GS::RenderTargetOp&&);

		// Copy Constructor
		RenderTargetOp(const GS::RenderTargetOp&) = delete;
		RenderTargetOp& operator=(const GS::RenderTargetOp& r) = delete;

		protected:
		GS::RenderTarget* m_renderTarget;
	};
}
