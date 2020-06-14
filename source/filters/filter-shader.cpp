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

#include "filter-shader.hpp"
#include "strings.hpp"
#include <stdexcept>
#include "obs/gs/gs-helper.hpp"
#include "utility.hpp"

#define ST "Filter.Shader"

using namespace streamfx::filter::shader;

shader_instance::shader_instance(obs_data_t* data, obs_source_t* self) : obs::source_instance(data, self)
{
	_fx = std::make_shared<gfx::shader::shader>(self, gfx::shader::shader_mode::Filter);
	_rt = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
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

void shader_instance::migrate(obs_data_t* data, std::uint64_t version) {}

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

	if (obs_source_t* tgt = obs_filter_get_target(_self); tgt != nullptr) {
		_fx->set_size(obs_source_get_base_width(tgt), obs_source_get_base_height(tgt));
	} else if (obs_source* src = obs_filter_get_parent(_self); src != nullptr) {
		_fx->set_size(obs_source_get_base_width(src), obs_source_get_base_height(src));
	}
}

void shader_instance::video_render(gs_effect_t* effect)
{
	if (!_fx || !_fx->width() || !_fx->height()) {
		obs_source_skip_video_filter(_self);
		return;
	}

#ifdef ENABLE_PROFILING
	gs::debug_marker gdmp{gs::debug_color_source, "Shader Filter '%s' on '%s'", obs_source_get_name(_self),
						  obs_source_get_name(obs_filter_get_parent(_self))};
#endif

	{
#ifdef ENABLE_PROFILING
		gs::debug_marker gdm{gs::debug_color_source, "Cache"};
#endif

		auto op = _rt->render(_fx->width(), _fx->height());

		gs_ortho(0, static_cast<float_t>(_fx->width()), 0, static_cast<float_t>(_fx->height()), -1, 1);

		vec4 clear_color = {0, 0, 0, 0};
		gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &clear_color, 0, 0);

		/// Render original source
		if (obs_source_process_filter_begin(_self, GS_RGBA, OBS_NO_DIRECT_RENDERING)) {
			gs_blend_state_push();
			gs_reset_blend_state();
			gs_enable_blending(false);
			gs_blend_function_separate(GS_BLEND_ONE, GS_BLEND_ZERO, GS_BLEND_SRCALPHA, GS_BLEND_ZERO);
			gs_enable_depth_test(false);
			gs_enable_stencil_test(false);
			gs_enable_stencil_write(false);
			gs_enable_color(true, true, true, true);
			gs_set_cull_mode(GS_NEITHER);

			obs_source_process_filter_end(_self, obs_get_base_effect(OBS_EFFECT_DEFAULT), _fx->width(), _fx->height());

			gs_blend_state_pop();
		} else {
			obs_source_skip_video_filter(_self);
			return;
		}
	}

	{
#ifdef ENABLE_PROFILING
		gs::debug_marker gdm{gs::debug_color_render, "Render"};
#endif

		_fx->prepare_render();
		_fx->set_input_a(_rt->get_texture());
		_fx->render();
	}
}

shader_factory::shader_factory()
{
	_info.id           = PREFIX "filter-shader";
	_info.type         = OBS_SOURCE_TYPE_FILTER;
	_info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW;

	finish_setup();
	register_proxy("obs-stream-effects-filter-shader");
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

std::shared_ptr<shader_factory> _filter_shader_factory_instance = nullptr;

void streamfx::filter::shader::shader_factory::initialize()
{
	if (!_filter_shader_factory_instance)
		_filter_shader_factory_instance = std::make_shared<shader_factory>();
}

void streamfx::filter::shader::shader_factory::finalize()
{
	_filter_shader_factory_instance.reset();
}

std::shared_ptr<shader_factory> streamfx::filter::shader::shader_factory::get()
{
	return _filter_shader_factory_instance;
}
