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
#include <memory>
#include <vector>
#include "obs/gs/gs-mipmapper.hpp"
#include "obs/gs/gs-rendertarget.hpp"
#include "obs/gs/gs-texture.hpp"
#include "obs/gs/gs-vertexbuffer.hpp"
#include "plugin.hpp"

namespace filter {
	namespace transform {
		class transform_factory {
			obs_source_info _source_info;

			public: // Singleton
			static void                               initialize();
			static void                               finalize();
			static std::shared_ptr<transform_factory> get();

			public:
			transform_factory();
			~transform_factory();

			static const char*       get_name(void*);
			static void              get_defaults(obs_data_t*);
			static obs_properties_t* get_properties(void*);
			static bool              modified_properties(obs_properties_t*, obs_property_t*, obs_data_t*);

			static void*    create(obs_data_t*, obs_source_t*);
			static void     destroy(void*);
			static uint32_t get_width(void*);
			static uint32_t get_height(void*);
			static void     update(void*, obs_data_t*);
			static void     activate(void*);
			static void     deactivate(void*);
			static void     show(void*);
			static void     hide(void*);
			static void     video_tick(void*, float);
			static void     video_render(void*, gs_effect_t*);
		};

		class transform_instance {
			bool          _active;
			obs_source_t* _self;

			// Input
			std::shared_ptr<gs::rendertarget> _source_rendertarget;
			std::shared_ptr<gs::texture>      _source_texture;
			bool                              _source_rendered;
			std::pair<uint32_t, uint32_t>     _source_size;

			// Mipmapping
			bool                     _mipmap_enabled;
			double_t                 _mipmap_strength;
			gs::mipmapper::generator _mipmap_generator;
			gs::mipmapper            _mipmapper;

			// Rendering
			std::shared_ptr<gs::rendertarget> _shape_rendertarget;
			std::shared_ptr<gs::texture>      _shape_texture;

			// Mesh
			bool                               _update_mesh;
			std::shared_ptr<gs::vertex_buffer> _vertex_buffer;
			uint32_t                           _rotation_order;
			std::unique_ptr<util::vec3a>       _position;
			std::unique_ptr<util::vec3a>       _rotation;
			std::unique_ptr<util::vec3a>       _scale;
			std::unique_ptr<util::vec3a>       _shear;

			// Camera
			bool    _camera_orthographic;
			float_t _camera_fov;

			public:
			~transform_instance();
			transform_instance(obs_data_t*, obs_source_t*);

			uint32_t get_width();
			uint32_t get_height();

			void update(obs_data_t*);
			void activate();
			void deactivate();
			void video_tick(float);
			void video_render(gs_effect_t*);
		};
	} // namespace transform
} // namespace filter
