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
#include "strings.hpp"
#include <stdexcept>
#include <sys/stat.h>

#define ST "Filter.Displacement"
#define ST_FILE "Filter.Displacement.File"
#define ST_SCALE "Filter.Displacement.Scale"
#define ST_SCALE_TYPE "Filter.Displacement.Scale.Type"

using namespace filter;

displacement::displacement_instance::displacement_instance(obs_data_t* data, obs_source_t* context)
	: obs::source_instance(data, context)
{
	std::string effect = "";
	{
		char* buf = obs_module_file("effects/displace.effect");
		effect    = buf;
		bfree(buf);
	}

	_effect = gs::effect::create(effect);

	update(data);
}

displacement::displacement_instance::~displacement_instance()
{
	_texture.reset();
}

void displacement::displacement_instance::load(obs_data_t* settings)
{
	update(settings);
}

inline void migrate_settings(obs_data_t* settings)
{
	uint64_t version = static_cast<uint64_t>(obs_data_get_int(settings, S_VERSION));

	switch (version & STREAMFX_MASK_COMPAT) {
	case 0:
		obs_data_set_double(settings, ST_SCALE, obs_data_get_double(settings, "Filter.Displacement.Scale") * 0.5);
		obs_data_set_double(settings, ST_SCALE_TYPE,
							obs_data_get_double(settings, "Filter.Displacement.Ratio") * 100.0);
		obs_data_unset_user_value(settings, "Filter.Displacement.Ratio");
	case STREAMFX_MAKE_VERSION(0, 8, 0, 0):
		break;
	}

	obs_data_set_int(settings, S_VERSION, STREAMFX_VERSION);
}

void displacement::displacement_instance::update(obs_data_t* settings)
{
	migrate_settings(settings);

	_scale[0] = _scale[1] = static_cast<float_t>(obs_data_get_double(settings, ST_SCALE));
	_scale_type           = static_cast<float_t>(obs_data_get_double(settings, ST_SCALE_TYPE) / 100.0);

	std::string new_file = obs_data_get_string(settings, ST_FILE);
	if (new_file != _texture_file) {
		try {
			_texture      = std::make_shared<gs::texture>(new_file);
			_texture_file = new_file;
		} catch (...) {
			_texture.reset();
		}
	}
}

void displacement::displacement_instance::video_tick(float_t)
{
	_width  = obs_source_get_base_width(_self);
	_height = obs_source_get_base_height(_self);
}

void displacement::displacement_instance::video_render(gs_effect_t*)
{
	if (!_texture) { // No displacement map, so just skip us for now.
		obs_source_skip_video_filter(_self);
		return;
	}

	if (!obs_source_process_filter_begin(_self, GS_RGBA, OBS_ALLOW_DIRECT_RENDERING)) {
		obs_source_skip_video_filter(_self);
		return;
	}

	_effect.get_parameter("image_size").set_float2(static_cast<float_t>(_width), static_cast<float_t>(_height));
	_effect.get_parameter("image_inverse_size")
		.set_float2(static_cast<float_t>(1.0 / _width), static_cast<float_t>(1.0 / _height));
	_effect.get_parameter("normal").set_texture(_texture->get_object());
	_effect.get_parameter("scale").set_float2(_scale[0], _scale[1]);
	_effect.get_parameter("scale_type").set_float(_scale_type);

	obs_source_process_filter_end(_self, _effect.get_object(), _width, _height);
}

std::string displacement::displacement_instance::get_file()
{
	return _texture_file;
}

std::shared_ptr<displacement::displacement_factory> displacement::displacement_factory::factory_instance = nullptr;

displacement::displacement_factory::displacement_factory()
{
	_info.id           = "obs-stream-effects-filter-displacement";
	_info.type         = OBS_SOURCE_TYPE_FILTER;
	_info.output_flags = OBS_SOURCE_VIDEO;

	set_resolution_enabled(false);
	finish_setup();
}

displacement::displacement_factory::~displacement_factory() {}

const char* displacement::displacement_factory::get_name()
{
	return D_TRANSLATE(ST);
}

void displacement::displacement_factory::get_defaults2(obs_data_t* data)
{
	{
		char* disp = obs_module_file("examples/normal-maps/neutral.png");
		obs_data_set_default_string(data, ST_FILE, disp);
		bfree(disp);
	}

	obs_data_set_default_double(data, ST_SCALE, 0.0);
	obs_data_set_default_double(data, ST_SCALE_TYPE, 0.0);
}

obs_properties_t* displacement::displacement_factory::get_properties2(displacement::displacement_instance* data)
{
	obs_properties_t* pr = obs_properties_create();

	std::string path = "";
	if (data) {
		path = data->get_file();
	} else {
		char* buf = obs_module_file("examples/normal-maps/neutral.png");
		path      = buf;
		bfree(buf);
	}

	{
		auto p = obs_properties_add_path(pr, ST_FILE, D_TRANSLATE(ST_FILE), obs_path_type::OBS_PATH_FILE,
										 D_TRANSLATE(S_FILEFILTERS_TEXTURE), path.c_str());
		obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_FILE)));
	}
	{
		auto p = obs_properties_add_float(pr, ST_SCALE, D_TRANSLATE(ST_SCALE), -10000000.0, 10000000.0, 0.01);
		obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SCALE)));
	}
	{
		auto p = obs_properties_add_float_slider(pr, ST_SCALE_TYPE, D_TRANSLATE(ST_SCALE_TYPE), 0.0, 100.0, 0.01);
		obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SCALE_TYPE)));
	}

	return pr;
}
