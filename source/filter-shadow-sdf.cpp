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

#include "filter-shadow-sdf.hpp"
#include "strings.hpp"

// Translation Strings
#define SOURCE_NAME "Filter.ShadowSDF"

#define P_INNER "Filter.ShadowSDF.Inner"
#define P_INNER_RANGE_MINIMUM "Filter.ShadowSDF.Inner.Range.Minimum"
#define P_INNER_RANGE_MAXIMUM "Filter.ShadowSDF.Inner.Range.Maximum"
#define P_INNER_OFFSET_X "Filter.ShadowSDF.Inner.Offset.X"
#define P_INNER_OFFSET_Y "Filter.ShadowSDF.Inner.Offset.Y"
#define P_INNER_COLOR "Filter.ShadowSDF.Inner.Color"
#define P_INNER_ALPHA "Filter.ShadowSDF.Inner.Alpha"
#define P_OUTER "Filter.ShadowSDF.Outer"
#define P_OUTER_RANGE_MINIMUM "Filter.ShadowSDF.Outer.Range.Minimum"
#define P_OUTER_RANGE_MAXIMUM "Filter.ShadowSDF.Outer.Range.Maximum"
#define P_OUTER_OFFSET_X "Filter.ShadowSDF.Outer.Offset.X"
#define P_OUTER_OFFSET_Y "Filter.ShadowSDF.Outer.Offset.Y"
#define P_OUTER_COLOR "Filter.ShadowSDF.Outer.Color"
#define P_OUTER_ALPHA "Filter.ShadowSDF.Outer.Alpha"

// Initializer & Finalizer
INITIALIZER(filterShadowFactoryInitializer)
{
	initializerFunctions.push_back([] { filter::shadow_sdf::shadow_sdf_factory::initialize(); });
	finalizerFunctions.push_back([] { filter::shadow_sdf::shadow_sdf_factory::finalize(); });
}

bool filter::shadow_sdf::shadow_sdf_instance::cb_modified_inside(void*, obs_properties_t* props, obs_property*,
																 obs_data_t* settings)
{
	bool v = obs_data_get_bool(settings, P_INNER);
	obs_property_set_visible(obs_properties_get(props, P_INNER_RANGE_MINIMUM), v);
	obs_property_set_visible(obs_properties_get(props, P_INNER_RANGE_MAXIMUM), v);
	obs_property_set_visible(obs_properties_get(props, P_INNER_OFFSET_X), v);
	obs_property_set_visible(obs_properties_get(props, P_INNER_OFFSET_Y), v);
	obs_property_set_visible(obs_properties_get(props, P_INNER_COLOR), v);
	obs_property_set_visible(obs_properties_get(props, P_INNER_ALPHA), v);
	return true;
}

bool filter::shadow_sdf::shadow_sdf_instance::cb_modified_outside(void*, obs_properties_t* props, obs_property*,
																  obs_data_t* settings)
{
	bool v = obs_data_get_bool(settings, P_OUTER);
	obs_property_set_visible(obs_properties_get(props, P_OUTER_RANGE_MINIMUM), v);
	obs_property_set_visible(obs_properties_get(props, P_OUTER_RANGE_MAXIMUM), v);
	obs_property_set_visible(obs_properties_get(props, P_OUTER_OFFSET_X), v);
	obs_property_set_visible(obs_properties_get(props, P_OUTER_OFFSET_Y), v);
	obs_property_set_visible(obs_properties_get(props, P_OUTER_COLOR), v);
	obs_property_set_visible(obs_properties_get(props, P_OUTER_ALPHA), v);
	return true;
}

filter::shadow_sdf::shadow_sdf_instance::shadow_sdf_instance(obs_data_t* settings, obs_source_t* self)
{
	this->m_self      = self;
	this->m_input     = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
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

filter::shadow_sdf::shadow_sdf_instance::~shadow_sdf_instance() {}

obs_properties_t* filter::shadow_sdf::shadow_sdf_instance::get_properties()
{
	obs_properties_t* props = obs_properties_create();
	obs_property_t*   p     = nullptr;

	p = obs_properties_add_bool(props, P_INNER, P_TRANSLATE(P_INNER));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_INNER)));
	obs_property_set_modified_callback2(p, cb_modified_inside, this);

	p = obs_properties_add_float_slider(props, P_INNER_RANGE_MINIMUM, P_TRANSLATE(P_INNER_RANGE_MINIMUM), 0.0, 16.0,
										0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_INNER_RANGE_MINIMUM)));

	p = obs_properties_add_float_slider(props, P_INNER_RANGE_MAXIMUM, P_TRANSLATE(P_INNER_RANGE_MAXIMUM), 0.0, 16.0,
										0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_INNER_RANGE_MAXIMUM)));

	p = obs_properties_add_float_slider(props, P_INNER_OFFSET_X, P_TRANSLATE(P_INNER_OFFSET_X), -100.0, 100.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_INNER_OFFSET_X)));

	p = obs_properties_add_float_slider(props, P_INNER_OFFSET_Y, P_TRANSLATE(P_INNER_OFFSET_Y), -100.0, 100.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_INNER_OFFSET_Y)));

	p = obs_properties_add_color(props, P_INNER_COLOR, P_TRANSLATE(P_INNER_COLOR));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_INNER_COLOR)));

	p = obs_properties_add_float_slider(props, P_INNER_ALPHA, P_TRANSLATE(P_INNER_ALPHA), 0.0, 100.0, 0.1);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_INNER_ALPHA)));

	p = obs_properties_add_bool(props, P_OUTER, P_TRANSLATE(P_OUTER));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_OUTER)));
	obs_property_set_modified_callback2(p, cb_modified_outside, this);

	p = obs_properties_add_float_slider(props, P_OUTER_RANGE_MINIMUM, P_TRANSLATE(P_OUTER_RANGE_MINIMUM), 0.0, 16.0,
										0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_OUTER_RANGE_MINIMUM)));

	p = obs_properties_add_float_slider(props, P_OUTER_RANGE_MAXIMUM, P_TRANSLATE(P_OUTER_RANGE_MAXIMUM), 0.0, 16.0,
										0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_OUTER_RANGE_MAXIMUM)));

	p = obs_properties_add_float_slider(props, P_OUTER_OFFSET_X, P_TRANSLATE(P_OUTER_OFFSET_X), -100.0, 100.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_OUTER_OFFSET_X)));

	p = obs_properties_add_float_slider(props, P_OUTER_OFFSET_Y, P_TRANSLATE(P_OUTER_OFFSET_Y), -100.0, 100.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_OUTER_OFFSET_Y)));

	p = obs_properties_add_color(props, P_OUTER_COLOR, P_TRANSLATE(P_OUTER_COLOR));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_OUTER_COLOR)));

	p = obs_properties_add_float_slider(props, P_OUTER_ALPHA, P_TRANSLATE(P_OUTER_ALPHA), 0.0, 100.0, 0.1);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_OUTER_ALPHA)));

	return props;
}

void filter::shadow_sdf::shadow_sdf_instance::update(obs_data_t* data)
{
	this->m_inner_shadow    = obs_data_get_bool(data, P_INNER);
	this->m_inner_range_min = float_t(obs_data_get_double(data, P_INNER_RANGE_MINIMUM));
	this->m_inner_range_max = float_t(obs_data_get_double(data, P_INNER_RANGE_MAXIMUM));
	if (this->m_inner_range_max < this->m_inner_range_min) {
		std::swap(this->m_inner_range_max, this->m_inner_range_min);
	}
	this->m_inner_offset_x = float_t(obs_data_get_double(data, P_INNER_OFFSET_X));
	this->m_inner_offset_y = float_t(obs_data_get_double(data, P_INNER_OFFSET_Y));
	this->m_inner_color    = (obs_data_get_int(data, P_INNER_COLOR) & 0x00FFFFFF)
						  | (int32_t(obs_data_get_double(data, P_INNER_ALPHA) * 2.55) << 24);

	this->m_outer_shadow    = obs_data_get_bool(data, P_OUTER);
	this->m_outer_range_min = float_t(obs_data_get_double(data, P_OUTER_RANGE_MINIMUM));
	this->m_outer_range_max = float_t(obs_data_get_double(data, P_OUTER_RANGE_MAXIMUM));
	if (this->m_outer_range_max < this->m_outer_range_min) {
		std::swap(this->m_outer_range_max, this->m_outer_range_min);
	}
	this->m_outer_offset_x = float_t(obs_data_get_double(data, P_OUTER_OFFSET_X));
	this->m_outer_offset_y = float_t(obs_data_get_double(data, P_OUTER_OFFSET_Y));
	this->m_outer_color    = (obs_data_get_int(data, P_OUTER_COLOR) & 0x00FFFFFF)
						  | (int32_t(obs_data_get_double(data, P_OUTER_ALPHA) * 2.55) << 24);
}

uint32_t filter::shadow_sdf::shadow_sdf_instance::get_width()
{
	return uint32_t(0);
}

uint32_t filter::shadow_sdf::shadow_sdf_instance::get_height()
{
	return uint32_t(0);
}

void filter::shadow_sdf::shadow_sdf_instance::activate() {}

void filter::shadow_sdf::shadow_sdf_instance::deactivate() {}

void filter::shadow_sdf::shadow_sdf_instance::video_tick(float time)
{
	this->m_tick += time;
}

void filter::shadow_sdf::shadow_sdf_instance::video_render(gs_effect_t*)
{
	obs_source_t* parent            = obs_filter_get_parent(this->m_self);
	obs_source_t* target            = obs_filter_get_target(this->m_self);
	uint32_t      baseW             = obs_source_get_base_width(target);
	uint32_t      baseH             = obs_source_get_base_height(target);
	gs_effect_t*  default_effect    = obs_get_base_effect(obs_base_effect::OBS_EFFECT_DEFAULT);
	vec4          color_transparent;
	vec4_zero(&color_transparent);

	if (!parent || !target || (baseW == 0) || (baseH == 0)) {
		obs_source_skip_video_filter(this->m_self);
		return;
	}

	try {
		if (this->m_tick != this->m_last_tick) {
			// Store input texture.
			{
				auto op = m_input->render(baseW, baseH);
				gs_ortho(0, (float)baseW, 0, (float)baseH, -1, 1);
				gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &color_transparent, 0, 0);
				gs_set_cull_mode(GS_NEITHER);
				gs_reset_blend_state();
				gs_enable_blending(false);
				gs_blend_function(GS_BLEND_SRCALPHA, GS_BLEND_ZERO);
				gs_enable_depth_test(false);
				gs_enable_stencil_test(false);
				gs_enable_stencil_write(false);
				gs_enable_color(true, true, true, true);

				if (obs_source_process_filter_begin(this->m_self, GS_RGBA, OBS_ALLOW_DIRECT_RENDERING)) {
					obs_source_process_filter_end(this->m_self, default_effect, baseW, baseH);
				} else {
					throw std::runtime_error("failed to process source");
				}
			}
			m_input->get_texture(this->m_source_texture);
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
					filter::shadow_sdf::shadow_sdf_factory::get()->get_sdf_generator_effect();
				if (!sdf_effect) {
					throw std::runtime_error("SDF Effect no loaded");
				}

				{
					auto op = m_sdf_write->render(baseW, baseH);
					gs_ortho(0, (float)baseW, 0, (float)baseH, -1, 1);
					gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &color_transparent, 0, 0);
					gs_set_cull_mode(GS_NEITHER);
					gs_reset_blend_state();
					gs_enable_blending(false);
					gs_enable_depth_test(false);
					gs_enable_stencil_test(false);
					gs_enable_stencil_write(false);
					gs_enable_color(true, true, true, true);

					sdf_effect->get_parameter("_image").set_texture(this->m_source_texture);
					sdf_effect->get_parameter("_size").set_float2(float_t(baseW), float_t(baseH));
					sdf_effect->get_parameter("_sdf").set_texture(this->m_sdf_texture);
					sdf_effect->get_parameter("_threshold").set_float(0.5);

					while (gs_effect_loop(sdf_effect->get_object(), "Draw")) {
						gs_draw_sprite(this->m_sdf_texture->get_object(), 0, baseW, baseH);
					}
				}
				this->m_sdf_write.swap(this->m_sdf_read);
				this->m_sdf_read->get_texture(this->m_sdf_texture);
				if (!this->m_sdf_texture) {
					throw std::runtime_error("SDF Backbuffer empty");
				}
			}
		}

		{
			std::shared_ptr<gs::effect> shadow_effect =
				filter::shadow_sdf::shadow_sdf_factory::get()->get_sdf_shadow_effect();
			if (!shadow_effect) {
				throw std::runtime_error("Shadow Effect no loaded");
			}

			gs_set_cull_mode(GS_NEITHER);
			gs_reset_blend_state();
			gs_enable_blending(true);
			gs_blend_function(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA);
			gs_enable_depth_test(false);
			gs_enable_stencil_test(false);
			gs_enable_stencil_write(false);
			gs_enable_color(true, true, true, true);

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
		}
	} catch (...) {
		gs_reset_blend_state();
		gs_enable_depth_test(false);
		obs_source_skip_video_filter(this->m_self);
		return;
	}
	gs_reset_blend_state();
	gs_enable_depth_test(false);
	this->m_last_tick = this->m_tick;
}

filter::shadow_sdf::shadow_sdf_factory::shadow_sdf_factory()
{
	memset(&source_info, 0, sizeof(obs_source_info));
	source_info.id             = "obs-stream-effects-filter-shadow-sdf";
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

filter::shadow_sdf::shadow_sdf_factory::~shadow_sdf_factory() {}

void filter::shadow_sdf::shadow_sdf_factory::on_list_fill()
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

void filter::shadow_sdf::shadow_sdf_factory::on_list_empty()
{
	sdf_generator_effect.reset();
	sdf_shadow_effect.reset();
}

void* filter::shadow_sdf::shadow_sdf_factory::create(obs_data_t* data, obs_source_t* parent)
{
	if (get()->sources.empty()) {
		get()->on_list_fill();
	}
	filter::shadow_sdf::shadow_sdf_instance* ptr = new filter::shadow_sdf::shadow_sdf_instance(data, parent);
	get()->sources.push_back(ptr);
	return ptr;
}

void filter::shadow_sdf::shadow_sdf_factory::destroy(void* inptr)
{
	filter::shadow_sdf::shadow_sdf_instance* ptr = reinterpret_cast<filter::shadow_sdf::shadow_sdf_instance*>(inptr);
	get()->sources.remove(ptr);
	if (get()->sources.empty()) {
		get()->on_list_empty();
	}
}

void filter::shadow_sdf::shadow_sdf_factory::get_defaults(obs_data_t* data)
{
	obs_data_set_bool(data, P_INNER, false);
	obs_data_set_double(data, P_INNER_RANGE_MINIMUM, 0.0);
	obs_data_set_double(data, P_INNER_RANGE_MAXIMUM, 4.0);
	obs_data_set_double(data, P_INNER_OFFSET_X, 0.0);
	obs_data_set_double(data, P_INNER_OFFSET_Y, 0.0);
	obs_data_set_int(data, P_INNER_COLOR, 0x00000000);
	obs_data_set_double(data, P_INNER_ALPHA, 100.0);

	obs_data_set_bool(data, P_OUTER, false);
	obs_data_set_double(data, P_OUTER_RANGE_MINIMUM, 0.0);
	obs_data_set_double(data, P_OUTER_RANGE_MAXIMUM, 4.0);
	obs_data_set_double(data, P_OUTER_OFFSET_X, 0.0);
	obs_data_set_double(data, P_OUTER_OFFSET_Y, 0.0);
	obs_data_set_int(data, P_OUTER_COLOR, 0x00000000);
	obs_data_set_double(data, P_OUTER_ALPHA, 100.0);
}

obs_properties_t* filter::shadow_sdf::shadow_sdf_factory::get_properties(void* inptr)
{
	return reinterpret_cast<filter::shadow_sdf::shadow_sdf_instance*>(inptr)->get_properties();
}

void filter::shadow_sdf::shadow_sdf_factory::update(void* inptr, obs_data_t* settings)
{
	reinterpret_cast<filter::shadow_sdf::shadow_sdf_instance*>(inptr)->update(settings);
}

const char* filter::shadow_sdf::shadow_sdf_factory::get_name(void*)
{
	return P_TRANSLATE(SOURCE_NAME);
}

uint32_t filter::shadow_sdf::shadow_sdf_factory::get_width(void* inptr)
{
	return reinterpret_cast<filter::shadow_sdf::shadow_sdf_instance*>(inptr)->get_width();
}

uint32_t filter::shadow_sdf::shadow_sdf_factory::get_height(void* inptr)
{
	return reinterpret_cast<filter::shadow_sdf::shadow_sdf_instance*>(inptr)->get_height();
}

void filter::shadow_sdf::shadow_sdf_factory::activate(void* inptr)
{
	reinterpret_cast<filter::shadow_sdf::shadow_sdf_instance*>(inptr)->activate();
}

void filter::shadow_sdf::shadow_sdf_factory::deactivate(void* inptr)
{
	reinterpret_cast<filter::shadow_sdf::shadow_sdf_instance*>(inptr)->deactivate();
}

void filter::shadow_sdf::shadow_sdf_factory::video_tick(void* inptr, float delta)
{
	reinterpret_cast<filter::shadow_sdf::shadow_sdf_instance*>(inptr)->video_tick(delta);
}

void filter::shadow_sdf::shadow_sdf_factory::video_render(void* inptr, gs_effect_t* effect)
{
	reinterpret_cast<filter::shadow_sdf::shadow_sdf_instance*>(inptr)->video_render(effect);
}

std::shared_ptr<gs::effect> filter::shadow_sdf::shadow_sdf_factory::get_sdf_generator_effect()
{
	return sdf_generator_effect;
}

std::shared_ptr<gs::effect> filter::shadow_sdf::shadow_sdf_factory::get_sdf_shadow_effect()
{
	return sdf_shadow_effect;
}

static filter::shadow_sdf::shadow_sdf_factory* factory_instance = nullptr;

void filter::shadow_sdf::shadow_sdf_factory::initialize()
{
	factory_instance = new filter::shadow_sdf::shadow_sdf_factory();
}

void filter::shadow_sdf::shadow_sdf_factory::finalize()
{
	delete factory_instance;
}

filter::shadow_sdf::shadow_sdf_factory* filter::shadow_sdf::shadow_sdf_factory::get()
{
	return factory_instance;
}
