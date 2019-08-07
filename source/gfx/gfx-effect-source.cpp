// Modern effects for a modern Streamer
// Copyright (C) 2017 Michael Fabian Dirks
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

#include "gfx-effect-source.hpp"
#include <cfloat>
#include <climits>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include "obs/gs/gs-helper.hpp"
#include "strings.hpp"

#define ST_FILE S_SHADER_FILE
#define ST_TECHNIQUE S_SHADER_TECHNIQUE

#define UNIQUE_PREFIX "HelloThisIsPatrick."

static std::vector<std::string> static_parameters{
	"ViewProj",
	"Time",
	"Random",
};

gfx::effect_source::parameter::parameter(std::shared_ptr<gs::effect>           effect,
										 std::shared_ptr<gs::effect_parameter> param)
	: _effect(effect), _param(param), _description(""), _formulae(""), _visible(true)
{
	if (!effect)
		throw std::invalid_argument("effect");
	if (!effect)
		throw std::invalid_argument("param");
	if (!effect->has_parameter(param->get_name(), param->get_type()))
		throw std::invalid_argument("param");
	_name = UNIQUE_PREFIX + _param->get_name();

	_visible_name = _name;
	if (param->has_annotation("name", gs::effect_parameter::type::String)) {
		param->get_annotation("name")->get_default_string(_visible_name);
	}
	if (param->has_annotation("description", gs::effect_parameter::type::String)) {
		param->get_annotation("description")->get_default_string(_description);
	}
	if (param->has_annotation("formulae", gs::effect_parameter::type::String)) {
		param->get_annotation("formulae")->get_default_string(_formulae);
	}
	if (param->has_annotation("visible", gs::effect_parameter::type::Boolean)) {
		param->get_annotation("visible")->get_default_bool(_visible);
	} else {
		_visible = true;
	}
}

gfx::effect_source::parameter::~parameter() {}

void gfx::effect_source::parameter::defaults(obs_properties_t* props, obs_data_t* data) {}

void gfx::effect_source::parameter::properties(obs_properties_t* props) {}

void gfx::effect_source::parameter::remove_properties(obs_properties_t* props) {}

void gfx::effect_source::parameter::update(obs_data_t* data) {}

void gfx::effect_source::parameter::tick(float_t time) {}

void gfx::effect_source::parameter::prepare() {}

void gfx::effect_source::parameter::assign() {}

std::shared_ptr<gs::effect_parameter> gfx::effect_source::parameter::get_param()
{
	return _param;
}

std::shared_ptr<gfx::effect_source::parameter>
	gfx::effect_source::parameter::create(std::shared_ptr<gs::effect>           effect,
										  std::shared_ptr<gs::effect_parameter> param)
{
	if (!effect)
		throw std::invalid_argument("effect");
	if (!effect)
		throw std::invalid_argument("param");
	if (!effect->has_parameter(param->get_name(), param->get_type()))
		throw std::invalid_argument("param");

	switch (param->get_type()) {
	case gs::effect_parameter::type::Boolean:
		return std::make_shared<gfx::effect_source::bool_parameter>(effect, param);
	case gs::effect_parameter::type::Float:
	case gs::effect_parameter::type::Float2:
	case gs::effect_parameter::type::Float3:
	case gs::effect_parameter::type::Float4:
	case gs::effect_parameter::type::Integer:
	case gs::effect_parameter::type::Integer2:
	case gs::effect_parameter::type::Integer3:
	case gs::effect_parameter::type::Integer4:
		return std::make_shared<gfx::effect_source::value_parameter>(effect, param);
	case gs::effect_parameter::type::Matrix:
		return std::make_shared<gfx::effect_source::matrix_parameter>(effect, param);
	case gs::effect_parameter::type::Texture:
		return std::make_shared<gfx::effect_source::texture_parameter>(effect, param);
	}

	return nullptr;
}

gfx::effect_source::bool_parameter::bool_parameter(std::shared_ptr<gs::effect>           effect,
												   std::shared_ptr<gs::effect_parameter> param)
	: parameter(effect, param)
{
	if (param->get_type() != gs::effect_parameter::type::Boolean)
		throw std::bad_cast();

	param->get_default_bool(_value);
}

void gfx::effect_source::bool_parameter::defaults(obs_properties_t* props, obs_data_t* data) {}

void gfx::effect_source::bool_parameter::properties(obs_properties_t* props)
{
	auto p = obs_properties_add_bool(props, _name.c_str(), _visible_name.c_str());
	obs_property_set_long_description(p, _description.c_str());
	obs_property_set_visible(p, _visible);
}

void gfx::effect_source::bool_parameter::remove_properties(obs_properties_t* props)
{
	obs_properties_remove_by_name(props, _name.c_str());
}

void gfx::effect_source::bool_parameter::update(obs_data_t* data)
{
	_value = obs_data_get_bool(data, _name.c_str());
}

void gfx::effect_source::bool_parameter::tick(float_t time) {}

void gfx::effect_source::bool_parameter::prepare() {}

void gfx::effect_source::bool_parameter::assign()
{
	_param->set_bool(_value);
}

gfx::effect_source::value_parameter::value_parameter(std::shared_ptr<gs::effect>           effect,
													 std::shared_ptr<gs::effect_parameter> param)
	: parameter(effect, param)
{
	std::shared_ptr<gs::effect_parameter> min = param->get_annotation("minimum");
	std::shared_ptr<gs::effect_parameter> max = param->get_annotation("maximum");
	std::shared_ptr<gs::effect_parameter> stp = param->get_annotation("step");

	bool is_int = false;

	switch (param->get_type()) {
	case gs::effect_parameter::type::Float:
		param->get_default_float(_value.f[0]);
		if (min)
			min->get_default_float(_minimum.f[0]);
		if (max)
			max->get_default_float(_maximum.f[0]);
		if (stp)
			stp->get_default_float(_step.f[0]);
		break;
	case gs::effect_parameter::type::Float2:
		param->get_default_float2(_value.f[0], _value.f[1]);
		if (min)
			min->get_default_float2(_minimum.f[0], _minimum.f[1]);
		if (max)
			max->get_default_float2(_maximum.f[0], _maximum.f[1]);
		if (stp)
			stp->get_default_float2(_step.f[0], _step.f[1]);
		break;
	case gs::effect_parameter::type::Float3:
		param->get_default_float3(_value.f[0], _value.f[1], _value.f[2]);
		if (min)
			min->get_default_float3(_minimum.f[0], _minimum.f[1], _minimum.f[2]);
		if (max)
			max->get_default_float3(_maximum.f[0], _maximum.f[1], _maximum.f[2]);
		if (stp)
			stp->get_default_float3(_step.f[0], _step.f[1], _step.f[2]);
		break;
	case gs::effect_parameter::type::Float4:
		param->get_default_float4(_value.f[0], _value.f[1], _value.f[2], _value.f[3]);
		if (min)
			min->get_default_float4(_minimum.f[0], _minimum.f[1], _minimum.f[2], _minimum.f[3]);
		if (max)
			max->get_default_float4(_maximum.f[0], _maximum.f[1], _maximum.f[2], _maximum.f[3]);
		if (stp)
			stp->get_default_float4(_step.f[0], _step.f[1], _step.f[2], _step.f[3]);
		break;
	case gs::effect_parameter::type::Integer:
		param->get_default_int(_value.i[0]);
		if (min)
			min->get_default_int(_minimum.i[0]);
		if (max)
			max->get_default_int(_maximum.i[0]);
		if (stp)
			stp->get_default_int(_step.i[0]);
		is_int = true;
		break;
	case gs::effect_parameter::type::Integer2:
		param->get_default_int2(_value.i[0], _value.i[1]);
		if (min)
			min->get_default_int2(_minimum.i[0], _minimum.i[1]);
		if (max)
			max->get_default_int2(_maximum.i[0], _maximum.i[1]);
		if (stp)
			stp->get_default_int2(_step.i[0], _step.i[1]);
		is_int = true;
		break;
	case gs::effect_parameter::type::Integer3:
		param->get_default_int3(_value.i[0], _value.i[1], _value.i[2]);
		if (min)
			min->get_default_int3(_minimum.i[0], _minimum.i[1], _minimum.i[2]);
		if (max)
			max->get_default_int3(_maximum.i[0], _maximum.i[1], _maximum.i[2]);
		if (stp)
			stp->get_default_int3(_step.i[0], _step.i[1], _step.i[2]);
		is_int = true;
		break;
	case gs::effect_parameter::type::Integer4:
		param->get_default_int4(_value.i[0], _value.i[1], _value.i[2], _value.i[3]);
		if (min)
			min->get_default_int4(_minimum.i[0], _minimum.i[1], _minimum.i[2], _minimum.i[3]);
		if (max)
			max->get_default_int4(_maximum.i[0], _maximum.i[1], _maximum.i[2], _maximum.i[3]);
		if (stp)
			stp->get_default_int4(_step.i[0], _step.i[1], _step.i[2], _step.i[3]);
		is_int = true;
		break;
	default:
		throw std::bad_cast();
	}

	if (!min) {
		if (is_int) {
			_minimum.i[0] = _minimum.i[1] = _minimum.i[2] = _minimum.i[3] = std::numeric_limits<int32_t>::min();
		} else {
			_minimum.f[0] = _minimum.f[1] = _minimum.f[2] = _minimum.f[3] = -167772.16f;
		}
	}
	if (!max) {
		if (is_int) {
			_maximum.i[0] = _maximum.i[1] = _maximum.i[2] = _maximum.i[3] = std::numeric_limits<int32_t>::max();
		} else {
			_maximum.f[0] = _maximum.f[1] = _maximum.f[2] = _maximum.f[3] = +167772.16f;
		}
	}
	if (!stp) {
		if (is_int) {
			_step.i[0] = _step.i[1] = _step.i[2] = _step.i[3] = 1;
		} else {
			_step.f[0] = _step.f[1] = _step.f[2] = _step.f[3] = 0.01f;
		}
	}

	std::shared_ptr<gs::effect_parameter> mode = param->get_annotation("mode");
	if (mode && (mode->get_type() == gs::effect_parameter::type::String)) {
		std::string mode_str = mode->get_default_string();
		if (strcmpi(mode_str.c_str(), "slider") == 0) {
			_mode = value_mode::SLIDER;
		} else {
			_mode = value_mode::INPUT;
		}
	}

	for (size_t idx = 0; idx < 4; idx++) {
		std::stringstream name_sstr;
		std::stringstream ui_sstr;

		name_sstr << _name << '[' << idx << ']';
		ui_sstr << _visible_name << '[' << idx << ']';

		_cache.name[idx]         = name_sstr.str();
		_cache.visible_name[idx] = ui_sstr.str();
	}
}

void gfx::effect_source::value_parameter::defaults(obs_properties_t* props, obs_data_t* data) {}

void gfx::effect_source::value_parameter::properties(obs_properties_t* props)
{
	auto grp = props;
	if (!util::are_property_groups_broken()) {
		grp    = obs_properties_create();
		auto p = obs_properties_add_group(props, _name.c_str(), _visible_name.c_str(), OBS_GROUP_NORMAL, grp);
		obs_property_set_long_description(p, _description.c_str());
		obs_property_set_visible(p, _visible);
	}

	bool   is_int = false;
	size_t limit  = 0;

	switch (_param->get_type()) {
	case gs::effect_parameter::type::Integer:
		is_int = true;
		limit  = 1;
		break;
	case gs::effect_parameter::type::Integer2:
		is_int = true;
		limit  = 2;
		break;
	case gs::effect_parameter::type::Integer3:
		is_int = true;
		limit  = 3;
		break;
	case gs::effect_parameter::type::Integer4:
		is_int = true;
		limit  = 4;
		break;
	case gs::effect_parameter::type::Float:
		is_int = false;
		limit  = 1;
		break;
	case gs::effect_parameter::type::Float2:
		is_int = false;
		limit  = 2;
		break;
	case gs::effect_parameter::type::Float3:
		is_int = false;
		limit  = 3;
		break;
	case gs::effect_parameter::type::Float4:
		is_int = false;
		limit  = 4;
		break;
	}

	for (size_t idx = 0; idx < limit; idx++) {
		if (is_int) {
			if (_mode == value_mode::INPUT) {
				auto p = obs_properties_add_int(grp, _cache.name[idx].c_str(), _cache.visible_name[idx].c_str(),
												_minimum.i[idx], _maximum.i[idx], _step.i[idx]);
				obs_property_set_visible(p, _visible);
			} else {
				auto p = obs_properties_add_int_slider(grp, _cache.name[idx].c_str(), _cache.visible_name[idx].c_str(),
													   _minimum.i[idx], _maximum.i[idx], _step.i[idx]);
				obs_property_set_visible(p, _visible);
			}
		} else {
			if (_mode == value_mode::INPUT) {
				auto p = obs_properties_add_float(grp, _cache.name[idx].c_str(), _cache.visible_name[idx].c_str(),
												  _minimum.f[idx], _maximum.f[idx], _step.f[idx]);
				obs_property_set_visible(p, _visible);
			} else {
				auto p =
					obs_properties_add_float_slider(grp, _cache.name[idx].c_str(), _cache.visible_name[idx].c_str(),
													_minimum.f[idx], _maximum.f[idx], _step.f[idx]);
				obs_property_set_visible(p, _visible);
			}
		}
	}
}

void gfx::effect_source::value_parameter::remove_properties(obs_properties_t* props)
{
	bool   is_int = false;
	size_t limit  = 0;

	switch (_param->get_type()) {
	case gs::effect_parameter::type::Integer:
		is_int = true;
		limit  = 1;
		break;
	case gs::effect_parameter::type::Integer2:
		is_int = true;
		limit  = 2;
		break;
	case gs::effect_parameter::type::Integer3:
		is_int = true;
		limit  = 3;
		break;
	case gs::effect_parameter::type::Integer4:
		is_int = true;
		limit  = 4;
		break;
	case gs::effect_parameter::type::Float:
		is_int = false;
		limit  = 1;
		break;
	case gs::effect_parameter::type::Float2:
		is_int = false;
		limit  = 2;
		break;
	case gs::effect_parameter::type::Float3:
		is_int = false;
		limit  = 3;
		break;
	case gs::effect_parameter::type::Float4:
		is_int = false;
		limit  = 4;
		break;
	}

	for (size_t idx = 0; idx < limit; idx++) {
		obs_properties_remove_by_name(props, _cache.name[idx].c_str());
	}
}

void gfx::effect_source::value_parameter::update(obs_data_t* data)
{
	bool   is_int = false;
	size_t limit  = 0;

	switch (_param->get_type()) {
	case gs::effect_parameter::type::Integer:
		is_int = true;
		limit  = 1;
		break;
	case gs::effect_parameter::type::Integer2:
		is_int = true;
		limit  = 2;
		break;
	case gs::effect_parameter::type::Integer3:
		is_int = true;
		limit  = 3;
		break;
	case gs::effect_parameter::type::Integer4:
		is_int = true;
		limit  = 4;
		break;
	case gs::effect_parameter::type::Float:
		is_int = false;
		limit  = 1;
		break;
	case gs::effect_parameter::type::Float2:
		is_int = false;
		limit  = 2;
		break;
	case gs::effect_parameter::type::Float3:
		is_int = false;
		limit  = 3;
		break;
	case gs::effect_parameter::type::Float4:
		is_int = false;
		limit  = 4;
		break;
	}

	for (size_t idx = 0; idx < limit; idx++) {
		if (is_int) {
			_value.i[idx] = obs_data_get_int(data, _cache.name[idx].c_str());
		} else {
			_value.f[idx] = static_cast<float_t>(obs_data_get_double(data, _cache.name[idx].c_str()));
		}
	}
}

void gfx::effect_source::value_parameter::tick(float_t time) {}

void gfx::effect_source::value_parameter::prepare() {}

void gfx::effect_source::value_parameter::assign()
{
	switch (_param->get_type()) {
	case gs::effect_parameter::type::Integer:
		_param->set_int(_value.i[0]);
		break;
	case gs::effect_parameter::type::Integer2:
		_param->set_int2(_value.i[0], _value.i[1]);
		break;
	case gs::effect_parameter::type::Integer3:
		_param->set_int3(_value.i[0], _value.i[1], _value.i[2]);
		break;
	case gs::effect_parameter::type::Integer4:
		_param->set_int4(_value.i[0], _value.i[1], _value.i[2], _value.i[3]);
		break;
	case gs::effect_parameter::type::Float:
		_param->set_float(_value.f[0]);
		break;
	case gs::effect_parameter::type::Float2:
		_param->set_float2(_value.f[0], _value.f[1]);
		break;
	case gs::effect_parameter::type::Float3:
		_param->set_float3(_value.f[0], _value.f[1], _value.f[2]);
		break;
	case gs::effect_parameter::type::Float4:
		_param->set_float4(_value.f[0], _value.f[1], _value.f[2], _value.f[3]);
		break;
	}
}

gfx::effect_source::matrix_parameter::matrix_parameter(std::shared_ptr<gs::effect>           effect,
													   std::shared_ptr<gs::effect_parameter> param)
	: parameter(effect, param)
{
	std::shared_ptr<gs::effect_parameter> min = param->get_annotation("minimum");
	std::shared_ptr<gs::effect_parameter> max = param->get_annotation("maximum");
	std::shared_ptr<gs::effect_parameter> stp = param->get_annotation("step");

	param->get_default_matrix(_value);
	if (min)
		min->get_default_matrix(_minimum);
	else
		_minimum = matrix4{vec4{-167772.16f, -167772.16f, -167772.16f, -167772.16f},
						   vec4{-167772.16f, -167772.16f, -167772.16f, -167772.16f},
						   vec4{-167772.16f, -167772.16f, -167772.16f, -167772.16f},
						   vec4{-167772.16f, -167772.16f, -167772.16f, -167772.16f}};

	if (max)
		max->get_default_matrix(_maximum);
	else
		_maximum = matrix4{vec4{-167772.16f, -167772.16f, -167772.16f, -167772.16f},
						   vec4{-167772.16f, -167772.16f, -167772.16f, -167772.16f},
						   vec4{-167772.16f, -167772.16f, -167772.16f, -167772.16f},
						   vec4{-167772.16f, -167772.16f, -167772.16f, -167772.16f}};

	if (stp)
		stp->get_default_matrix(_step);
	else
		_step = matrix4{vec4{0.01f, 0.01f, 0.01f, 0.01f}, vec4{0.01f, 0.01f, 0.01f, 0.01f},
						vec4{0.01f, 0.01f, 0.01f, 0.01f}, vec4{0.01f, 0.01f, 0.01f, 0.01f}};

	std::shared_ptr<gs::effect_parameter> mode = param->get_annotation("mode");
	if (mode && (mode->get_type() == gs::effect_parameter::type::String)) {
		std::string mode_str = mode->get_default_string();
		if (strcmpi(mode_str.c_str(), "slider") == 0) {
			_mode = value_mode::SLIDER;
		} else {
			_mode = value_mode::INPUT;
		}
	}

	for (size_t x = 0; x < 4; x++) {
		for (size_t y = 0; y < 4; y++) {
			std::stringstream name_sstr;
			std::stringstream ui_sstr;

			name_sstr << _name << '[' << x << ']' << '[' << y << ']';
			ui_sstr << _visible_name << '[' << x << ']' << '[' << y << ']';

			_cache.name[x * 4 + y]         = name_sstr.str();
			_cache.visible_name[x * 4 + y] = ui_sstr.str();
		}
	}
}

void gfx::effect_source::matrix_parameter::defaults(obs_properties_t* props, obs_data_t* data) {}

void gfx::effect_source::matrix_parameter::properties(obs_properties_t* props)
{
	auto grp = props;
	if (!util::are_property_groups_broken()) {
		grp    = obs_properties_create();
		auto p = obs_properties_add_group(props, _name.c_str(), _visible_name.c_str(), OBS_GROUP_NORMAL, grp);
		obs_property_set_long_description(p, _description.c_str());
		obs_property_set_visible(p, _visible);
	}

	for (size_t x = 0; x < 4; x++) {
		vec4& min_ref = _minimum.x;
		vec4& max_ref = _maximum.x;
		vec4& stp_ref = _step.x;
		if (x == 0) {
			min_ref = _minimum.x;
			max_ref = _maximum.x;
			stp_ref = _step.x;
		} else if (x == 1) {
			min_ref = _minimum.y;
			max_ref = _maximum.y;
			stp_ref = _step.y;
		} else if (x == 2) {
			min_ref = _minimum.z;
			max_ref = _maximum.z;
			stp_ref = _step.z;
		} else {
			min_ref = _minimum.t;
			max_ref = _maximum.t;
			stp_ref = _step.t;
		}

		for (size_t y = 0; y < 4; y++) {
			size_t idx = x * 4 + y;

			if (_mode == value_mode::INPUT) {
				auto p = obs_properties_add_float(grp, _cache.name[idx].c_str(), _cache.visible_name[idx].c_str(),
												  min_ref.ptr[y], max_ref.ptr[y], stp_ref.ptr[y]);
				obs_property_set_visible(p, _visible);
			} else {
				auto p =
					obs_properties_add_float_slider(grp, _cache.name[idx].c_str(), _cache.visible_name[idx].c_str(),
													min_ref.ptr[y], max_ref.ptr[y], stp_ref.ptr[y]);
				obs_property_set_visible(p, _visible);
			}
		}
	}
}

void gfx::effect_source::matrix_parameter::remove_properties(obs_properties_t* props)
{
	for (size_t x = 0; x < 4; x++) {
		for (size_t y = 0; y < 4; y++) {
			size_t idx = x * 4 + y;
			obs_properties_remove_by_name(props, _cache.name[idx].c_str());
		}
	}
}

void gfx::effect_source::matrix_parameter::update(obs_data_t* data)
{
	for (size_t x = 0; x < 4; x++) {
		vec4& v_ref = _value.x;
		if (x == 0) {
			vec4& v_ref = _value.x;
		} else if (x == 1) {
			vec4& v_ref = _value.y;
		} else if (x == 2) {
			vec4& v_ref = _value.z;
		} else {
			vec4& v_ref = _value.t;
		}

		for (size_t y = 0; y < 4; y++) {
			size_t idx   = x * 4 + y;
			v_ref.ptr[y] = obs_data_get_double(data, _cache.name[idx].c_str());
		}
	}
}

void gfx::effect_source::matrix_parameter::tick(float_t time) {}

void gfx::effect_source::matrix_parameter::prepare() {}

void gfx::effect_source::matrix_parameter::assign()
{
	_param->set_matrix(_value);
}

gfx::effect_source::string_parameter::string_parameter(std::shared_ptr<gs::effect>           effect,
													   std::shared_ptr<gs::effect_parameter> param)
	: parameter(effect, param)
{
	param->get_default_string(_value);
}

void gfx::effect_source::string_parameter::defaults(obs_properties_t* props, obs_data_t* data) {}

void gfx::effect_source::string_parameter::properties(obs_properties_t* props) {}

void gfx::effect_source::string_parameter::remove_properties(obs_properties_t* props) {}

void gfx::effect_source::string_parameter::update(obs_data_t* data) {}

void gfx::effect_source::string_parameter::tick(float_t time) {}

void gfx::effect_source::string_parameter::prepare() {}

void gfx::effect_source::string_parameter::assign() {}

gfx::effect_source::texture_parameter::texture_parameter(std::shared_ptr<gs::effect>           effect,
														 std::shared_ptr<gs::effect_parameter> param)
	: parameter(effect, param)
{}

void gfx::effect_source::texture_parameter::defaults(obs_properties_t* props, obs_data_t* data) {}

void gfx::effect_source::texture_parameter::properties(obs_properties_t* props) {}

void gfx::effect_source::texture_parameter::remove_properties(obs_properties_t* props) {}

void gfx::effect_source::texture_parameter::update(obs_data_t* data) {}

void gfx::effect_source::texture_parameter::tick(float_t time) {}

void gfx::effect_source::texture_parameter::prepare() {}

void gfx::effect_source::texture_parameter::assign() {}

bool gfx::effect_source::effect_source::modified2(obs_properties_t* props, obs_property_t* property,
												  obs_data_t* settings)
{
	// Broken, gets stuck locking gs::context.
	/*
	auto gctx = gs::context();
	for (auto& kv : _params) {
		if (kv.second)
			kv.second->remove_properties(props);
	}

	try {
		const char* str = obs_data_get_string(settings, ST_FILE);
		load_file(str ? str : "");
	} catch (std::exception& ex) {
		P_LOG_ERROR("<gfx::effect_source> Failed to load effect \"%s\" due to error: %s", _file.c_str(), ex.what());
	}

	for (auto& kv : _params) {
		if (kv.second)
			kv.second->properties(props);
	}*/

	for (auto& kv : _params) {
		if (kv.second)
			kv.second->defaults(props, settings);
	}

	return true;
}

void gfx::effect_source::effect_source::load_file(std::string file)
{
	auto gctx = gs::context();

	_params.clear();
	_effect.reset();
	_file        = file;
	_time        = 0;
	_time_active = 0;

	struct stat st;
	if (os_stat(_file.c_str(), &st) == -1) {
		_last_size        = 0;
		_last_modify_time = 0;
		_last_create_time = 0;
		throw std::system_error(std::error_code(ENOENT, std::system_category()), file.c_str());
	} else {
		_last_size        = st.st_size;
		_last_modify_time = st.st_mtime;
		_last_create_time = st.st_ctime;
	}

	_effect   = gs::effect::create(file);
	auto prms = _effect->get_parameters();
	for (auto prm : prms) {
		param_ident_t identity;
		identity.first  = prm->get_type();
		identity.second = prm->get_name();

		bool skip = false;
		for (auto v : static_parameters) {
			if (prm->get_name() == v) {
				skip = true;
				break;
			}
		}
		if (_cb_valid)
			skip = skip || !_cb_valid(prm);
		if (skip)
			continue;

		_params.emplace(identity, parameter::create(_effect, prm));
	}
}

gfx::effect_source::effect_source::effect_source()
	: _last_check(0), _last_size(0), _last_modify_time(0), _last_create_time(0), _time(0), _time_active(0),
	  _time_since_last_tick(0)
{
	auto gctx = gs::context();

	_tri = std::make_shared<gs::vertex_buffer>(3ul, uint8_t(1));
	{
		auto& vtx = _tri->at(0);
		vec3_set(vtx.position, 0, 0, 0);
		vec4_set(vtx.uv[0], 0, 0, 0, 0);
	}
	{
		auto& vtx = _tri->at(1);
		vec3_set(vtx.position, 2, 0, 0);
		vec4_set(vtx.uv[0], 2, 0, 0, 0);
	}
	{
		auto& vtx = _tri->at(2);
		vec3_set(vtx.position, 0, 2, 0);
		vec4_set(vtx.uv[0], 0, 2, 0, 0);
	}
}

gfx::effect_source::effect_source::~effect_source() {}

void gfx::effect_source::effect_source::properties(obs_properties_t* props)
{
	auto p = obs_properties_add_path(props, ST_FILE, D_TRANSLATE(ST_FILE), OBS_PATH_FILE, "Effects (*.effect);;*.*",
									 nullptr);
	obs_property_set_modified_callback2(
		p,
		[](void* priv, obs_properties_t* props, obs_property_t* property, obs_data_t* settings) {
			return reinterpret_cast<gfx::effect_source::effect_source*>(priv)->modified2(props, property, settings);
		},
		this);
	obs_properties_add_text(props, ST_TECHNIQUE, D_TRANSLATE(ST_TECHNIQUE), OBS_TEXT_DEFAULT);

	for (auto& kv : _params) {
		if (kv.second)
			kv.second->properties(props);
	}
}

void gfx::effect_source::effect_source::update(obs_data_t* data)
{
	const char* file = obs_data_get_string(data, ST_FILE);
	if (file != _file) {
		try {
			load_file(file);
		} catch (std::exception& ex) {
			P_LOG_ERROR("<gfx::effect_source> Failed to load effect \"%s\" due to error: %s", _file.c_str(), ex.what());
		}
	}

	const char* str = obs_data_get_string(data, ST_TECHNIQUE);
	_tech           = str ? str : "Draw";

	for (auto& kv : _params) {
		if (kv.second)
			kv.second->update(data);
	}
}

bool gfx::effect_source::effect_source::tick(float_t time)
{
	_last_check += time;
	if (_last_check >= 0.5f) {
		_last_check -= 0.5f;
		bool changed = false;

		struct stat st;
		if (os_stat(_file.c_str(), &st) != -1) {
			changed =
				(_last_size != st.st_size) || (_last_modify_time != st.st_mtime) || (_last_create_time != st.st_ctime);
		}
		if (changed) {
			try {
				load_file(_file);
			} catch (std::exception& ex) {
				P_LOG_ERROR("Loading shader \"%s\" failed, error: %s", _file.c_str(), ex.what());
			}
			return true;
		}
	}

	for (auto& kv : _params) {
		if (kv.second)
			kv.second->tick(time);
	}

	_time += time;
	_time_since_last_tick = time;

	return false;
}

void gfx::effect_source::effect_source::render()
{
	if (!_effect)
		return;

	for (auto& kv : _params) {
		if (kv.second)
			kv.second->prepare();
	}

	for (auto& kv : _params) {
		if (kv.second)
			kv.second->assign();
	}

	// Apply "special" parameters.
	_time_active += _time_since_last_tick;
	{
		auto p_time = _effect->get_parameter("Time");
		if (p_time && (p_time->get_type() == gs::effect_parameter::type::Float4)) {
			p_time->set_float4(_time, _time_active, _time_since_last_tick, _random_dist(_random_generator));
		}
		auto p_random = _effect->get_parameter("Random");
		if (p_random && (p_random->get_type() == gs::effect_parameter::type::Matrix)) {
			matrix4 m;
			vec4_set(&m.x, _random_dist(_random_generator), _random_dist(_random_generator),
					 _random_dist(_random_generator), _random_dist(_random_generator));
			vec4_set(&m.y, _random_dist(_random_generator), _random_dist(_random_generator),
					 _random_dist(_random_generator), _random_dist(_random_generator));
			vec4_set(&m.z, _random_dist(_random_generator), _random_dist(_random_generator),
					 _random_dist(_random_generator), _random_dist(_random_generator));
			vec4_set(&m.t, _random_dist(_random_generator), _random_dist(_random_generator),
					 _random_dist(_random_generator), _random_dist(_random_generator));
			p_random->set_matrix(m);
		}
	}

	if (_cb_override) {
		_cb_override(_effect);
	}

	gs_blend_state_push();
	gs_matrix_push();

	gs_reset_blend_state();
	gs_enable_blending(false);
	gs_enable_color(true, true, true, true);
	gs_enable_depth_test(false);
	gs_enable_stencil_test(false);
	gs_enable_stencil_write(false);
	gs_ortho(0, 1, 0, 1, -1., 1.);

	while (gs_effect_loop(_effect->get_object(), _tech.c_str())) {
		gs_load_vertexbuffer(_tri->update());
		gs_load_indexbuffer(nullptr);
		gs_draw(gs_draw_mode::GS_TRIS, 0, _tri->size());
	}

	gs_matrix_pop();
	gs_blend_state_pop();
}

void gfx::effect_source::effect_source::set_valid_property_cb(valid_property_cb_t cb)
{
	_cb_valid = cb;
}

void gfx::effect_source::effect_source::set_override_cb(param_override_cb_t cb)
{
	_cb_override = cb;
}
