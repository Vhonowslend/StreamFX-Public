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

#include "gs-helper.h"

gs_effect_param* gs_effect_get_param(gs_effect_t* effect, const char* name) {
	gs_effect_param* p = gs_effect_get_param_by_name(effect, name);
	if (!p)
		P_LOG_ERROR("Failed to find parameter %s in effect.", name);
	return p;
}

bool gs_set_param_int(gs_effect_t* effect, const char* name, int value) {
	gs_effect_param* p = nullptr;
	if (nullptr != (p = gs_effect_get_param(effect, name))) {
		gs_effect_set_int(p, value);
		return true;
	}
	P_LOG_ERROR("Failed to set value %d for parameter %s in"
		" effect.", value, name);
	return false;
}

bool gs_set_param_float(gs_effect_t* effect, const char* name, float value) {
	gs_effect_param* p = nullptr;
	if (nullptr != (p = gs_effect_get_param(effect, name))) {
		gs_effect_set_float(p, value);
		return true;
	}
	P_LOG_ERROR("Failed to set value %f for parameter %s in"
		" effect.", value, name);
	return false;
}

bool gs_set_param_float2(gs_effect_t* effect, const char* name, vec2* value) {
	gs_effect_param* p = nullptr;
	if (nullptr != (p = gs_effect_get_param(effect, name))) {
		gs_effect_set_vec2(p, value);
		return true;
	}
	P_LOG_ERROR("Failed to set value {%f,%f} for parameter %s"
		" in effect.", value->x, value->y, name);
	return false;
}

bool gs_set_param_float3(gs_effect_t* effect, const char* name, vec3* value) {
	gs_effect_param* p = nullptr;
	if (nullptr != (p = gs_effect_get_param(effect, name))) {
		gs_effect_set_vec3(p, value);
		return true;
	}
	P_LOG_ERROR("Failed to set value {%f,%f,%f} for parameter"
		"%s in effect.", value->x, value->y, value->z, name);
	return false;
}

bool gs_set_param_float4(gs_effect_t* effect, const char* name, vec4* value) {
	gs_effect_param* p = nullptr;
	if (nullptr != (p = gs_effect_get_param(effect, name))) {
		gs_effect_set_vec4(p, value);
		return true;
	}
	P_LOG_ERROR("Failed to set value {%f,%f,%f,%f} for"
		" parameter %s in effect.", value->x, value->y, value->z,
		value->w, name);
	return false;
}

bool gs_set_param_texture(gs_effect_t* effect, const char* name, gs_texture_t* value) {
	gs_effect_param* p = nullptr;
	if (nullptr != (p = gs_effect_get_param(effect, name))) {
		gs_effect_set_texture(p, value);
		return true;
	}
	P_LOG_ERROR("Failed to set texture for"
		" parameter %s in effect.", name);
	return false;
}
