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
#include <cstring>
#include <stdexcept>

#include <graphics/effect.h>

gs::effect_technique::effect_technique(gs_technique_t* technique, std::shared_ptr<gs_effect_t>* parent) : _parent(parent)
{
	reset(technique, [](void*) {});
}

gs::effect_technique::~effect_technique() {}

std::string gs::effect_technique::name()
{
	
	return std::string(get()->name, get()->name + strnlen_s(get()->name, 256));
}

size_t gs::effect_technique::count_passes()
{
	return static_cast<size_t>(get()->passes.num);
}

gs::effect_pass gs::effect_technique::get_pass(size_t idx)
{
	if (idx >= get()->passes.num) {
		throw std::out_of_range("Index is out of range.");
	}

	return gs::effect_pass(get()->passes.array + idx, this);
}

gs::effect_pass gs::effect_technique::get_pass(std::string name)
{
	for (size_t idx = 0; idx < get()->passes.num; idx++) {
		auto ptr = get()->passes.array + idx;
		if (strcmp(ptr->name, name.c_str()) == 0)
			return gs::effect_pass(ptr, this);
	}

	throw std::invalid_argument("Pass with given name does not exist.");
}

bool gs::effect_technique::has_pass(std::string name)
{
	for (size_t idx = 0; idx < get()->passes.num; idx++) {
		auto ptr = get()->passes.array + idx;
		if (strcmp(ptr->name, name.c_str()) == 0)
			return true;
	}
	return false;
}
