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
#include "plugin.h"

// OBS
#pragma warning(push)
#pragma warning(disable : 4201)
#include <graphics/graphics.h>
#pragma warning(pop)

gs_effect_param* gs_effect_get_param(gs_effect_t* effect, const char* name);
bool             gs_set_param_int(gs_effect_t* effect, const char* name, int value);
bool             gs_set_param_float(gs_effect_t* effect, const char* name, float value);
bool             gs_set_param_float2(gs_effect_t* effect, const char* name, vec2* value);
bool             gs_set_param_float3(gs_effect_t* effect, const char* name, vec3* value);
bool             gs_set_param_float4(gs_effect_t* effect, const char* name, vec4* value);
bool             gs_set_param_texture(gs_effect_t* effect, const char* name, gs_texture_t* value);
