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
#include "plugin.h"
#include "gs-vertexbuffer.h"
#include <memory>

namespace Filter {
	class Transform {
		public:
		Transform();
		~Transform();

		static const char *get_name(void *);
		static void get_defaults(obs_data_t *);
		static obs_properties_t *get_properties(void *);
		static bool modified_properties(obs_properties_t *,
			obs_property_t *, obs_data_t *);

		static void *create(obs_data_t *, obs_source_t *);
		static void destroy(void *);
		static uint32_t get_width(void *);
		static uint32_t get_height(void *);
		static void update(void *, obs_data_t *);
		static void activate(void *);
		static void deactivate(void *);
		static void show(void *);
		static void hide(void *);
		static void video_tick(void *, float);
		static void video_render(void *, gs_effect_t *);

		private:
		obs_source_info sourceInfo;

		private:
		class Instance {
			public:
			Instance(obs_data_t*, obs_source_t*);
			~Instance();

			void update(obs_data_t*);
			uint32_t get_width();
			uint32_t get_height();
			void activate();
			void deactivate();
			void video_tick(float);
			void video_render(gs_effect_t*);

			private:
			obs_source_t *m_sourceContext;
			GS::VertexBuffer *m_vertexHelper;
			gs_vertbuffer_t *m_vertexBuffer;
			gs_texrender_t *m_texRender, *m_shapeRender;

			// Camera
			bool m_isCameraOrthographic;
			float_t m_cameraFieldOfView;

			// Source
			bool m_isInactive, m_isHidden;
			bool m_isMeshUpdateRequired;

			// 3D Information
			uint32_t m_rotationOrder;
			struct {
				std::unique_ptr<util::vec3a> m_position;
				std::unique_ptr<util::vec3a> m_rotation;
				std::unique_ptr<util::vec3a> m_scale;
				std::unique_ptr<util::vec3a> m_shear;
			};
		};
	};
}
