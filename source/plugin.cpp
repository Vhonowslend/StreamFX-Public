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

#include "plugin.hpp"
#include <fstream>
#include <stdexcept>
#include "configuration.hpp"
#include "obs/obs-source-tracker.hpp"

#ifdef ENABLE_ENCODER_FFMPEG
#include "encoders/encoder-ffmpeg.hpp"
#endif

#ifdef ENABLE_FILTER_BLUR
#include "filters/filter-blur.hpp"
#endif
#ifdef ENABLE_FILTER_COLOR_GRADE
#include "filters/filter-color-grade.hpp"
#endif
#ifdef ENABLE_FILTER_DISPLACEMENT
#include "filters/filter-displacement.hpp"
#endif
#ifdef ENABLE_FILTER_DYNAMIC_MASK
#include "filters/filter-dynamic-mask.hpp"
#endif
#ifdef ENABLE_FILTER_NVIDIA_FACE_TRACKING
#include "filters/filter-nv-face-tracking.hpp"
#endif
#ifdef ENABLE_FILTER_SDF_EFFECTS
#include "filters/filter-sdf-effects.hpp"
#endif
#ifdef ENABLE_FILTER_SHADER
#include "filters/filter-shader.hpp"
#endif
#ifdef ENABLE_FILTER_TRANSFORM
#include "filters/filter-transform.hpp"
#endif

#ifdef ENABLE_SOURCE_MIRROR
#include "sources/source-mirror.hpp"
#endif
#ifdef ENABLE_SOURCE_SHADER
#include "sources/source-shader.hpp"
#endif

#ifdef ENABLE_TRANSITION_SHADER
#include "transitions/transition-shader.hpp"
#endif

#ifdef ENABLE_FRONTEND
#include "ui/ui.hpp"
#endif

static std::shared_ptr<util::threadpool>  _threadpool;
static std::shared_ptr<gs::vertex_buffer> _gs_fstri_vb;

MODULE_EXPORT bool obs_module_load(void)
try {
	LOG_INFO("Loading Version %s", STREAMFX_VERSION_STRING);

	// Initialize global configuration.
	streamfx::configuration::initialize();

	// Initialize global Thread Pool.
	_threadpool = std::make_shared<util::threadpool>();

	// Initialize Source Tracker
	obs::source_tracker::initialize();

	// GS Stuff
	{
		_gs_fstri_vb = std::make_shared<gs::vertex_buffer>(3, 1);
		{
			auto vtx = _gs_fstri_vb->at(0);
			vec3_set(vtx.position, 0, 0, 0);
			vec4_set(vtx.uv[0], 0, 0, 0, 0);
		}
		{
			auto vtx = _gs_fstri_vb->at(1);
			vec3_set(vtx.position, 2, 0, 0);
			vec4_set(vtx.uv[0], 2, 0, 0, 0);
		}
		{
			auto vtx = _gs_fstri_vb->at(2);
			vec3_set(vtx.position, 0, 2, 0);
			vec4_set(vtx.uv[0], 0, 2, 0, 0);
		}
		_gs_fstri_vb->update();
	}

	// Encoders
	{
#ifdef ENABLE_ENCODER_FFMPEG
		using namespace streamfx::encoder::ffmpeg;
		ffmpeg_manager::initialize();
#endif
	}

	// Filters
	{
		using namespace streamfx::filter;
#ifdef ENABLE_FILTER_BLUR
		blur::blur_factory::initialize();
#endif
#ifdef ENABLE_FILTER_COLOR_GRADE
		color_grade::color_grade_factory::initialize();
#endif
#ifdef ENABLE_FILTER_DISPLACEMENT
		displacement::displacement_factory::initialize();
#endif
#ifdef ENABLE_FILTER_DYNAMIC_MASK
		dynamic_mask::dynamic_mask_factory::initialize();
#endif
#ifdef ENABLE_FILTER_NVIDIA_FACE_TRACKING
		streamfx::filter::nvidia::face_tracking_factory::initialize();
#endif
#ifdef ENABLE_FILTER_SDF_EFFECTS
		sdf_effects::sdf_effects_factory::initialize();
#endif
#ifdef ENABLE_FILTER_SHADER
		shader::shader_factory::initialize();
#endif
#ifdef ENABLE_FILTER_TRANSFORM
		transform::transform_factory::initialize();
#endif
	}

	// Sources
	{
		using namespace streamfx::source;
#ifdef ENABLE_SOURCE_MIRROR
		mirror::mirror_factory::initialize();
#endif
#ifdef ENABLE_SOURCE_SHADER
		shader::shader_factory::initialize();
#endif
	}

	// Transitions
	{
		using namespace streamfx::transition;
#ifdef ENABLE_TRANSITION_SHADER
		shader::shader_factory::initialize();
#endif
	}

// Frontend
#ifdef ENABLE_FRONTEND
	streamfx::ui::handler::initialize();
#endif

	LOG_INFO("Loaded Version %s", STREAMFX_VERSION_STRING);
	return true;
} catch (...) {
	LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
	return false;
}

MODULE_EXPORT void obs_module_unload(void)
try {
	LOG_INFO("Unloading Version %s", STREAMFX_VERSION_STRING);

	// Frontend
#ifdef ENABLE_FRONTEND
	streamfx::ui::handler::finalize();
#endif

	// Transitions
	{
		using namespace streamfx::transition;
#ifdef ENABLE_TRANSITION_SHADER
		shader::shader_factory::finalize();
#endif
	}

	// Sources
	{
		using namespace streamfx::source;
#ifdef ENABLE_SOURCE_MIRROR
		mirror::mirror_factory::finalize();
#endif
#ifdef ENABLE_SOURCE_SHADER
		shader::shader_factory::finalize();
#endif
	}

	// Filters
	{
		using namespace streamfx::filter;
#ifdef ENABLE_FILTER_BLUR
		blur::blur_factory::finalize();
#endif
#ifdef ENABLE_FILTER_COLOR_GRADE
		color_grade::color_grade_factory::finalize();
#endif
#ifdef ENABLE_FILTER_DISPLACEMENT
		displacement::displacement_factory::finalize();
#endif
#ifdef ENABLE_FILTER_DYNAMIC_MASK
		dynamic_mask::dynamic_mask_factory::finalize();
#endif
#ifdef ENABLE_FILTER_NVIDIA_FACE_TRACKING
		streamfx::filter::nvidia::face_tracking_factory::finalize();
#endif
#ifdef ENABLE_FILTER_SDF_EFFECTS
		sdf_effects::sdf_effects_factory::finalize();
#endif
#ifdef ENABLE_FILTER_SHADER
		shader::shader_factory::finalize();
#endif
#ifdef ENABLE_FILTER_TRANSFORM
		transform::transform_factory::finalize();
#endif
	}

	// Encoders
	{
#ifdef ENABLE_ENCODER_FFMPEG
		using namespace streamfx::encoder::ffmpeg;
		ffmpeg_manager::finalize();
#endif
	}

	// GS Stuff
	{
		_gs_fstri_vb.reset();
	}

	// Finalize Source Tracker
	obs::source_tracker::finalize();

	// Finalize Thread Pool
	_threadpool.reset();

	// Finalize Configuration
	streamfx::configuration::finalize();

	LOG_INFO("Unloaded Version %s", STREAMFX_VERSION_STRING);
} catch (...) {
	LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

std::shared_ptr<util::threadpool> streamfx::threadpool()
{
	return _threadpool;
}

void streamfx::gs_draw_fullscreen_tri()
{
	gs_load_vertexbuffer(_gs_fstri_vb->update(false));
	gs_draw(GS_TRIS, 0, 3); //_gs_fstri_vb->size());
}
