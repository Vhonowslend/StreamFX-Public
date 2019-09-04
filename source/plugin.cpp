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
#include "filters/filter-blur.hpp"
#include "filters/filter-color-grade.hpp"
#include "filters/filter-displacement.hpp"
#include "filters/filter-dynamic-mask.hpp"
#include "filters/filter-sdf-effects.hpp"
#include "filters/filter-shader.hpp"
#include "filters/filter-transform.hpp"
#include "sources/source-mirror.hpp"
#include "sources/source-shader.hpp"

std::list<std::function<void()>> initializer_functions;
std::list<std::function<void()>> finalizer_functions;

MODULE_EXPORT bool obs_module_load(void)
{
	// Previously through P_INITIALIZER.
	initializer_functions.push_back([] { filter::blur::blur_factory::initialize(); });
	finalizer_functions.push_back([] { filter::blur::blur_factory::finalize(); });
	initializer_functions.push_back([] { filter::color_grade::color_grade_factory::initialize(); });
	finalizer_functions.push_back([] { filter::color_grade::color_grade_factory::finalize(); });
	initializer_functions.push_back([] { filter::displacement::displacement_factory::initialize(); });
	finalizer_functions.push_back([] { filter::displacement::displacement_factory::finalize(); });
	initializer_functions.push_back([] { filter::dynamic_mask::dynamic_mask_factory::initialize(); });
	finalizer_functions.push_back([] { filter::dynamic_mask::dynamic_mask_factory::finalize(); });
	initializer_functions.push_back([] { filter::sdf_effects::sdf_effects_factory::initialize(); });
	finalizer_functions.push_back([] { filter::sdf_effects::sdf_effects_factory::finalize(); });
	initializer_functions.push_back([] { filter::shader::shader_factory::initialize(); });
	finalizer_functions.push_back([] { filter::shader::shader_factory::finalize(); });
	initializer_functions.push_back([] { filter::transform::transform_factory::initialize(); });
	finalizer_functions.push_back([] { filter::transform::transform_factory::finalize(); });
	initializer_functions.push_back([] { source::mirror::mirror_factory::initialize(); });
	finalizer_functions.push_back([] { source::mirror::mirror_factory::finalize(); });
	initializer_functions.push_back([] { source::shader::shader_factory::initialize(); });
	finalizer_functions.push_back([] { source::shader::shader_factory::finalize(); });

	P_LOG_INFO("Loading Version %u.%u.%u (Build %u)", PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR,
			   PROJECT_VERSION_PATCH, PROJECT_VERSION_TWEAK);
	obs::source_tracker::initialize();
	for (auto func : initializer_functions) {
		func();
	}
	return true;
}

MODULE_EXPORT void obs_module_unload(void)
{
	P_LOG_INFO("Unloading Version %u.%u.%u (Build %u)", PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR,
			   PROJECT_VERSION_PATCH, PROJECT_VERSION_TWEAK);
	for (auto func : finalizer_functions) {
		func();
	}
	obs::source_tracker::finalize();
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
