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

		std::size_t get_length_from_effect_type(gs::effect_parameter::type type);

		parameter_type get_type_from_string(std::string v);

		class parameter {
			// Parameter used for all functionality.
			gs::effect_parameter _param;

			// Real type of the parameter (libobs gets it wrong often).
			parameter_type _type;

			// Real size of the parameter (libobs gets it wrong often).
			std::size_t _size;

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
			inline gs::effect_parameter get_parameter()
			{
				return _param;
			}

			inline parameter_type get_type()
			{
				return _type;
			}

			inline std::size_t get_size()
			{
				return _size;
			}

			inline int32_t get_order()
			{
				return _order;
			}

			inline std::string_view get_key()
			{
				return _key;
			}

			inline bool is_visible()
			{
				return _visible && !_automatic;
			}

			inline bool is_automatic()
			{
				return _automatic;
			}

			inline bool has_name()
			{
				return _name.length() > 0;
			}

			inline std::string_view get_name()
			{
				return _name;
			}

			inline bool has_description()
			{
				return _description.length() > 0;
			}

			inline std::string_view get_description()
			{
				return _description;
			}

			public:
			static std::shared_ptr<parameter> make_parameter(gs::effect_parameter param, std::string prefix);
		};
	} // namespace shader
} // namespace gfx
