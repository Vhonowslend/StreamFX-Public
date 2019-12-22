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
		class parameter {
			protected:
			gs::effect_parameter _param;

			parameter(gs::effect_parameter param) : _param(param){};
			virtual ~parameter(){};

			public:
			virtual void defaults(obs_data_t* settings);

			virtual void properties(obs_properties_t* props, obs_data_t* settings);

			virtual void update(obs_data_t* settings);

			virtual void assign();

			public:
			static std::shared_ptr<parameter> make_parameter(gs::effect_parameter param, std::string prefix);
		};
	} // namespace shader
} // namespace gfx
