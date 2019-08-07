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
#include <sys/stat.h>
#include "strings.hpp"

#define ST "Shader"
#define ST_FILE "Shader.File"

gfx::effect_source::parameter::parameter(std::shared_ptr<gs::effect> effect, std::string name)
	: effect(effect), name(name), description(""), formulae(""), visible(true)
{
	if (!effect)
		throw std::invalid_argument("effect");
	if (!effect->has_parameter(name))
		throw std::invalid_argument("name");
	param = effect->get_parameter(name);

	if (param->has_annotation("description", gs::effect_parameter::type::String)) {
		param->get_annotation("description")->get_default_string(description);
	}
	if (param->has_annotation("formulae", gs::effect_parameter::type::String)) {
		param->get_annotation("formulae")->get_default_string(formulae);
	}
	if (param->has_annotation("visible", gs::effect_parameter::type::Boolean)) {
		param->get_annotation("visible")->get_default_bool(visible);
	} else {
		visible = true;
	}
}

gfx::effect_source::bool_parameter::bool_parameter(std::shared_ptr<gs::effect> effect, std::string name)
	: parameter(effect, name)
{
	if (param->get_type() != gs::effect_parameter::type::Boolean)
		throw std::bad_cast();

	param->get_default_bool(value);
}

gfx::effect_source::value_parameter::value_parameter(std::shared_ptr<gs::effect> effect, std::string name)
	: parameter(effect, name)
{
	std::shared_ptr<gs::effect_parameter> min = param->get_annotation("minimum");
	std::shared_ptr<gs::effect_parameter> max = param->get_annotation("maximum");

	switch (param->get_type()) {
	case gs::effect_parameter::type::Float:
		param->get_default_float(value.f[0]);
		if (min)
			min->get_default_float(minimum.f[0]);
		if (max)
			max->get_default_float(maximum.f[0]);
		break;
	case gs::effect_parameter::type::Float2:
		param->get_default_float2(value.f[0], value.f[1]);
		if (min)
			min->get_default_float2(minimum.f[0], minimum.f[1]);
		if (max)
			max->get_default_float2(maximum.f[0], maximum.f[1]);
		break;
	case gs::effect_parameter::type::Float3:
		param->get_default_float3(value.f[0], value.f[1], value.f[2]);
		if (min)
			min->get_default_float3(minimum.f[0], minimum.f[1], minimum.f[2]);
		if (max)
			max->get_default_float3(maximum.f[0], maximum.f[1], maximum.f[2]);
		break;
	case gs::effect_parameter::type::Float4:
		param->get_default_float4(value.f[0], value.f[1], value.f[2], value.f[3]);
		if (min)
			min->get_default_float4(minimum.f[0], minimum.f[1], minimum.f[2], minimum.f[3]);
		if (max)
			max->get_default_float4(maximum.f[0], maximum.f[1], maximum.f[2], maximum.f[3]);
		break;
	case gs::effect_parameter::type::Integer:
		param->get_default_int(value.i[0]);
		if (min)
			min->get_default_int(minimum.i[0]);
		if (max)
			max->get_default_int(maximum.i[0]);
		break;
	case gs::effect_parameter::type::Integer2:
		param->get_default_int2(value.i[0], value.i[1]);
		if (min)
			min->get_default_int2(minimum.i[0], minimum.i[1]);
		if (max)
			max->get_default_int2(maximum.i[0], maximum.i[1]);
		break;
	case gs::effect_parameter::type::Integer3:
		param->get_default_int3(value.i[0], value.i[1], value.i[2]);
		if (min)
			min->get_default_int3(minimum.i[0], minimum.i[1], minimum.i[2]);
		if (max)
			max->get_default_int3(maximum.i[0], maximum.i[1], maximum.i[2]);
		break;
	case gs::effect_parameter::type::Integer4:
		param->get_default_int4(value.i[0], value.i[1], value.i[2], value.i[3]);
		if (min)
			min->get_default_int4(minimum.i[0], minimum.i[1], minimum.i[2], minimum.i[3]);
		if (max)
			max->get_default_int4(maximum.i[0], maximum.i[1], maximum.i[2], maximum.i[3]);
		break;
	default:
		throw std::bad_cast();
	}
}

gfx::effect_source::matrix_parameter::matrix_parameter(std::shared_ptr<gs::effect> effect, std::string name)
	: parameter(effect, name)
{
	std::shared_ptr<gs::effect_parameter> min = param->get_annotation("minimum");
	std::shared_ptr<gs::effect_parameter> max = param->get_annotation("maximum");

	param->get_default_matrix(value);
	if (min)
		min->get_default_matrix(minimum);
	if (max)
		max->get_default_matrix(maximum);
}

gfx::effect_source::string_parameter::string_parameter(std::shared_ptr<gs::effect> effect, std::string name)
	: parameter(effect, name)
{
	param->get_default_string(value);
}

gfx::effect_source::texture_parameter::texture_parameter(std::shared_ptr<gs::effect> effect, std::string name)
	: parameter(effect, name)
{}

void gfx::effect_source::shader_instance::load_file(std::string file)
{
	_params.clear();
	_effect.reset();
	_file = file;

	struct stat st;
	if (!os_stat(_file.c_str(), &st)) {
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
		std::shared_ptr<parameter> parameter;

		switch (prm->get_type()) {
		case gs::effect_parameter::type::Boolean:
			parameter = std::make_shared<bool_parameter>(_effect, prm->get_name());
			break;
		case gs::effect_parameter::type::Float:
		case gs::effect_parameter::type::Float2:
		case gs::effect_parameter::type::Float3:
		case gs::effect_parameter::type::Float4:
		case gs::effect_parameter::type::Integer:
		case gs::effect_parameter::type::Integer2:
		case gs::effect_parameter::type::Integer3:
		case gs::effect_parameter::type::Integer4:
			parameter = std::make_shared<value_parameter>(_effect, prm->get_name());
			break;
		case gs::effect_parameter::type::Matrix:
			parameter = std::make_shared<matrix_parameter>(_effect, prm->get_name());
			break;
		case gs::effect_parameter::type::String:
			parameter = std::make_shared<string_parameter>(_effect, prm->get_name());
			break;
		case gs::effect_parameter::type::Texture:
			parameter = std::make_shared<texture_parameter>(_effect, prm->get_name());
			break;
		}

		_params.emplace(identity, parameter);
	}
}

gfx::effect_source::shader_instance::shader_instance(std::string file)
	: _last_check(0), _last_size(0), _last_modify_time(0), _last_create_time(0)
{
	load_file(file);
}

gfx::effect_source::shader_instance::~shader_instance() {}

void gfx::effect_source::shader_instance::tick(float_t time)
{
	_last_check += time;
	if (_last_check >= 0.5f) {
		_last_check -= 0.5f;
		bool changed = false;

		struct stat st;
		if (os_stat(_file.c_str(), &st)) {
			changed =
				(_last_size != st.st_size) || (_last_modify_time != st.st_mtime) || (_last_create_time != st.st_ctime);
		}
		if (changed) {
			load_file(_file);
		}
	}
}

void gfx::effect_source::shader_instance::render(std::string technique)
{
	gs_blend_state_push();
	gs_matrix_push();

	gs_reset_blend_state();
	gs_enable_blending(false);
	gs_enable_color(true, true, true, true);
	gs_enable_depth_test(false);
	gs_enable_stencil_test(false);
	gs_enable_stencil_write(false);
	gs_ortho(0, 1, 0, 1, -1., 1.);

	while (gs_effect_loop(_effect->get_object(), technique.c_str())) {
	}

	gs_matrix_pop();
	gs_blend_state_pop();
}
