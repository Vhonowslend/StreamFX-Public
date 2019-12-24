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

#include "gfx-shader-param.hpp"
#include <sstream>
#include "gfx-shader-param-basic.hpp"

gfx::shader::parameter::parameter(gs::effect_parameter param, std::string key_prefix)
	: _param(param), _order(0), _key(_param.get_name()), _name(_key), _description()
{
	{
		std::stringstream ss;
		ss << key_prefix << "." << param.get_name();
		_name = (_key);
	}

	if (auto anno = _param.get_annotation("order"); anno) {
		_order = anno.get_default_int();
	}
	if (auto anno = _param.get_annotation("name"); anno) {
		if (std::string v = anno.get_default_string(); v.length() > 0)
			_name = v;
	}
	if (auto anno = _param.get_annotation("description"); anno) {
		if (std::string v = anno.get_default_string(); v.length() > 0)
			_description = v;
	}
}

void gfx::shader::parameter::defaults(obs_data_t* settings) {}

void gfx::shader::parameter::properties(obs_properties_t* props, obs_data_t* settings) {}

void gfx::shader::parameter::update(obs_data_t* settings) {}

void gfx::shader::parameter::assign() {}

int32_t gfx::shader::parameter::get_order()
{
	return _order;
}

const std::string& gfx::shader::parameter::get_name()
{
	return _name;
}

bool gfx::shader::parameter::has_description()
{
	return _description.length() > 0;
}

const std::string& gfx::shader::parameter::get_description()
{
	return _description;
}

std::shared_ptr<gfx::shader::parameter> gfx::shader::parameter::make_parameter(gs::effect_parameter param,
																			   std::string          prefix)
{
	if (!param)
		return nullptr;

	// ToDo: Allow other parameters to specify hidden properties, as well as the shader itself, and the source/filter/transition.
	if (auto anno = param.get_annotation("visible"); anno != nullptr) {
		if (!anno.get_default_bool()) {
			return nullptr;
		}
	}

	typedef gs::effect_parameter::type eptype;
	switch (param.get_type()) {
	case eptype::Boolean: {
		auto el = std::make_shared<gfx::shader::bool_parameter>(param, prefix);
		return el;
	}
	case eptype::Integer:
	case eptype::Integer2:
	case eptype::Integer3:
	case eptype::Integer4: {
		auto el = std::make_shared<gfx::shader::int_parameter>(param, prefix);
		return el;
	}
	case eptype::Float:
	case eptype::Float2:
	case eptype::Float3:
	case eptype::Float4: {
		auto el = std::make_shared<gfx::shader::float_parameter>(param, prefix);
		return el;
	}
	default:
		return nullptr;
	}
}
