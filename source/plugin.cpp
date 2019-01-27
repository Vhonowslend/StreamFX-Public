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
#include "filter-blur.hpp"
#include "filter-displacement.hpp"
#include "filter-shape.hpp"
#include "filter-transform.hpp"
#include "obs-source-tracker.hpp"

std::list<std::function<void()>> initializerFunctions;
std::list<std::function<void()>> finalizerFunctions;

MODULE_EXPORT bool obs_module_load(void)
{
	P_LOG_INFO("Loading Version %u.%u.%u (Build %u)", PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR,
			   PROJECT_VERSION_PATCH, PROJECT_VERSION_TWEAK);
	obs::source_tracker::initialize();
	for (auto func : initializerFunctions) {
		func();
	}
	return true;
}

MODULE_EXPORT void obs_module_unload(void)
{
	P_LOG_INFO("Unloading Version %u.%u.%u (Build %u)", PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR,
			   PROJECT_VERSION_PATCH, PROJECT_VERSION_TWEAK);
	for (auto func : finalizerFunctions) {
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
