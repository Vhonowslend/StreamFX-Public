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

#include "gs-effect-parameter.hpp"
#include <cstring>
#include <stdexcept>
#include "gs-effect-pass.hpp"

#include <graphics/effect.h>

gs::effect_parameter::effect_parameter(gs_eparam_t* param)
	: _effect_parent(nullptr), _pass_parent(nullptr), _param_parent(nullptr)
{
	reset(param, [](void*) {});
}

gs::effect_parameter::effect_parameter(gs_eparam_t* param, std::shared_ptr<gs_effect_t>* parent)
	: effect_parameter(param)
{
	_effect_parent = parent;
}

gs::effect_parameter::effect_parameter(gs_eparam_t* param, std::shared_ptr<gs_epass_t>* parent)
	: effect_parameter(param)
{
	_pass_parent = parent;
}

gs::effect_parameter::effect_parameter(gs_eparam_t* param, std::shared_ptr<gs_eparam_t>* parent)
	: effect_parameter(param)
{
	_param_parent = parent;
}

gs::effect_parameter::~effect_parameter() {}

std::string gs::effect_parameter::get_name()
{
	return std::string(get()->name, get()->name + strnlen_s(get()->name, 256));
}

gs::effect_parameter::type gs::effect_parameter::get_type()
{
	switch (get()->type) {
	case GS_SHADER_PARAM_BOOL:
		return type::Boolean;
	case GS_SHADER_PARAM_FLOAT:
		return type::Float;
	case GS_SHADER_PARAM_VEC2:
		return type::Float2;
	case GS_SHADER_PARAM_VEC3:
		return type::Float3;
	case GS_SHADER_PARAM_VEC4:
		return type::Float4;
	case GS_SHADER_PARAM_INT:
		return type::Integer;
	case GS_SHADER_PARAM_INT2:
		return type::Integer2;
	case GS_SHADER_PARAM_INT3:
		return type::Integer3;
	case GS_SHADER_PARAM_INT4:
		return type::Integer4;
	case GS_SHADER_PARAM_MATRIX4X4:
		return type::Matrix;
	case GS_SHADER_PARAM_TEXTURE:
		return type::Texture;
	case GS_SHADER_PARAM_STRING:
		return type::String;
	default:
	case GS_SHADER_PARAM_UNKNOWN:
		return type::Unknown;
	}
}

inline size_t gs::effect_parameter::count_annotations()
{
	return gs_param_get_num_annotations(get());
}

std::shared_ptr<gs::effect_parameter> gs::effect_parameter::get_annotation(size_t idx)
{
	if (idx >= get()->annotations.num) {
		return nullptr;
	}

	return std::make_shared<effect_parameter>(get()->annotations.array + idx, this);
}

std::shared_ptr<gs::effect_parameter> gs::effect_parameter::get_annotation(std::string name)
{
	for (size_t idx = 0; idx < get()->annotations.num; idx++) {
		auto ptr = get()->annotations.array + idx;
		if (strcmp(ptr->name, name.c_str()) == 0) {
			return std::make_shared<effect_parameter>(ptr, this);
		}
	}

	return nullptr;
}

bool gs::effect_parameter::has_annotation(std::string name)
{
	auto eprm = get_annotation(name);
	if (eprm)
		return true;
	return false;
}

bool gs::effect_parameter::has_annotation(std::string name, effect_parameter::type type)
{
	auto eprm = get_annotation(name);
	if (eprm)
		return eprm->get_type() == type;
	return false;
}
