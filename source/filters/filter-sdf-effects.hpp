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
#include "obs/obs-source-factory.hpp"
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
		class sdf_effects_instance : public obs::source_instance {
			gs::effect _sdf_producer_effect;
			gs::effect _sdf_consumer_effect;

			// Input
			std::shared_ptr<gs::rendertarget> _source_rt;
			std::shared_ptr<gs::texture>      _source_texture;
			bool                              _source_rendered;

			// Distance Field
			std::shared_ptr<gs::rendertarget> _sdf_write;
			std::shared_ptr<gs::rendertarget> _sdf_read;
			std::shared_ptr<gs::texture>      _sdf_texture;
			double_t                          _sdf_scale;
			float_t                           _sdf_threshold;

			// Effects
			bool                              _output_rendered;
			std::shared_ptr<gs::texture>      _output_texture;
			std::shared_ptr<gs::rendertarget> _output_rt;
			/// Inner Shadow
			bool    _inner_shadow;
			vec4    _inner_shadow_color;
			float_t _inner_shadow_range_min;
			float_t _inner_shadow_range_max;
			float_t _inner_shadow_offset_x;
			float_t _inner_shadow_offset_y;
			/// Outer Shadow
			bool    _outer_shadow;
			vec4    _outer_shadow_color;
			float_t _outer_shadow_range_min;
			float_t _outer_shadow_range_max;
			float_t _outer_shadow_offset_x;
			float_t _outer_shadow_offset_y;
			/// Inner Glow
			bool    _inner_glow;
			vec4    _inner_glow_color;
			float_t _inner_glow_width;
			float_t _inner_glow_sharpness;
			float_t _inner_glow_sharpness_inv;
			/// Outer Glow
			bool    _outer_glow;
			vec4    _outer_glow_color;
			float_t _outer_glow_width;
			float_t _outer_glow_sharpness;
			float_t _outer_glow_sharpness_inv;
			/// Outline
			bool    _outline;
			vec4    _outline_color;
			float_t _outline_width;
			float_t _outline_offset;
			float_t _outline_sharpness;
			float_t _outline_sharpness_inv;

			public:
			sdf_effects_instance(obs_data_t* settings, obs_source_t* self);
			virtual ~sdf_effects_instance();

			virtual void update(obs_data_t* settings) override;
			virtual void load(obs_data_t* settings) override;

			virtual void video_tick(float) override;
			virtual void video_render(gs_effect_t*) override;
		};

		class sdf_effects_factory : public obs::source_factory<filter::sdf_effects::sdf_effects_factory,
															   filter::sdf_effects::sdf_effects_instance> {
			static std::shared_ptr<filter::sdf_effects::sdf_effects_factory> factory_instance;

			public: // Singleton
			static void initialize()
			{
				factory_instance = std::make_shared<filter::sdf_effects::sdf_effects_factory>();
			}

			static void finalize()
			{
				factory_instance.reset();
			}

			static std::shared_ptr<sdf_effects_factory> get()
			{
				return factory_instance;
			}

			public:
			sdf_effects_factory();
			virtual ~sdf_effects_factory();

			virtual const char* get_name() override;

			virtual void get_defaults2(obs_data_t* data) override;

			virtual obs_properties_t* get_properties2(filter::sdf_effects::sdf_effects_instance* data) override;
		};

	} // namespace sdf_effects
} // namespace filter
