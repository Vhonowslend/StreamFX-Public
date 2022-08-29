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

#include "gs-effect-technique.hpp"

#include "warning-disable.hpp"
#include <cstring>
#include <stdexcept>
#include "warning-enable.hpp"

extern "C" {
#include "warning-disable.hpp"
#include <graphics/effect.h>
#include "warning-enable.hpp"
}

streamfx::obs::gs::effect_technique::effect_technique(gs_technique_t* technique, std::shared_ptr<gs_effect_t> parent)
	: _parent(parent)
{
	reset(technique, [](void*) {});
}

streamfx::obs::gs::effect_technique::~effect_technique() = default;

std::string streamfx::obs::gs::effect_technique::name()
{
	const char* name_c   = get()->name;
	std::size_t name_len = strnlen(name_c, 256);
	return name_c ? std::string(name_c, name_c + name_len) : std::string();
}

std::size_t streamfx::obs::gs::effect_technique::count_passes()
{
	return static_cast<size_t>(get()->passes.num);
}

streamfx::obs::gs::effect_pass streamfx::obs::gs::effect_technique::get_pass(std::size_t idx)
{
	if (idx >= get()->passes.num) {
		return nullptr;
	}

	return streamfx::obs::gs::effect_pass(get()->passes.array + idx, *this);
}

streamfx::obs::gs::effect_pass streamfx::obs::gs::effect_technique::get_pass(std::string_view name)
{
	for (std::size_t idx = 0; idx < get()->passes.num; idx++) {
		auto ptr = get()->passes.array + idx;
		if (strcmp(ptr->name, name.data()) == 0)
			return streamfx::obs::gs::effect_pass(ptr, *this);
	}

	return nullptr;
}

bool streamfx::obs::gs::effect_technique::has_pass(std::string_view name)
{
	if (get_pass(name) != nullptr)
		return true;
	return false;
}
