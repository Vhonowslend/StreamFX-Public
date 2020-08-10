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

#include "source-shader.hpp"
#include "strings.hpp"
#include <stdexcept>
#include "obs/gs/gs-helper.hpp"

#define ST "Source.Shader"

using namespace streamfx::source::shader;

shader_instance::shader_instance(obs_data_t* data, obs_source_t* self) : obs::source_instance(data, self), _fx()
{
	_fx = std::make_shared<::gfx::shader::shader>(self, ::gfx::shader::shader_mode::Source);

	update(data);
}

shader_instance::~shader_instance() {}

uint32_t shader_instance::get_width()
{
	return _fx->width();
}

uint32_t shader_instance::get_height()
{
	return _fx->height();
}

void shader_instance::properties(obs_properties_t* props)
{
	_fx->properties(props);
}

void shader_instance::load(obs_data_t* data)
{
	_fx->update(data);
}

void shader_instance::update(obs_data_t* data)
{
	_fx->update(data);
}

void shader_instance::video_tick(float_t sec_since_last)
{
	if (_fx->tick(sec_since_last)) {
		obs_data_t* data = obs_source_get_settings(_self);
		_fx->update(data);
		obs_data_release(data);
	}

	obs_video_info ovi;
	obs_get_video_info(&ovi);
	_fx->set_size(ovi.base_width, ovi.base_height);
}

void shader_instance::video_render(gs_effect_t* effect)
{
	if (!_fx) {
		return;
	}

#ifdef ENABLE_PROFILING
	gs::debug_marker gdmp{gs::debug_color_source, "Shader Source '%s'", obs_source_get_name(_self)};
#endif

	_fx->prepare_render();
	_fx->render();
}

void streamfx::source::shader::shader_instance::activate()
{
	_fx->set_active(true);
}

void streamfx::source::shader::shader_instance::deactivate()
{
	_fx->set_active(false);
}

shader_factory::shader_factory()
{
	_info.id           = PREFIX "source-shader";
	_info.type         = OBS_SOURCE_TYPE_INPUT;
	_info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW;

	set_activity_tracking_enabled(true);
	finish_setup();
	register_proxy("obs-stream-effects-source-shader");
}

shader_factory::~shader_factory() {}

const char* shader_factory::get_name()
{
	return D_TRANSLATE(ST);
}

void shader_factory::get_defaults2(obs_data_t* data)
{
	::gfx::shader::shader::defaults(data);
}

obs_properties_t* shader_factory::get_properties2(shader_instance* data)
{
	auto pr = obs_properties_create();
	obs_properties_set_param(pr, data, nullptr);

	if (data) {
		reinterpret_cast<shader_instance*>(data)->properties(pr);
	}

	return pr;
}

std::shared_ptr<shader_factory> _source_shader_factory_instance = nullptr;

void streamfx::source::shader::shader_factory::initialize()
{
	if (!_source_shader_factory_instance)
		_source_shader_factory_instance = std::make_shared<shader_factory>();
}

void streamfx::source::shader::shader_factory::finalize()
{
	_source_shader_factory_instance.reset();
}

std::shared_ptr<shader_factory> streamfx::source::shader::shader_factory::get()
{
	return _source_shader_factory_instance;
}
