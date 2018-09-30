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
#include <list>
#include <map>
#include <memory>
#include "gs-effect.h"
#include "gs-helper.h"
#include "gs-texture.h"
#include "plugin.h"

namespace filter {
	namespace blur {
		enum type : int64_t {
			Box,
			Gaussian,
			Bilateral,
		};

		class instance {
			obs_source_t*   m_source;
			gs_texrender_t* primary_rendertarget;
			gs_texrender_t* secondary_rendertarget;
			gs_texrender_t* horizontal_rendertarget;
			gs_texrender_t* vertical_rendertarget;

			// blur
			std::shared_ptr<gs::effect> blur_effect;
			filter::blur::type          type;
			uint64_t                    size;

			// bilateral
			double_t bilateral_smoothing;
			double_t bilateral_sharpness;

			// Regional
			struct Region {
				bool    enabled;
				float_t left;
				float_t top;
				float_t right;
				float_t bottom;
				float_t feather;
				float_t feather_shift;
				bool    invert;
			} region;

			// advanced
			bool     have_logged_error = false;
			uint64_t color_format;

			bool apply_shared_param(gs_texture_t* input, float texelX, float texelY);
			bool apply_bilateral_param();
			bool apply_gaussian_param();

			static bool modified_properties(void* ptr, obs_properties_t* props, obs_property* prop,
											obs_data_t* settings);

			public:
			instance(obs_data_t* settings, obs_source_t* parent);
			~instance();

			obs_properties_t* get_properties();
			void              update(obs_data_t*);

			uint32_t get_width();
			uint32_t get_height();

			void activate();
			void deactivate();

			void video_tick(float);
			void video_render(gs_effect_t*);
		};

		class factory {
			obs_source_info             source_info;
			std::list<instance*>        sources;
			std::shared_ptr<gs::effect> color_converter_effect;

			std::map<filter::blur::type, std::shared_ptr<gs::effect>>  effects;
			std::map<filter::blur::type, std::shared_ptr<gs::texture>> kernels;

			private:
			factory();
			~factory();

			void on_list_fill();
			void on_list_empty();

			void generate_gaussian_kernels();
			void generate_kernel_textures();

			protected:
			static void* create(obs_data_t* settings, obs_source_t* parent);
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
			std::shared_ptr<gs::effect> get_effect(filter::blur::type type);

			std::shared_ptr<gs::effect> get_color_converter_effect();

			std::shared_ptr<gs::texture> get_kernel(filter::blur::type type);

			public: // Singleton
			static void     initialize();
			static void     finalize();
			static factory* get();
		};

	} // namespace blur

} // namespace filter
