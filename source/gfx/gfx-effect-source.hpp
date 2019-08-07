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
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "gfx-source-texture.hpp"
#include "obs/gs/gs-effect.hpp"
#include "obs/gs/gs-mipmapper.hpp"
#include "obs/gs/gs-rendertarget.hpp"
#include "obs/gs/gs-texture.hpp"
#include "obs/gs/gs-vertexbuffer.hpp"

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
	namespace effect_source {
		struct parameter {
			std::shared_ptr<gs::effect>           effect;
			std::shared_ptr<gs::effect_parameter> param;

			std::string name;
			std::string description;
			std::string formulae;
			bool        visible;

			parameter(std::shared_ptr<gs::effect> effect, std::string name);
		};
		struct bool_parameter : parameter {
			bool value;

			bool_parameter(std::shared_ptr<gs::effect> effect, std::string name);
		};
		struct value_parameter : parameter {
			union {
				float_t f[16];
				int32_t i[4];
			} value;
			union {
				float_t f[16];
				int32_t i[4];
			} minimum;
			union {
				float_t f[16];
				int32_t i[4];
			} maximum;

			value_parameter(std::shared_ptr<gs::effect> effect, std::string name);
		};
		struct matrix_parameter : parameter {
			matrix4 value;
			matrix4 minimum;
			matrix4 maximum;

			matrix_parameter(std::shared_ptr<gs::effect> effect, std::string name);
		};
		struct string_parameter : parameter {
			std::string value;

			string_parameter(std::shared_ptr<gs::effect> effect, std::string name);
		};
		struct texture_parameter : parameter {
			std::shared_ptr<gs::texture> value;

			texture_parameter(std::shared_ptr<gs::effect> effect, std::string name);
		};

		typedef std::pair<gs::effect_parameter::type, std::string> param_ident_t;

		class shader_instance {
			std::string                                         _file;
			std::shared_ptr<gs::effect>                         _effect;
			std::map<param_ident_t, std::shared_ptr<parameter>> _params;

			std::shared_ptr<gs::vertex_buffer> _tri;

			float_t _last_check;
			size_t  _last_size;
			time_t  _last_modify_time;
			time_t  _last_create_time;

			void load_file(std::string file);

			public:
			shader_instance(std::string file);
			~shader_instance();

			void tick(float_t time);

			void render(std::string technique);
		};
	} // namespace effect_source
} // namespace gfx
