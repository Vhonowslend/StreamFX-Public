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
#include "gs-texture.h"
#include "gs-sampler.h"
#include <inttypes.h>
#include <memory>
#include <string>
#include <list>
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/graphics/graphics.h>
	#include <libobs/graphics/matrix4.h>
	#include <libobs/graphics/vec4.h>
	#include <libobs/graphics/vec3.h>
	#include <libobs/graphics/vec2.h>
	#pragma warning( pop )
}

namespace GS {
	class EffectParameter {
		public:
		enum class Type : uint8_t {
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
		EffectParameter(gs_eparam_t* param);
		
		std::string GetName();
		Type GetType();

		void SetBoolean(bool v);
		void SetBooleanArray(bool v[], size_t sz);
		void SetFloat(float_t x);
		void SetFloat2(vec2& v);
		void SetFloat2(float_t x, float_t y);
		void SetFloat3(vec3& v);
		void SetFloat3(float_t x, float_t y, float_t z);
		void SetFloat4(vec4& v);
		void SetFloat4(float_t x, float_t y, float_t z, float_t w);
		void SetFloatArray(float_t v[], size_t sz);
		void SetInteger(int32_t x);
		void SetInteger2(int32_t x, int32_t y);
		void SetInteger3(int32_t x, int32_t y, int32_t z);
		void SetInteger4(int32_t x, int32_t y, int32_t z, int32_t w);
		void SetIntegerArray(int32_t v[], size_t sz);
		void SetMatrix(matrix4& v);
		void SetTexture(std::shared_ptr<GS::Texture> v);
		void SetTexture(gs_texture_t* v);
		void SetSampler(std::shared_ptr<GS::Sampler> v);
		void SetSampler(gs_sampler_state* v);

		private:
		gs_eparam_t* m_param;
		gs_effect_param_info m_paramInfo;
	};

	class Effect {
		public:
		Effect();
		Effect(std::string file);
		Effect(std::string code, std::string name);
		virtual ~Effect();

		gs_effect_t* GetObject();

		size_t CountParameters();
		std::list<EffectParameter> GetParameters();
		EffectParameter GetParameter(size_t idx);
		EffectParameter GetParameter(std::string name);
		bool HasParameter(std::string name);
		bool HasParameter(std::string name, EffectParameter::Type type);

		protected:
		gs_effect_t* m_effect;
	};
}
