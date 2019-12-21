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

gfx::shader::bool_parameter::bool_parameter(gs::effect_parameter param, std::string prefix) : parameter(param)
{
	std::stringstream ss;
	ss << prefix << "." << param.get_name();

	_key  = ss.str();
	_name = _key;
	_desc = "";

	get_annotation_string(_param, "name", _name);
	get_annotation_string(_param, "description", _desc);
}

gfx::shader::bool_parameter::~bool_parameter() {}

void gfx::shader::bool_parameter::properties(obs_properties_t* props, obs_data_t* settings)
{
	auto p = obs_properties_add_list(props, _key.c_str(), _name.c_str(), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	if (_desc.length() > 0)
		obs_property_set_long_description(p, _desc.c_str());
	obs_property_list_add_int(p, D_TRANSLATE(S_STATE_DISABLED), 0);
	obs_property_list_add_int(p, D_TRANSLATE(S_STATE_ENABLED), 1);

	obs_data_set_default_bool(settings, _key.c_str(), _param.get_default_bool());
}

void gfx::shader::bool_parameter::update(obs_data_t* settings)
{
	_value = obs_data_get_int(settings, _key.c_str()) != 0;
}

void gfx::shader::bool_parameter::assign()
{
	_param.set_bool(_value);
}

gfx::shader::float_parameter::float_parameter(gs::effect_parameter param, std::string prefix) : parameter(param)
{
	switch (_param.get_type()) {
	case gs::effect_parameter::type::Float:
		_len = 1;
		break;
	case gs::effect_parameter::type::Float2:
		_len = 2;
		break;
	case gs::effect_parameter::type::Float3:
		_len = 3;
		break;
	case gs::effect_parameter::type::Float4:
		_len = 4;
		break;
	default:
		_len = 0;
	}

	// Build baseline key.
	std::stringstream ss;
	ss << prefix << "." << param.get_name();

	// Build primary key.
	_key[4]  = ss.str();
	_name[4] = _key[4];
	_desc    = "";
	get_annotation_string(_param, "name", _name[4]);
	get_annotation_string(_param, "description", _desc);

	// Build sub-keys
	for (size_t len = 0; len < _len; len++) {
		char buf[5];
		snprintf(buf, 4, "[%d]", static_cast<int32_t>(len));
		_name[len] = std::string(buf, buf + sizeof(buf));
		_key[len]  = _key[4] + _name[len];

		_min[len]  = std::numeric_limits<float_t>::lowest();
		_max[len]  = std::numeric_limits<float_t>::max();
		_step[len] = 0.01f;
	}

	if (auto anno = _param.get_annotation("minimum"); (anno != nullptr)) {
		if (anno.get_type() == gs::effect_parameter::type::Float) {
			for (size_t len = 0; len < _len; len++) {
				anno.get_default_value(&_min[len], 1);
			}
		} else if (anno.get_type() == _param.get_type()) {
			anno.get_default_value(_min, _len);
		}
	}
	if (auto anno = _param.get_annotation("maximum"); (anno != nullptr)) {
		if (anno.get_type() == gs::effect_parameter::type::Float) {
			for (size_t len = 0; len < _len; len++) {
				anno.get_default_value(&_max[len], 1);
			}
		} else if (anno.get_type() == _param.get_type()) {
			anno.get_default_value(_max, _len);
		}
	}
	if (auto anno = _param.get_annotation("step"); (anno != nullptr)) {
		if (anno.get_type() == gs::effect_parameter::type::Float) {
			for (size_t len = 0; len < _len; len++) {
				anno.get_default_value(&_step[len], 1);
			}
		} else if (anno.get_type() == _param.get_type()) {
			anno.get_default_value(_step, _len);
		}
	}
}

gfx::shader::float_parameter::~float_parameter() {}

void gfx::shader::float_parameter::properties(obs_properties_t* props, obs_data_t* settings)
{
	auto grp = obs_properties_create();
	obs_properties_add_group(props, _key[4].c_str(), _name[4].c_str(), OBS_GROUP_NORMAL, grp);

	float_t defaults[4] = {0, 0, 0, 0};
	_param.get_default_value(defaults, _len);

	for (size_t len = 0; len < _len; len++) {
		auto p = obs_properties_add_float(grp, _key[len].c_str(), _name[len].c_str(), _min[len], _max[len], _step[len]);
		obs_property_set_long_description(p, _desc.c_str());
		obs_data_set_default_double(settings, _key[len].c_str(), static_cast<double_t>(defaults[len]));
	}
}

void gfx::shader::float_parameter::update(obs_data_t* settings)
{
	for (size_t len = 0; len < _len; len++) {
		_value[len] = static_cast<float_t>(obs_data_get_double(settings, _key[len].c_str()));
	}
}

void gfx::shader::float_parameter::assign()
{
	_param.set_value(_value, _len);
}

gfx::shader::int_parameter::int_parameter(gs::effect_parameter param, std::string prefix) : parameter(param) {}

gfx::shader::int_parameter::~int_parameter() {}

void gfx::shader::int_parameter::properties(obs_properties_t* props, obs_data_t* settings) {}
