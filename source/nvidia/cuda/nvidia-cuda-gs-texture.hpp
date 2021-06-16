/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2020 Michael Fabian Dirks
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
#include <cstddef>
#include <memory>
#include "nvidia-cuda-stream.hpp"
#include "nvidia-cuda.hpp"
#include "obs/gs/gs-texture.hpp"

namespace streamfx::nvidia::cuda {
	class gstexture {
		std::shared_ptr<::streamfx::nvidia::cuda::cuda> _cuda;
		std::shared_ptr<streamfx::obs::gs::texture>     _texture;
		graphics_resource_t                             _resource;

		bool                                            _is_mapped;
		array_t                                         _pointer;
		std::shared_ptr<streamfx::nvidia::cuda::stream> _stream;

		public:
		~gstexture();
		gstexture(std::shared_ptr<streamfx::obs::gs::texture> texture);

		array_t map(std::shared_ptr<streamfx::nvidia::cuda::stream> stream);
		void    unmap();

		std::shared_ptr<streamfx::obs::gs::texture>   get_texture();
		::streamfx::nvidia::cuda::graphics_resource_t get();
	};
} // namespace streamfx::nvidia::cuda
