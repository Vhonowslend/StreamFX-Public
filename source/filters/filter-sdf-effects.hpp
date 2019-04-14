/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2017-2018 Michael Fabian Dirks
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
#include "obs/gs/gs-effect.hpp"
#include "obs/gs/gs-rendertarget.hpp"
#include "obs/gs/gs-sampler.hpp"
#include "obs/gs/gs-texture.hpp"
#include "obs/gs/gs-vertexbuffer.hpp"
#include "plugin.hpp"

// OBS
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <obs.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace filter {
	namespace sdf_effects {
		class sdf_effects_instance;

		class sdf_effects_factory {
			friend class std::_Ptr_base<filter::sdf_effects::sdf_effects_factory>;

			obs_source_info source_info;

			std::list<sdf_effects_instance*> sources;

			std::shared_ptr<gs::effect> sdf_producer_effect;
			std::shared_ptr<gs::effect> sdf_consumer_effect;

			public: // Singleton
			static void                                 initialize();
			static void                                 finalize();
			static std::shared_ptr<sdf_effects_factory> get();

			public:
			sdf_effects_factory();
			~sdf_effects_factory();

			void on_list_fill();
			void on_list_empty();

			static void* create(obs_data_t* settings, obs_source_t* self);
			static void  destroy(void* source);

			static void              get_defaults(obs_data_t* settings);
			static obs_properties_t* get_properties(void* source);
			static void              update(void* source, obs_data_t* settings);

			static const char* get_name(void* source);
			static uint32_t    get_width(void* source);
			static uint32_t    get_height(void* source);

			static void activate(void* source);
			static void deactivate(void* source);

			static void video_tick(void* source, float delta);
			static void video_render(void* source, gs_effect_t* effect);

			public:
			std::shared_ptr<gs::effect> get_sdf_producer_effect();
			std::shared_ptr<gs::effect> get_sdf_consumer_effect();
		};

		class sdf_effects_instance {
			obs_source_t* m_self;

			// Input
			std::shared_ptr<gs::rendertarget> m_source_rt;
			std::shared_ptr<gs::texture>      m_source_texture;
			bool                              m_source_rendered;

			// Distance Field
			std::shared_ptr<gs::rendertarget> m_sdf_write, m_sdf_read;
			std::shared_ptr<gs::texture>      m_sdf_texture;
			double_t                          m_sdf_scale;
			float_t                           m_sdf_threshold;

			// Effects
			bool                              m_output_rendered;
			std::shared_ptr<gs::texture>      m_output_texture;
			std::shared_ptr<gs::rendertarget> m_output_rt;
			/// Inner Shadow
			bool    m_inner_shadow;
			vec4    m_inner_shadow_color;
			float_t m_inner_shadow_range_min;
			float_t m_inner_shadow_range_max;
			float_t m_inner_shadow_offset_x;
			float_t m_inner_shadow_offset_y;
			/// Outer Shadow
			bool    m_outer_shadow;
			vec4    m_outer_shadow_color;
			float_t m_outer_shadow_range_min;
			float_t m_outer_shadow_range_max;
			float_t m_outer_shadow_offset_x;
			float_t m_outer_shadow_offset_y;
			/// Inner Glow
			bool    m_inner_glow;
			vec4    m_inner_glow_color;
			float_t m_inner_glow_width;
			float_t m_inner_glow_sharpness;
			float_t m_inner_glow_sharpness_inv;
			/// Outer Glow
			bool    m_outer_glow;
			vec4    m_outer_glow_color;
			float_t m_outer_glow_width;
			float_t m_outer_glow_sharpness;
			float_t m_outer_glow_sharpness_inv;
			/// Outline
			bool    m_outline;
			vec4    m_outline_color;
			float_t m_outline_width;
			float_t m_outline_offset;
			float_t m_outline_sharpness;
			float_t m_outline_sharpness_inv;

			static bool cb_modified_shadow_inside(void* ptr, obs_properties_t* props, obs_property* prop,
												  obs_data_t* settings);

			static bool cb_modified_shadow_outside(void* ptr, obs_properties_t* props, obs_property* prop,
												   obs_data_t* settings);

			static bool cb_modified_glow_inside(void* ptr, obs_properties_t* props, obs_property* prop,
												obs_data_t* settings);

			static bool cb_modified_glow_outside(void* ptr, obs_properties_t* props, obs_property* prop,
												 obs_data_t* settings);

			static bool cb_modified_outline(void* ptr, obs_properties_t* props, obs_property* prop,
											obs_data_t* settings);

			static bool cb_modified_advanced(void* ptr, obs_properties_t* props, obs_property* prop,
											 obs_data_t* settings);

			public:
			sdf_effects_instance(obs_data_t* settings, obs_source_t* self);
			~sdf_effects_instance();

			obs_properties_t* get_properties();
			void              update(obs_data_t*);

			uint32_t get_width();
			uint32_t get_height();

			void activate();
			void deactivate();

			void video_tick(float);
			void video_render(gs_effect_t*);
		};
	} // namespace sdf_effects
} // namespace filter
