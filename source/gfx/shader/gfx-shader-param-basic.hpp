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
#include <string>
#include "gfx-shader-param.hpp"
#include "obs/gs/gs-effect-parameter.hpp"

namespace gfx {
	namespace shader {
		class bool_parameter : public parameter {
			bool _value;

			public:
			bool_parameter(gs::effect_parameter param, std::string prefix);
			virtual ~bool_parameter();

			public:
			virtual void defaults(obs_data_t* settings);

			virtual void properties(obs_properties_t* props, obs_data_t* settings) override;

			virtual void update(obs_data_t* settings) override;

			virtual void assign() override;
		};

		struct float_parameter : public parameter {
			size_t      _array_size;
			std::string _keys[4];
			std::string _names[4];

			float_t _min[4];
			float_t _max[4];
			float_t _step[4];
			float_t _value[4];

			public:
			float_parameter(gs::effect_parameter param, std::string prefix);
			virtual ~float_parameter();

			public:
			virtual void defaults(obs_data_t* settings);

			virtual void properties(obs_properties_t* props, obs_data_t* settings) override;

			virtual void update(obs_data_t* settings) override;

			virtual void assign() override;
		};

		struct int_parameter : public parameter {
			public:
			int_parameter(gs::effect_parameter param, std::string prefix);
			virtual ~int_parameter();

			public:
			virtual void properties(obs_properties_t* props, obs_data_t* settings) override;
		};

	} // namespace shader
} // namespace gfx
