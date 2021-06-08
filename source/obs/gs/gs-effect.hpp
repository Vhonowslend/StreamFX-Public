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
#include "common.hpp"
#include <filesystem>
#include <list>
#include "gs-effect-parameter.hpp"
#include "gs-effect-technique.hpp"

namespace streamfx::obs::gs {
	class effect : public std::shared_ptr<gs_effect_t> {
		public:
		effect(){};
		effect(const std::string& code, const std::string& name);
		effect(std::filesystem::path file);
		~effect();

		std::size_t                         count_techniques();
		streamfx::obs::gs::effect_technique get_technique(std::size_t idx);
		streamfx::obs::gs::effect_technique get_technique(const std::string& name);
		bool                                has_technique(const std::string& name);

		std::size_t                         count_parameters();
		streamfx::obs::gs::effect_parameter get_parameter(std::size_t idx);
		streamfx::obs::gs::effect_parameter get_parameter(const std::string& name);
		bool                                has_parameter(const std::string& name);
		bool                                has_parameter(const std::string& name, effect_parameter::type type);

		public /* Legacy Support */:
		inline gs_effect_t* get_object()
		{
			return get();
		}

		static streamfx::obs::gs::effect create(const std::string& file)
		{
			return streamfx::obs::gs::effect(file);
		};

		static streamfx::obs::gs::effect create(const std::string& code, const std::string& name)
		{
			return streamfx::obs::gs::effect(code, name);
		};
	};
} // namespace streamfx::obs::gs
