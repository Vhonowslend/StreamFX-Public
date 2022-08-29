// Modern effects for a modern Streamer
// Copyright (C) 2017 Michael Fabian Dirks
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

#pragma once
#include "common.hpp"
#include "obs/gs/gs-rendertarget.hpp"
#include "obs/gs/gs-texture.hpp"
#include "obs/obs-weak-source.hpp"

#include "warning-disable.hpp"
#include <map>
#include "warning-enable.hpp"

namespace streamfx::gfx {
	class source_texture {
		streamfx::obs::source _parent;
		streamfx::obs::source _child;

		std::shared_ptr<streamfx::obs::gs::rendertarget> _rt;

		public:
		~source_texture();
		source_texture(streamfx::obs::weak_source child, streamfx::obs::weak_source parent);

		public /*copy*/:
		source_texture(source_texture const& other)            = delete;
		source_texture& operator=(source_texture const& other) = delete;

		public /*move*/:
		source_texture(source_texture&& other)            = delete;
		source_texture& operator=(source_texture&& other) = delete;

		public:
		std::shared_ptr<streamfx::obs::gs::texture> render(std::size_t width, std::size_t height);

		public: // Unsafe Methods
		void clear();

		obs_source_t* get_object();
		obs_source_t* get_parent();
	};
} // namespace streamfx::gfx
