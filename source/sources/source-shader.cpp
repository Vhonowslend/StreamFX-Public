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
#include <stdexcept>
#include "strings.hpp"
#include "utility.hpp"

#define ST "Source.Shader"

using namespace source;

shader::shader_instance::shader_instance(obs_data_t* data, obs_source_t* self)
	: obs::source_instance(data, self), _is_main(false)
{
	_fx = std::make_shared<gfx::shader::shader>(self, gfx::shader::shader_mode::Source);

	update(data);
}

shader::shader_instance::~shader_instance() {}

uint32_t shader::shader_instance::get_width()
{
	return _fx->width();
}

uint32_t shader::shader_instance::get_height()
{
	return _fx->height();
}

void shader::shader_instance::properties(obs_properties_t* props)
{
	_fx->properties(props);
}

void shader::shader_instance::load(obs_data_t* data)
{
	_fx->update(data);
}

void shader::shader_instance::update(obs_data_t* data)
{
	_fx->update(data);
}

void shader::shader_instance::video_tick(float_t sec_since_last)
{
	if (_fx->tick(sec_since_last)) {
		obs_data_t* data = obs_source_get_settings(_self);
		_fx->update(data);
		obs_data_release(data);
	}

	_is_main = true;
}

void shader::shader_instance::video_render(gs_effect_t* effect)
{
	if (!_fx) {
		return;
	}

	if (_is_main) { // Dirty hack to only take the value from the first render, which usually is the main view.
		gs_rect vect;
		gs_get_viewport(&vect);
		_fx->set_size(static_cast<uint32_t>(vect.cx), static_cast<uint32_t>(vect.cy));
		_is_main = false;
	}

	_fx->render();
}

std::shared_ptr<shader::shader_factory> shader::shader_factory::factory_instance = nullptr;

shader::shader_factory::shader_factory()
{
	_info.id           = "obs-stream-effects-source-shader";
	_info.type         = OBS_SOURCE_TYPE_INPUT;
	_info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW;

	finish_setup();
}

shader::shader_factory::~shader_factory() {}

const char* shader::shader_factory::get_name()
{
	return D_TRANSLATE(ST);
}

void shader::shader_factory::get_defaults2(obs_data_t* data) {}

obs_properties_t* shader::shader_factory::get_properties2(shader::shader_instance* data)
{
	auto pr = obs_properties_create();
	obs_properties_set_param(pr, data, nullptr);

	if (data) {
		reinterpret_cast<shader_instance*>(data)->properties(pr);
	}

	return pr;
}
