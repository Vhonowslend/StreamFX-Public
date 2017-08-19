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
#include "gs-helper.h"

#define S_FILTER_TRANSFORM				"Filter.Transform"
#define S_FILTER_TRANSFORM_CAMERA			"Filter.Transform.Camera"
#define S_FILTER_TRANSFORM_CAMERA_ORTHOGRAPHIC		"Filter.Transform.Camera.Orthographic"
#define S_FILTER_TRANSFORM_CAMERA_PERSPECTIVE		"Filter.Transform.Camera.Perspective"
#define S_FILTER_TRANSFORM_CAMERA_FIELDOFVIEW		"Filter.Transform.Camera.FieldOfView"
#define S_FILTER_TRANSFORM_POSITION			"Filter.Transform.Position"
#define S_FILTER_TRANSFORM_POSITION_X			"Filter.Transform.Position.X"
#define S_FILTER_TRANSFORM_POSITION_Y			"Filter.Transform.Position.Y"
#define S_FILTER_TRANSFORM_POSITION_Z			"Filter.Transform.Position.Z"
#define S_FILTER_TRANSFORM_ROTATION			"Filter.Transform.Rotation"
#define S_FILTER_TRANSFORM_ROTATION_ORDER		"Filter.Transform.Rotation.Order"
#define S_FILTER_TRANSFORM_ROTATION_ORDER_XYZ		"Filter.Transform.Rotation.Order.XYZ"
#define S_FILTER_TRANSFORM_ROTATION_ORDER_XZY		"Filter.Transform.Rotation.Order.XZY"
#define S_FILTER_TRANSFORM_ROTATION_ORDER_YXZ		"Filter.Transform.Rotation.Order.YXZ"
#define S_FILTER_TRANSFORM_ROTATION_ORDER_YZX		"Filter.Transform.Rotation.Order.YZX"
#define S_FILTER_TRANSFORM_ROTATION_ORDER_ZXY		"Filter.Transform.Rotation.Order.ZXY"
#define S_FILTER_TRANSFORM_ROTATION_ORDER_ZYX		"Filter.Transform.Rotation.Order.ZYX"
#define S_FILTER_TRANSFORM_ROTATION_X			"Filter.Transform.Rotation.X"
#define S_FILTER_TRANSFORM_ROTATION_Y			"Filter.Transform.Rotation.Y"
#define S_FILTER_TRANSFORM_ROTATION_Z			"Filter.Transform.Rotation.Z"
#define S_FILTER_TRANSFORM_SCALE			"Filter.Transform.Scale"
#define S_FILTER_TRANSFORM_SCALE_X			"Filter.Transform.Scale.X"
#define S_FILTER_TRANSFORM_SCALE_Y			"Filter.Transform.Scale.Y"

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
			void show();
			void hide();
			void video_tick(float);
			void video_render(gs_effect_t*);

			private:
			obs_source_t *m_sourceContext;
			Helper::VertexBuffer *m_vertexHelper;
			gs_vertbuffer_t *m_vertexBuffer;
			gs_texrender_t *m_texRender, *m_shapeRender;

			// Camera
			bool m_isCameraOrthographic;
			float_t m_cameraFieldOfView;

			// Source
			bool m_isInactive, m_isHidden;
			bool m_isMeshUpdateRequired;
			uint32_t m_rotationOrder;
			vec3 m_position,
				m_rotation,
				m_scale;
		};
	};
}
