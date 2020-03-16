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
#include <stdexcept>

#include "obs/obs-source-tracker.hpp"

#include "encoders/ffmpeg-encoder.hpp"

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
#ifdef ENABLE_FILTER_SDF_EFFECTS
#include "filters/filter-sdf-effects.hpp"
#endif
#ifdef ENABLE_FILTER_SHADER
//#include "filters/filter-shader.hpp"
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
//#include "transitions/source-shader.hpp"
#endif

static std::shared_ptr<util::threadpool> global_threadpool;

MODULE_EXPORT bool obs_module_load(void)
try {
	LOG_INFO("Loading Version %s", STREAMFX_VERSION_STRING);

	global_threadpool = std::make_shared<util::threadpool>();

	// Initialize Source Tracker
	obs::source_tracker::initialize();

// Encoders
#ifdef ENABLE_ENCODER_FFMPEG
	encoder::ffmpeg::ffmpeg_manager::initialize();
#endif

// Filters
#ifdef ENABLE_FILTER_BLUR
	filter::blur::blur_factory::initialize();
#endif
#ifdef ENABLE_FILTER_COLOR_GRADE
	filter::color_grade::color_grade_factory::initialize();
#endif
#ifdef ENABLE_FILTER_DISPLACEMENT
	filter::displacement::displacement_factory::initialize();
#endif
#ifdef ENABLE_FILTER_DYNAMIC_MASK
	filter::dynamic_mask::dynamic_mask_factory::initialize();
#endif
#ifdef ENABLE_FILTER_SDF_EFFECTS
	filter::sdf_effects::sdf_effects_factory::initialize();
#endif
#ifdef ENABLE_FILTER_SHADER
//filter::shader::shader_factory::initialize();
#endif
#ifdef ENABLE_FILTER_TRANSFORM
	filter::transform::transform_factory::initialize();
#endif

// Sources
#ifdef ENABLE_SOURCE_MIRROR
	source::mirror::mirror_factory::initialize();
#endif
#ifdef ENABLE_SOURCE_SHADER
	source::shader::shader_factory::initialize();
#endif

// Transitions
#ifdef ENABLE_TRANSITION_SHADER
//transition::shader::shader_factory::initialize();
#endif

	return true;
} catch (...) {
	LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
	return false;
}

MODULE_EXPORT void obs_module_unload(void)
try {
	LOG_INFO("Unloading Version %s", STREAMFX_VERSION_STRING);

	// Transitions
#ifdef ENABLE_TRANSITION_SHADER
//transition::shader::shader_factory::finalize();
#endif

// Sources
#ifdef ENABLE_SOURCE_MIRROR
	source::mirror::mirror_factory::finalize();
#endif
#ifdef ENABLE_SOURCE_SHADER
	source::shader::shader_factory::finalize();
#endif

// Filters
#ifdef ENABLE_FILTER_BLUR
	filter::blur::blur_factory::finalize();
#endif
#ifdef ENABLE_FILTER_COLOR_GRADE
	filter::color_grade::color_grade_factory::finalize();
#endif
#ifdef ENABLE_FILTER_DISPLACEMENT
	filter::displacement::displacement_factory::finalize();
#endif
#ifdef ENABLE_FILTER_DYNAMIC_MASK
	filter::dynamic_mask::dynamic_mask_factory::finalize();
#endif
#ifdef ENABLE_FILTER_SDF_EFFECTS
	filter::sdf_effects::sdf_effects_factory::finalize();
#endif
#ifdef ENABLE_FILTER_SHADER
//filter::shader::shader_factory::finalize();
#endif
#ifdef ENABLE_FILTER_TRANSFORM
	filter::transform::transform_factory::finalize();
#endif

// Encoders
#ifdef ENABLE_ENCODER_FFMPEG
	encoder::ffmpeg::ffmpeg_manager::finalize();
#endif

	// Finalize Source Tracker
	obs::source_tracker::finalize();

	global_threadpool.reset();
} catch (...) {
	LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

#ifdef _WIN32
// Windows Only
extern "C" {
#include <windows.h>
}

BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID)
{
	return TRUE;
}
#endif

std::shared_ptr<util::threadpool> get_global_threadpool()
{
	return global_threadpool;
}
