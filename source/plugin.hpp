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
#include <functional>
#include <list>

// OBS
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include "obs-module.h"
#include "util/platform.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

// Plugin
#define PLUGIN_NAME "Stream Effects"
#include "version.hpp"

#define P_LOG(level, ...) blog(level, "[" PLUGIN_NAME "] " __VA_ARGS__);
#define P_LOG_ERROR(...) P_LOG(LOG_ERROR, __VA_ARGS__)
#define P_LOG_WARNING(...) P_LOG(LOG_WARNING, __VA_ARGS__)
#define P_LOG_INFO(...) P_LOG(LOG_INFO, __VA_ARGS__)
#define P_LOG_DEBUG(...) P_LOG(LOG_DEBUG, __VA_ARGS__)

// Initializer & Finalizer
extern std::list<std::function<void()>> initializerFunctions;
extern std::list<std::function<void()>> finalizerFunctions;
