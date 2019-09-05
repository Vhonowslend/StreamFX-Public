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
#include "filters/filter-blur.hpp"
#include "filters/filter-color-grade.hpp"
#include "filters/filter-displacement.hpp"
#include "filters/filter-dynamic-mask.hpp"
#include "filters/filter-sdf-effects.hpp"
#include "filters/filter-shader.hpp"
#include "filters/filter-transform.hpp"
#include "obs/obs-source-tracker.hpp"
#include "sources/source-mirror.hpp"
#include "sources/source-shader.hpp"

MODULE_EXPORT bool obs_module_load(void) try {
	P_LOG_INFO("Loading Version %u.%u.%u (Build %u)", PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR,
			   PROJECT_VERSION_PATCH, PROJECT_VERSION_TWEAK);

	// Initialize Source Tracker
	obs::source_tracker::initialize();

	// Initialize Filters
	filter::blur::blur_factory::initialize();
	filter::color_grade::color_grade_factory::initialize();
	filter::displacement::displacement_factory::initialize();
	filter::dynamic_mask::dynamic_mask_factory::initialize();
	filter::sdf_effects::sdf_effects_factory::initialize();
	filter::shader::shader_factory::initialize();
	filter::transform::transform_factory::initialize();

	// Initialize Sources
	source::mirror::mirror_factory::initialize();
	source::shader::shader_factory::initialize();

	return true;
} catch (...) {
	P_LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
	return false;
}

MODULE_EXPORT void obs_module_unload(void) try {
	P_LOG_INFO("Unloading Version %u.%u.%u (Build %u)", PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR,
			   PROJECT_VERSION_PATCH, PROJECT_VERSION_TWEAK);

	// Clean up Sources
	source::mirror::mirror_factory::finalize();
	source::shader::shader_factory::finalize();

	// Clean up Filters
	filter::blur::blur_factory::finalize();
	filter::color_grade::color_grade_factory::finalize();
	filter::displacement::displacement_factory::finalize();
	filter::dynamic_mask::dynamic_mask_factory::finalize();
	filter::sdf_effects::sdf_effects_factory::finalize();
	filter::shader::shader_factory::finalize();
	filter::transform::transform_factory::finalize();

	// Clean up Source Tracker
	obs::source_tracker::finalize();
} catch (...) {
	P_LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
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
