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

#include "strings.hpp"
#include <stdexcept>
#include "transition-shader.hpp"
#include "utility.hpp"

#define ST "Transition.Shader"

transition::shader::shader_instance::shader_instance(obs_data_t* data, obs_source_t* self)
	: obs::source_instance(data, self), _is_main(false)
{
	_fx = std::make_shared<gfx::shader::shader>(self, gfx::shader::shader_mode::Transition);

	update(data);
}

transition::shader::shader_instance::~shader_instance() {}

uint32_t transition::shader::shader_instance::get_width()
{
	return _fx->width();
}

uint32_t transition::shader::shader_instance::get_height()
{
	return _fx->height();
}

void transition::shader::shader_instance::properties(obs_properties_t* props)
{
	_fx->properties(props);
}

void transition::shader::shader_instance::load(obs_data_t* data)
{
	update(data);
}

void transition::shader::shader_instance::update(obs_data_t* data)
{
	_fx->update(data);
}

void transition::shader::shader_instance::video_tick(float_t sec_since_last)
{
	if (_fx->tick(sec_since_last)) {
		obs_data_t* data = obs_source_get_settings(_self);
		_fx->update(data);
		obs_data_release(data);
	}

	// Update Size from global base resolution.
	obs_video_info ovi;
	obs_get_video_info(&ovi);
	_fx->set_size(ovi.base_width, ovi.base_height);
}

void transition::shader::shader_instance::video_render(gs_effect_t* effect)
{
	if (!_fx) {
		return;
	}

	_fx->prepare_render();
	obs_transition_video_render(_self,
								[](void* data, gs_texture_t* a, gs_texture_t* b, float t, uint32_t cx, uint32_t cy) {
									reinterpret_cast<shader_instance*>(data)->transition_render(a, b, t, cx, cy);
								});
}

void transition::shader::shader_instance::transition_render(gs_texture_t* a, gs_texture_t* b, float t, uint32_t cx,
															uint32_t cy)
{
	_fx->set_input_a(std::make_shared<::gs::texture>(a, false));
	_fx->set_input_b(std::make_shared<::gs::texture>(b, false));
	_fx->set_transition_time(t);
	_fx->set_transition_size(cx, cy);
	_fx->render();
}

bool transition::shader::shader_instance::audio_render(uint64_t* ts_out, obs_source_audio_mix* audio_output,
													   uint32_t mixers, size_t channels, size_t sample_rate)
{
	return obs_transition_audio_render(
		_self, ts_out, audio_output, mixers, channels, sample_rate, [](void*, float_t t) { return 1.0f - t; },
		[](void*, float_t t) { return t; });
}

void transition::shader::shader_instance::transition_start() {}

void transition::shader::shader_instance::transition_stop() {}

std::shared_ptr<transition::shader::shader_factory> transition::shader::shader_factory::factory_instance = nullptr;

transition::shader::shader_factory::shader_factory()
{
	_info.id           = "obs-stream-effects-transition-shader";
	_info.type         = OBS_SOURCE_TYPE_TRANSITION;
	_info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW;

	finish_setup();
}

transition::shader::shader_factory::~shader_factory() {}

const char* transition::shader::shader_factory::get_name()
{
	return D_TRANSLATE(ST);
}

void transition::shader::shader_factory::get_defaults2(obs_data_t* data)
{
	gfx::shader::shader::defaults(data);
}

obs_properties_t* transition::shader::shader_factory::get_properties2(shader::shader_instance* data)
{
	auto pr = obs_properties_create();
	obs_properties_set_param(pr, data, nullptr);

	if (data) {
		reinterpret_cast<shader_instance*>(data)->properties(pr);
	}

	return pr;
}
