/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2019 Michael Fabian Dirks
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

#include "gs-effect-pass.hpp"
#include <cstring>

extern "C" {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <graphics/effect.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

streamfx::obs::gs::effect_pass::effect_pass(gs_epass_t* pass, std::shared_ptr<gs_technique_t> parent) : _parent(parent)
{
	reset(pass, [](void*) {});
}

streamfx::obs::gs::effect_pass::~effect_pass() {}

std::string streamfx::obs::gs::effect_pass::name()
{
	const char* name_c   = get()->name;
	std::size_t name_len = strnlen(name_c, 256);
	return name_c ? std::string(name_c, name_c + name_len) : std::string();
}

std::size_t streamfx::obs::gs::effect_pass::count_vertex_parameters()
{
	return static_cast<size_t>(get()->vertshader_params.num);
}

streamfx::obs::gs::effect_parameter streamfx::obs::gs::effect_pass::get_vertex_parameter(std::size_t idx)
{
	if (idx >= count_vertex_parameters())
		return nullptr;

	return streamfx::obs::gs::effect_parameter((get()->vertshader_params.array + idx)->eparam, *this);
}

streamfx::obs::gs::effect_parameter streamfx::obs::gs::effect_pass::get_vertex_parameter(std::string name)
{
	for (std::size_t idx = 0; idx < count_vertex_parameters(); idx++) {
		auto ptr = get()->vertshader_params.array + idx;
		if (strcmp(ptr->eparam->name, name.c_str()) == 0)
			return streamfx::obs::gs::effect_parameter(ptr->eparam, *this);
	}
	return nullptr;
}

bool streamfx::obs::gs::effect_pass::has_vertex_parameter(std::string name)
{
	return (get_vertex_parameter(name) != nullptr);
}

bool streamfx::obs::gs::effect_pass::has_vertex_parameter(std::string                               name,
														  streamfx::obs::gs::effect_parameter::type type)
{
	if (auto el = get_vertex_parameter(name); el != nullptr) {
		return el.get_type() == type;
	}
	return false;
}

std::size_t streamfx::obs::gs::effect_pass::count_pixel_parameters()
{
	return static_cast<size_t>(get()->pixelshader_params.num);
}

streamfx::obs::gs::effect_parameter streamfx::obs::gs::effect_pass::get_pixel_parameter(std::size_t idx)
{
	if (idx >= count_pixel_parameters())
		return nullptr;

	return streamfx::obs::gs::effect_parameter((get()->pixelshader_params.array + idx)->eparam, *this);
}

streamfx::obs::gs::effect_parameter streamfx::obs::gs::effect_pass::get_pixel_parameter(std::string name)
{
	for (std::size_t idx = 0; idx < count_pixel_parameters(); idx++) {
		auto ptr = get()->pixelshader_params.array + idx;
		if (strcmp(ptr->eparam->name, name.c_str()) == 0)
			return streamfx::obs::gs::effect_parameter(ptr->eparam, *this);
	}
	return nullptr;
}

bool streamfx::obs::gs::effect_pass::has_pixel_parameter(std::string name)
{
	return (get_pixel_parameter(name) != nullptr);
}

bool streamfx::obs::gs::effect_pass::has_pixel_parameter(std::string                               name,
														 streamfx::obs::gs::effect_parameter::type type)
{
	if (auto el = get_pixel_parameter(name); el != nullptr) {
		return el.get_type() == type;
	}
	return false;
}
