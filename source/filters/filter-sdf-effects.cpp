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
#include "obs/gs/gs-helper.hpp"
#include "strings.hpp"

#define LOG_PREFIX "<filter-sdf-effects> "

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

#define P_GLOW_INNER "Filter.SDFEffects.Glow.Inner"
#define P_GLOW_INNER_COLOR "Filter.SDFEffects.Glow.Inner.Color"
#define P_GLOW_INNER_ALPHA "Filter.SDFEffects.Glow.Inner.Alpha"
#define P_GLOW_INNER_WIDTH "Filter.SDFEffects.Glow.Inner.Width"
#define P_GLOW_INNER_SHARPNESS "Filter.SDFEffects.Glow.Inner.Sharpness"

#define P_GLOW_OUTER "Filter.SDFEffects.Glow.Outer"
#define P_GLOW_OUTER_COLOR "Filter.SDFEffects.Glow.Outer.Color"
#define P_GLOW_OUTER_ALPHA "Filter.SDFEffects.Glow.Outer.Alpha"
#define P_GLOW_OUTER_WIDTH "Filter.SDFEffects.Glow.Outer.Width"
#define P_GLOW_OUTER_SHARPNESS "Filter.SDFEffects.Glow.Outer.Sharpness"

#define P_OUTLINE "Filter.SDFEffects.Outline"
#define P_OUTLINE_COLOR "Filter.SDFEffects.Outline.Color"
#define P_OUTLINE_ALPHA "Filter.SDFEffects.Outline.Alpha"
#define P_OUTLINE_WIDTH "Filter.SDFEffects.Outline.Width"
#define P_OUTLINE_OFFSET "Filter.SDFEffects.Outline.Offset"
#define P_OUTLINE_SHARPNESS "Filter.SDFEffects.Outline.Sharpness"

#define P_SDF_SCALE "Filter.SDFEffects.SDF.Scale"
#define P_SDF_THRESHOLD "Filter.SDFEffects.SDF.Threshold"

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
	auto gctx = gs::context();

	std::pair<const char*, std::shared_ptr<gs::effect>&> load_arr[] = {
		{"effects/sdf/sdf-producer.effect", this->sdf_producer_effect},
		{"effects/sdf/sdf-consumer.effect", this->sdf_consumer_effect},
	};
	for (auto& kv : load_arr) {
		char* path = obs_module_file(kv.first);
		if (!path) {
			P_LOG_ERROR(LOG_PREFIX "Unable to load effect '%s' as file is missing or locked.", kv.first);
			continue;
		}
		try {
			kv.second = std::make_shared<gs::effect>(path);
		} catch (std::exception ex) {
			P_LOG_ERROR(LOG_PREFIX "Failed to load effect '%s' (located at '%s') with error(s): %s", kv.first, path,
						ex.what());
		}
		bfree(path);
	}
}

void filter::sdf_effects::sdf_effects_factory::on_list_empty()
{
	this->sdf_producer_effect.reset();
	this->sdf_consumer_effect.reset();
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
	obs_data_set_default_bool(data, P_SHADOW_OUTER, false);
	obs_data_set_default_int(data, P_SHADOW_OUTER_COLOR, 0x00000000);
	obs_data_set_default_double(data, P_SHADOW_OUTER_ALPHA, 100.0);
	obs_data_set_default_double(data, P_SHADOW_OUTER_RANGE_MINIMUM, 0.0);
	obs_data_set_default_double(data, P_SHADOW_OUTER_RANGE_MAXIMUM, 4.0);
	obs_data_set_default_double(data, P_SHADOW_OUTER_OFFSET_X, 0.0);
	obs_data_set_default_double(data, P_SHADOW_OUTER_OFFSET_Y, 0.0);

	obs_data_set_default_bool(data, P_SHADOW_INNER, false);
	obs_data_set_default_int(data, P_SHADOW_INNER_COLOR, 0x00000000);
	obs_data_set_default_double(data, P_SHADOW_INNER_ALPHA, 100.0);
	obs_data_set_default_double(data, P_SHADOW_INNER_RANGE_MINIMUM, 0.0);
	obs_data_set_default_double(data, P_SHADOW_INNER_RANGE_MAXIMUM, 4.0);
	obs_data_set_default_double(data, P_SHADOW_INNER_OFFSET_X, 0.0);
	obs_data_set_default_double(data, P_SHADOW_INNER_OFFSET_Y, 0.0);

	obs_data_set_default_bool(data, P_GLOW_OUTER, false);
	obs_data_set_default_int(data, P_GLOW_OUTER_COLOR, 0xFFFFFFFF);
	obs_data_set_default_double(data, P_GLOW_OUTER_ALPHA, 100.0);
	obs_data_set_default_double(data, P_GLOW_OUTER_WIDTH, 4.0);
	obs_data_set_default_double(data, P_GLOW_OUTER_SHARPNESS, 50.0);

	obs_data_set_default_bool(data, P_GLOW_INNER, false);
	obs_data_set_default_int(data, P_GLOW_INNER_COLOR, 0xFFFFFFFF);
	obs_data_set_default_double(data, P_GLOW_INNER_ALPHA, 100.0);
	obs_data_set_default_double(data, P_GLOW_INNER_WIDTH, 4.0);
	obs_data_set_default_double(data, P_GLOW_INNER_SHARPNESS, 50.0);

	obs_data_set_default_bool(data, P_OUTLINE, false);
	obs_data_set_default_int(data, P_OUTLINE_COLOR, 0x00000000);
	obs_data_set_default_double(data, P_OUTLINE_ALPHA, 100.0);
	obs_data_set_default_double(data, P_OUTLINE_WIDTH, 4.0);
	obs_data_set_default_double(data, P_OUTLINE_OFFSET, 0.0);
	obs_data_set_default_double(data, P_OUTLINE_SHARPNESS, 50.0);

	obs_data_set_default_bool(data, S_ADVANCED, false);
	obs_data_set_default_double(data, P_SDF_SCALE, 100.0);
	obs_data_set_default_double(data, P_SDF_THRESHOLD, 50.0);
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

std::shared_ptr<gs::effect> filter::sdf_effects::sdf_effects_factory::get_sdf_producer_effect()
{
	return this->sdf_producer_effect;
}

std::shared_ptr<gs::effect> filter::sdf_effects::sdf_effects_factory::get_sdf_consumer_effect()
{
	return this->sdf_consumer_effect;
}

bool filter::sdf_effects::sdf_effects_instance::cb_modified_shadow_inside(void*, obs_properties_t* props, obs_property*,
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

bool filter::sdf_effects::sdf_effects_instance::cb_modified_shadow_outside(void*, obs_properties_t*   props,
																		   obs_property*, obs_data_t* settings)
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

bool filter::sdf_effects::sdf_effects_instance::cb_modified_glow_inside(void*, obs_properties_t* props, obs_property*,
																		obs_data_t* settings)
{
	bool v = obs_data_get_bool(settings, P_GLOW_INNER);
	obs_property_set_visible(obs_properties_get(props, P_GLOW_INNER_COLOR), v);
	obs_property_set_visible(obs_properties_get(props, P_GLOW_INNER_ALPHA), v);
	obs_property_set_visible(obs_properties_get(props, P_GLOW_INNER_WIDTH), v);
	obs_property_set_visible(obs_properties_get(props, P_GLOW_INNER_SHARPNESS), v);
	return true;
}

bool filter::sdf_effects::sdf_effects_instance::cb_modified_glow_outside(void*, obs_properties_t* props, obs_property*,
																		 obs_data_t* settings)
{
	bool v = obs_data_get_bool(settings, P_GLOW_OUTER);
	obs_property_set_visible(obs_properties_get(props, P_GLOW_OUTER_COLOR), v);
	obs_property_set_visible(obs_properties_get(props, P_GLOW_OUTER_ALPHA), v);
	obs_property_set_visible(obs_properties_get(props, P_GLOW_OUTER_WIDTH), v);
	obs_property_set_visible(obs_properties_get(props, P_GLOW_OUTER_SHARPNESS), v);
	return true;
}

bool filter::sdf_effects::sdf_effects_instance::cb_modified_outline(void*, obs_properties_t* props, obs_property*,
																	obs_data_t* settings)
{
	bool v = obs_data_get_bool(settings, P_OUTLINE);
	obs_property_set_visible(obs_properties_get(props, P_OUTLINE_COLOR), v);
	obs_property_set_visible(obs_properties_get(props, P_OUTLINE_ALPHA), v);
	obs_property_set_visible(obs_properties_get(props, P_OUTLINE_WIDTH), v);
	obs_property_set_visible(obs_properties_get(props, P_OUTLINE_OFFSET), v);
	obs_property_set_visible(obs_properties_get(props, P_OUTLINE_SHARPNESS), v);
	return true;
}

bool filter::sdf_effects::sdf_effects_instance::cb_modified_advanced(void*, obs_properties_t* props, obs_property*,
																	 obs_data_t* settings)
{
	bool show_advanced = obs_data_get_bool(settings, S_ADVANCED);
	obs_property_set_visible(obs_properties_get(props, P_SDF_SCALE), show_advanced);
	obs_property_set_visible(obs_properties_get(props, P_SDF_THRESHOLD), show_advanced);
	return true;
}

filter::sdf_effects::sdf_effects_instance::sdf_effects_instance(obs_data_t* settings, obs_source_t* self)
	: m_self(self), m_source_rendered(false), m_sdf_scale(1.0)
{
	{
		auto gctx        = gs::context();
		vec4 transparent = {0};

		this->m_source_rt = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
		this->m_sdf_write = std::make_shared<gs::rendertarget>(GS_RGBA32F, GS_ZS_NONE);
		this->m_sdf_read  = std::make_shared<gs::rendertarget>(GS_RGBA32F, GS_ZS_NONE);
		this->m_output_rt = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);

		std::shared_ptr<gs::rendertarget> initialize_rts[] = {this->m_source_rt, this->m_sdf_write, this->m_sdf_read,
															  this->m_output_rt};
		for (auto rt : initialize_rts) {
			auto op = rt->render(1, 1);
			gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &transparent, 0, 0);
		}
	}
	update(settings);
}

filter::sdf_effects::sdf_effects_instance::~sdf_effects_instance() {}

obs_properties_t* filter::sdf_effects::sdf_effects_instance::get_properties()
{
	obs_properties_t* props = obs_properties_create();
	obs_property_t*   p     = nullptr;

	{
		p = obs_properties_add_bool(props, P_SHADOW_OUTER, P_TRANSLATE(P_SHADOW_OUTER));
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_OUTER)));
		obs_property_set_modified_callback2(p, cb_modified_shadow_outside, this);
		p = obs_properties_add_float_slider(props, P_SHADOW_OUTER_RANGE_MINIMUM,
											P_TRANSLATE(P_SHADOW_OUTER_RANGE_MINIMUM), -16.0, 16.0, 0.01);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_OUTER_RANGE_MINIMUM)));
		p = obs_properties_add_float_slider(props, P_SHADOW_OUTER_RANGE_MAXIMUM,
											P_TRANSLATE(P_SHADOW_OUTER_RANGE_MAXIMUM), -16.0, 16.0, 0.01);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_OUTER_RANGE_MAXIMUM)));
		p = obs_properties_add_float_slider(props, P_SHADOW_OUTER_OFFSET_X, P_TRANSLATE(P_SHADOW_OUTER_OFFSET_X),
											-100.0, 100.0, 0.01);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_OUTER_OFFSET_X)));
		p = obs_properties_add_float_slider(props, P_SHADOW_OUTER_OFFSET_Y, P_TRANSLATE(P_SHADOW_OUTER_OFFSET_Y),
											-100.0, 100.0, 0.01);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_OUTER_OFFSET_Y)));
		p = obs_properties_add_color(props, P_SHADOW_OUTER_COLOR, P_TRANSLATE(P_SHADOW_OUTER_COLOR));
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_OUTER_COLOR)));
		p = obs_properties_add_float_slider(props, P_SHADOW_OUTER_ALPHA, P_TRANSLATE(P_SHADOW_OUTER_ALPHA), 0.0, 100.0,
											0.1);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_OUTER_ALPHA)));
	}

	{
		p = obs_properties_add_bool(props, P_SHADOW_INNER, P_TRANSLATE(P_SHADOW_INNER));
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_INNER)));
		obs_property_set_modified_callback2(p, cb_modified_shadow_inside, this);
		p = obs_properties_add_float_slider(props, P_SHADOW_INNER_RANGE_MINIMUM,
											P_TRANSLATE(P_SHADOW_INNER_RANGE_MINIMUM), -16.0, 16.0, 0.01);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_INNER_RANGE_MINIMUM)));
		p = obs_properties_add_float_slider(props, P_SHADOW_INNER_RANGE_MAXIMUM,
											P_TRANSLATE(P_SHADOW_INNER_RANGE_MAXIMUM), -16.0, 16.0, 0.01);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_INNER_RANGE_MAXIMUM)));
		p = obs_properties_add_float_slider(props, P_SHADOW_INNER_OFFSET_X, P_TRANSLATE(P_SHADOW_INNER_OFFSET_X),
											-100.0, 100.0, 0.01);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_INNER_OFFSET_X)));
		p = obs_properties_add_float_slider(props, P_SHADOW_INNER_OFFSET_Y, P_TRANSLATE(P_SHADOW_INNER_OFFSET_Y),
											-100.0, 100.0, 0.01);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_INNER_OFFSET_Y)));
		p = obs_properties_add_color(props, P_SHADOW_INNER_COLOR, P_TRANSLATE(P_SHADOW_INNER_COLOR));
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_INNER_COLOR)));
		p = obs_properties_add_float_slider(props, P_SHADOW_INNER_ALPHA, P_TRANSLATE(P_SHADOW_INNER_ALPHA), 0.0, 100.0,
											0.1);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SHADOW_INNER_ALPHA)));
	}

	{
		p = obs_properties_add_bool(props, P_GLOW_OUTER, P_TRANSLATE(P_GLOW_OUTER));
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_GLOW_OUTER)));
		obs_property_set_modified_callback2(p, cb_modified_glow_outside, this);

		p = obs_properties_add_color(props, P_GLOW_OUTER_COLOR, P_TRANSLATE(P_GLOW_OUTER_COLOR));
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_GLOW_OUTER_COLOR)));
		p = obs_properties_add_float_slider(props, P_GLOW_OUTER_ALPHA, P_TRANSLATE(P_GLOW_OUTER_ALPHA), 0.0, 100.0,
											0.1);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_GLOW_OUTER_ALPHA)));

		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_GLOW_OUTER_WIDTH)));
		p = obs_properties_add_float_slider(props, P_GLOW_OUTER_WIDTH, P_TRANSLATE(P_GLOW_OUTER_WIDTH), 0.0, 16.0,
											0.01);

		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_GLOW_OUTER_SHARPNESS)));
		p = obs_properties_add_float_slider(props, P_GLOW_OUTER_SHARPNESS, P_TRANSLATE(P_GLOW_OUTER_SHARPNESS), 0.00,
											100.0, 0.01);
	}

	{
		p = obs_properties_add_bool(props, P_GLOW_INNER, P_TRANSLATE(P_GLOW_INNER));
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_GLOW_INNER)));
		obs_property_set_modified_callback2(p, cb_modified_glow_inside, this);

		p = obs_properties_add_color(props, P_GLOW_INNER_COLOR, P_TRANSLATE(P_GLOW_INNER_COLOR));
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_GLOW_INNER_COLOR)));
		p = obs_properties_add_float_slider(props, P_GLOW_INNER_ALPHA, P_TRANSLATE(P_GLOW_INNER_ALPHA), 0.0, 100.0,
											0.1);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_GLOW_INNER_ALPHA)));

		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_GLOW_INNER_WIDTH)));
		p = obs_properties_add_float_slider(props, P_GLOW_INNER_WIDTH, P_TRANSLATE(P_GLOW_INNER_WIDTH), 0.0, 16.0,
											0.01);

		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_GLOW_INNER_SHARPNESS)));
		p = obs_properties_add_float_slider(props, P_GLOW_INNER_SHARPNESS, P_TRANSLATE(P_GLOW_INNER_SHARPNESS), 0.00,
											100.0, 0.01);
	}

	{
		p = obs_properties_add_bool(props, P_OUTLINE, P_TRANSLATE(P_OUTLINE));
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_OUTLINE)));
		obs_property_set_modified_callback2(p, cb_modified_outline, this);
		p = obs_properties_add_color(props, P_OUTLINE_COLOR, P_TRANSLATE(P_OUTLINE_COLOR));
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_OUTLINE_COLOR)));
		p = obs_properties_add_float_slider(props, P_OUTLINE_ALPHA, P_TRANSLATE(P_OUTLINE_ALPHA), 0.0, 100.0, 0.1);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_OUTLINE_ALPHA)));

		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_OUTLINE_WIDTH)));
		p = obs_properties_add_float_slider(props, P_OUTLINE_WIDTH, P_TRANSLATE(P_OUTLINE_WIDTH), 0.0, 16.0, 0.01);

		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_OUTLINE_OFFSET)));
		p = obs_properties_add_float_slider(props, P_OUTLINE_OFFSET, P_TRANSLATE(P_OUTLINE_OFFSET), -16.0, 16.0, 0.01);

		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_OUTLINE_SHARPNESS)));
		p = obs_properties_add_float_slider(props, P_OUTLINE_SHARPNESS, P_TRANSLATE(P_OUTLINE_SHARPNESS), 0.00, 100.0,
											0.01);
	}

	{
		p = obs_properties_add_bool(props, S_ADVANCED, P_TRANSLATE(S_ADVANCED));
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(S_ADVANCED)));
		obs_property_set_modified_callback2(p, cb_modified_advanced, this);

		p = obs_properties_add_float_slider(props, P_SDF_SCALE, P_TRANSLATE(P_SDF_SCALE), 0.1, 500.0, 0.1);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SDF_SCALE)));

		p = obs_properties_add_float_slider(props, P_SDF_THRESHOLD, P_TRANSLATE(P_SDF_THRESHOLD), 0.0, 100.0, 0.01);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SDF_THRESHOLD)));
	}

	return props;
}

void filter::sdf_effects::sdf_effects_instance::update(obs_data_t* data)
{
	{
		this->m_outer_shadow =
			obs_data_get_bool(data, P_SHADOW_OUTER) && (obs_data_get_double(data, P_SHADOW_OUTER_ALPHA) >= std::numeric_limits<double_t>::epsilon());
		{
			union {
				uint32_t color;
				uint8_t  channel[4];
				struct c {
					uint8_t r, g, b, a;
				} c;
			};
			color                        = uint32_t(obs_data_get_int(data, P_SHADOW_OUTER_COLOR));
			this->m_outer_shadow_color.x = float_t(c.r / 255.0);
			this->m_outer_shadow_color.y = float_t(c.g / 255.0);
			this->m_outer_shadow_color.z = float_t(c.b / 255.0);
			this->m_outer_shadow_color.w = float_t(obs_data_get_double(data, P_SHADOW_OUTER_ALPHA) / 100.0);
		}
		this->m_outer_shadow_range_min = float_t(obs_data_get_double(data, P_SHADOW_OUTER_RANGE_MINIMUM));
		this->m_outer_shadow_range_max = float_t(obs_data_get_double(data, P_SHADOW_OUTER_RANGE_MAXIMUM));
		this->m_outer_shadow_offset_x  = float_t(obs_data_get_double(data, P_SHADOW_OUTER_OFFSET_X));
		this->m_outer_shadow_offset_y  = float_t(obs_data_get_double(data, P_SHADOW_OUTER_OFFSET_Y));
	}

	{
		this->m_inner_shadow =
			obs_data_get_bool(data, P_SHADOW_INNER) && (obs_data_get_double(data, P_SHADOW_INNER_ALPHA) >= std::numeric_limits<double_t>::epsilon());
		{
			union {
				uint32_t color;
				uint8_t  channel[4];
				struct c {
					uint8_t r, g, b, a;
				} c;
			};
			color                        = uint32_t(obs_data_get_int(data, P_SHADOW_INNER_COLOR));
			this->m_inner_shadow_color.x = float_t(c.r / 255.0);
			this->m_inner_shadow_color.y = float_t(c.g / 255.0);
			this->m_inner_shadow_color.z = float_t(c.b / 255.0);
			this->m_inner_shadow_color.w = float_t(obs_data_get_double(data, P_SHADOW_INNER_ALPHA) / 100.0);
		}
		this->m_inner_shadow_range_min = float_t(obs_data_get_double(data, P_SHADOW_INNER_RANGE_MINIMUM));
		this->m_inner_shadow_range_max = float_t(obs_data_get_double(data, P_SHADOW_INNER_RANGE_MAXIMUM));
		this->m_inner_shadow_offset_x  = float_t(obs_data_get_double(data, P_SHADOW_INNER_OFFSET_X));
		this->m_inner_shadow_offset_y  = float_t(obs_data_get_double(data, P_SHADOW_INNER_OFFSET_Y));
	}

	{
		this->m_outer_glow =
			obs_data_get_bool(data, P_GLOW_OUTER) && (obs_data_get_double(data, P_GLOW_OUTER_ALPHA) >= std::numeric_limits<double_t>::epsilon());
		{
			union {
				uint32_t color;
				uint8_t  channel[4];
				struct c {
					uint8_t r, g, b, a;
				} c;
			};
			color                      = uint32_t(obs_data_get_int(data, P_GLOW_OUTER_COLOR));
			this->m_outer_glow_color.x = float_t(c.r / 255.0);
			this->m_outer_glow_color.y = float_t(c.g / 255.0);
			this->m_outer_glow_color.z = float_t(c.b / 255.0);
			this->m_outer_glow_color.w = float_t(obs_data_get_double(data, P_GLOW_OUTER_ALPHA) / 100.0);
		}
		this->m_outer_glow_width         = float_t(obs_data_get_double(data, P_GLOW_OUTER_WIDTH));
		this->m_outer_glow_sharpness     = float_t(obs_data_get_double(data, P_GLOW_OUTER_SHARPNESS) / 100.0);
		this->m_outer_glow_sharpness_inv = float_t(1.0f / (1.0f - this->m_outer_glow_sharpness));
		if (this->m_outer_glow_sharpness >= (1.0f - std::numeric_limits<float_t>::epsilon())) {
			this->m_outer_glow_sharpness = 1.0f - std::numeric_limits<float_t>::epsilon();
		}
	}

	{
		this->m_inner_glow =
			obs_data_get_bool(data, P_GLOW_INNER) && (obs_data_get_double(data, P_GLOW_INNER_ALPHA) >= std::numeric_limits<double_t>::epsilon());
		{
			union {
				uint32_t color;
				uint8_t  channel[4];
				struct c {
					uint8_t r, g, b, a;
				} c;
			};
			color                      = uint32_t(obs_data_get_int(data, P_GLOW_INNER_COLOR));
			this->m_inner_glow_color.x = float_t(c.r / 255.0);
			this->m_inner_glow_color.y = float_t(c.g / 255.0);
			this->m_inner_glow_color.z = float_t(c.b / 255.0);
			this->m_inner_glow_color.w = float_t(obs_data_get_double(data, P_GLOW_INNER_ALPHA) / 100.0);
		}
		this->m_inner_glow_width         = float_t(obs_data_get_double(data, P_GLOW_INNER_WIDTH));
		this->m_inner_glow_sharpness     = float_t(obs_data_get_double(data, P_GLOW_INNER_SHARPNESS) / 100.0);
		this->m_inner_glow_sharpness_inv = float_t(1.0f / (1.0f - this->m_inner_glow_sharpness));
		if (this->m_inner_glow_sharpness >= (1.0f - std::numeric_limits<float_t>::epsilon())) {
			this->m_inner_glow_sharpness = 1.0f - std::numeric_limits<float_t>::epsilon();
		}
	}

	{
		this->m_outline =
			obs_data_get_bool(data, P_OUTLINE) && (obs_data_get_double(data, P_OUTLINE_ALPHA) >= std::numeric_limits<double_t>::epsilon());
		{
			union {
				uint32_t color;
				uint8_t  channel[4];
				struct c {
					uint8_t r, g, b, a;
				} c;
			};
			color                   = uint32_t(obs_data_get_int(data, P_OUTLINE_COLOR));
			this->m_outline_color.x = float_t(c.r / 255.0);
			this->m_outline_color.y = float_t(c.g / 255.0);
			this->m_outline_color.z = float_t(c.b / 255.0);
			this->m_outline_color.w = float_t(obs_data_get_double(data, P_OUTLINE_ALPHA) / 100.0);
		}
		this->m_outline_width         = float_t(obs_data_get_double(data, P_OUTLINE_WIDTH));
		this->m_outline_offset        = float_t(obs_data_get_double(data, P_OUTLINE_OFFSET));
		this->m_outline_sharpness     = float_t(obs_data_get_double(data, P_OUTLINE_SHARPNESS) / 100.0);
		this->m_outline_sharpness_inv = float_t(1.0f / (1.0f - this->m_outline_sharpness));
		if (this->m_outline_sharpness >= (1.0f - std::numeric_limits<float_t>::epsilon())) {
			this->m_outline_sharpness = 1.0f - std::numeric_limits<float_t>::epsilon();
		}
	}

	this->m_sdf_scale     = double_t(obs_data_get_double(data, P_SDF_SCALE) / 100.0);
	this->m_sdf_threshold = float_t(obs_data_get_double(data, P_SDF_THRESHOLD) / 100.0);
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

void filter::sdf_effects::sdf_effects_instance::video_tick(float)
{
	uint32_t width  = 1;
	uint32_t height = 1;

	// Figure out the actual source size.
	do {
		obs_source_t* target = obs_filter_get_target(this->m_self);
		if (target == nullptr) {
			break;
		}

		// Grab width an height of the target source (child filter or source).
		width  = obs_source_get_width(target);
		height = obs_source_get_height(target);
	} while (false);

	this->m_source_rendered = false;
	this->m_output_rendered = false;
}

void filter::sdf_effects::sdf_effects_instance::video_render(gs_effect_t* effect)
{
	obs_source_t* parent         = obs_filter_get_parent(this->m_self);
	obs_source_t* target         = obs_filter_get_target(this->m_self);
	uint32_t      baseW          = obs_source_get_base_width(target);
	uint32_t      baseH          = obs_source_get_base_height(target);
	gs_effect_t*  final_effect   = effect ? effect : obs_get_base_effect(obs_base_effect::OBS_EFFECT_DEFAULT);
	gs_effect_t*  default_effect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_DEFAULT);

	if (!this->m_self || !parent || !target || !baseW || !baseH || !final_effect) {
		obs_source_skip_video_filter(this->m_self);
		return;
	}

	auto gctx              = gs::context();
	vec4 color_transparent = {0};
	vec4_zero(&color_transparent);

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
					obs_source_process_filter_end(this->m_self, final_effect, baseW, baseH);
				} else {
					throw std::runtime_error("failed to process source");
				}
			}
			this->m_source_rt->get_texture(this->m_source_texture);
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
					filter::sdf_effects::sdf_effects_factory::get()->get_sdf_producer_effect();
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
					auto op = this->m_sdf_write->render(uint32_t(sdfW), uint32_t(sdfH));
					gs_ortho(0, (float)sdfW, 0, (float)sdfH, -1, 1);
					gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &color_transparent, 0, 0);

					sdf_effect->get_parameter("_image").set_texture(this->m_source_texture);
					sdf_effect->get_parameter("_size").set_float2(float_t(sdfW), float_t(sdfH));
					sdf_effect->get_parameter("_sdf").set_texture(this->m_sdf_texture);
					sdf_effect->get_parameter("_threshold").set_float(this->m_sdf_threshold);

					while (gs_effect_loop(sdf_effect->get_object(), "Draw")) {
						gs_draw_sprite(this->m_sdf_texture->get_object(), 0, uint32_t(sdfW), uint32_t(sdfH));
					}
				}
				std::swap(this->m_sdf_read, this->m_sdf_write);
				this->m_sdf_read->get_texture(this->m_sdf_texture);
				if (!this->m_sdf_texture) {
					throw std::runtime_error("SDF Backbuffer empty");
				}
			}

			this->m_source_rendered = true;
		}

		gs_blend_state_pop();
	} catch (...) {
		gs_blend_state_pop();
		obs_source_skip_video_filter(this->m_self);
		return;
	}

	if (!this->m_output_rendered) {
		this->m_output_texture = this->m_source_texture;

		std::shared_ptr<gs::effect> consumer_effect =
			filter::sdf_effects::sdf_effects_factory::get()->get_sdf_consumer_effect();
		if (!consumer_effect) {
			obs_source_skip_video_filter(this->m_self);
			return;
		}

		gs_blend_state_push();
		gs_reset_blend_state();
		gs_enable_color(true, true, true, true);
		gs_enable_depth_test(false);
		gs_set_cull_mode(GS_NEITHER);

		// SDF Effects Stack:
		//   Normal Source
		//   Outer Shadow
		//   Inner Shadow
		//   Outer Glow
		//   Inner Glow
		//   Outline

		{ // Normal Source
			gs_enable_blending(false);
			gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);

			auto param = gs_effect_get_param_by_name(default_effect, "image");
			if (param) {
				gs_effect_set_texture(param, this->m_output_texture->get_object());
			}

			{
				auto op = this->m_output_rt->render(baseW, baseH);
				gs_ortho(0, 1, 0, 1, 0, 1);
				while (gs_effect_loop(default_effect, "Draw")) {
					gs_draw_sprite(0, 0, 1, 1);
				}
			}

			// Flip Buffers
			m_output_rt->get_texture(this->m_output_texture);
		}

		if (m_outer_shadow) {
			try {
				gs_enable_blending(true);
				gs_blend_function_separate(GS_BLEND_SRCALPHA, GS_BLEND_DSTALPHA, GS_BLEND_ONE, GS_BLEND_ONE);

				consumer_effect->get_parameter("pSDFTexture").set_texture(this->m_sdf_texture);
				consumer_effect->get_parameter("pSDFThreshold").set_float(this->m_sdf_threshold);
				consumer_effect->get_parameter("pImageTexture").set_texture(this->m_source_texture->get_object());
				consumer_effect->get_parameter("pShadowColor").set_float4(this->m_outer_shadow_color);
				consumer_effect->get_parameter("pShadowMin").set_float(this->m_outer_shadow_range_min);
				consumer_effect->get_parameter("pShadowMax").set_float(this->m_outer_shadow_range_max);
				consumer_effect->get_parameter("pShadowOffset")
					.set_float2(this->m_outer_shadow_offset_x / float_t(baseW),
								this->m_outer_shadow_offset_y / float_t(baseH));

				{
					auto op = this->m_output_rt->render(baseW, baseH);
					gs_ortho(0, 1, 0, 1, 0, 1);
					while (gs_effect_loop(consumer_effect->get_object(), "ShadowOuter")) {
						gs_draw_sprite(0, 0, 1, 1);
					}
				}

				// Flip Buffers
				m_output_rt->get_texture(this->m_output_texture);
			} catch (...) {
			}
		}

		if (m_inner_shadow) {
			try {
				gs_enable_blending(true);
				gs_blend_function_separate(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA, GS_BLEND_ONE, GS_BLEND_ONE);

				consumer_effect->get_parameter("pSDFTexture").set_texture(this->m_sdf_texture);
				consumer_effect->get_parameter("pSDFThreshold").set_float(this->m_sdf_threshold);
				consumer_effect->get_parameter("pImageTexture").set_texture(this->m_source_texture->get_object());
				consumer_effect->get_parameter("pShadowColor").set_float4(this->m_inner_shadow_color);
				consumer_effect->get_parameter("pShadowMin").set_float(this->m_inner_shadow_range_min);
				consumer_effect->get_parameter("pShadowMax").set_float(this->m_inner_shadow_range_max);
				consumer_effect->get_parameter("pShadowOffset")
					.set_float2(this->m_inner_shadow_offset_x / float_t(baseW),
								this->m_inner_shadow_offset_y / float_t(baseH));

				{
					auto op = this->m_output_rt->render(baseW, baseH);
					gs_ortho(0, 1, 0, 1, 0, 1);
					while (gs_effect_loop(consumer_effect->get_object(), "ShadowInner")) {
						gs_draw_sprite(0, 0, 1, 1);
					}
				}

				// Flip Buffers
				m_output_rt->get_texture(this->m_output_texture);
			} catch (...) {
			}
		}

		if (m_outer_glow) {
			try {
				gs_enable_blending(true);
				gs_blend_function_separate(GS_BLEND_SRCALPHA, GS_BLEND_DSTALPHA, GS_BLEND_ONE, GS_BLEND_ONE);

				consumer_effect->get_parameter("pSDFTexture").set_texture(this->m_sdf_texture);
				consumer_effect->get_parameter("pSDFThreshold").set_float(this->m_sdf_threshold);
				consumer_effect->get_parameter("pImageTexture").set_texture(this->m_source_texture->get_object());
				consumer_effect->get_parameter("pGlowColor").set_float4(this->m_outer_glow_color);
				consumer_effect->get_parameter("pGlowWidth").set_float(this->m_outer_glow_width);
				consumer_effect->get_parameter("pGlowSharpness").set_float(this->m_outer_glow_sharpness);
				consumer_effect->get_parameter("pGlowSharpnessInverse").set_float(this->m_outer_glow_sharpness_inv);

				{
					auto op = this->m_output_rt->render(baseW, baseH);
					gs_ortho(0, 1, 0, 1, 0, 1);
					while (gs_effect_loop(consumer_effect->get_object(), "GlowOuter")) {
						gs_draw_sprite(0, 0, 1, 1);
					}
				}

				// Flip Buffers
				m_output_rt->get_texture(this->m_output_texture);
			} catch (...) {
			}
		}

		if (m_inner_glow) {
			try {
				gs_enable_blending(true);
				gs_blend_function_separate(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA, GS_BLEND_ONE, GS_BLEND_ONE);

				consumer_effect->get_parameter("pSDFTexture").set_texture(this->m_sdf_texture);
				consumer_effect->get_parameter("pSDFThreshold").set_float(this->m_sdf_threshold);
				consumer_effect->get_parameter("pImageTexture").set_texture(this->m_source_texture->get_object());
				consumer_effect->get_parameter("pGlowColor").set_float4(this->m_inner_glow_color);
				consumer_effect->get_parameter("pGlowWidth").set_float(this->m_inner_glow_width);
				consumer_effect->get_parameter("pGlowSharpness").set_float(this->m_inner_glow_sharpness);
				consumer_effect->get_parameter("pGlowSharpnessInverse").set_float(this->m_inner_glow_sharpness_inv);

				{
					auto op = this->m_output_rt->render(baseW, baseH);
					gs_ortho(0, 1, 0, 1, 0, 1);
					while (gs_effect_loop(consumer_effect->get_object(), "GlowInner")) {
						gs_draw_sprite(0, 0, 1, 1);
					}
				}

				// Flip Buffers
				m_output_rt->get_texture(this->m_output_texture);
			} catch (...) {
			}
		}

		if (m_outline) {
			try {
				gs_enable_blending(true);
				gs_blend_function_separate(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA, GS_BLEND_ONE, GS_BLEND_ONE);

				consumer_effect->get_parameter("pSDFTexture").set_texture(this->m_sdf_texture);
				consumer_effect->get_parameter("pSDFThreshold").set_float(this->m_sdf_threshold);
				consumer_effect->get_parameter("pImageTexture").set_texture(this->m_source_texture->get_object());
				consumer_effect->get_parameter("pOutlineColor").set_float4(this->m_outline_color);
				consumer_effect->get_parameter("pOutlineWidth").set_float(this->m_outline_width);
				consumer_effect->get_parameter("pOutlineOffset").set_float(this->m_outline_offset);
				consumer_effect->get_parameter("pOutlineSharpness").set_float(this->m_outline_sharpness);
				consumer_effect->get_parameter("pOutlineSharpnessInverse").set_float(this->m_outline_sharpness_inv);

				{
					auto op = this->m_output_rt->render(baseW, baseH);
					gs_ortho(0, 1, 0, 1, 0, 1);
					while (gs_effect_loop(consumer_effect->get_object(), "Outline")) {
						gs_draw_sprite(0, 0, 1, 1);
					}
				}

				// Flip Buffers
				m_output_rt->get_texture(this->m_output_texture);
			} catch (...) {
			}
		}

		gs_blend_state_pop();
		this->m_output_rendered = true;
	}

	gs_eparam_t* ep = gs_effect_get_param_by_name(final_effect, "image");
	if (ep) {
		gs_effect_set_texture(ep, this->m_output_texture->get_object());
	}
	while (gs_effect_loop(final_effect, "Draw")) {
		gs_draw_sprite(0, 0, baseW, baseH);
	}
}
