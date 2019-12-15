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

#include <graphics/effect.h>

gs::effect_pass::effect_pass(gs_epass_t* pass, std::shared_ptr<gs_technique_t>* parent) : _parent(parent)
{
	reset(pass, [](void*) {});
}

gs::effect_pass::~effect_pass() {}

std::string gs::effect_pass::name()
{
	return std::string(get()->name, get()->name + strnlen_s(get()->name, 256));
}

size_t gs::effect_pass::count_vertex_parameters()
{
	return static_cast<size_t>(get()->vertshader_params.num);
}

size_t gs::effect_pass::count_pixel_parameters()
{
	return static_cast<size_t>(get()->pixelshader_params.num);
}
