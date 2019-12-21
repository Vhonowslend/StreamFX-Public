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
#include <filesystem>
#include <list>
#include <map>
#include <random>
#include <string>
#include "gfx/shader/gfx-shader-param.hpp"
#include "obs/gs/gs-effect.hpp"

// OBS
extern "C" {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <obs.h>
#include <util/platform.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

namespace gfx {
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

		class shader {
			obs_source_t* _self;

			// Inputs
			shader_mode                  _mode;
			uint32_t                     _base_width;
			uint32_t                     _base_height;
			std::shared_ptr<gs::texture> _input_a;
			std::shared_ptr<gs::texture> _input_b;

			// Shader
			gs::effect                                        _shader;
			std::filesystem::path                             _shader_file;
			std::string                                       _shader_tech;
			std::filesystem::file_time_type                   _shader_file_mt;
			uintmax_t                                         _shader_file_sz;
			float_t                                           _shader_file_tick;
			bool                                              _shader_params_invalid;
			std::map<std::string, std::shared_ptr<parameter>> _shader_params;

			// Options
			size_type _width_type;
			double_t  _width_value;
			size_type _height_type;
			double_t  _height_value;

			// Cache
			float_t         _time;
			std::mt19937_64 _random;

			public:
			shader(obs_source_t* self, shader_mode mode);
			~shader();

			bool load_shader(std::filesystem::path file);

			void load_shader_params();

			void properties(obs_properties_t* props);

			bool is_shader_different(std::filesystem::path file);

			bool on_shader_changed(obs_properties_t* props, obs_property_t* prop, obs_data_t* data);

			bool on_technique_changed(obs_properties_t* props, obs_property_t* prop, obs_data_t* data);

			void update_shader(obs_data_t* data);

			void update_technique(obs_data_t* data);

			void update(obs_data_t* data);

			uint32_t width();

			uint32_t height();

			bool tick(float_t time);

			void render();

			public:
			void set_size(uint32_t w, uint32_t h);

			void set_input_a(std::shared_ptr<gs::texture> tex);

			void set_input_b(std::shared_ptr<gs::texture> tex);
		};
	} // namespace shader
} // namespace gfx
