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

#pragma once
#include <cinttypes>
#include <list>
#include <memory>
#include <string>
#include "gs-sampler.hpp"
#include "gs-texture.hpp"

#pragma warning(push)
#pragma warning(disable : 4201)
#include <graphics/graphics.h>
#include <graphics/matrix4.h>
#include <graphics/vec2.h>
#include <graphics/vec3.h>
#include <graphics/vec4.h>
#pragma warning(pop)

namespace gs {
	class effect_parameter {
		public:
		enum class type : uint8_t {
			Unknown,
			Boolean,
			Float,
			Float2,
			Float3,
			Float4,
			Integer,
			Integer2,
			Integer3,
			Integer4,
			Matrix,
			String,
			Texture,
		};

		public:
		effect_parameter(gs_eparam_t* param);

		std::string get_name();
		type        get_type();

		void set_bool(bool v);
		void set_bool_array(bool v[], size_t sz);
		void set_float(float_t x);
		void set_float2(vec2& v);
		void set_float2(float_t x, float_t y);
		void set_float3(vec3& v);
		void set_float3(float_t x, float_t y, float_t z);
		void set_float4(vec4& v);
		void set_float4(float_t x, float_t y, float_t z, float_t w);
		void set_float_array(float_t v[], size_t sz);
		void set_int(int32_t x);
		void set_int2(int32_t x, int32_t y);
		void set_int3(int32_t x, int32_t y, int32_t z);
		void set_int4(int32_t x, int32_t y, int32_t z, int32_t w);
		void set_int_array(int32_t v[], size_t sz);
		void set_matrix(matrix4& v);
		void set_texture(std::shared_ptr<gs::texture> v);
		void set_texture(gs_texture_t* v);
		void set_sampler(std::shared_ptr<gs::sampler> v);
		void set_sampler(gs_sampler_state* v);

		private:
		gs_eparam_t*         m_param;
		gs_effect_param_info m_paramInfo;
	};

	class effect {
		public:
		effect();
		effect(std::string file);
		effect(std::string code, std::string name);
		virtual ~effect();

		gs_effect_t* get_object();

		size_t                      count_parameters();
		std::list<effect_parameter> get_parameters();
		effect_parameter            get_parameter(size_t idx);
		effect_parameter            get_parameter(std::string name);
		bool                        has_parameter(std::string name);
		bool                        has_parameter(std::string name, effect_parameter::type type);

		protected:
		gs_effect_t* m_effect;
	};
} // namespace gs
