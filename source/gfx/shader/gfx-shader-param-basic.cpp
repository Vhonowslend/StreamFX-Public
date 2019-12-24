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

#include "gfx-shader-param-basic.hpp"
#include <sstream>
#include "strings.hpp"

inline bool get_annotation_string(gs::effect_parameter param, std::string anno_name, std::string& out)
{
	if (!param)
		return false;

	if (auto el = param.get_annotation(anno_name); el != nullptr) {
		if (auto val = el.get_default_string(); val.length() > 0) {
			out = val;
			return true;
		}
	}

	return false;
}

inline bool get_annotation_float(gs::effect_parameter param, std::string anno_name, float_t& out)
{
	if (!param) {
		return false;
	}

	if (auto el = param.get_annotation(anno_name); el != nullptr) {
		out = el.get_default_float();
		return true;
	}

	return false;
}

gfx::shader::bool_parameter::bool_parameter(gs::effect_parameter param, std::string prefix) : parameter(param, prefix)
{}

gfx::shader::bool_parameter::~bool_parameter() {}

void gfx::shader::bool_parameter::defaults(obs_data_t* settings)
{
	obs_data_set_default_bool(settings, _key.c_str(), _param.get_default_bool());
}

void gfx::shader::bool_parameter::properties(obs_properties_t* props, obs_data_t* settings)
{
	auto p = obs_properties_add_list(props, _key.c_str(), _name.c_str(), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	if (has_description())
		obs_property_set_long_description(p, get_description().c_str());
	obs_property_list_add_int(p, D_TRANSLATE(S_STATE_DISABLED), 0);
	obs_property_list_add_int(p, D_TRANSLATE(S_STATE_ENABLED), 1);
}

void gfx::shader::bool_parameter::update(obs_data_t* settings)
{
	_value = obs_data_get_int(settings, _key.c_str()) != 0;
}

void gfx::shader::bool_parameter::assign()
{
	_param.set_bool(_value);
}

gfx::shader::float_parameter::float_parameter(gs::effect_parameter param, std::string prefix) : parameter(param, prefix)
{
	switch (_param.get_type()) {
	case gs::effect_parameter::type::Float:
		_array_size = 1;
		break;
	case gs::effect_parameter::type::Float2:
		_array_size = 2;
		break;
	case gs::effect_parameter::type::Float3:
		_array_size = 3;
		break;
	case gs::effect_parameter::type::Float4:
		_array_size = 4;
		break;
	default:
		_array_size = 0;
	}

	// Build sub-keys
	char buffer[16]; // Fits on the stack, and in the L1 cache.
	for (size_t idx = 0; idx < _array_size; idx++) {
		snprintf(buffer, sizeof(buffer), "[%d]", static_cast<int32_t>(idx));
		_names[idx] = std::string(buffer, buffer + sizeof(buffer));
		_keys[idx]  = _key + _names[idx];

		_min[idx]  = std::numeric_limits<float_t>::lowest();
		_max[idx]  = std::numeric_limits<float_t>::max();
		_step[idx] = 0.01f;
	}

	if (auto anno = _param.get_annotation("minimum"); (anno != nullptr)) {
		if (anno.get_type() == gs::effect_parameter::type::Float) {
			for (size_t len = 0; len < _array_size; len++) {
				anno.get_default_value(&_min[len], 1);
			}
		} else if (anno.get_type() == _param.get_type()) {
			anno.get_default_value(_min, _array_size);
		}
	}
	if (auto anno = _param.get_annotation("maximum"); (anno != nullptr)) {
		if (anno.get_type() == gs::effect_parameter::type::Float) {
			for (size_t len = 0; len < _array_size; len++) {
				anno.get_default_value(&_max[len], 1);
			}
		} else if (anno.get_type() == _param.get_type()) {
			anno.get_default_value(_max, _array_size);
		}
	}
	if (auto anno = _param.get_annotation("step"); (anno != nullptr)) {
		if (anno.get_type() == gs::effect_parameter::type::Float) {
			for (size_t len = 0; len < _array_size; len++) {
				anno.get_default_value(&_step[len], 1);
			}
		} else if (anno.get_type() == _param.get_type()) {
			anno.get_default_value(_step, _array_size);
		}
	}
}

gfx::shader::float_parameter::~float_parameter() {}

void gfx::shader::float_parameter::defaults(obs_data_t* settings)
{
	float_t defaults[4] = {0, 0, 0, 0};
	_param.get_default_value(defaults, _array_size);
	for (size_t idx = 0; idx < _array_size; idx++) {
		obs_data_set_default_double(settings, _keys[idx].c_str(), static_cast<double_t>(defaults[idx]));
	}
}

void gfx::shader::float_parameter::properties(obs_properties_t* props, obs_data_t* settings)
{
	auto grp = obs_properties_create();
	auto p   = obs_properties_add_group(props, _key.c_str(), _name.c_str(), OBS_GROUP_NORMAL, grp);
	if (has_description())
		obs_property_set_long_description(p, get_description().c_str());

	for (size_t idx = 0; idx < _array_size; idx++) {
		auto p =
			obs_properties_add_float(grp, _keys[idx].c_str(), _names[idx].c_str(), _min[idx], _max[idx], _step[idx]);
		if (has_description())
			obs_property_set_long_description(p, get_description().c_str());
	}
}

void gfx::shader::float_parameter::update(obs_data_t* settings)
{
	for (size_t len = 0; len < _array_size; len++) {
		_value[len] = static_cast<float_t>(obs_data_get_double(settings, _keys[len].c_str()));
	}
}

void gfx::shader::float_parameter::assign()
{
	_param.set_value(_value, _array_size);
}

gfx::shader::int_parameter::int_parameter(gs::effect_parameter param, std::string prefix) : parameter(param, prefix) {}

gfx::shader::int_parameter::~int_parameter() {}

void gfx::shader::int_parameter::properties(obs_properties_t* props, obs_data_t* settings) {}
