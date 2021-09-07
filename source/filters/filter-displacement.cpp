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
#include "obs/gs/gs-helper.hpp"
#include "util/util-logging.hpp"

#ifdef _DEBUG
#define ST_PREFIX "<%s> "
#define D_LOG_ERROR(x, ...) P_LOG_ERROR(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_WARNING(x, ...) P_LOG_WARN(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_INFO(x, ...) P_LOG_INFO(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_DEBUG(x, ...) P_LOG_DEBUG(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#else
#define ST_PREFIX "<filter::displacement> "
#define D_LOG_ERROR(...) P_LOG_ERROR(ST_PREFIX __VA_ARGS__)
#define D_LOG_WARNING(...) P_LOG_WARN(ST_PREFIX __VA_ARGS__)
#define D_LOG_INFO(...) P_LOG_INFO(ST_PREFIX __VA_ARGS__)
#define D_LOG_DEBUG(...) P_LOG_DEBUG(ST_PREFIX __VA_ARGS__)
#endif

#define ST_I18N "Filter.Displacement"
#define ST_I18N_FILE "Filter.Displacement.File"
#define ST_KEY_FILE "Filter.Displacement.File"
#define ST_I18N_SCALE "Filter.Displacement.Scale"
#define ST_KEY_SCALE "Filter.Displacement.Scale"
#define ST_I18N_SCALE_TYPE "Filter.Displacement.Scale.Type"
#define ST_KEY_SCALE_TYPE "Filter.Displacement.Scale.Type"

using namespace streamfx::filter::displacement;

displacement_instance::displacement_instance(obs_data_t* data, obs_source_t* context)
	: obs::source_instance(data, context)
{
	_effect = streamfx::obs::gs::effect::create(streamfx::data_file_path("effects/displace.effect").u8string());

	update(data);
}

displacement_instance::~displacement_instance()
{
	_texture.reset();
}

void displacement_instance::load(obs_data_t* settings)
{
	update(settings);
}

void displacement_instance::migrate(obs_data_t* data, uint64_t version)
{
	switch (version & STREAMFX_MASK_COMPAT) {
	case 0:
		obs_data_set_double(data, ST_KEY_SCALE, obs_data_get_double(data, "Filter.Displacement.Scale") * 0.5);
		obs_data_set_double(data, ST_KEY_SCALE_TYPE, obs_data_get_double(data, "Filter.Displacement.Ratio") * 100.0);
		obs_data_unset_user_value(data, "Filter.Displacement.Ratio");
	case STREAMFX_MAKE_VERSION(0, 8, 0, 0):
		break;
	}
}

void displacement_instance::update(obs_data_t* settings)
{
	_scale[0] = _scale[1] = static_cast<float_t>(obs_data_get_double(settings, ST_KEY_SCALE));
	_scale_type           = static_cast<float_t>(obs_data_get_double(settings, ST_KEY_SCALE_TYPE) / 100.0);

	std::string new_file = obs_data_get_string(settings, ST_KEY_FILE);
	if (new_file != _texture_file) {
		try {
			_texture      = std::make_shared<streamfx::obs::gs::texture>(new_file);
			_texture_file = new_file;
		} catch (...) {
			_texture.reset();
		}
	}
}

void displacement_instance::video_tick(float_t)
{
	_width  = obs_source_get_base_width(_self);
	_height = obs_source_get_base_height(_self);
}

void displacement_instance::video_render(gs_effect_t*)
{
	if (!_texture) { // No displacement map, so just skip us for now.
		obs_source_skip_video_filter(_self);
		return;
	}

#ifdef ENABLE_PROFILING
	streamfx::obs::gs::debug_marker gdmp{streamfx::obs::gs::debug_color_source, "Displacement Mapping '%s' on '%s'",
										 obs_source_get_name(_self), obs_source_get_name(obs_filter_get_parent(_self))};
#endif

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

std::string displacement_instance::get_file()
{
	return _texture_file;
}

displacement_factory::displacement_factory()
{
	_info.id           = S_PREFIX "filter-displacement";
	_info.type         = OBS_SOURCE_TYPE_FILTER;
	_info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW | OBS_SOURCE_DEPRECATED | OBS_SOURCE_CAP_DISABLED;

	set_resolution_enabled(false);
	finish_setup();
	register_proxy("obs-stream-effects-filter-displacement");
}

displacement_factory::~displacement_factory() {}

const char* displacement_factory::get_name()
{
	return D_TRANSLATE(ST_I18N);
}

void displacement_factory::get_defaults2(obs_data_t* data)
{
	obs_data_set_default_string(data, ST_KEY_FILE,
								streamfx::data_file_path("examples/normal-maps/neutral.png").u8string().c_str());
	obs_data_set_default_double(data, ST_KEY_SCALE, 0.0);
	obs_data_set_default_double(data, ST_KEY_SCALE_TYPE, 0.0);
}

obs_properties_t* displacement_factory::get_properties2(displacement_instance* data)
{
	obs_properties_t* pr = obs_properties_create();

	std::string path = "";
	if (data) {
		path = data->get_file();
	} else {
		path = streamfx::data_file_path("examples/normal-maps/neutral.png").u8string();
	}

	obs_properties_add_path(pr, ST_KEY_FILE, D_TRANSLATE(ST_I18N_FILE), obs_path_type::OBS_PATH_FILE,
							D_TRANSLATE(S_FILEFILTERS_TEXTURE), path.c_str());
	obs_properties_add_float(pr, ST_KEY_SCALE, D_TRANSLATE(ST_I18N_SCALE), -10000000.0, 10000000.0, 0.01);
	obs_properties_add_float_slider(pr, ST_KEY_SCALE_TYPE, D_TRANSLATE(ST_I18N_SCALE_TYPE), 0.0, 100.0, 0.01);

	return pr;
}

std::shared_ptr<displacement_factory> _filter_displacement_factory_instance = nullptr;

void streamfx::filter::displacement::displacement_factory::initialize()
try {
	if (!_filter_displacement_factory_instance)
		_filter_displacement_factory_instance = std::make_shared<displacement_factory>();
} catch (const std::exception& ex) {
	D_LOG_ERROR("Failed to initialize due to error: %s", ex.what());
} catch (...) {
	D_LOG_ERROR("Failed to initialize due to unknown error.", "");
}

void streamfx::filter::displacement::displacement_factory::finalize()
{
	_filter_displacement_factory_instance.reset();
}

std::shared_ptr<displacement_factory> streamfx::filter::displacement::displacement_factory::get()
{
	return _filter_displacement_factory_instance;
}
