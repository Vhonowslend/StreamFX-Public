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
#include <string>
#include "gfx/gfx-source-texture.hpp"
#include "obs/gs/gs-effect.hpp"
#include "obs/obs-source-tracker.hpp"
#include "obs/obs-source.hpp"
#include "plugin.hpp"

// OBS
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <obs-source.h>
#include <util/platform.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace filter {
	namespace dynamic_mask {
		enum class channel : int8_t { Invalid = -1, Red, Green, Blue, Alpha };

		class dynamic_mask_factory {
			obs_source_info _source_info;

			public: // Singleton
			static void                                  initialize();
			static void                                  finalize();
			static std::shared_ptr<dynamic_mask_factory> get();

			public:
			dynamic_mask_factory();
			~dynamic_mask_factory();
		};

		class dynamic_mask_instance {
			obs_source_t* _self;

			std::map<std::tuple<channel, channel, std::string>, std::string> _translation_map;

			std::shared_ptr<gs::effect> _effect;

			bool                              _have_filter_texture;
			std::shared_ptr<gs::rendertarget> _filter_rt;
			std::shared_ptr<gs::texture>      _filter_texture;

			bool                                 _have_input_texture;
			std::shared_ptr<obs::source>         _input;
			std::shared_ptr<gfx::source_texture> _input_capture;
			std::shared_ptr<gs::texture>         _input_texture;

			bool                              _have_final_texture;
			std::shared_ptr<gs::rendertarget> _final_rt;
			std::shared_ptr<gs::texture>      _final_texture;

			struct channel_data {
				float_t value  = 0.0;
				float_t scale  = 1.0;
				vec4    values = {0};
			};
			std::map<channel, channel_data> _channels;

			struct _precalc {
				vec4    base;
				vec4    scale;
				matrix4 matrix;
			} _precalc;

			public:
			dynamic_mask_instance(obs_data_t* data, obs_source_t* self);
			~dynamic_mask_instance();

			uint32_t get_width();
			uint32_t get_height();

			void get_properties(obs_properties_t* properties);
			void update(obs_data_t* settings);
			void load(obs_data_t* settings);
			void save(obs_data_t* settings);

			void input_renamed(obs::source* src, std::string old_name, std::string new_name);

			static bool modified(void* self, obs_properties_t* properties, obs_property_t* property,
								 obs_data_t* settings) noexcept;

			void video_tick(float _time);
			void video_render(gs_effect_t* effect);
		};
	} // namespace dynamic_mask
} // namespace filter
