/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2017 Michael Fabian Dirks
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
#include <cinttypes>
#include <filesystem>
#include <list>
#include <memory>
#include <string>
#include "gs-effect-parameter.hpp"
#include "gs-effect-technique.hpp"

// OBS
extern "C" {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <graphics/graphics.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

namespace gs {
	class effect : public std::shared_ptr<gs_effect_t> {
		public:
		effect() {};
		effect(std::string code, std::string name);
		effect(std::filesystem::path file);
		~effect();

		size_t               count_techniques();
		gs::effect_technique get_technique(size_t idx);
		gs::effect_technique get_technique(std::string name);
		bool                 has_technique(std::string name);

		size_t               count_parameters();
		gs::effect_parameter get_parameter(size_t idx);
		gs::effect_parameter get_parameter(std::string name);
		bool                 has_parameter(std::string name);
		bool                 has_parameter(std::string name, effect_parameter::type type);

		public /* Legacy Support */:
		inline gs_effect_t* get_object()
		{
			return get();
		}

		static gs::effect create(std::string file)
		{
			return gs::effect(file);
		};
		static gs::effect create(std::string code, std::string name)
		{
			return gs::effect(code, name);
		};
	};
} // namespace gs
