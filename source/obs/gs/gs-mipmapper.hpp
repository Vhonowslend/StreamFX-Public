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
#include "common.hpp"
#include "gs-effect.hpp"
#include "gs-rendertarget.hpp"
#include "gs-texture.hpp"
#include "gs-vertexbuffer.hpp"

/* gs::mipmapper is an attempt at adding dynamic mip-map generation to a software
 *  which only supports static mip-maps. It is effectively an incredibly bad hack
 *  instead of a proper solution - can break any time and likely already has.
 *
 * Needless to say, dynamic mip-map generation costs a lot of GPU time, especially
 *  when things need to be synchronized. In the ideal case we would just render 
 *  straight to the mip level, but this is not possible in DirectX 11 and OpenGL.
 * 
 * So instead we render to a render target and copy from there to the actual
 *  resource. Super wasteful, but what else can we actually do?
 */

namespace streamfx::obs::gs {
	class mipmapper {
		std::unique_ptr<streamfx::obs::gs::rendertarget> _rt;
		streamfx::obs::gs::effect                        _effect;

		public:
		~mipmapper();
		mipmapper();

		uint32_t calculate_max_mip_level(uint32_t width, uint32_t height);

		void rebuild(std::shared_ptr<streamfx::obs::gs::texture> source,
					 std::shared_ptr<streamfx::obs::gs::texture> target);
	};
} // namespace streamfx::obs::gs
