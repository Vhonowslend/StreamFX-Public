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
			friend class std::_Ptr_base<filter::transform::transform_factory>;

			obs_source_info sourceInfo;

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
			bool          m_active;
			obs_source_t* m_self;

			// Input
			std::shared_ptr<gs::rendertarget> m_source_rendertarget;
			std::shared_ptr<gs::texture>      m_source_texture;
			bool                              m_source_rendered;

			// Mipmapping
			bool                     m_mipmap_enabled;
			double_t                 m_mipmap_strength;
			gs::mipmapper::generator m_mipmap_generator;
			gs::mipmapper            m_mipmapper;

			// Rendering
			std::shared_ptr<gs::rendertarget> m_shape_rendertarget;
			std::shared_ptr<gs::texture>      m_shape_texture;

			// Mesh
			bool                               m_update_mesh;
			std::shared_ptr<gs::vertex_buffer> m_vertex_buffer;
			uint32_t                           m_rotation_order;
			std::unique_ptr<util::vec3a>       m_position;
			std::unique_ptr<util::vec3a>       m_rotation;
			std::unique_ptr<util::vec3a>       m_scale;
			std::unique_ptr<util::vec3a>       m_shear;

			// Camera
			bool    m_camera_orthographic;
			float_t m_camera_fov;

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
