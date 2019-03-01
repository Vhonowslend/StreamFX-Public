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
#include <chrono>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include "gfx/gfx-source-texture.hpp"
#include "obs/gs/gs-effect.hpp"
#include "obs/gs/gs-helper.hpp"
#include "obs/gs/gs-rendertarget.hpp"
#include "obs/gs/gs-texture.hpp"
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
	namespace blur {
		class blur_instance;

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

		class blur_factory {
			friend class std::_Ptr_base<filter::blur::blur_factory>;

			obs_source_info             source_info;
			std::list<blur_instance*>   sources;
			std::shared_ptr<gs::effect> color_converter_effect;
			std::shared_ptr<gs::effect> mask_effect;
			std::shared_ptr<gs::effect> blur_effect;

			std::map<std::string, std::string> translation_map;

			std::vector<double_t>                                    gaussian_widths;
			std::map<uint8_t, std::shared_ptr<std::vector<float_t>>> gaussian_kernels;

			public: // Singleton
			static void                          initialize();
			static void                          finalize();
			static std::shared_ptr<blur_factory> get();

			public:
			blur_factory();
			~blur_factory();

			void on_list_fill();
			void on_list_empty();

			void generate_gaussian_kernels();
			void generate_kernel_textures();

			std::string const& get_translation(std::string const key);

			static void* create(obs_data_t* settings, obs_source_t* self);
			static void  destroy(void* source);

			static void              get_defaults(obs_data_t* settings);
			static obs_properties_t* get_properties(void* source);
			static void              update(void* source, obs_data_t* settings);
			static void              load(void* source, obs_data_t* settings);

			static const char* get_name(void* source);
			static uint32_t    get_width(void* source);
			static uint32_t    get_height(void* source);

			static void activate(void* source);
			static void deactivate(void* source);

			static void video_tick(void* source, float delta);
			static void video_render(void* source, gs_effect_t* effect);

			public:
			std::shared_ptr<gs::effect> get_effect(filter::blur::type type);

			std::string get_technique(filter::blur::type type);

			std::shared_ptr<gs::effect> get_color_converter_effect();

			std::shared_ptr<gs::effect> get_mask_effect();

			std::shared_ptr<std::vector<float_t>> get_gaussian_kernel(uint8_t size);
		};

		class blur_instance {
			obs_source_t* m_self;

			// Input
			std::shared_ptr<gs::rendertarget> m_source_rt;
			std::shared_ptr<gs::texture>      m_source_texture;
			bool                              m_source_rendered;

			// Rendering
			std::shared_ptr<gs::rendertarget> m_output_rt1;
			std::shared_ptr<gs::rendertarget> m_output_rt2;
			std::shared_ptr<gs::texture>      m_output_texture;
			bool                              m_output_rendered;

			// Blur
			std::shared_ptr<gs::effect> m_blur_effect;
			std::string                 m_blur_technique;
			filter::blur::type          m_blur_type;
			uint64_t                    m_blur_size;
			/// Bilateral Blur
			double_t m_blur_bilateral_smoothing;
			double_t m_blur_bilateral_sharpness;
			/// Directional Blur
			bool     m_blur_directional;
			double_t m_blur_angle;
			/// Step Scaling
			bool                          m_blur_step_scaling;
			std::pair<double_t, double_t> m_blur_step_scale;

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
			} m_mask;

			// advanced
			uint64_t m_color_format;

			public:
			blur_instance(obs_data_t* settings, obs_source_t* self);
			~blur_instance();

			private:
			bool apply_shared_param(gs_texture_t* input, float texelX, float texelY);
			bool apply_bilateral_param();
			bool apply_gaussian_param(uint8_t width);
			bool apply_mask_parameters(std::shared_ptr<gs::effect> effect, gs_texture_t* original_texture,
									   gs_texture_t* blurred_texture);

			static bool modified_properties(void* ptr, obs_properties_t* props, obs_property* prop,
											obs_data_t* settings);

			public:
			obs_properties_t* get_properties();
			void              update(obs_data_t*);
			void              load(obs_data_t*);

			uint32_t get_width();
			uint32_t get_height();

			void activate();
			void deactivate();

			void video_tick(float);
			void video_render(gs_effect_t*);
		};
	} // namespace blur
} // namespace filter
