/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2017 Michael Fabian Dirks
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

#include "gs-effect.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#include "obs/gs/gs-helper.hpp"

// OBS
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <obs.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

//#define OBS_LOAD_EFFECT_FILE

gs::effect::effect() : m_effect(nullptr) {}

gs::effect::effect(std::string file) : effect()
{
#ifdef OBS_LOAD_EFFECT_FILE
	char* errorMessage = nullptr;
	auto  gctx         = gs::context();
	m_effect           = gs_effect_create_from_file(file.c_str(), &errorMessage);
	if (!m_effect || errorMessage) {
		std::string error = "Generic Error";
		if (errorMessage) {
			error = std::string(errorMessage);
			bfree((void*)errorMessage);
		}
		throw std::runtime_error(error);
	}
#else
	std::ifstream filestream = std::ifstream(file, std::ios::binary);
	if (!filestream.is_open()) {
		throw std::runtime_error("Failed to open file.");
	}

	filestream.ignore(std::numeric_limits<std::streamsize>::max());
	std::streamsize length = filestream.gcount();
	filestream.clear(); //  Since ignore will have set eof.
	filestream.seekg(0, std::ios_base::beg);

	if (length > 256 * 1024 * 1024) {
		throw std::runtime_error("Shader too large (>256mb)");
	}

	std::vector<char> shader_buf(size_t(length + 1), 0);
	filestream.read(shader_buf.data(), length);

	char* errorMessage = nullptr;
	auto  gctx         = gs::context();
	m_effect           = gs_effect_create(shader_buf.data(), file.c_str(), &errorMessage);
	if (!m_effect || errorMessage) {
		std::string error = "Generic Error";
		if (errorMessage) {
			error = std::string(errorMessage);
			bfree((void*)errorMessage);
		}
		throw std::runtime_error(error);
	}
#endif
}

gs::effect::effect(std::string code, std::string name) : effect()
{
	char* errorMessage = nullptr;
	auto  gctx         = gs::context();
	m_effect           = gs_effect_create(code.c_str(), name.c_str(), &errorMessage);
	if (!m_effect || errorMessage) {
		std::string error = "Generic Error";
		if (errorMessage) {
			error = std::string(errorMessage);
			bfree((void*)errorMessage);
		}
		throw std::runtime_error(error);
	}
}

gs::effect::~effect()
{
	auto gctx = gs::context();
	gs_effect_destroy(m_effect);
}

gs_effect_t* gs::effect::get_object()
{
	return m_effect;
}

size_t gs::effect::count_parameters()
{
	return (size_t)gs_effect_get_num_params(m_effect);
}

std::list<gs::effect_parameter> gs::effect::get_parameters()
{
	size_t                          num = gs_effect_get_num_params(m_effect);
	std::list<gs::effect_parameter> ps;
	for (size_t idx = 0; idx < num; idx++) {
		ps.emplace_back(get_parameter(idx));
	}
	return ps;
}

gs::effect_parameter gs::effect::get_parameter(size_t idx)
{
	gs_eparam_t* param = gs_effect_get_param_by_idx(m_effect, idx);
	if (!param)
		throw std::invalid_argument("parameter with index not found");
	return effect_parameter(param);
}

bool gs::effect::has_parameter(std::string name)
{
	gs_eparam_t* param = gs_effect_get_param_by_name(m_effect, name.c_str());
	return (param != nullptr);
}

bool gs::effect::has_parameter(std::string name, effect_parameter::type type)
{
	gs_eparam_t* param = gs_effect_get_param_by_name(m_effect, name.c_str());
	if (param == nullptr)
		return false;
	gs::effect_parameter eprm(param);
	return eprm.get_type() == type;
}

gs::effect_parameter gs::effect::get_parameter(std::string name)
{
	gs_eparam_t* param = gs_effect_get_param_by_name(m_effect, name.c_str());
	if (!param)
		throw std::invalid_argument("parameter with name not found");
	return effect_parameter(param);
}

gs::effect_parameter::effect_parameter(gs_eparam_t* param) : m_param(param)
{
	if (!param)
		throw std::invalid_argument("param is null");

	gs_effect_get_param_info(m_param, &m_paramInfo);
}

std::string gs::effect_parameter::get_name()
{
	return m_paramInfo.name;
}

gs::effect_parameter::type gs::effect_parameter::get_type()
{
	switch (m_paramInfo.type) {
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
	//case GS_SHADER_PARAM_STRING:
	//	return Type::String;
	default:
	case GS_SHADER_PARAM_UNKNOWN:
		return type::Unknown;
	}
}

void gs::effect_parameter::set_bool(bool v)
{
	if (get_type() != type::Boolean)
		throw std::bad_cast();
	gs_effect_set_bool(m_param, v);
}

void gs::effect_parameter::set_bool_array(bool v[], size_t sz)
{
	if (get_type() != type::Boolean)
		throw std::bad_cast();
	gs_effect_set_val(m_param, v, sz);
}

void gs::effect_parameter::set_float(float_t x)
{
	if (get_type() != type::Float)
		throw std::bad_cast();
	gs_effect_set_float(m_param, x);
}

void gs::effect_parameter::set_float2(vec2& v)
{
	if (get_type() != type::Float2)
		throw std::bad_cast();
	gs_effect_set_vec2(m_param, &v);
}

void gs::effect_parameter::set_float2(float_t x, float_t y)
{
	if (get_type() != type::Float2)
		throw std::bad_cast();
	vec2 v = {{x, y}};
	gs_effect_set_vec2(m_param, &v);
}

void gs::effect_parameter::set_float3(vec3& v)
{
	if (get_type() != type::Float3)
		throw std::bad_cast();
	gs_effect_set_vec3(m_param, &v);
}

void gs::effect_parameter::set_float3(float_t x, float_t y, float_t z)
{
	if (get_type() != type::Float3)
		throw std::bad_cast();
	vec3 v = {{x, y, z, 0}};
	gs_effect_set_vec3(m_param, &v);
}

void gs::effect_parameter::set_float4(vec4& v)
{
	if (get_type() != type::Float4)
		throw std::bad_cast();
	gs_effect_set_vec4(m_param, &v);
}

void gs::effect_parameter::set_float4(float_t x, float_t y, float_t z, float_t w)
{
	if (get_type() != type::Float4)
		throw std::bad_cast();
	vec4 v = {{x, y, z, w}};
	gs_effect_set_vec4(m_param, &v);
}

void gs::effect_parameter::set_float_array(float_t v[], size_t sz)
{
	if ((get_type() != type::Float) && (get_type() != type::Float2) && (get_type() != type::Float3)
		&& (get_type() != type::Float4))
		throw std::bad_cast();
	gs_effect_set_val(m_param, v, sizeof(float_t) * sz);
}

void gs::effect_parameter::set_int(int32_t x)
{
	if ((get_type() != type::Integer) && (get_type() != type::Unknown))
		throw std::bad_cast();
	gs_effect_set_int(m_param, x);
}

void gs::effect_parameter::set_int2(int32_t x, int32_t y)
{
	if ((get_type() != type::Integer2) && (get_type() != type::Unknown))
		throw std::bad_cast();
	int32_t v[2] = {x, y};
	gs_effect_set_val(m_param, v, sizeof(int) * 2);
}

void gs::effect_parameter::set_int3(int32_t x, int32_t y, int32_t z)
{
	if ((get_type() != type::Integer3) && (get_type() != type::Unknown))
		throw std::bad_cast();
	int32_t v[3] = {x, y, z};
	gs_effect_set_val(m_param, v, sizeof(int) * 3);
}

void gs::effect_parameter::set_int4(int32_t x, int32_t y, int32_t z, int32_t w)
{
	if ((get_type() != type::Integer4) && (get_type() != type::Unknown))
		throw std::bad_cast();
	int32_t v[4] = {x, y, z, w};
	gs_effect_set_val(m_param, v, sizeof(int) * 4);
}

void gs::effect_parameter::set_int_array(int32_t v[], size_t sz)
{
	if ((get_type() != type::Integer) && (get_type() != type::Integer2) && (get_type() != type::Integer3)
		&& (get_type() != type::Integer4) && (get_type() != type::Unknown))
		throw std::bad_cast();
	gs_effect_set_val(m_param, v, sizeof(int) * sz);
}

void gs::effect_parameter::set_matrix(matrix4& v)
{
	if (get_type() != type::Matrix)
		throw std::bad_cast();
	gs_effect_set_matrix4(m_param, &v);
}

void gs::effect_parameter::set_texture(std::shared_ptr<gs::texture> v)
{
	if (get_type() != type::Texture)
		throw std::bad_cast();
	gs_effect_set_texture(m_param, v->get_object());
}

void gs::effect_parameter::set_texture(gs_texture_t* v)
{
	if (get_type() != type::Texture)
		throw std::bad_cast();
	gs_effect_set_texture(m_param, v);
}

void gs::effect_parameter::set_sampler(std::shared_ptr<gs::sampler> v)
{
	if (get_type() != type::Texture)
		throw std::bad_cast();
	gs_effect_set_next_sampler(m_param, v->get_object());
}

void gs::effect_parameter::set_sampler(gs_sampler_state* v)
{
	if (get_type() != type::Texture)
		throw std::bad_cast();
	gs_effect_set_next_sampler(m_param, v);
}
