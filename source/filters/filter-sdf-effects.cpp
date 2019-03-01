/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2017-2018 Michael Fabian Dirks
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

#include "filter-sdf-effects.hpp"
#include "strings.hpp"

// Translation Strings
#define SOURCE_NAME "Filter.SDFEffects"

#define P_SHADOW_INNER "Filter.SDFEffects.Shadow.Inner"
#define P_SHADOW_INNER_RANGE_MINIMUM "Filter.SDFEffects.Shadow.Inner.Range.Minimum"
#define P_SHADOW_INNER_RANGE_MAXIMUM "Filter.SDFEffects.Shadow.Inner.Range.Maximum"
#define P_SHADOW_INNER_OFFSET_X "Filter.SDFEffects.Shadow.Inner.Offset.X"
#define P_SHADOW_INNER_OFFSET_Y "Filter.SDFEffects.Shadow.Inner.Offset.Y"
#define P_SHADOW_INNER_COLOR "Filter.SDFEffects.Shadow.Inner.Color"
#define P_SHADOW_INNER_ALPHA "Filter.SDFEffects.Shadow.Inner.Alpha"

#define P_SHADOW_OUTER "Filter.SDFEffects.Shadow.Outer"
#define P_SHADOW_OUTER_RANGE_MINIMUM "Filter.SDFEffects.Shadow.Outer.Range.Minimum"
#define P_SHADOW_OUTER_RANGE_MAXIMUM "Filter.SDFEffects.Shadow.Outer.Range.Maximum"
#define P_SHADOW_OUTER_OFFSET_X "Filter.SDFEffects.Shadow.Outer.Offset.X"
#define P_SHADOW_OUTER_OFFSET_Y "Filter.SDFEffects.Shadow.Outer.Offset.Y"
#define P_SHADOW_OUTER_COLOR "Filter.SDFEffects.Shadow.Outer.Color"
#define P_SHADOW_OUTER_ALPHA "Filter.SDFEffects.Shadow.Outer.Alpha"

#define P_SDF_SCALE "Filter.SDFEffects.SDF.Scale"

// Initializer & Finalizer
INITIALIZER(filterShadowFactoryInitializer)
{
	initializerFunctions.push_back([] { filter::sdf_effects::sdf_effects_factory::initialize(); });
	finalizerFunctions.push_back([] { filter::sdf_effects::sdf_effects_factory::finalize(); });
}

static std::shared_ptr<filter::sdf_effects::sdf_effects_factory> factory_instance = nullptr;

void filter::sdf_effects::sdf_effects_factory::initialize()
{
	factory_instance = std::make_shared<filter::sdf_effects::sdf_effects_factory>();
}

void filter::sdf_effects::sdf_effects_factory::finalize()
{
	factory_instance.reset();
}

std::shared_ptr<filter::sdf_effects::sdf_effects_factory> filter::sdf_effects::sdf_effects_factory::get()
{
	return factory_instance;
}

filter::sdf_effects::sdf_effects_factory::sdf_effects_factory()
{
	memset(&source_info, 0, sizeof(obs_source_info));
	source_info.id             = "obs-stream-effects-filter-sdf-effects";
	source_info.type           = OBS_SOURCE_TYPE_FILTER;
	source_info.output_flags   = OBS_SOURCE_VIDEO;
	source_info.get_name       = get_name;
	source_info.get_defaults   = get_defaults;
	source_info.get_properties = get_properties;

	source_info.create       = create;
	source_info.destroy      = destroy;
	source_info.update       = update;
	source_info.activate     = activate;
	source_info.deactivate   = deactivate;
	source_info.video_tick   = video_tick;
	source_info.video_render = video_render;

	obs_register_source(&source_info);
}

filter::sdf_effects::sdf_effects_factory::~sdf_effects_factory() {}

void filter::sdf_effects::sdf_effects_factory::on_list_fill()
{
	{
		char* file = obs_module_file("effects/sdf-generator.effect");
		try {
			sdf_generator_effect = std::make_shared<gs::effect>(file);
		} catch (std::runtime_error ex) {
			P_LOG_ERROR("<filter-shadow> Loading effect '%s' failed with error(s): %s", file, ex.what());
		}
		bfree(file);
	}
	{
		char* file = obs_module_file("effects/sdf-shadow.effect");
		try {
			sdf_shadow_effect = std::make_shared<gs::effect>(file);
		} catch (std::runtime_error ex) {
			P_LOG_ERROR("<filter-shadow> Loading effect '%s' failed with error(s): %s", file, ex.what());
		}
		bfree(file);
	}
}

void filter::sdf_effects::sdf_effects_factory::on_list_empty()
{
	sdf_generator_effect.reset();
	sdf_shadow_effect.reset();
}

void* filter::sdf_effects::sdf_effects_factory::create(obs_data_t* data, obs_source_t* parent)
{
	if (get()->sources.empty()) {
		get()->on_list_fill();
	}
	filter::sdf_effects::sdf_effects_instance* ptr = new filter::sdf_effects::sdf_effects_instance(data, parent);
	get()->sources.push_back(ptr);
	return ptr;
}

void filter::sdf_effects::sdf_effects_factory::destroy(void* inptr)
{
	filter::sdf_effects::sdf_effects_instance* ptr =
		reinterpret_cast<filter::sdf_effects::sdf_effects_instance*>(inptr);
	get()->sources.remove(ptr);
	if (get()->sources.empty()) {
		get()->on_list_empty();
	}
	delete ptr;
}

void filter::sdf_effects::sdf_effects_factory::get_defaults(obs_data_t* data)
{
	obs_data_set_default_bool(data, P_SHADOW_INNER, false);
	obs_data_set_default_double(data, P_SHADOW_INNER_RANGE_MINIMUM, 0.0);
	obs_data_set_default_double(data, P_SHADOW_INNER_RANGE_MAXIMUM, 4.0);
	obs_data_set_default_double(data, P_SHADOW_INNER_OFFSET_X, 0.0);
	obs_data_set_default_double(data, P_SHADOW_INNER_OFFSET_Y, 0.0);
	obs_data_set_default_int(data, P_SHADOW_INNER_COLOR, 0x00000000);
	obs_data_set_default_double(data, P_SHADOW_INNER_ALPHA, 100.0);

	obs_data_set_default_bool(data, P_SHADOW_OUTER, false);
	obs_data_set_default_double(data, P_SHADOW_OUTER_RANGE_MINIMUM, 0.0);
	obs_data_set_default_double(data, P_SHADOW_OUTER_RANGE_MAXIMUM, 4.0);
	obs_data_set_default_double(data, P_SHADOW_OUTER_OFFSET_X, 0.0);
	obs_data_set_default_double(data, P_SHADOW_OUTER_OFFSET_Y, 0.0);
	obs_data_set_default_int(data, P_SHADOW_OUTER_COLOR, 0x00000000);
	obs_data_set_default_double(data, P_SHADOW_OUTER_ALPHA, 100.0);

	obs_data_set_default_bool(data, S_ADVANCED, false);
	obs_data_set_default_double(data, P_SDF_SCALE, 100.0);
}

obs_properties_t* filter::sdf_effects::sdf_effects_factory::get_properties(void* inptr)
{
	return reinterpret_cast<filter::sdf_effects::sdf_effects_instance*>(inptr)->get_properties();
}

void filter::sdf_effects::sdf_effects_factory::update(void* inptr, obs_data_t* settings)
{
	reinterpret_cast<filter::sdf_effects::sdf_effects_instance*>(inptr)->update(settings);
}

const char* filter::sdf_effects::sdf_effects_factory::get_name(void*)
{
	return P_TRANSLATE(SOURCE_NAME);
}

uint32_t filter::sdf_effects::sdf_effects_factory::get_width(void* inptr)
{
	return reinterpret_cast<filter::sdf_effects::sdf_effects_instance*>(inptr)->get_width();
}

uint32_t filter::sdf_effects::sdf_effects_factory::get_height(void* inptr)
{
	return reinterpret_cast<filter::sdf_effects::sdf_effects_instance*>(inptr)->get_height();
}

void filter::sdf_effects::sdf_effects_factory::activate(void* inptr)
{
	reinterpret_cast<filter::sdf_effects::sdf_effects_instance*>(inptr)->activate();
}

void filter::sdf_effects::sdf_effects_factory::deactivate(void* inptr)
{
	reinterpret_cast<filter::sdf_effects::sdf_effects_instance*>(inptr)->deactivate();
}

void filter::sdf_effects::sdf_effects_factory::video_tick(void* inptr, float delta)
{
	reinterpret_cast<filter::sdf_effects::sdf_effects_instance*>(inptr)->video_tick(delta);
}

void filter::sdf_effects::sdf_effects_factory::video_render(void* inptr, gs_effect_t* effect)
{
	reinterpret_cast<filter::sdf_effects::sdf_effects_instance*>(inptr)->video_render(effect);
}

std::shared_ptr<gs::effect> filter::sdf_effects::sdf_effects_factory::get_sdf_generator_effect()
{
	return sdf_generator_effect;
}

std::shared_ptr<gs::effect> filter::sdf_effects::sdf_effects_factory::get_sdf_shadow_effect()
{
	return sdf_shadow_effect;
}

bool filter::sdf_effects::sdf_effects_instance::cb_modified_inside(void*, obs_properties_t* props, obs_property*,
																   obs_data_t* settings)
{
	bool v = obs_data_get_bool(settings, P_SHADOW_INNER);
	obs_property_set_visible(obs_properties_get(props, P_SHADOW_INNER_RANGE_MINIMUM), v);
	obs_property_set_visible(obs_properties_get(props, P_SHADOW_INNER_RANGE_MAXIMUM), v);
	obs_property_set_visible(obs_properties_get(props, P_SHADOW_INNER_OFFSET_X), v);
	obs_property_set_visible(obs_properties_get(props, P_SHADOW_INNER_OFFSET_Y), v);
	obs_property_set_visible(obs_properties_get(props, P_SHADOW_INNER_COLOR), v);
	obs_property_set_visible(obs_properties_get(props, P_SHADOW_INNER_ALPHA), v);
	return true;
}

bool filter::sdf_effects::sdf_effects_instance::cb_modified_outside(void*, obs_properties_t* props, obs_property*,
																	obs_data_t* settings)
{
	bool v = obs_data_get_bool(settings, P_SHADOW_OUTER);
	obs_property_set_visible(obs_properties_get(props, P_SHADOW_OUTER_RANGE_MINIMUM), v);
	obs_property_set_visible(obs_properties_get(props, P_SHADOW_OUTER_RANGE_MAXIMUM), v);
	obs_property_set_visible(obs_properties_get(props, P_SHADOW_OUTER_OFFSET_X), v);
	obs_property_set_visible(obs_properties_get(props, P_SHADOW_OUTER_OFFSET_Y), v);
	obs_property_set_visible(obs_properties_get(props, P_SHADOW_OUTER_COLOR), v);
	obs_property_set_visible(obs_properties_get(props, P_SHADOW_OUTER_ALPHA), v);
	return true;
}

bool filter::sdf_effects::sdf_effects_instance::cb_modified_advanced(void* , obs_properties_t* props,
																	 obs_property* , obs_data_t* settings)
{
	bool show_advanced = obs_data_get_bool(settings, S_ADVANCED);
	obs_property_set_visible(obs_properties_get(props, P_SDF_SCALE), show_advanced);
	return true;
}

filter::sdf_effects::sdf_effects_instance::sdf_effects_instance(obs_data_t* settings, obs_source_t* self)
	: m_self(self), m_source_rendered(false), m_sdf_scale(1.0)
{
	this->m_source_rt = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
	this->m_sdf_write = std::make_shared<gs::rendertarget>(GS_RGBA32F, GS_ZS_NONE);
	{
		auto op = this->m_sdf_write->render(1, 1);
	}
	this->m_sdf_read = std::make_shared<gs::rendertarget>(GS_RGBA32F, GS_ZS_NONE);
	{
		auto op = this->m_sdf_read->render(1, 1);
	}
	this->update(settings);
}

filter::sdf_effects::sdf_effects_instance::~sdf_effects_instance() {}

obs_properties_t* filter::sdf_effects::sdf_effects_instance::get_properties()
{
	obs_properties_t* props = obs_properties_create();
	obs_property_t*   p     = nullptr;

	p = obs_properties_add_bool(props, P_SHADOW_INNER, P_TRANSLATE(P_SHADOW_INNER));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_INNER)));
	obs_property_set_modified_callback2(p, cb_modified_inside, this);

	p = obs_properties_add_float_slider(props, P_SHADOW_INNER_RANGE_MINIMUM, P_TRANSLATE(P_SHADOW_INNER_RANGE_MINIMUM),
										-16.0, 16.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_INNER_RANGE_MINIMUM)));

	p = obs_properties_add_float_slider(props, P_SHADOW_INNER_RANGE_MAXIMUM, P_TRANSLATE(P_SHADOW_INNER_RANGE_MAXIMUM),
										-16.0, 16.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_INNER_RANGE_MAXIMUM)));

	p = obs_properties_add_float_slider(props, P_SHADOW_INNER_OFFSET_X, P_TRANSLATE(P_SHADOW_INNER_OFFSET_X), -100.0,
										100.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_INNER_OFFSET_X)));

	p = obs_properties_add_float_slider(props, P_SHADOW_INNER_OFFSET_Y, P_TRANSLATE(P_SHADOW_INNER_OFFSET_Y), -100.0,
										100.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_INNER_OFFSET_Y)));

	p = obs_properties_add_color(props, P_SHADOW_INNER_COLOR, P_TRANSLATE(P_SHADOW_INNER_COLOR));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_INNER_COLOR)));

	p = obs_properties_add_float_slider(props, P_SHADOW_INNER_ALPHA, P_TRANSLATE(P_SHADOW_INNER_ALPHA), 0.0, 100.0,
										0.1);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_INNER_ALPHA)));

	p = obs_properties_add_bool(props, P_SHADOW_OUTER, P_TRANSLATE(P_SHADOW_OUTER));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_OUTER)));
	obs_property_set_modified_callback2(p, cb_modified_outside, this);

	p = obs_properties_add_float_slider(props, P_SHADOW_OUTER_RANGE_MINIMUM, P_TRANSLATE(P_SHADOW_OUTER_RANGE_MINIMUM),
										-16.0, 16.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_OUTER_RANGE_MINIMUM)));

	p = obs_properties_add_float_slider(props, P_SHADOW_OUTER_RANGE_MAXIMUM, P_TRANSLATE(P_SHADOW_OUTER_RANGE_MAXIMUM),
										-16.0, 16.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_OUTER_RANGE_MAXIMUM)));

	p = obs_properties_add_float_slider(props, P_SHADOW_OUTER_OFFSET_X, P_TRANSLATE(P_SHADOW_OUTER_OFFSET_X), -100.0,
										100.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_OUTER_OFFSET_X)));

	p = obs_properties_add_float_slider(props, P_SHADOW_OUTER_OFFSET_Y, P_TRANSLATE(P_SHADOW_OUTER_OFFSET_Y), -100.0,
										100.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_OUTER_OFFSET_Y)));

	p = obs_properties_add_color(props, P_SHADOW_OUTER_COLOR, P_TRANSLATE(P_SHADOW_OUTER_COLOR));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_OUTER_COLOR)));

	p = obs_properties_add_float_slider(props, P_SHADOW_OUTER_ALPHA, P_TRANSLATE(P_SHADOW_OUTER_ALPHA), 0.0, 100.0,
										0.1);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_OUTER_ALPHA)));

	p = obs_properties_add_bool(props, S_ADVANCED, P_TRANSLATE(S_ADVANCED));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(S_ADVANCED)));
	obs_property_set_modified_callback2(p, cb_modified_advanced, this);

	p = obs_properties_add_float_slider(props, P_SDF_SCALE, P_TRANSLATE(P_SDF_SCALE), 0.1, 500.0, 0.1);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SDF_SCALE)));

	return props;
}

void filter::sdf_effects::sdf_effects_instance::update(obs_data_t* data)
{
	this->m_inner_shadow    = obs_data_get_bool(data, P_SHADOW_INNER);
	this->m_inner_range_min = float_t(obs_data_get_double(data, P_SHADOW_INNER_RANGE_MINIMUM));
	this->m_inner_range_max = float_t(obs_data_get_double(data, P_SHADOW_INNER_RANGE_MAXIMUM));
	this->m_inner_offset_x  = float_t(obs_data_get_double(data, P_SHADOW_INNER_OFFSET_X));
	this->m_inner_offset_y  = float_t(obs_data_get_double(data, P_SHADOW_INNER_OFFSET_Y));
	this->m_inner_color     = (obs_data_get_int(data, P_SHADOW_INNER_COLOR) & 0x00FFFFFF)
						  | (int32_t(obs_data_get_double(data, P_SHADOW_INNER_ALPHA) * 2.55) << 24);

	this->m_outer_shadow    = obs_data_get_bool(data, P_SHADOW_OUTER);
	this->m_outer_range_min = float_t(obs_data_get_double(data, P_SHADOW_OUTER_RANGE_MINIMUM));
	this->m_outer_range_max = float_t(obs_data_get_double(data, P_SHADOW_OUTER_RANGE_MAXIMUM));
	this->m_outer_offset_x  = float_t(obs_data_get_double(data, P_SHADOW_OUTER_OFFSET_X));
	this->m_outer_offset_y  = float_t(obs_data_get_double(data, P_SHADOW_OUTER_OFFSET_Y));
	this->m_outer_color     = (obs_data_get_int(data, P_SHADOW_OUTER_COLOR) & 0x00FFFFFF)
						  | (int32_t(obs_data_get_double(data, P_SHADOW_OUTER_ALPHA) * 2.55) << 24);

	this->m_sdf_scale = double_t(obs_data_get_double(data, P_SDF_SCALE) / 100.0);
}

uint32_t filter::sdf_effects::sdf_effects_instance::get_width()
{
	return uint32_t(0);
}

uint32_t filter::sdf_effects::sdf_effects_instance::get_height()
{
	return uint32_t(0);
}

void filter::sdf_effects::sdf_effects_instance::activate() {}

void filter::sdf_effects::sdf_effects_instance::deactivate() {}

void filter::sdf_effects::sdf_effects_instance::video_tick(float )
{
	m_source_rendered = false;
}

void filter::sdf_effects::sdf_effects_instance::video_render(gs_effect_t*)
{
	obs_source_t* parent         = obs_filter_get_parent(this->m_self);
	obs_source_t* target         = obs_filter_get_target(this->m_self);
	uint32_t      baseW          = obs_source_get_base_width(target);
	uint32_t      baseH          = obs_source_get_base_height(target);
	gs_effect_t*  default_effect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_DEFAULT);
	vec4          color_transparent;
	vec4_zero(&color_transparent);

	if (!parent || !target || (baseW == 0) || (baseH == 0)) {
		obs_source_skip_video_filter(this->m_self);
		return;
	}

	try {
		gs_blend_state_push();
		gs_reset_blend_state();
		gs_enable_blending(false);
		gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);

		gs_set_cull_mode(GS_NEITHER);
		gs_enable_color(true, true, true, true);
		gs_enable_depth_test(false);
		gs_depth_function(GS_ALWAYS);
		gs_enable_stencil_test(false);
		gs_enable_stencil_write(false);
		gs_stencil_function(GS_STENCIL_BOTH, GS_ALWAYS);
		gs_stencil_op(GS_STENCIL_BOTH, GS_ZERO, GS_ZERO, GS_ZERO);

		if (!this->m_source_rendered) {
			// Store input texture.
			{
				auto op = m_source_rt->render(baseW, baseH);
				gs_ortho(0, (float)baseW, 0, (float)baseH, -1, 1);
				gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &color_transparent, 0, 0);

				if (obs_source_process_filter_begin(this->m_self, GS_RGBA, OBS_ALLOW_DIRECT_RENDERING)) {
					obs_source_process_filter_end(this->m_self, default_effect, baseW, baseH);
				} else {
					throw std::runtime_error("failed to process source");
				}
			}
			m_source_rt->get_texture(this->m_source_texture);
			if (!this->m_source_texture) {
				throw std::runtime_error("failed to draw source");
			}

			// Generate SDF Buffers
			{
				this->m_sdf_read->get_texture(this->m_sdf_texture);
				if (!this->m_sdf_texture) {
					throw std::runtime_error("SDF Backbuffer empty");
				}

				std::shared_ptr<gs::effect> sdf_effect =
					filter::sdf_effects::sdf_effects_factory::get()->get_sdf_generator_effect();
				if (!sdf_effect) {
					throw std::runtime_error("SDF Effect no loaded");
				}

				// Scale SDF Size
				double_t sdfW, sdfH;
				sdfW = baseW * m_sdf_scale;
				sdfH = baseH * m_sdf_scale;
				if (sdfW <= 1) {
					sdfW = 1.0;
				}
				if (sdfH <= 1) {
					sdfH = 1.0;
				}

				{
					auto op = m_sdf_write->render(uint32_t(sdfW), uint32_t(sdfH));
					gs_ortho(0, (float)sdfW, 0, (float)sdfH, -1, 1);
					gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &color_transparent, 0, 0);

					sdf_effect->get_parameter("_image").set_texture(this->m_source_texture);
					sdf_effect->get_parameter("_size").set_float2(float_t(sdfW), float_t(sdfH));
					sdf_effect->get_parameter("_sdf").set_texture(this->m_sdf_texture);
					sdf_effect->get_parameter("_threshold").set_float(0.5);

					while (gs_effect_loop(sdf_effect->get_object(), "Draw")) {
						gs_draw_sprite(this->m_sdf_texture->get_object(), 0, uint32_t(sdfW), uint32_t(sdfH));
					}
				}
				this->m_sdf_write.swap(this->m_sdf_read);
				this->m_sdf_read->get_texture(this->m_sdf_texture);
				if (!this->m_sdf_texture) {
					throw std::runtime_error("SDF Backbuffer empty");
				}
			}
		}

		gs_blend_state_pop();
	} catch (...) {
		gs_blend_state_pop();
		obs_source_skip_video_filter(this->m_self);
		return;
	}

	try {
		std::shared_ptr<gs::effect> shadow_effect =
			filter::sdf_effects::sdf_effects_factory::get()->get_sdf_shadow_effect();
		if (!shadow_effect) {
			throw std::runtime_error("Shadow Effect no loaded");
		}

		shadow_effect->get_parameter("_sdf").set_texture(this->m_sdf_texture);
		shadow_effect->get_parameter("_image").set_texture(this->m_source_texture);
		shadow_effect->get_parameter("_threshold").set_float(0.5f);

		if (this->m_inner_shadow) {
			shadow_effect->get_parameter("_inner_min").set_float(this->m_inner_range_min);
			shadow_effect->get_parameter("_inner_max").set_float(this->m_inner_range_max);
			shadow_effect->get_parameter("_inner_offset")
				.set_float2(this->m_inner_offset_x / float_t(baseW), this->m_inner_offset_y / float_t(baseH));
			shadow_effect->get_parameter("_inner_color")
				.set_float4((this->m_inner_color & 0xFF) / 255.0f, ((this->m_inner_color >> 8) & 0xFF) / 255.0f,
							((this->m_inner_color >> 16) & 0xFF) / 255.0f,
							((this->m_inner_color >> 24) & 0xFF) / 255.0f);
		} else {
			shadow_effect->get_parameter("_inner_min").set_float(0.);
			shadow_effect->get_parameter("_inner_max").set_float(0.);
			shadow_effect->get_parameter("_inner_offset").set_float2(0., 0.);
			shadow_effect->get_parameter("_inner_color").set_float4(0., 0., 0., 0.);
		}
		if (this->m_outer_shadow) {
			shadow_effect->get_parameter("_outer_min").set_float(this->m_outer_range_min);
			shadow_effect->get_parameter("_outer_max").set_float(this->m_outer_range_max);
			shadow_effect->get_parameter("_outer_offset")
				.set_float2(this->m_outer_offset_x / float_t(baseW), this->m_outer_offset_y / float_t(baseH));
			shadow_effect->get_parameter("_outer_color")
				.set_float4((this->m_outer_color & 0xFF) / 255.0f, ((this->m_outer_color >> 8) & 0xFF) / 255.0f,
							((this->m_outer_color >> 16) & 0xFF) / 255.0f,
							((this->m_outer_color >> 24) & 0xFF) / 255.0f);
		} else {
			shadow_effect->get_parameter("_outer_min").set_float(0.);
			shadow_effect->get_parameter("_outer_max").set_float(0.);
			shadow_effect->get_parameter("_outer_offset").set_float2(0., 0.);
			shadow_effect->get_parameter("_outer_color").set_float4(0., 0., 0., 0.);
		}

		while (gs_effect_loop(shadow_effect->get_object(), "Draw")) {
			gs_draw_sprite(this->m_source_texture->get_object(), 0, baseW, baseH);
		}
	} catch (...) {
		obs_source_skip_video_filter(this->m_self);
		return;
	}
}
