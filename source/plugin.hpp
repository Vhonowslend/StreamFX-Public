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
#include "strings.hpp"
#include "version.hpp"
#include "util-threadpool.hpp"

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

// Logging
#define LOG_(level, ...) blog(level, "[" PLUGIN_NAME "] " __VA_ARGS__)
#define LOG_ERROR(...) LOG_(LOG_ERROR, __VA_ARGS__)
#define LOG_WARNING(...) LOG_(LOG_WARNING, __VA_ARGS__)
#define LOG_INFO(...) LOG_(LOG_INFO, __VA_ARGS__)
#define LOG_DEBUG(...) LOG_(LOG_DEBUG, __VA_ARGS__)

#ifndef __FUNCTION_NAME__
#ifdef WIN32 // WINDOWS
#define __FUNCTION_NAME__ __FUNCTION__
#else // *NIX
#define __FUNCTION_NAME__ __func__
#endif
#endif

// Threadpool
std::shared_ptr<util::threadpool> get_global_threadpool();
