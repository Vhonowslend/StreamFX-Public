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

#include "filter-custom-shader.hpp"
#include "strings.hpp"
#include "utility.hpp"

#define ST "Filter.Shader"

P_INITIALIZER(FilterShaderInit)
{
	initializer_functions.push_back([] { filter::shader::shader_factory::initialize(); });
	finalizer_functions.push_back([] { filter::shader::shader_factory::finalize(); });
}

static std::shared_ptr<filter::shader::shader_factory> factory_instance = nullptr;

void filter::shader::shader_factory::initialize()
{
	factory_instance = std::make_shared<filter::shader::shader_factory>();
}

void filter::shader::shader_factory::finalize()
{
	factory_instance.reset();
}

std::shared_ptr<filter::shader::shader_factory> filter::shader::shader_factory::get()
{
	return factory_instance;
}

static void get_defaults(obs_data_t* data) {}

filter::shader::shader_factory::shader_factory()
{
	memset(&_source_info, 0, sizeof(obs_source_info));
	_source_info.id           = "obs-stream-effects-filter-shader";
	_source_info.type         = OBS_SOURCE_TYPE_FILTER;
	_source_info.output_flags = OBS_SOURCE_VIDEO;
	_source_info.get_name     = [](void*) { return D_TRANSLATE(ST); };
	_source_info.get_defaults = get_defaults;

	_source_info.create = [](obs_data_t* data, obs_source_t* self) {
		try {
			return static_cast<void*>(new filter::shader::shader_instance(data, self));
		} catch (std::exception& ex) {
			P_LOG_ERROR("<filter-shader> Failed to create source, error: %s", ex.what());
		} catch (...) {
			P_LOG_ERROR("<filter-shader> Failed to create source.");
		}
		return static_cast<void*>(nullptr);
	};
	_source_info.destroy = [](void* ptr) {
		try {
			delete reinterpret_cast<filter::shader::shader_instance*>(ptr);
		} catch (std::exception& ex) {
			P_LOG_ERROR("<filter-shader> Failed to delete source, error: %s", ex.what());
		} catch (...) {
			P_LOG_ERROR("<filter-shader> Failed to delete source.");
		}
	};
	_source_info.get_properties = [](void* ptr) {
		obs_properties_t* pr = obs_properties_create();
		try {
			if (ptr)
				reinterpret_cast<filter::shader::shader_instance*>(ptr)->properties(pr);
		} catch (std::exception& ex) {
			P_LOG_ERROR("<filter-shader> Failed to retrieve options, error: %s", ex.what());
		} catch (...) {
			P_LOG_ERROR("<filter-shader> Failed to retrieve options.");
		}
		return pr;
	};
	_source_info.update = [](void* ptr, obs_data_t* data) {
		try {
			if (ptr)
				reinterpret_cast<filter::shader::shader_instance*>(ptr)->update(data);
		} catch (std::exception& ex) {
			P_LOG_ERROR("<filter-shader> Failed to update, error: %s", ex.what());
		} catch (...) {
			P_LOG_ERROR("<filter-shader> Failed to update.");
		}
	};
	_source_info.activate = [](void* ptr) {
		try {
			if (ptr)
				reinterpret_cast<filter::shader::shader_instance*>(ptr)->activate();
		} catch (std::exception& ex) {
			P_LOG_ERROR("<filter-shader> Failed to activate, error: %s", ex.what());
		} catch (...) {
			P_LOG_ERROR("<filter-shader> Failed to activate.");
		}
	};
	_source_info.deactivate = [](void* ptr) {
		try {
			if (ptr)
				reinterpret_cast<filter::shader::shader_instance*>(ptr)->deactivate();
		} catch (std::exception& ex) {
			P_LOG_ERROR("<filter-shader> Failed to deactivate, error: %s", ex.what());
		} catch (...) {
			P_LOG_ERROR("<filter-shader> Failed to deactivate.");
		}
	};
	_source_info.video_tick = [](void* ptr, float_t time) {
		try {
			if (ptr)
				reinterpret_cast<filter::shader::shader_instance*>(ptr)->video_tick(time);
		} catch (std::exception& ex) {
			P_LOG_ERROR("<filter-shader> Failed to tick, error: %s", ex.what());
		} catch (...) {
			P_LOG_ERROR("<filter-shader> Failed to tick.");
		}
	};
	_source_info.video_render = [](void* ptr, gs_effect_t* effect) {
		try {
			if (ptr)
				reinterpret_cast<filter::shader::shader_instance*>(ptr)->video_render(effect);
		} catch (std::exception& ex) {
			P_LOG_ERROR("<filter-shader> Failed to render, error: %s", ex.what());
		} catch (...) {
			P_LOG_ERROR("<filter-shader> Failed to render.");
		}
	};

	obs_register_source(&_source_info);
}

filter::shader::shader_factory::~shader_factory() {}

filter::shader::shader_instance::shader_instance(obs_data_t* data, obs_source_t* self)
	: _self(self), _active(true), _width(0), _height(0)
{
	_fx = std::make_shared<gfx::effect_source::effect_source>();
	_rt = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);

	update(data);
}

filter::shader::shader_instance::~shader_instance() {}

uint32_t filter::shader::shader_instance::width()
{
	return _width;
}

uint32_t filter::shader::shader_instance::height()
{
	return _height;
}

void filter::shader::shader_instance::properties(obs_properties_t* props) {
	_fx->properties(props);
}

void filter::shader::shader_instance::update(obs_data_t* data) {
	_fx->update(data);
}

void filter::shader::shader_instance::activate()
{
	_active = true;
}

void filter::shader::shader_instance::deactivate()
{
	_active = false;
}

void filter::shader::shader_instance::video_tick(float_t sec_since_last)
{
	obs_source_t* target = obs_filter_get_target(_self);

	{ // Update width and height.
		_width  = obs_source_get_base_width(target);
		_height = obs_source_get_base_height(target);
	}

	_fx->tick(sec_since_last);
}

void filter::shader::shader_instance::video_render(gs_effect_t* effect)
{
	// Grab initial values.
	obs_source_t* parent         = obs_filter_get_parent(_self);
	obs_source_t* target         = obs_filter_get_target(_self);
	uint32_t      width          = obs_source_get_base_width(target);
	uint32_t      height         = obs_source_get_base_height(target);
	gs_effect_t*  effect_default = obs_get_base_effect(obs_base_effect::OBS_EFFECT_DEFAULT);

	// Skip filter if anything is wrong.
	if (!_active || !parent || !target || !width || !height || !effect_default) {
		obs_source_skip_video_filter(_self);
		return;
	}

	try {
		_fx->render();
	} catch (...) {
		obs_source_skip_video_filter(_self);
	}
}
