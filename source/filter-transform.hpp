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
#include "gs-mipmapper.hpp"
#include "gs-rendertarget.hpp"
#include "gs-texture.hpp"
#include "gs-vertexbuffer.hpp"
#include "plugin.hpp"

namespace filter {
	class TransformAddon {
		obs_source_info sourceInfo;

		public:
		TransformAddon();
		~TransformAddon();

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

	class Transform {
		obs_source_t* source_context;

		// Graphics Data
		std::shared_ptr<gs::vertex_buffer> vertex_buffer;
		std::shared_ptr<gs::rendertarget>  source_rt;
		std::shared_ptr<gs::rendertarget>  shape_rt;

		// Mipmapping
		bool                         enable_mipmapping;
		double_t                     generator_strength;
		gs::mipmapper::generator     generator;
		std::shared_ptr<gs::texture> source_texture;
		gs::mipmapper                mipmapper;

		// Camera
		bool    is_orthographic;
		float_t field_of_view;

		// Source
		bool is_inactive;
		bool is_hidden;
		bool is_mesh_update_required;

		// 3D Information
		uint32_t                     rotation_order;
		std::unique_ptr<util::vec3a> position;
		std::unique_ptr<util::vec3a> rotation;
		std::unique_ptr<util::vec3a> scale;
		std::unique_ptr<util::vec3a> shear;

		public:
		~Transform();
		Transform(obs_data_t*, obs_source_t*);

		uint32_t get_width();
		uint32_t get_height();

		void     update(obs_data_t*);
		void     activate();
		void     deactivate();
		void     video_tick(float);
		void     video_render(gs_effect_t*);
	};
} // namespace filter
