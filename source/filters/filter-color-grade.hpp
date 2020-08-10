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
#include <vector>
#include "obs/gs/gs-mipmapper.hpp"
#include "obs/gs/gs-rendertarget.hpp"
#include "obs/gs/gs-texture.hpp"
#include "obs/gs/gs-vertexbuffer.hpp"
#include "obs/obs-source-factory.hpp"
#include "plugin.hpp"

namespace streamfx::filter::color_grade {
	enum class detection_mode {
		HSV,
		HSL,
		YUV_SDR,
	};

	enum class luma_mode {
		Linear,
		Exp,
		Exp2,
		Log,
		Log10,
	};

	class color_grade_instance : public obs::source_instance {
		gs::effect _effect;

		// Source
		std::unique_ptr<gs::rendertarget> _rt_source;
		std::shared_ptr<gs::texture>      _tex_source;
		bool                              _source_updated;

		// Grading
		std::unique_ptr<gs::rendertarget> _rt_grade;
		std::shared_ptr<gs::texture>      _tex_grade;
		bool                              _grade_updated;

		// Parameters
		vec4           _lift;
		vec4           _gamma;
		vec4           _gain;
		vec4           _offset;
		detection_mode _tint_detection;
		luma_mode      _tint_luma;
		float_t        _tint_exponent;
		vec3           _tint_low;
		vec3           _tint_mid;
		vec3           _tint_hig;
		vec4           _correction;

		public:
		color_grade_instance(obs_data_t* data, obs_source_t* self);
		virtual ~color_grade_instance();

		virtual void load(obs_data_t* data) override;
		virtual void migrate(obs_data_t* data, uint64_t version) override;
		virtual void update(obs_data_t* data) override;

		virtual void video_tick(float_t time) override;
		virtual void video_render(gs_effect_t* effect) override;
	};

	class color_grade_factory : public obs::source_factory<filter::color_grade::color_grade_factory,
														   filter::color_grade::color_grade_instance> {
		public:
		color_grade_factory();
		virtual ~color_grade_factory();

		virtual const char* get_name() override;

		virtual void get_defaults2(obs_data_t* data) override;

		virtual obs_properties_t* get_properties2(color_grade_instance* data) override;

		public: // Singleton
		static void initialize();

		static void finalize();

		static std::shared_ptr<color_grade_factory> get();
	};
} // namespace streamfx::filter::color_grade
