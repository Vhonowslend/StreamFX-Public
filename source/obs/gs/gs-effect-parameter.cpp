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
#include <graphics/vec2.h>

gs::effect_parameter::effect_parameter() : _effect_parent(nullptr), _pass_parent(nullptr), _param_parent(nullptr)
{
	reset();
}

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
	const char* name_c   = get()->name;
	size_t      name_len = strnlen_s(name_c, 256);
	return name_c ? std::string(name_c, name_c + name_len) : std::string();
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

gs::effect_parameter gs::effect_parameter::get_annotation(size_t idx)
{
	if (idx >= get()->annotations.num) {
		return nullptr;
	}

	return effect_parameter(get()->annotations.array + idx, this);
}

gs::effect_parameter gs::effect_parameter::get_annotation(std::string name)
{
	for (size_t idx = 0; idx < get()->annotations.num; idx++) {
		auto ptr = get()->annotations.array + idx;
		if (strcmp(ptr->name, name.c_str()) == 0) {
			return gs::effect_parameter(ptr, this);
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
		return eprm.get_type() == type;
	return false;
}

void gs::effect_parameter::set_bool(bool v)
{
	if (get_type() != type::Boolean)
		throw std::bad_cast();
	gs_effect_set_bool(get(), v);
}

void gs::effect_parameter::get_bool(bool& v)
{
	if (get_type() != type::Boolean)
		throw std::bad_cast();
	void* ptr = gs_effect_get_val(get());
	if (ptr) {
		v = *reinterpret_cast<bool*>(ptr);
		bfree(ptr);
	} else {
		v = false;
	}
}

void gs::effect_parameter::get_default_bool(bool& v)
{
	if (get_type() != type::Boolean)
		throw std::bad_cast();
	void* ptr = gs_effect_get_default_val(get());
	if (ptr) {
		v = *reinterpret_cast<bool*>(ptr);
		bfree(ptr);
	} else {
		v = false;
	}
}

void gs::effect_parameter::set_bool_array(bool v[], size_t sz)
{
	if (get_type() != type::Boolean)
		throw std::bad_cast();
	gs_effect_set_val(get(), v, sz);
}

void gs::effect_parameter::set_float(float_t x)
{
	if (get_type() != type::Float)
		throw std::bad_cast();
	gs_effect_set_float(get(), x);
}

void gs::effect_parameter::get_float(float_t& x)
{
	if (get_type() != type::Float)
		throw std::bad_cast();
	void* ptr = gs_effect_get_val(get());
	if (ptr) {
		x = *reinterpret_cast<float_t*>(ptr);
		bfree(ptr);
	} else {
		x = 0;
	}
}

void gs::effect_parameter::get_default_float(float_t& x)
{
	if (get_type() != type::Float)
		throw std::bad_cast();
	void* ptr = gs_effect_get_default_val(get());
	if (ptr) {
		x = *reinterpret_cast<float_t*>(ptr);
		bfree(ptr);
	} else {
		x = 0;
	}
}

void gs::effect_parameter::set_float2(vec2 const& v)
{
	if (get_type() != type::Float2)
		throw std::bad_cast();
	gs_effect_set_vec2(get(), &v);
}

void gs::effect_parameter::get_float2(vec2& v)
{
	get_float2(v.x, v.y);
}

void gs::effect_parameter::get_default_float2(vec2& v)
{
	get_default_float2(v.x, v.y);
}

void gs::effect_parameter::set_float2(float_t x, float_t y)
{
	vec2 data;
	data.x = x;
	data.y = y;
	set_float2(data);
}

void gs::effect_parameter::get_float2(float_t& x, float_t& y)
{
	if (get_type() != type::Float2)
		throw std::bad_cast();
	uint8_t* ptr = static_cast<uint8_t*>(gs_effect_get_val(get()));
	if (ptr) {
		x = *reinterpret_cast<float_t*>(ptr);
		y = *reinterpret_cast<float_t*>(ptr + sizeof(float_t));
		bfree(ptr);
	} else {
		x = y = 0;
	}
}

void gs::effect_parameter::get_default_float2(float_t& x, float_t& y)
{
	if (get_type() != type::Float2)
		throw std::bad_cast();
	uint8_t* ptr = static_cast<uint8_t*>(gs_effect_get_default_val(get()));
	if (ptr) {
		x = *reinterpret_cast<float_t*>(ptr);
		y = *reinterpret_cast<float_t*>(ptr + sizeof(float_t));
		bfree(ptr);
	} else {
		x = y = 0;
	}
}

void gs::effect_parameter::set_float3(vec3 const& v)
{
	if (get_type() != type::Float3)
		throw std::bad_cast();
	gs_effect_set_vec3(get(), &v);
}

void gs::effect_parameter::get_float3(vec3& v)
{
	get_float3(v.x, v.y, v.z);
}

void gs::effect_parameter::get_default_float3(vec3& v)
{
	get_default_float3(v.x, v.y, v.z);
}

void gs::effect_parameter::set_float3(float_t x, float_t y, float_t z)
{
	if (get_type() != type::Float3)
		throw std::bad_cast();
	vec3 v = {{x, y, z, 0}};
	gs_effect_set_vec3(get(), &v);
}

void gs::effect_parameter::get_float3(float_t& x, float_t& y, float_t& z)
{
	if (get_type() != type::Float3)
		throw std::bad_cast();
	uint8_t* ptr = static_cast<uint8_t*>(gs_effect_get_val(get()));
	if (ptr) {
		x = *reinterpret_cast<float_t*>(ptr);
		y = *reinterpret_cast<float_t*>(ptr + sizeof(float_t));
		z = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 2);
		bfree(ptr);
	} else {
		x = y = z = 0;
	}
}

void gs::effect_parameter::get_default_float3(float_t& x, float_t& y, float_t& z)
{
	if (get_type() != type::Float3)
		throw std::bad_cast();
	uint8_t* ptr = static_cast<uint8_t*>(gs_effect_get_default_val(get()));
	if (ptr) {
		x = *reinterpret_cast<float_t*>(ptr);
		y = *reinterpret_cast<float_t*>(ptr + sizeof(float_t));
		z = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 2);
		bfree(ptr);
	} else {
		x = y = z = 0;
	}
}

void gs::effect_parameter::set_float4(vec4 const& v)
{
	if (get_type() != type::Float4)
		throw std::bad_cast();
	gs_effect_set_vec4(get(), &v);
}

void gs::effect_parameter::get_float4(vec4& v)
{
	get_float4(v.x, v.y, v.z, v.w);
}

void gs::effect_parameter::get_default_float4(vec4& v)
{
	get_default_float4(v.x, v.y, v.z, v.w);
}

void gs::effect_parameter::set_float4(float_t x, float_t y, float_t z, float_t w)
{
	if (get_type() != type::Float4)
		throw std::bad_cast();
	vec4 v = {{x, y, z, w}};
	gs_effect_set_vec4(get(), &v);
}

void gs::effect_parameter::get_float4(float_t& x, float_t& y, float_t& z, float_t& w)
{
	if (get_type() != type::Float4)
		throw std::bad_cast();
	uint8_t* ptr = static_cast<uint8_t*>(gs_effect_get_val(get()));
	if (ptr) {
		x = *reinterpret_cast<float_t*>(ptr);
		y = *reinterpret_cast<float_t*>(ptr + sizeof(float_t));
		z = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 2);
		w = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 3);
		bfree(ptr);
	} else {
		x = y = z = w = 0;
	}
}

void gs::effect_parameter::get_default_float4(float_t& x, float_t& y, float_t& z, float_t& w)
{
	if (get_type() != type::Float4)
		throw std::bad_cast();
	uint8_t* ptr = static_cast<uint8_t*>(gs_effect_get_default_val(get()));
	if (ptr) {
		x = *reinterpret_cast<float_t*>(ptr);
		y = *reinterpret_cast<float_t*>(ptr + sizeof(float_t));
		z = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 2);
		w = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 3);
		bfree(ptr);
	} else {
		x = y = z = w = 0;
	}
}

void gs::effect_parameter::set_int(int32_t x)
{
	if ((get_type() != type::Integer) && (get_type() != type::Unknown))
		throw std::bad_cast();
	gs_effect_set_int(get(), x);
}

void gs::effect_parameter::get_int(int32_t& x)
{
	if ((get_type() != type::Integer) && (get_type() != type::Unknown))
		throw std::bad_cast();
	uint8_t* ptr = static_cast<uint8_t*>(gs_effect_get_val(get()));
	if (ptr) {
		x = *reinterpret_cast<int32_t*>(ptr);
		bfree(ptr);
	} else {
		x = 0;
	}
}

void gs::effect_parameter::get_default_int(int32_t& x)
{
	if ((get_type() != type::Integer) && (get_type() != type::Unknown))
		throw std::bad_cast();
	uint8_t* ptr = static_cast<uint8_t*>(gs_effect_get_default_val(get()));
	if (ptr) {
		x = *reinterpret_cast<int32_t*>(ptr);
		bfree(ptr);
	} else {
		x = 0;
	}
}

void gs::effect_parameter::set_int2(int32_t x, int32_t y)
{
	if ((get_type() != type::Integer2) && (get_type() != type::Unknown))
		throw std::bad_cast();
	int32_t v[2] = {x, y};
	gs_effect_set_val(get(), v, sizeof(int) * 2);
}

void gs::effect_parameter::get_int2(int32_t& x, int32_t& y)
{
	if ((get_type() != type::Integer2) && (get_type() != type::Unknown))
		throw std::bad_cast();
	uint8_t* ptr = static_cast<uint8_t*>(gs_effect_get_val(get()));
	if (ptr) {
		x = *reinterpret_cast<int32_t*>(ptr);
		y = *reinterpret_cast<int32_t*>(ptr + sizeof(int32_t));
		bfree(ptr);
	} else {
		x = y = 0;
	}
}

void gs::effect_parameter::get_default_int2(int32_t& x, int32_t& y)
{
	if ((get_type() != type::Integer2) && (get_type() != type::Unknown))
		throw std::bad_cast();
	uint8_t* ptr = static_cast<uint8_t*>(gs_effect_get_default_val(get()));
	if (ptr) {
		x = *reinterpret_cast<int32_t*>(ptr);
		y = *reinterpret_cast<int32_t*>(ptr + sizeof(int32_t));
		bfree(ptr);
	} else {
		x = y = 0;
	}
}

void gs::effect_parameter::set_int3(int32_t x, int32_t y, int32_t z)
{
	if ((get_type() != type::Integer3) && (get_type() != type::Unknown))
		throw std::bad_cast();
	int32_t v[3] = {x, y, z};
	gs_effect_set_val(get(), v, sizeof(int) * 3);
}

void gs::effect_parameter::get_int3(int32_t& x, int32_t& y, int32_t& z)
{
	if ((get_type() != type::Integer3) && (get_type() != type::Unknown))
		throw std::bad_cast();
	uint8_t* ptr = static_cast<uint8_t*>(gs_effect_get_val(get()));
	if (ptr) {
		x = *reinterpret_cast<int32_t*>(ptr);
		y = *reinterpret_cast<int32_t*>(ptr + sizeof(int32_t));
		z = *reinterpret_cast<int32_t*>(ptr + sizeof(int32_t) * 2);
		bfree(ptr);
	} else {
		x = y = z = 0;
	}
}

void gs::effect_parameter::get_default_int3(int32_t& x, int32_t& y, int32_t& z)
{
	if ((get_type() != type::Integer3) && (get_type() != type::Unknown))
		throw std::bad_cast();
	uint8_t* ptr = static_cast<uint8_t*>(gs_effect_get_default_val(get()));
	if (ptr) {
		x = *reinterpret_cast<int32_t*>(ptr);
		y = *reinterpret_cast<int32_t*>(ptr + sizeof(int32_t));
		z = *reinterpret_cast<int32_t*>(ptr + sizeof(int32_t) * 2);
		bfree(ptr);
	} else {
		x = y = z = 0;
	}
}

void gs::effect_parameter::set_int4(int32_t x, int32_t y, int32_t z, int32_t w)
{
	if ((get_type() != type::Integer4) && (get_type() != type::Unknown))
		throw std::bad_cast();
	int32_t v[4] = {x, y, z, w};
	gs_effect_set_val(get(), v, sizeof(int) * 4);
}

void gs::effect_parameter::get_int4(int32_t& x, int32_t& y, int32_t& z, int32_t& w)
{
	if ((get_type() != type::Integer4) && (get_type() != type::Unknown))
		throw std::bad_cast();
	uint8_t* ptr = static_cast<uint8_t*>(gs_effect_get_val(get()));
	if (ptr) {
		x = *reinterpret_cast<int32_t*>(ptr);
		y = *reinterpret_cast<int32_t*>(ptr + sizeof(int32_t));
		z = *reinterpret_cast<int32_t*>(ptr + sizeof(int32_t) * 2);
		w = *reinterpret_cast<int32_t*>(ptr + sizeof(int32_t) * 3);
		bfree(ptr);
	} else {
		x = y = z = w = 0;
	}
}

void gs::effect_parameter::get_default_int4(int32_t& x, int32_t& y, int32_t& z, int32_t& w)
{
	if ((get_type() != type::Integer4) && (get_type() != type::Unknown))
		throw std::bad_cast();
	uint8_t* ptr = static_cast<uint8_t*>(gs_effect_get_default_val(get()));
	if (ptr) {
		x = *reinterpret_cast<int32_t*>(ptr);
		y = *reinterpret_cast<int32_t*>(ptr + sizeof(int32_t));
		z = *reinterpret_cast<int32_t*>(ptr + sizeof(int32_t) * 2);
		w = *reinterpret_cast<int32_t*>(ptr + sizeof(int32_t) * 3);
		bfree(ptr);
	} else {
		x = y = z = w = 0;
	}
}

void gs::effect_parameter::set_matrix(matrix4 const& v)
{
	if (get_type() != type::Matrix)
		throw std::bad_cast();
	gs_effect_set_matrix4(get(), &v);
}

void gs::effect_parameter::get_matrix(matrix4& v)
{
	if (get_type() != type::Matrix)
		throw std::bad_cast();
	uint8_t* ptr = static_cast<uint8_t*>(gs_effect_get_val(get()));
	if (ptr) {
		v.x.x = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 0);
		v.x.y = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 1);
		v.x.z = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 2);
		v.x.w = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 3);
		v.y.x = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 4);
		v.y.y = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 5);
		v.y.z = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 6);
		v.y.w = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 7);
		v.z.x = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 8);
		v.z.y = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 9);
		v.z.z = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 10);
		v.z.w = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 11);
		v.t.x = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 12);
		v.t.y = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 13);
		v.t.z = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 14);
		v.t.w = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 15);
		bfree(ptr);
	} else {
		v.x = vec4{};
		v.y = vec4{};
		v.z = vec4{};
		v.t = vec4{};
	}
}

void gs::effect_parameter::get_default_matrix(matrix4& v)
{
	if (get_type() != type::Matrix)
		throw std::bad_cast();
	uint8_t* ptr = static_cast<uint8_t*>(gs_effect_get_default_val(get()));
	if (ptr) {
		v.x.x = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 0);
		v.x.y = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 1);
		v.x.z = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 2);
		v.x.w = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 3);
		v.y.x = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 4);
		v.y.y = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 5);
		v.y.z = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 6);
		v.y.w = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 7);
		v.z.x = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 8);
		v.z.y = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 9);
		v.z.z = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 10);
		v.z.w = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 11);
		v.t.x = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 12);
		v.t.y = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 13);
		v.t.z = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 14);
		v.t.w = *reinterpret_cast<float_t*>(ptr + sizeof(float_t) * 15);
		bfree(ptr);
	} else {
		v.x = vec4{};
		v.y = vec4{};
		v.z = vec4{};
		v.t = vec4{};
	}
}

void gs::effect_parameter::set_texture(std::shared_ptr<gs::texture> v)
{
	if (get_type() != type::Texture)
		throw std::bad_cast();
	gs_effect_set_texture(get(), v->get_object());
}

void gs::effect_parameter::set_texture(gs_texture_t* v)
{
	if (get_type() != type::Texture)
		throw std::bad_cast();
	gs_effect_set_texture(get(), v);
}

void gs::effect_parameter::set_sampler(std::shared_ptr<gs::sampler> v)
{
	if (get_type() != type::Texture)
		throw std::bad_cast();
	gs_effect_set_next_sampler(get(), v->get_object());
}

void gs::effect_parameter::set_sampler(gs_sampler_state* v)
{
	if (get_type() != type::Texture)
		throw std::bad_cast();
	gs_effect_set_next_sampler(get(), v);
}

void gs::effect_parameter::set_string(std::string const& v)
{
	if (get_type() != type::String)
		throw std::bad_cast();
	gs_effect_set_val(get(), v.c_str(), v.length());
}

void gs::effect_parameter::get_string(std::string& v)
{
	if (get_type() != type::String)
		throw std::bad_cast();
	size_t   ptr_len = gs_effect_get_val_size(get());
	uint8_t* ptr     = static_cast<uint8_t*>(gs_effect_get_val(get()));
	if (ptr) {
		v = std::string(ptr, ptr + ptr_len - 1);
		bfree(ptr);
	} else {
		v = "";
	}
}

void gs::effect_parameter::get_default_string(std::string& v)
{
	if (get_type() != type::String)
		throw std::bad_cast();
	size_t   ptr_len = gs_effect_get_default_val_size(get());
	uint8_t* ptr     = static_cast<uint8_t*>(gs_effect_get_default_val(get()));
	if (ptr) {
		v = std::string(ptr, ptr + ptr_len - 1);
		bfree(ptr);
	} else {
		v = "";
	}
}
