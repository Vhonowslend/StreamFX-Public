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
#include <algorithm>
#include <map>
#include <sstream>
#include <stdexcept>
#include "strings.hpp"

#define ANNO_FIELD_TYPE "field_type"
#define ANNO_SUFFIX "suffix"
#define ANNO_VALUE_MINIMUM "minimum"
#define ANNO_VALUE_MAXIMUM "maximum"
#define ANNO_VALUE_STEP "step"
#define ANNO_VALUE_SCALE "scale"
#define ANNO_ENUM_VALUES "values"

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

gfx::shader::basic_field_type gfx::shader::get_field_type_from_string(std::string v)
{
	std::map<std::string, basic_field_type> matches = {
		{"input", basic_field_type::Input},
		{"slider", basic_field_type::Slider},
		{"enum", basic_field_type::Enum},
		{"enumeration", basic_field_type::Enum},
	};

	auto fnd = matches.find(v);
	if (fnd != matches.end())
		return fnd->second;

	return basic_field_type::Input;
}

gfx::shader::basic_parameter::basic_parameter(gs::effect_parameter param, std::string prefix)
	: parameter(param, prefix), _field_type(basic_field_type::Input), _suffix(), _keys(), _names(), _min(), _max(),
	  _step(), _values()
{
	char string_buffer[256];

	_keys.resize(get_size());
	_names.resize(get_size());
	/*
	_min.resize(get_size());
	_max.resize(get_size());
	_step.resize(get_size());
	_scale.resize(get_size());
	_value.resize(get_size());
	*/

	// Build sub-keys
	if (get_size() == 1) {
		_names[0] = get_name();
		_keys[0]  = get_key();
	} else {
		for (size_t idx = 0; idx < get_size(); idx++) {
			snprintf(string_buffer, sizeof(string_buffer), "[%d]", static_cast<int32_t>(idx));
			_names[idx] = std::string(string_buffer, string_buffer + strnlen(string_buffer, sizeof(string_buffer)));
			snprintf(string_buffer, sizeof(string_buffer), "%s[%d]", get_key().c_str(), static_cast<int32_t>(idx));
			_keys[idx] = std::string(string_buffer, string_buffer + strnlen(string_buffer, sizeof(string_buffer)));
		}
	}

	// Detect Field Types
	if (auto anno = get_parameter().get_annotation(ANNO_FIELD_TYPE); anno) {
		_field_type = get_field_type_from_string(anno.get_default_string());
	}

	// Read Suffix Data
	if (auto anno = get_parameter().get_annotation(ANNO_SUFFIX); anno) {
		if (anno.get_type() == gs::effect_parameter::type::String)
			_suffix = anno.get_default_string();
	}

	// Read Enumeration Data if Enumeration
	if (get_field_type() == basic_field_type::Enum) {
		if (auto anno = get_parameter().get_annotation(ANNO_ENUM_VALUES);
			anno && (anno.get_type() == gs::effect_parameter::type::Integer)) {
			_values.resize(static_cast<size_t>(std::max(anno.get_default_int(), 0)));
			for (size_t idx = 0; idx < _values.size(); idx++) {
				auto& entry = _values[idx];
				snprintf(string_buffer, sizeof(string_buffer), "_%zu", idx);
				std::string key =
					std::string(string_buffer, string_buffer + strnlen(string_buffer, sizeof(string_buffer)));
				if (auto annoe = anno.get_annotation(key);
					annoe && (annoe.get_type() == gs::effect_parameter::type::String)) {
					entry.name = annoe.get_default_string();
					load_parameter_data(annoe, entry.data);
				} else {
					P_LOG_WARNING("[%s] Parameter enumeration entry '%s' is of invalid type, must be string.",
								  get_name().c_str(), string_buffer);
				}
			}
		} else {
			P_LOG_WARNING("[%s] Enumeration is missing entries.", get_name().c_str());
			_field_type = basic_field_type::Input;
		}
	}
}

gfx::shader::basic_parameter::~basic_parameter() {}

void gfx::shader::basic_parameter::load_parameter_data(gs::effect_parameter parameter, basic_data& data)
{
	data.i32 = 0;
}

gfx::shader::basic_field_type gfx::shader::basic_parameter::get_field_type()
{
	return _field_type;
}

const std::string& gfx::shader::basic_parameter::get_suffix()
{
	return _suffix;
}

const std::string& gfx::shader::basic_parameter::get_keys(size_t idx)
{
	if (idx >= get_size())
		throw std::out_of_range("Index out of range.");
	return _keys[idx];
}

const std::string& gfx::shader::basic_parameter::get_names(size_t idx)
{
	if (idx >= get_size())
		throw std::out_of_range("Index out of range.");
	return _names[idx];
}

gfx::shader::bool_parameter::bool_parameter(gs::effect_parameter param, std::string prefix)
	: basic_parameter(param, prefix)
{
	_min.resize(0);
	_max.resize(0);
	_step.resize(0);
	_scale.resize(0);

	_data.resize(get_size(), true);
}

gfx::shader::bool_parameter::~bool_parameter() {}

void gfx::shader::bool_parameter::defaults(obs_data_t* settings)
{
	// TODO: Support for bool[]
	if (get_size() == 1) {
		obs_data_set_default_bool(settings, get_key().c_str(), get_parameter().get_default_bool());
	}
}

void gfx::shader::bool_parameter::properties(obs_properties_t* props, obs_data_t* settings)
{
	if (!is_visible())
		return;

	// TODO: Support for bool[]
	if (get_size() == 1) {
		auto p = obs_properties_add_list(props, get_key().c_str(), get_name().c_str(), OBS_COMBO_TYPE_LIST,
										 OBS_COMBO_FORMAT_INT);
		if (has_description())
			obs_property_set_long_description(p, get_description().c_str());
		obs_property_list_add_int(p, D_TRANSLATE(S_STATE_DISABLED), 0);
		obs_property_list_add_int(p, D_TRANSLATE(S_STATE_ENABLED), 1);
	}
}

void gfx::shader::bool_parameter::update(obs_data_t* settings)
{
	if (is_automatic())
		return;

	// TODO: Support for bool[]
	if (get_size() == 1) {
		_data[0] = static_cast<bool>(obs_data_get_int(settings, get_key().c_str()));
	}
}

void gfx::shader::bool_parameter::assign()
{
	get_parameter().set_value(_data.data(), sizeof(uint8_t));
}

gfx::shader::float_parameter::float_parameter(gs::effect_parameter param, std::string prefix)
	: basic_parameter(param, prefix)
{
	_data.resize(get_size());

	// Reset minimum, maximum, step and scale.
	for (size_t idx = 0; idx < get_size(); idx++) {
		_min[idx].f32   = std::numeric_limits<float_t>::lowest();
		_max[idx].f32   = std::numeric_limits<float_t>::max();
		_step[idx].f32  = 0.01f;
		_scale[idx].f32 = 1.00f;
	}

	// Load Limits
	if (auto anno = get_parameter().get_annotation(ANNO_VALUE_MINIMUM); anno) {
		if (anno.get_type() == get_parameter().get_type()) {
			anno.get_default_value(_min.data(), get_size());
		}
	}
	if (auto anno = get_parameter().get_annotation(ANNO_VALUE_MAXIMUM); anno) {
		if (anno.get_type() == get_parameter().get_type()) {
			anno.get_default_value(_max.data(), get_size());
		}
	}
	if (auto anno = get_parameter().get_annotation(ANNO_VALUE_STEP); anno) {
		if (anno.get_type() == get_parameter().get_type()) {
			anno.get_default_value(_step.data(), get_size());
		}
	}
	if (auto anno = get_parameter().get_annotation(ANNO_VALUE_SCALE); anno) {
		if (anno.get_type() == get_parameter().get_type()) {
			anno.get_default_value(_scale.data(), get_size());
		}
	}
}

gfx::shader::float_parameter::~float_parameter() {}

void gfx::shader::float_parameter::defaults(obs_data_t* settings)
{
	std::vector<float_t> defaults;
	defaults.resize(get_size());
	get_parameter().get_default_value(defaults.data(), get_size());

	for (size_t idx = 0; idx < get_size(); idx++) {
		obs_data_set_default_double(settings, get_keys(idx).c_str(), static_cast<double_t>(defaults[idx]));
	}
}

static inline obs_property_t* build_float_property(gfx::shader::basic_field_type ft, obs_properties_t* props,
												   const char* key, const char* name, float_t min, float_t max,
												   float_t step, std::vector<gfx::shader::basic_enum_data> edata)
{
	switch (ft) {
	case gfx::shader::basic_field_type::Enum: {
		auto p = obs_properties_add_list(props, key, name, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_FLOAT);
		for (size_t idx = 0; idx < edata.size(); idx++) {
			auto& el = edata.at(idx);
			obs_property_list_add_float(p, el.name.c_str(), el.data.f32);
		}
		return p;
	}
	case gfx::shader::basic_field_type::Slider:
		return obs_properties_add_float_slider(props, key, name, min, max, step);
	default:
	case gfx::shader::basic_field_type::Input:
		return obs_properties_add_float(props, key, name, min, max, step);
	}
}

void gfx::shader::float_parameter::properties(obs_properties_t* props, obs_data_t* settings)
{
	if (!is_visible())
		return;

	if (get_size() == 1) {
		auto p = build_float_property(_field_type, props, _keys[0].c_str(), _names[0].c_str(), _min[0].f32, _max[0].f32,
									  _step[0].f32, _values);
		if (has_description())
			obs_property_set_long_description(p, get_description().c_str());
	} else {
		auto grp = obs_properties_create();
		auto p = obs_properties_add_group(props, get_key().c_str(), has_name() ? get_name().c_str() : get_key().c_str(),
										  OBS_GROUP_NORMAL, grp);
		if (has_description())
			obs_property_set_long_description(p, get_description().c_str());

		for (size_t idx = 0; idx < get_size(); idx++) {
			p = build_float_property(_field_type, grp, _keys[idx].c_str(), _names[idx].c_str(), _min[idx].f32,
									 _max[idx].f32, _step[idx].f32, _values);
			if (has_description())
				obs_property_set_long_description(p, get_description().c_str());
		}
	}
}

void gfx::shader::float_parameter::update(obs_data_t* settings)
{
	for (size_t idx = 0; idx < get_size(); idx++) {
		_data[idx].f32 = static_cast<float_t>(obs_data_get_double(settings, _keys[idx].c_str()));
	}
}

void gfx::shader::float_parameter::assign()
{
	if (is_automatic())
		return;

	get_parameter().set_value(_data.data(), get_size());
}
static inline obs_property_t* build_int_property(gfx::shader::basic_field_type ft, obs_properties_t* props,
												 const char* key, const char* name, int32_t min, int32_t max,
												 int32_t step, std::vector<gfx::shader::basic_enum_data> edata)
{
	switch (ft) {
	case gfx::shader::basic_field_type::Enum: {
		auto p = obs_properties_add_list(props, key, name, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
		for (size_t idx = 0; idx < edata.size(); idx++) {
			auto& el = edata.at(idx);
			obs_property_list_add_int(p, el.name.c_str(), el.data.i32);
		}
		return p;
	}
	case gfx::shader::basic_field_type::Slider:
		return obs_properties_add_int_slider(props, key, name, min, max, step);
	default:
	case gfx::shader::basic_field_type::Input:
		return obs_properties_add_int(props, key, name, min, max, step);
	}
}

gfx::shader::int_parameter::int_parameter(gs::effect_parameter param, std::string prefix)
	: basic_parameter(param, prefix)
{
	_data.resize(get_size());

	// Reset minimum, maximum, step and scale.
	for (size_t idx = 0; idx < get_size(); idx++) {
		_min[idx].i32   = std::numeric_limits<int32_t>::lowest();
		_max[idx].i32   = std::numeric_limits<int32_t>::max();
		_step[idx].i32  = 1;
		_scale[idx].i32 = 1;
	}

	// Load Limits
	if (auto anno = get_parameter().get_annotation(ANNO_VALUE_MINIMUM); anno) {
		if (anno.get_type() == get_parameter().get_type()) {
			anno.get_default_value(_min.data(), get_size());
		}
	}
	if (auto anno = get_parameter().get_annotation(ANNO_VALUE_MAXIMUM); anno) {
		if (anno.get_type() == get_parameter().get_type()) {
			anno.get_default_value(_max.data(), get_size());
		}
	}
	if (auto anno = get_parameter().get_annotation(ANNO_VALUE_STEP); anno) {
		if (anno.get_type() == get_parameter().get_type()) {
			anno.get_default_value(_step.data(), get_size());
		}
	}
	if (auto anno = get_parameter().get_annotation(ANNO_VALUE_SCALE); anno) {
		if (anno.get_type() == get_parameter().get_type()) {
			anno.get_default_value(_scale.data(), get_size());
		}
	}
}

gfx::shader::int_parameter::~int_parameter() {}

void gfx::shader::int_parameter::defaults(obs_data_t* settings)
{
	std::vector<int32_t> defaults;
	defaults.resize(get_size());
	get_parameter().get_default_value(defaults.data(), get_size());
	for (size_t idx = 0; idx < get_size(); idx++) {
		obs_data_set_default_int(settings, get_keys(idx).c_str(), defaults[idx]);
	}
}

void gfx::shader::int_parameter::properties(obs_properties_t* props, obs_data_t* settings)
{
	if (!is_visible())
		return;

	if (get_size() == 1) {
		auto p = build_int_property(_field_type, props, _keys[0].c_str(), _names[0].c_str(), _min[0].i32, _max[0].i32,
									_step[0].i32, _values);
		if (has_description())
			obs_property_set_long_description(p, get_description().c_str());
	} else {
		auto grp = obs_properties_create();
		auto p = obs_properties_add_group(props, get_key().c_str(), has_name() ? get_name().c_str() : get_key().c_str(),
										  OBS_GROUP_NORMAL, grp);
		if (has_description())
			obs_property_set_long_description(p, get_description().c_str());

		for (size_t idx = 0; idx < get_size(); idx++) {
			p = build_int_property(_field_type, grp, _keys[idx].c_str(), _names[idx].c_str(), _min[idx].i32,
								   _max[idx].i32, _step[idx].i32, _values);
			if (has_description())
				obs_property_set_long_description(p, get_description().c_str());
		}
	}
}

void gfx::shader::int_parameter::update(obs_data_t* settings)
{
	for (size_t idx = 0; idx < get_size(); idx++) {
		_data[idx].i32 = static_cast<int32_t>(obs_data_get_int(settings, _keys[idx].c_str()));
	}
}

void gfx::shader::int_parameter::assign()
{
	if (is_automatic())
		return;

	get_parameter().set_value(_data.data(), get_size());
}
