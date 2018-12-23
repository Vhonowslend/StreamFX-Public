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

#ifndef OBS_STREAM_EFFECTS_FILTER_BLUR_HPP
#define OBS_STREAM_EFFECTS_FILTER_BLUR_HPP
#pragma once

#include <chrono>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include "gfx-source-texture.h"
#include "gs-effect.h"
#include "gs-helper.h"
#include "gs-texture.h"
#include "gs-rendertarget.h"
#include "plugin.h"

extern "C" {
#pragma warning(push)
#pragma warning(disable : 4201)
#include "callback/signal.h"
#pragma warning(pop)
}

namespace filter {
	namespace blur {
		enum type : int64_t {
			Box,
			Gaussian,
			Bilateral,
			BoxLinear,
			GaussianLinear,
		};

		enum mask_type : int64_t {
			Region,
			Image,
			Source,
		};

		class blur_instance {
			obs_source_t*   m_source;
			std::shared_ptr<gs::rendertarget> rt_source; 
			std::shared_ptr<gs::rendertarget> rt_primary;
			std::shared_ptr<gs::rendertarget> rt_secondary;

			// blur
			std::shared_ptr<gs::effect> blur_effect;
			std::string                 blur_technique;
			filter::blur::type          type;
			uint64_t                    size;

			// bilateral
			double_t bilateral_smoothing;
			double_t bilateral_sharpness;

			// Masking
			struct {
				bool      enabled;
				mask_type type;
				struct {
					float_t left;
					float_t top;
					float_t right;
					float_t bottom;
					float_t feather;
					float_t feather_shift;
					bool    invert;
				} region;
				struct {
					std::string                  path;
					std::string                  path_old;
					std::shared_ptr<gs::texture> texture;
				} image;
				struct {
					std::string                          name_old;
					std::string                          name;
					bool                                 is_scene;
					std::shared_ptr<gfx::source_texture> source_texture;
					std::shared_ptr<gs::texture>         texture;
				} source;
				struct {
					float_t r;
					float_t g;
					float_t b;
					float_t a;
				} color;
				float_t multiplier;
			} mask;

			// advanced
			uint64_t color_format;

			bool apply_shared_param(gs_texture_t* input, float texelX, float texelY);
			bool apply_bilateral_param();
			bool apply_gaussian_param(uint8_t width);
			bool apply_mask_parameters(std::shared_ptr<gs::effect> effect, gs_texture_t* original_texture,
									   gs_texture_t* blurred_texture);

			static bool modified_properties(void* ptr, obs_properties_t* props, obs_property* prop,
											obs_data_t* settings);

			// Logging
			std::chrono::high_resolution_clock::time_point last_log;
			bool can_log();

			public:
			blur_instance(obs_data_t* settings, obs_source_t* self);
			~blur_instance();

			obs_properties_t* get_properties();
			void              update(obs_data_t*);

			uint32_t get_width();
			uint32_t get_height();

			void activate();
			void deactivate();

			void video_tick(float);
			void video_render(gs_effect_t*);
		};

		class blur_factory {
			obs_source_info             source_info;
			std::list<blur_instance*>   sources;
			std::shared_ptr<gs::effect> color_converter_effect;
			std::shared_ptr<gs::effect> mask_effect;

			std::shared_ptr<gs::effect>                                blur_effect;
			std::map<filter::blur::type, std::shared_ptr<gs::texture>> kernels;
			std::map<uint8_t, std::shared_ptr<std::vector<float_t>>>   gaussian_kernels;

			std::map<std::string, obs_scene_t*> scenes;

			private:
			blur_factory();
			~blur_factory();

			void on_list_fill();
			void on_list_empty();

			void generate_gaussian_kernels();
			void generate_kernel_textures();

			protected:
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

			static void scene_create_handler(void* ptr, calldata_t* data);
			static void scene_destroy_handler(void* ptr, calldata_t* data);

			public:
			std::shared_ptr<gs::effect> get_effect(filter::blur::type type);

			std::string get_technique(filter::blur::type type);

			std::shared_ptr<gs::effect> get_color_converter_effect();

			std::shared_ptr<gs::effect> get_mask_effect();

			std::shared_ptr<gs::texture> get_kernel(filter::blur::type type);

			std::shared_ptr<std::vector<float_t>> get_gaussian_kernel(uint8_t size);

			obs_scene_t* get_scene(std::string name);

			void enum_scenes(std::function<bool(obs_scene_t*)> fnc);

			public: // Singleton
			static void          initialize();
			static void          finalize();
			static blur_factory* get();
		};

	} // namespace blur

} // namespace filter

#endif
