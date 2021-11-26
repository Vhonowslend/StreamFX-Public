// Modern effects for a modern Streamer
// Copyright (C) 2017 Michael Fabian Dirks
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

#pragma once
#include "common.hpp"
#include <filesystem>
#include <list>
#include <map>
#include <random>
#include "gfx/shader/gfx-shader-param.hpp"
#include "obs/gs/gs-effect.hpp"
#include "obs/gs/gs-rendertarget.hpp"

namespace streamfx::gfx {
	namespace shader {
		enum class size_type {
			Pixel,
			Percent,
		};

		enum class shader_mode {
			Source,
			Filter,
			Transition,
		};

		typedef std::map<std::string_view, std::shared_ptr<parameter>> shader_param_map_t;

		class shader {
			obs_source_t* _self;

			// Inputs
			shader_mode _mode;
			uint32_t    _base_width;
			uint32_t    _base_height;
			bool        _active;
			bool        _visible;

			// Shader
			streamfx::obs::gs::effect       _shader;
			std::filesystem::path           _shader_file;
			std::string                     _shader_tech;
			std::filesystem::file_time_type _shader_file_mt;
			uintmax_t                       _shader_file_sz;
			float_t                         _shader_file_tick;
			shader_param_map_t              _shader_params;

			// Options
			size_type _width_type;
			double_t  _width_value;
			size_type _height_type;
			double_t  _height_value;

			// Cache
			bool            _have_current_params;
			float_t         _time;
			float_t         _time_loop;
			int32_t         _loops;
			std::mt19937_64 _random;
			int32_t         _random_seed;
			float_t _random_values[16]; // 0..4 Per-Instance-Random, 4..8 Per-Activation-Random 9..15 Per-Frame-Random

			// Rendering
			bool                                             _rt_up_to_date;
			std::shared_ptr<streamfx::obs::gs::rendertarget> _rt;

			public:
			shader(obs_source_t* self, shader_mode mode);
			~shader();

			bool is_shader_different(const std::filesystem::path& file);

			bool is_technique_different(const std::string& tech);

			bool load_shader(const std::filesystem::path& file, const std::string& tech, bool& shader_dirty,
							 bool& param_dirty);

			static void defaults(obs_data_t* data);

			void properties(obs_properties_t* props);

			bool on_refresh_properties(obs_properties_t* props, obs_property_t* prop);

			bool on_shader_or_technique_modified(obs_properties_t* props, obs_property_t* prop, obs_data_t* data);

			bool update_shader(obs_data_t* data, bool& shader_dirty, bool& param_dirty);

			void update(obs_data_t* data);

			uint32_t width();

			uint32_t height();

			uint32_t base_width();

			uint32_t base_height();

			bool tick(float_t time);

			void prepare_render();

			void render(gs_effect* effect);

			obs_source_t* get();

			std::filesystem::path get_shader_file();

			public:
			void set_size(uint32_t w, uint32_t h);

			void set_input_a(std::shared_ptr<streamfx::obs::gs::texture> tex, bool srgb = false);

			void set_input_b(std::shared_ptr<streamfx::obs::gs::texture> tex, bool srgb = false);

			void set_transition_time(float_t t);

			void set_transition_size(uint32_t w, uint32_t h);

			void set_visible(bool visible);

			void set_active(bool active);
		};
	} // namespace shader
} // namespace streamfx::gfx
