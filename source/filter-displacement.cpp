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

#include "filter-displacement.hpp"
#include <sys/stat.h>
#include "strings.hpp"

// Initializer & Finalizer
static filter::DisplacementAddon* filterDisplacementInstance;
INITIALIZER(FilterDisplacementInit)
{
	initializerFunctions.push_back([] { filterDisplacementInstance = new filter::DisplacementAddon(); });
	finalizerFunctions.push_back([] { delete filterDisplacementInstance; });
}

filter::DisplacementAddon::DisplacementAddon()
{
	memset(&sourceInfo, 0, sizeof(obs_source_info));
	sourceInfo.id             = "obs-stream-effects-filter-displacement";
	sourceInfo.type           = OBS_SOURCE_TYPE_FILTER;
	sourceInfo.output_flags   = OBS_SOURCE_VIDEO;
	sourceInfo.get_name       = get_name;
	sourceInfo.get_defaults   = get_defaults;
	sourceInfo.get_properties = get_properties;

	sourceInfo.create       = create;
	sourceInfo.destroy      = destroy;
	sourceInfo.update       = update;
	sourceInfo.activate     = activate;
	sourceInfo.deactivate   = deactivate;
	sourceInfo.show         = show;
	sourceInfo.hide         = hide;
	sourceInfo.video_tick   = video_tick;
	sourceInfo.video_render = video_render;

	obs_register_source(&sourceInfo);
}

filter::DisplacementAddon::~DisplacementAddon() {}

const char* filter::DisplacementAddon::get_name(void*)
{
	return P_TRANSLATE(S_FILTER_DISPLACEMENT);
}

void* filter::DisplacementAddon::create(obs_data_t* data, obs_source_t* source)
{
	return new Displacement(data, source);
}

void filter::DisplacementAddon::destroy(void* ptr)
{
	delete reinterpret_cast<Displacement*>(ptr);
}

uint32_t filter::DisplacementAddon::get_width(void* ptr)
{
	return reinterpret_cast<Displacement*>(ptr)->get_width();
}

uint32_t filter::DisplacementAddon::get_height(void* ptr)
{
	return reinterpret_cast<Displacement*>(ptr)->get_height();
}

void filter::DisplacementAddon::get_defaults(obs_data_t* data)
{
	char* disp = obs_module_file("filter-displacement/neutral.png");
	obs_data_set_default_string(data, S_FILTER_DISPLACEMENT_FILE, disp);
	obs_data_set_default_double(data, S_FILTER_DISPLACEMENT_RATIO, 0);
	obs_data_set_default_double(data, S_FILTER_DISPLACEMENT_SCALE, 0);
	bfree(disp);
}

obs_properties_t* filter::DisplacementAddon::get_properties(void* ptr)
{
	obs_properties_t* pr = obs_properties_create();

	std::string path = "";
	if (ptr)
		path = reinterpret_cast<Displacement*>(ptr)->get_file();

	obs_properties_add_path(pr, S_FILTER_DISPLACEMENT_FILE, P_TRANSLATE(S_FILTER_DISPLACEMENT_FILE),
							obs_path_type::OBS_PATH_FILE, P_TRANSLATE(S_FILTER_DISPLACEMENT_FILE_TYPES), path.c_str());
	obs_properties_add_float_slider(pr, S_FILTER_DISPLACEMENT_RATIO, P_TRANSLATE(S_FILTER_DISPLACEMENT_RATIO), 0, 1,
									0.01);
	obs_properties_add_float_slider(pr, S_FILTER_DISPLACEMENT_SCALE, P_TRANSLATE(S_FILTER_DISPLACEMENT_SCALE), -1000,
									1000, 0.01);
	return pr;
}

void filter::DisplacementAddon::update(void* ptr, obs_data_t* data)
{
	reinterpret_cast<Displacement*>(ptr)->update(data);
}

void filter::DisplacementAddon::activate(void* ptr)
{
	reinterpret_cast<Displacement*>(ptr)->activate();
}

void filter::DisplacementAddon::deactivate(void* ptr)
{
	reinterpret_cast<Displacement*>(ptr)->deactivate();
}

void filter::DisplacementAddon::show(void* ptr)
{
	reinterpret_cast<Displacement*>(ptr)->show();
}

void filter::DisplacementAddon::hide(void* ptr)
{
	reinterpret_cast<Displacement*>(ptr)->hide();
}

void filter::DisplacementAddon::video_tick(void* ptr, float time)
{
	reinterpret_cast<Displacement*>(ptr)->video_tick(time);
}

void filter::DisplacementAddon::video_render(void* ptr, gs_effect_t* effect)
{
	reinterpret_cast<Displacement*>(ptr)->video_render(effect);
}

filter::Displacement::Displacement(obs_data_t* data, obs_source_t* context)
	: m_self(context), m_active(true), m_effect(nullptr), m_distance(0), m_timer(0)
{
	this->m_displacement_map.texture      = nullptr;
	this->m_displacement_map.createTime   = 0;
	this->m_displacement_map.modifiedTime = 0;
	this->m_displacement_map.size         = 0;

	char* effectFile = obs_module_file("effects/displace.effect");
	try {
		m_effect = std::make_shared<gs::effect>(effectFile);
	} catch (...) {
		P_LOG_ERROR("<Displacement Filter:%s> Failed to load displacement effect.", obs_source_get_name(m_self));
	}
	bfree(effectFile);

	update(data);
}

filter::Displacement::~Displacement()
{
	m_effect.reset();

	obs_enter_graphics();
	gs_texture_destroy(m_displacement_map.texture);
	obs_leave_graphics();
}

void filter::Displacement::update(obs_data_t* data)
{
	updateDisplacementMap(obs_data_get_string(data, S_FILTER_DISPLACEMENT_FILE));

	m_distance = float_t(obs_data_get_double(data, S_FILTER_DISPLACEMENT_RATIO));
	vec2_set(&m_displacement_scale, float_t(obs_data_get_double(data, S_FILTER_DISPLACEMENT_SCALE)),
			 float_t(obs_data_get_double(data, S_FILTER_DISPLACEMENT_SCALE)));
}

uint32_t filter::Displacement::get_width()
{
	return 0;
}

uint32_t filter::Displacement::get_height()
{
	return 0;
}

void filter::Displacement::activate() {}

void filter::Displacement::deactivate() {}

void filter::Displacement::show() {}

void filter::Displacement::hide() {}

void filter::Displacement::video_tick(float time)
{
	m_timer += time;
	if (m_timer >= 1.0) {
		m_timer -= 1.0;
		updateDisplacementMap(m_displacement_map.file);
	}
}

float interp(float a, float b, float v)
{
	return (a * (1.0f - v)) + (b * v);
}

void filter::Displacement::video_render(gs_effect_t*)
{
	obs_source_t* parent = obs_filter_get_parent(m_self);
	obs_source_t* target = obs_filter_get_target(m_self);
	uint32_t      baseW = obs_source_get_base_width(target), baseH = obs_source_get_base_height(target);

	// Skip rendering if our target, parent or context is not valid.
	if (!parent || !target || !baseW || !baseH || !m_displacement_map.texture) {
		obs_source_skip_video_filter(m_self);
		return;
	}

	if (!obs_source_process_filter_begin(m_self, GS_RGBA, OBS_ALLOW_DIRECT_RENDERING))
		return;

	gs_eparam_t* param;

	vec2 texelScale;
	vec2_set(&texelScale, interp((1.0f / baseW), 1.0f, m_distance), interp((1.0f / baseH), 1.0f, m_distance));

	if (m_effect->has_parameter("texelScale")) {
		m_effect->get_parameter("texelScale").set_float2(texelScale);
	}
	if (m_effect->has_parameter("displacementScale")) {
		m_effect->get_parameter("displacmenetScale").set_float2(m_displacement_scale);
	}
	if (m_effect->has_parameter("displacementMap")) {
		m_effect->get_parameter("displacementMap").set_texture(m_displacement_map.texture);
	}

	obs_source_process_filter_end(m_self, m_effect->get_object(), baseW, baseH);
}

std::string filter::Displacement::get_file()
{
	return m_displacement_map.file;
}

void filter::Displacement::updateDisplacementMap(std::string file)
{
	bool shouldUpdateTexture = false;

	// Different File
	if (file != m_displacement_map.file) {
		m_displacement_map.file = file;
		shouldUpdateTexture     = true;
	} else { // Different Timestamps
		struct stat stats;
		if (os_stat(m_displacement_map.file.c_str(), &stats) != 0) {
			shouldUpdateTexture = shouldUpdateTexture || (m_displacement_map.createTime != stats.st_ctime)
								  || (m_displacement_map.modifiedTime != stats.st_mtime);
			m_displacement_map.createTime   = stats.st_ctime;
			m_displacement_map.modifiedTime = stats.st_mtime;
		}
	}

	if (shouldUpdateTexture) {
		obs_enter_graphics();
		if (m_displacement_map.texture) {
			gs_texture_destroy(m_displacement_map.texture);
			m_displacement_map.texture = nullptr;
		}
		if (os_file_exists(file.c_str()))
			m_displacement_map.texture = gs_texture_create_from_file(m_displacement_map.file.c_str());
		obs_leave_graphics();
	}
}
