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
#include "strings.hpp"
#include <algorithm>
#include <map>
#include <sstream>
#include <stdexcept>

static const std::string_view _annotation_field_type      = "field_type";
static const std::string_view _annotation_suffix          = "suffix";
static const std::string_view _annotation_minimum         = "minimum";
static const std::string_view _annotation_maximum         = "maximum";
static const std::string_view _annotation_step            = "step";
static const std::string_view _annotation_scale           = "scale";
static const std::string_view _annotation_enum_entry      = "enum_%zu";
static const std::string_view _annotation_enum_entry_name = "enum_%zu_name";

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

	_min.resize(get_size());
	_max.resize(get_size());
	_step.resize(get_size());
	_scale.resize(get_size());

	// Build sub-keys
	if (get_size() == 1) {
		_names[0] = get_name();
		_keys[0]  = get_key();
	} else {
		for (std::size_t idx = 0; idx < get_size(); idx++) {
			snprintf(string_buffer, sizeof(string_buffer), "[%d]", static_cast<int32_t>(idx));
			_names[idx] = std::string(string_buffer, string_buffer + strnlen(string_buffer, sizeof(string_buffer)));
			snprintf(string_buffer, sizeof(string_buffer), "%s[%d]", get_key().data(), static_cast<int32_t>(idx));
			_keys[idx] = std::string(string_buffer, string_buffer + strnlen(string_buffer, sizeof(string_buffer)));
		}
	}

	// Detect Field Types
	if (auto anno = get_parameter().get_annotation(_annotation_field_type); anno) {
		_field_type = get_field_type_from_string(anno.get_default_string());
	}

	// Read Suffix Data
	if (auto anno = get_parameter().get_annotation(_annotation_suffix); anno) {
		if (anno.get_type() == gs::effect_parameter::type::String)
			_suffix = anno.get_default_string();
	}

	// Read Enumeration Data if Enumeration
	if (field_type() == basic_field_type::Enum) {
		for (std::size_t idx = 0; idx < std::numeric_limits<std::size_t>::max(); idx++) {
			// Build key.
			std::string key_name;
			std::string key_value;
			{
				snprintf(string_buffer, sizeof(string_buffer), _annotation_enum_entry.data(), idx);
				key_value = std::string(string_buffer);
				snprintf(string_buffer, sizeof(string_buffer), _annotation_enum_entry_name.data(), idx);
				key_name = std::string(string_buffer);
			}

			// Value must be given, name is optional.
			if (auto eanno = get_parameter().get_annotation(key_value);
				eanno && (get_type_from_effect_type(eanno.get_type()) == get_type())) {
				basic_enum_data entry;

				load_parameter_data(eanno, entry.data);
				if (auto nanno = get_parameter().get_annotation(key_name);
					nanno && (nanno.get_type() == gs::effect_parameter::type::String)) {
					entry.name = nanno.get_default_string();
				} else {
					entry.name = "Unnamed Entry";
				}

				_values.push_back(entry);
			} else {
				break;
			}
		}

		if (_values.size() == 0) {
			_field_type = basic_field_type::Input;
		}
	}
}

gfx::shader::basic_parameter::~basic_parameter() {}

void gfx::shader::basic_parameter::load_parameter_data(gs::effect_parameter parameter, basic_data& data)
{
	parameter.get_default_value(&data.i32, 1);
}

gfx::shader::bool_parameter::bool_parameter(gs::effect_parameter param, std::string prefix)
	: basic_parameter(param, prefix)
{
	_min.resize(0);
	_max.resize(0);
	_step.resize(0);
	_scale.resize(0);

	_data.resize(get_size(), 1);
}

gfx::shader::bool_parameter::~bool_parameter() {}

void gfx::shader::bool_parameter::defaults(obs_data_t* settings)
{
	// TODO: Support for bool[]
	if (get_size() == 1) {
		obs_data_set_default_int(settings, get_key().data(), get_parameter().get_default_bool() ? 1 : 0);
	}
}

void gfx::shader::bool_parameter::properties(obs_properties_t* props, obs_data_t* settings)
{
	if (!is_visible())
		return;

	// TODO: Support for bool[]
	if (get_size() == 1) {
		auto p = obs_properties_add_list(props, get_key().data(), get_name().data(), OBS_COMBO_TYPE_LIST,
										 OBS_COMBO_FORMAT_INT);
		if (has_description())
			obs_property_set_long_description(p, get_description().data());
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
		_data[0] = obs_data_get_int(settings, get_key().data());
	}
}

void gfx::shader::bool_parameter::assign()
{
	get_parameter().set_value(_data.data(), _data.size());
}

gfx::shader::float_parameter::float_parameter(gs::effect_parameter param, std::string prefix)
	: basic_parameter(param, prefix)
{
	_data.resize(get_size());

	// Reset minimum, maximum, step and scale.
	for (std::size_t idx = 0; idx < get_size(); idx++) {
		_min[idx].f32   = std::numeric_limits<float_t>::lowest();
		_max[idx].f32   = std::numeric_limits<float_t>::max();
		_step[idx].f32  = 0.01f;
		_scale[idx].f32 = 1.00f;
	}

	// Load Limits
	if (auto anno = get_parameter().get_annotation(_annotation_minimum); anno) {
		if (anno.get_type() == get_parameter().get_type()) {
			anno.get_default_value(_min.data(), get_size());
		}
	}
	if (auto anno = get_parameter().get_annotation(_annotation_maximum); anno) {
		if (anno.get_type() == get_parameter().get_type()) {
			anno.get_default_value(_max.data(), get_size());
		}
	}
	if (auto anno = get_parameter().get_annotation(_annotation_step); anno) {
		if (anno.get_type() == get_parameter().get_type()) {
			anno.get_default_value(_step.data(), get_size());
		}
	}
	if (auto anno = get_parameter().get_annotation(_annotation_scale); anno) {
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

	for (std::size_t idx = 0; idx < get_size(); idx++) {
		obs_data_set_default_double(settings, key_at(idx).data(), static_cast<double_t>(defaults[idx]));
	}
}

static inline obs_property_t* build_float_property(gfx::shader::basic_field_type ft, obs_properties_t* props,
												   const char* key, const char* name, float_t min, float_t max,
												   float_t step, std::list<gfx::shader::basic_enum_data> edata)
{
	switch (ft) {
	case gfx::shader::basic_field_type::Enum: {
		auto p = obs_properties_add_list(props, key, name, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_FLOAT);
		for (auto& el : edata) {
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

	obs_properties_t* pr = props;
	if (get_size() > 1) {
		pr     = obs_properties_create();
		auto p = obs_properties_add_group(props, get_key().data(), has_name() ? get_name().data() : get_key().data(),
										  OBS_GROUP_NORMAL, pr);
		if (has_description())
			obs_property_set_long_description(p, get_description().data());
	}

	for (std::size_t idx = 0; idx < get_size(); idx++) {
		auto p = build_float_property(field_type(), pr, key_at(idx).data(), name_at(idx).data(), _min[idx].f32,
									  _max[idx].f32, _step[idx].f32, _values);
		if (has_description())
			obs_property_set_long_description(p, get_description().data());
		obs_property_float_set_suffix(p, suffix().data());
	}
}

void gfx::shader::float_parameter::update(obs_data_t* settings)
{
	for (std::size_t idx = 0; idx < get_size(); idx++) {
		_data[idx].f32 = static_cast<float_t>(obs_data_get_double(settings, key_at(idx).data())) * _scale[idx].f32;
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
												 int32_t step, std::list<gfx::shader::basic_enum_data> edata)
{
	switch (ft) {
	case gfx::shader::basic_field_type::Enum: {
		auto p = obs_properties_add_list(props, key, name, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
		for (auto& el : edata) {
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
	for (std::size_t idx = 0; idx < get_size(); idx++) {
		_min[idx].i32   = std::numeric_limits<int32_t>::lowest();
		_max[idx].i32   = std::numeric_limits<int32_t>::max();
		_step[idx].i32  = 1;
		_scale[idx].i32 = 1;
	}

	// Load Limits
	if (auto anno = get_parameter().get_annotation(_annotation_minimum); anno) {
		if (anno.get_type() == get_parameter().get_type()) {
			anno.get_default_value(_min.data(), get_size());
		}
	}
	if (auto anno = get_parameter().get_annotation(_annotation_maximum); anno) {
		if (anno.get_type() == get_parameter().get_type()) {
			anno.get_default_value(_max.data(), get_size());
		}
	}
	if (auto anno = get_parameter().get_annotation(_annotation_step); anno) {
		if (anno.get_type() == get_parameter().get_type()) {
			anno.get_default_value(_step.data(), get_size());
		}
	}
	if (auto anno = get_parameter().get_annotation(_annotation_scale); anno) {
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
	for (std::size_t idx = 0; idx < get_size(); idx++) {
		obs_data_set_default_int(settings, key_at(idx).data(), defaults[idx]);
	}
}

void gfx::shader::int_parameter::properties(obs_properties_t* props, obs_data_t* settings)
{
	if (!is_visible())
		return;

	obs_properties_t* pr = props;
	if (get_size() > 1) {
		pr     = obs_properties_create();
		auto p = obs_properties_add_group(props, get_key().data(), has_name() ? get_name().data() : get_key().data(),
										  OBS_GROUP_NORMAL, pr);
		if (has_description())
			obs_property_set_long_description(p, get_description().data());
	}

	for (std::size_t idx = 0; idx < get_size(); idx++) {
		auto p = build_int_property(field_type(), pr, key_at(idx).data(), name_at(idx).data(), _min[idx].i32,
									_max[idx].i32, _step[idx].i32, _values);
		if (has_description())
			obs_property_set_long_description(p, get_description().data());
		obs_property_int_set_suffix(p, suffix().data());
	}
}

void gfx::shader::int_parameter::update(obs_data_t* settings)
{
	for (std::size_t idx = 0; idx < get_size(); idx++) {
		_data[idx].i32 = static_cast<int32_t>(obs_data_get_int(settings, key_at(idx).data()) * _scale[idx].i32);
	}
}

void gfx::shader::int_parameter::assign()
{
	if (is_automatic())
		return;

	get_parameter().set_value(_data.data(), get_size());
}
