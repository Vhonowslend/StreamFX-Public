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
#include "gs-texture.hpp"

namespace streamfx::obs::gs {
	class rendertarget_op;

	class rendertarget {
		friend class rendertarget_op;

		protected:
		gs_texrender_t* _render_target;
		bool            _is_being_rendered;

		gs_color_format    _color_format;
		gs_zstencil_format _zstencil_format;

		public:
		~rendertarget();

		rendertarget(gs_color_format colorFormat, gs_zstencil_format zsFormat);

		gs_texture_t* get_object();

		std::shared_ptr<streamfx::obs::gs::texture> get_texture();

		void get_texture(streamfx::obs::gs::texture& tex);

		void get_texture(std::shared_ptr<streamfx::obs::gs::texture>& tex);

		void get_texture(std::unique_ptr<streamfx::obs::gs::texture>& tex);

		gs_color_format get_color_format();

		gs_zstencil_format get_zstencil_format();

		streamfx::obs::gs::rendertarget_op render(uint32_t width, uint32_t height);
	};

	class rendertarget_op {
		streamfx::obs::gs::rendertarget* parent;

		public:
		~rendertarget_op();

		rendertarget_op(streamfx::obs::gs::rendertarget* rt, uint32_t width, uint32_t height);

		// Move Constructor
		rendertarget_op(streamfx::obs::gs::rendertarget_op&&);

		// Copy Constructor
		rendertarget_op(const streamfx::obs::gs::rendertarget_op&) = delete;

		rendertarget_op& operator=(const streamfx::obs::gs::rendertarget_op& r) = delete;
	};
} // namespace streamfx::obs::gs
