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
#include <memory>
#include <vector>
#include "obs/gs/gs-mipmapper.hpp"
#include "obs/gs/gs-rendertarget.hpp"
#include "obs/gs/gs-texture.hpp"
#include "obs/gs/gs-vertexbuffer.hpp"
#include "plugin.hpp"

namespace filter {
	namespace color_grade {
		class color_grade_factory {
			obs_source_info sourceInfo;

			public: // Singleton
			static void                                 initialize();
			static void                                 finalize();
			static std::shared_ptr<color_grade_factory> get();

			public:
			color_grade_factory();
			~color_grade_factory();
		};

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

		class color_grade_instance {
			bool          _active;
			obs_source_t* _self;

			std::shared_ptr<gs::effect> _effect;

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
			~color_grade_instance();
			color_grade_instance(obs_data_t*, obs_source_t*);

			uint32_t get_width();
			uint32_t get_height();

			void update(obs_data_t*);
			void activate();
			void deactivate();
			void video_tick(float);
			void video_render(gs_effect_t*);
		};
	} // namespace color_grade
} // namespace filter
