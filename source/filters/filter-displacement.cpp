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

#define ST "Filter.Displacement"
#define ST_FILE "Filter.Displacement.File"
#define ST_FILE_TYPES "Filter.Displacement.File.Types"
#define ST_RATIO "Filter.Displacement.Ratio"
#define ST_SCALE "Filter.Displacement.Scale"

static std::shared_ptr<filter::displacement::displacement_factory> factory_instance = nullptr;

void filter::displacement::displacement_factory::initialize()
{
	factory_instance = std::make_shared<filter::displacement::displacement_factory>();
}

void filter::displacement::displacement_factory::finalize()
{
	factory_instance.reset();
}

std::shared_ptr<filter::displacement::displacement_factory> filter::displacement::displacement_factory::get()
{
	return factory_instance;
}

filter::displacement::displacement_factory::displacement_factory()
{
	memset(&_source_info, 0, sizeof(obs_source_info));
	_source_info.id             = "obs-stream-effects-filter-displacement";
	_source_info.type           = OBS_SOURCE_TYPE_FILTER;
	_source_info.output_flags   = OBS_SOURCE_VIDEO;
	_source_info.get_name       = get_name;
	_source_info.get_defaults   = get_defaults;
	_source_info.get_properties = get_properties;

	_source_info.create       = create;
	_source_info.destroy      = destroy;
	_source_info.update       = update;
	_source_info.activate     = activate;
	_source_info.deactivate   = deactivate;
	_source_info.show         = show;
	_source_info.hide         = hide;
	_source_info.video_tick   = video_tick;
	_source_info.video_render = video_render;

	obs_register_source(&_source_info);
}

filter::displacement::displacement_factory::~displacement_factory() {}

const char* filter::displacement::displacement_factory::get_name(void*)
{
	return D_TRANSLATE(ST);
}

void* filter::displacement::displacement_factory::create(obs_data_t* data, obs_source_t* source)
{
	return new displacement_instance(data, source);
}

void filter::displacement::displacement_factory::destroy(void* ptr)
{
	delete reinterpret_cast<displacement_instance*>(ptr);
}

uint32_t filter::displacement::displacement_factory::get_width(void* ptr)
{
	return reinterpret_cast<displacement_instance*>(ptr)->get_width();
}

uint32_t filter::displacement::displacement_factory::get_height(void* ptr)
{
	return reinterpret_cast<displacement_instance*>(ptr)->get_height();
}

void filter::displacement::displacement_factory::get_defaults(obs_data_t* data)
{
	char* disp = obs_module_file("filter-displacement/neutral.png");
	obs_data_set_default_string(data, ST_FILE, disp);
	obs_data_set_default_double(data, ST_RATIO, 0);
	obs_data_set_default_double(data, ST_SCALE, 0);
	bfree(disp);
}

obs_properties_t* filter::displacement::displacement_factory::get_properties(void* ptr)
{
	obs_properties_t* pr = obs_properties_create();

	std::string path = "";
	if (ptr)
		path = reinterpret_cast<displacement_instance*>(ptr)->get_file();

	obs_properties_add_path(pr, ST_FILE, D_TRANSLATE(ST_FILE), obs_path_type::OBS_PATH_FILE, D_TRANSLATE(ST_FILE_TYPES),
							path.c_str());
	obs_properties_add_float_slider(pr, ST_RATIO, D_TRANSLATE(ST_RATIO), 0, 1, 0.01);
	obs_properties_add_float_slider(pr, ST_SCALE, D_TRANSLATE(ST_SCALE), -1000, 1000, 0.01);
	return pr;
}

void filter::displacement::displacement_factory::update(void* ptr, obs_data_t* data)
{
	reinterpret_cast<displacement_instance*>(ptr)->update(data);
}

void filter::displacement::displacement_factory::activate(void* ptr)
{
	reinterpret_cast<displacement_instance*>(ptr)->activate();
}

void filter::displacement::displacement_factory::deactivate(void* ptr)
{
	reinterpret_cast<displacement_instance*>(ptr)->deactivate();
}

void filter::displacement::displacement_factory::show(void* ptr)
{
	reinterpret_cast<displacement_instance*>(ptr)->show();
}

void filter::displacement::displacement_factory::hide(void* ptr)
{
	reinterpret_cast<displacement_instance*>(ptr)->hide();
}

void filter::displacement::displacement_factory::video_tick(void* ptr, float time)
{
	reinterpret_cast<displacement_instance*>(ptr)->video_tick(time);
}

void filter::displacement::displacement_factory::video_render(void* ptr, gs_effect_t* effect)
{
	reinterpret_cast<displacement_instance*>(ptr)->video_render(effect);
}

void filter::displacement::displacement_instance::validate_file_texture(std::string file)
{
	bool do_update = false;

	// Don't allow empty file names.
	if (file.length() == 0) {
		return;
	}

	// File name different
	if (file != _file_name) {
		do_update  = true;
		_file_name = file;
	}

	// Timestamp verification
	struct stat stats;
	if (os_stat(_file_name.c_str(), &stats) != 0) {
		do_update           = do_update || (stats.st_ctime != _file_create_time);
		do_update           = do_update || (stats.st_mtime != _file_modified_time);
		do_update           = do_update || (static_cast<size_t>(stats.st_size) != _file_size);
		_file_create_time   = stats.st_ctime;
		_file_modified_time = stats.st_mtime;
		_file_size          = static_cast<size_t>(stats.st_size);
	}

	do_update = !_file_texture || do_update;

	if (do_update) {
		try {
			_file_texture = std::make_shared<gs::texture>(_file_name);
		} catch (...) {
		}
	}
}

filter::displacement::displacement_instance::displacement_instance(obs_data_t* data, obs_source_t* context)
	: _self(context), _timer(0), _effect(nullptr), _distance(0), _file_create_time(0), _file_modified_time(0),
	  _file_size(0)
{
	char* effectFile = obs_module_file("effects/displace.effect");
	if (effectFile) {
		try {
			_effect = gs::effect::create(effectFile);
		} catch (...) {
			P_LOG_ERROR("<Displacement Filter:%s> Failed to load displacement effect.", obs_source_get_name(_self));
		}
		bfree(effectFile);
	}

	update(data);
}

filter::displacement::displacement_instance::~displacement_instance()
{
	_effect.reset();
	_file_texture.reset();
}

void filter::displacement::displacement_instance::update(obs_data_t* data)
{
	validate_file_texture(obs_data_get_string(data, ST_FILE));

	_distance = float_t(obs_data_get_double(data, ST_RATIO));
	vec2_set(&_displacement_scale, float_t(obs_data_get_double(data, ST_SCALE)),
			 float_t(obs_data_get_double(data, ST_SCALE)));
}

uint32_t filter::displacement::displacement_instance::get_width()
{
	return 0;
}

uint32_t filter::displacement::displacement_instance::get_height()
{
	return 0;
}

void filter::displacement::displacement_instance::activate() {}

void filter::displacement::displacement_instance::deactivate() {}

void filter::displacement::displacement_instance::show() {}

void filter::displacement::displacement_instance::hide() {}

void filter::displacement::displacement_instance::video_tick(float time)
{
	_timer += time;
	if (_timer >= 1.0f) {
		_timer -= 1.0f;
		validate_file_texture(_file_name);
	}
}

static float interp(float a, float b, float v)
{
	return (a * (1.0f - v)) + (b * v);
}

void filter::displacement::displacement_instance::video_render(gs_effect_t*)
{
	obs_source_t* parent = obs_filter_get_parent(_self);
	obs_source_t* target = obs_filter_get_target(_self);
	uint32_t      baseW = obs_source_get_base_width(target), baseH = obs_source_get_base_height(target);

	// Skip rendering if our target, parent or context is not valid.
	if (!parent || !target || !baseW || !baseH || !_file_texture) {
		obs_source_skip_video_filter(_self);
		return;
	}

	if (!obs_source_process_filter_begin(_self, GS_RGBA, OBS_ALLOW_DIRECT_RENDERING)) {
		obs_source_skip_video_filter(_self);
		return;
	}

	if (_effect->has_parameter("texelScale")) {
		_effect->get_parameter("texelScale")
			->set_float2(interp((1.0f / baseW), 1.0f, _distance), interp((1.0f / baseH), 1.0f, _distance));
	}
	if (_effect->has_parameter("displacementScale")) {
		_effect->get_parameter("displacementScale")->set_float2(_displacement_scale);
	}
	if (_effect->has_parameter("displacementMap")) {
		_effect->get_parameter("displacementMap")->set_texture(_file_texture);
	}

	obs_source_process_filter_end(_self, _effect->get_object(), baseW, baseH);
}

std::string filter::displacement::displacement_instance::get_file()
{
	return _file_name;
}
