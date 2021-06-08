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
#include <algorithm>
#include <sstream>
#include "gfx-shader-param-basic.hpp"

#define ANNO_ORDER "order"
#define ANNO_VISIBILITY "visible"
#define ANNO_AUTOMATIC "automatic"
#define ANNO_NAME "name"
#define ANNO_DESCRIPTION "description"
#define ANNO_TYPE "type"
#define ANNO_SIZE "size"

typedef streamfx::obs::gs::effect_parameter::type eptype;

gfx::shader::parameter_type gfx::shader::get_type_from_effect_type(streamfx::obs::gs::effect_parameter::type type)
{
	switch (type) {
	case eptype::Boolean:
		return parameter_type::Boolean;
	case eptype::Integer:
	case eptype::Integer2:
	case eptype::Integer3:
	case eptype::Integer4:
		return parameter_type::Integer;
	case eptype::Float:
	case eptype::Float2:
	case eptype::Float3:
	case eptype::Float4:
	case eptype::Matrix:
		return parameter_type::Float;
	case eptype::String:
		return parameter_type::String;
	case eptype::Texture:
		return parameter_type::Texture;
	default:
		return parameter_type::Unknown;
	}
}

std::size_t gfx::shader::get_length_from_effect_type(streamfx::obs::gs::effect_parameter::type type)
{
	switch (type) {
	default:
	case eptype::Unknown:
	case eptype::Invalid:
	case eptype::String:
		return 0;
	case eptype::Boolean:
	case eptype::Float:
	case eptype::Integer:
	case eptype::Texture:
		return 1;
	case eptype::Float2:
	case eptype::Integer2:
		return 2;
	case eptype::Float3:
	case eptype::Integer3:
		return 3;
	case eptype::Float4:
	case eptype::Integer4:
		return 4;
	case eptype::Matrix:
		return 16;
	}
}

gfx::shader::parameter_type gfx::shader::get_type_from_string(std::string v)
{
	if ((v == "bool") || (v == "boolean")) {
		return parameter_type::Boolean;
	}
	if ((v == "float") || (v == "single")) {
		return parameter_type::Float;
	}
	if ((v == "int") || (v == "integer")) {
		return parameter_type::Integer;
	}
	if ((v == "text") || (v == "string")) {
		return parameter_type::String;
	}
	if ((v == "tex") || (v == "texture")) {
		return parameter_type::Texture;
	}
	if ((v == "sampler")) {
		return parameter_type::Sampler;
	}
	/* To decide on in the future:
	 * - Double support?
	 * - Half Support?
	 * - Texture Arrays? (Likely not supported in libobs)
	 */
	throw std::invalid_argument("Invalid parameter type string.");
}

gfx::shader::parameter::parameter(streamfx::obs::gs::effect_parameter param, std::string key_prefix)
	: _param(param), _order(0), _key(_param.get_name()), _visible(true), _automatic(false), _name(_key), _description()
{
	{
		std::stringstream ss;
		ss << key_prefix << "." << param.get_name();
		_name = (_key);
	}

	// Read Order
	if (auto anno = _param.get_annotation(ANNO_VISIBILITY); anno) {
		_visible = anno.get_default_bool();
	}
	if (auto anno = _param.get_annotation(ANNO_AUTOMATIC); anno) {
		_automatic = anno.get_default_bool();
	}

	// Read Order
	if (auto anno = _param.get_annotation(ANNO_ORDER); anno) {
		_order = anno.get_default_int();
	}

	// Read Name
	if (auto anno = _param.get_annotation(ANNO_NAME); anno) {
		if (std::string v = anno.get_default_string(); v.length() > 0) {
			_name = v;
		} else {
			throw std::out_of_range("'" ANNO_NAME "' annotation has zero length.");
		}
	}

	// Read Description
	if (auto anno = _param.get_annotation(ANNO_DESCRIPTION); anno) {
		if (std::string v = anno.get_default_string(); v.length() > 0) {
			_description = v;
		} else {
			throw std::out_of_range("'" ANNO_DESCRIPTION "' annotation has zero length.");
		}
	}

	// Read Type override.
	_type = get_type_from_effect_type(_param.get_type());
	if (auto anno = _param.get_annotation(ANNO_TYPE); anno) {
		// We have a type override.
		_type = get_type_from_string(anno.get_default_string());
	}

	// Read Size override.
	_size = get_length_from_effect_type(_param.get_type());
	if (auto anno = _param.get_annotation(ANNO_SIZE); anno) {
		std::size_t ov = static_cast<size_t>(anno.get_default_int());
		if (ov > 0)
			_size = ov;
	}
	_size = std::clamp<size_t>(_size, size_t{1}, size_t{32});
}

void gfx::shader::parameter::defaults(obs_data_t* settings) {}

void gfx::shader::parameter::properties(obs_properties_t* props, obs_data_t* settings) {}

void gfx::shader::parameter::update(obs_data_t* settings) {}

void gfx::shader::parameter::assign() {}

std::shared_ptr<gfx::shader::parameter>
	gfx::shader::parameter::make_parameter(streamfx::obs::gs::effect_parameter param, std::string prefix)
{
	if (!param) {
		throw std::runtime_error("Bad call to make_parameter. This is a bug in the plugin.");
	}

	parameter_type real_type = get_type_from_effect_type(param.get_type());
	if (auto anno = param.get_annotation(ANNO_TYPE); anno) {
		// We have a type override.
		real_type = get_type_from_string(param.get_default_string());
	}

	switch (real_type) {
	case parameter_type::Boolean:
		return std::make_shared<gfx::shader::bool_parameter>(param, prefix);
	case parameter_type::Integer:
		return std::make_shared<gfx::shader::int_parameter>(param, prefix);
	case parameter_type::Float:
		return std::make_shared<gfx::shader::float_parameter>(param, prefix);
	default:
		return nullptr;
	}
}
