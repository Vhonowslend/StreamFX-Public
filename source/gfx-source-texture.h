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
#include <memory>
#include <string>
#include "gs-rendertarget.h"
#include "gs-texture.h"

extern "C" {
#include <obs.h>
}

namespace gfx {
	class source_texture {
		obs_source_t* source;
		obs_source_t* target;

		std::shared_ptr<gs::rendertarget> m_rt;

		source_texture(obs_source_t* parent);

		public:
		~source_texture();
		source_texture(const char* name, obs_source_t* parent);
		source_texture(std::string name, obs_source_t* parent);
		source_texture(obs_source_t* src, obs_source_t* parent);

		obs_source_t* get_object();
		obs_source_t* get_parent();

		std::shared_ptr<gs::texture> render(size_t width, size_t height);
	};
} // namespace gfx
