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

#include "transition-shader.hpp"
#include "strings.hpp"
#include <stdexcept>
#include "obs/gs/gs-helper.hpp"

#define ST "Transition.Shader"

using namespace streamfx::transition::shader;

shader_instance::shader_instance(obs_data_t* data, obs_source_t* self) : obs::source_instance(data, self)
{
	_fx = std::make_shared<gfx::shader::shader>(self, gfx::shader::shader_mode::Transition);

	update(data);
}

shader_instance::~shader_instance() {}

std::uint32_t shader_instance::get_width()
{
	return _fx->width();
}

std::uint32_t shader_instance::get_height()
{
	return _fx->height();
}

void shader_instance::properties(obs_properties_t* props)
{
	_fx->properties(props);
}

void shader_instance::load(obs_data_t* data)
{
	update(data);
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

	// Update Size from global base resolution.
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
	gs::debug_marker gdmp{gs::debug_color_source, "Shader Transition '%s'", obs_source_get_name(_self)};
#endif

	obs_transition_video_render(
		_self, [](void* data, gs_texture_t* a, gs_texture_t* b, float t, std::uint32_t cx, std::uint32_t cy) {
			reinterpret_cast<shader_instance*>(data)->transition_render(a, b, t, cx, cy);
		});
}

void shader_instance::transition_render(gs_texture_t* a, gs_texture_t* b, float_t t, std::uint32_t cx, std::uint32_t cy)
{
	_fx->set_input_a(std::make_shared<::gs::texture>(a, false));
	_fx->set_input_b(std::make_shared<::gs::texture>(b, false));
	_fx->set_transition_time(t);
	_fx->set_transition_size(cx, cy);
	_fx->prepare_render();
	_fx->render();
}

bool shader_instance::audio_render(uint64_t* ts_out, obs_source_audio_mix* audio_output, std::uint32_t mixers,
								   std::size_t channels, std::size_t sample_rate)
{
	return obs_transition_audio_render(
		_self, ts_out, audio_output, mixers, channels, sample_rate, [](void*, float_t t) { return 1.0f - t; },
		[](void*, float_t t) { return t; });
}

void shader_instance::transition_start()
{
	_fx->set_active(true);
}

void shader_instance::transition_stop()
{
	_fx->set_active(false);
}

shader_factory::shader_factory()
{
	_info.id           = PREFIX "transition-shader";
	_info.type         = OBS_SOURCE_TYPE_TRANSITION;
	_info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW;

	//set_activity_tracking_enabled(true); // Handled via transition start/stop
	finish_setup();
	register_proxy("obs-stream-effects-transition-shader");
}

shader_factory::~shader_factory() {}

const char* shader_factory::get_name()
{
	return D_TRANSLATE(ST);
}

void shader_factory::get_defaults2(obs_data_t* data)
{
	gfx::shader::shader::defaults(data);
}

obs_properties_t* shader_factory::get_properties2(shader::shader_instance* data)
{
	auto pr = obs_properties_create();
	obs_properties_set_param(pr, data, nullptr);

	if (data) {
		reinterpret_cast<shader_instance*>(data)->properties(pr);
	}

	return pr;
}

std::shared_ptr<shader_factory> _transition_shader_factory_instance = nullptr;

void streamfx::transition::shader::shader_factory::initialize()
{
	if (!_transition_shader_factory_instance)
		_transition_shader_factory_instance = std::make_shared<shader_factory>();
}

void streamfx::transition::shader::shader_factory::finalize()
{
	_transition_shader_factory_instance.reset();
}

std::shared_ptr<shader_factory> streamfx::transition::shader::shader_factory::get()
{
	return _transition_shader_factory_instance;
}
