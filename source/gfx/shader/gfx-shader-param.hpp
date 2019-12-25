// Modern effects for a modern Streamer
// Copyright (C) 2019 Michael Fabian Dirks
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
#include <list>
#include <string>
#include "obs/gs/gs-effect-parameter.hpp"

// OBS
extern "C" {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <obs.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

namespace gfx {
	namespace shader {
		enum class parameter_type {
			// Unknown type, could be anything.
			Unknown,
			// Boolean, either false or true.
			Boolean,
			// Single Floating Point
			Float,
			// 32-Bit Integer
			Integer,
			// UTF-8 Character based String.
			String,
			// Texture with dimensions stored in size (1 = Texture1D, 2 = Texture2D, 3 = Texture3D, 6 = TextureCube).
			Texture,
			// Sampler for Textures.
			Sampler
		};

		parameter_type get_type_from_effect_type(gs::effect_parameter::type type);

		size_t get_length_from_effect_type(gs::effect_parameter::type type);

		parameter_type get_type_from_string(std::string v);

		class parameter {
			// Parameter used for all functionality.
			gs::effect_parameter _param;

			// Real type of the parameter (libobs gets it wrong often).
			parameter_type _type;

			// Real size of the parameter (libobs gets it wrong often).
			size_t _size;

			// Order of the parameter in a list/map.
			int32_t _order;

			// Key for the parameter (group) in a list/map.
			std::string _key;

			// Visibility, name and description.
			bool        _visible;
			bool        _automatic;
			std::string _name;
			std::string _description;

			protected:
			parameter(gs::effect_parameter param, std::string key_prefix);
			virtual ~parameter(){};

			public:
			virtual void defaults(obs_data_t* settings);

			virtual void properties(obs_properties_t* props, obs_data_t* settings);

			virtual void update(obs_data_t* settings);

			virtual void assign();

			public:
			gs::effect_parameter get_parameter();

			parameter_type get_type();

			size_t get_size();

			int32_t get_order();

			const std::string& get_key();

			bool is_visible();

			bool is_automatic();

			bool has_name();

			const std::string& get_name();

			bool has_description();

			const std::string& get_description();

			public:
			static std::shared_ptr<parameter> make_parameter(gs::effect_parameter param, std::string prefix);
		};
	} // namespace shader
} // namespace gfx
