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

#include "filter-blur.hpp"
#include <cfloat>
#include <cinttypes>
#include <cmath>
#include <map>
#include "gfx/blur/gfx-blur-box-linear.hpp"
#include "gfx/blur/gfx-blur-box.hpp"
#include "gfx/blur/gfx-blur-dual-filtering.hpp"
#include "gfx/blur/gfx-blur-gaussian-linear.hpp"
#include "gfx/blur/gfx-blur-gaussian.hpp"
#include "obs/gs/gs-helper.hpp"
#include "obs/obs-source-tracker.hpp"
#include "strings.hpp"
#include "util-math.hpp"

// OBS
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <callback/signal.h>
#include <graphics/graphics.h>
#include <graphics/matrix4.h>
#include <util/platform.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

// Translation Strings
#define SOURCE_NAME "Filter.Blur"

#define P_TYPE "Filter.Blur.Type"
#define P_SUBTYPE "Filter.Blur.SubType"
#define P_SIZE "Filter.Blur.Size"
#define P_ANGLE "Filter.Blur.Angle"
#define P_CENTER "Filter.Blur.Center"
#define P_CENTER_X "Filter.Blur.Center.X"
#define P_CENTER_Y "Filter.Blur.Center.Y"
#define P_STEPSCALE "Filter.Blur.StepScale"
#define P_STEPSCALE_X "Filter.Blur.StepScale.X"
#define P_STEPSCALE_Y "Filter.Blur.StepScale.Y"
#define P_MASK "Filter.Blur.Mask"
#define P_MASK_TYPE "Filter.Blur.Mask.Type"
#define P_MASK_TYPE_REGION "Filter.Blur.Mask.Type.Region"
#define P_MASK_TYPE_IMAGE "Filter.Blur.Mask.Type.Image"
#define P_MASK_TYPE_SOURCE "Filter.Blur.Mask.Type.Source"
#define P_MASK_REGION_LEFT "Filter.Blur.Mask.Region.Left"
#define P_MASK_REGION_RIGHT "Filter.Blur.Mask.Region.Right"
#define P_MASK_REGION_TOP "Filter.Blur.Mask.Region.Top"
#define P_MASK_REGION_BOTTOM "Filter.Blur.Mask.Region.Bottom"
#define P_MASK_REGION_FEATHER "Filter.Blur.Mask.Region.Feather"
#define P_MASK_REGION_FEATHER_SHIFT "Filter.Blur.Mask.Region.Feather.Shift"
#define P_MASK_REGION_INVERT "Filter.Blur.Mask.Region.Invert"
#define P_MASK_IMAGE "Filter.Blur.Mask.Image"
#define P_MASK_SOURCE "Filter.Blur.Mask.Source"
#define P_MASK_COLOR "Filter.Blur.Mask.Color"
#define P_MASK_ALPHA "Filter.Blur.Mask.Alpha"
#define P_MASK_MULTIPLIER "Filter.Blur.Mask.Multiplier"

struct local_blur_type_t {
	std::function<::gfx::blur::ifactory&()> fn;
	const char*                             name;
};
struct local_blur_subtype_t {
	::gfx::blur::type type;
	const char*       name;
};

static std::map<std::string, local_blur_type_t> list_of_types = {
	{"box", {&::gfx::blur::box_factory::get, S_BLUR_TYPE_BOX}},
	{"box_linear", {&::gfx::blur::box_linear_factory::get, S_BLUR_TYPE_BOX_LINEAR}},
	{"gaussian", {&::gfx::blur::gaussian_factory::get, S_BLUR_TYPE_GAUSSIAN}},
	{"gaussian_linear", {&::gfx::blur::gaussian_linear_factory::get, S_BLUR_TYPE_GAUSSIAN_LINEAR}},
	{"dual_filtering", {&::gfx::blur::dual_filtering_factory::get, S_BLUR_TYPE_DUALFILTERING}},
};
static std::map<std::string, local_blur_subtype_t> list_of_subtypes = {
	{"area", {::gfx::blur::type::Area, S_BLUR_SUBTYPE_AREA}},
	{"directional", {::gfx::blur::type::Directional, S_BLUR_SUBTYPE_DIRECTIONAL}},
	{"rotational", {::gfx::blur::type::Rotational, S_BLUR_SUBTYPE_ROTATIONAL}},
	{"zoom", {::gfx::blur::type::Zoom, S_BLUR_SUBTYPE_ZOOM}},
};

// Initializer & Finalizer
INITIALIZER(filterBlurFactoryInitializer)
{
	initializerFunctions.push_back([] { filter::blur::blur_factory::initialize(); });
	finalizerFunctions.push_back([] { filter::blur::blur_factory::finalize(); });
}

static std::shared_ptr<filter::blur::blur_factory> factory_instance = nullptr;

void filter::blur::blur_factory::initialize()
{
	factory_instance = std::make_shared<filter::blur::blur_factory>();
}

void filter::blur::blur_factory::finalize()
{
	factory_instance.reset();
}

std::shared_ptr<filter::blur::blur_factory> filter::blur::blur_factory::get()
{
	return factory_instance;
}

filter::blur::blur_factory::blur_factory()
{
	memset(&source_info, 0, sizeof(obs_source_info));
	source_info.id             = "obs-stream-effects-filter-blur";
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
	source_info.load         = load;

	obs_register_source(&source_info);

	// Translation Cache
	/// File Filter for Images
	translation_map.insert({std::string("image-filter"), std::string(P_TRANSLATE(S_FILETYPE_IMAGES))
															 + std::string(" (" T_FILEFILTERS_IMAGE ");;")
															 + std::string("* (*.*)")});
}

filter::blur::blur_factory::~blur_factory() {}

void filter::blur::blur_factory::on_list_fill()
{
	auto gctx = gs::context();

	{
		char* file = obs_module_file("effects/color-conversion.effect");
		try {
			color_converter_effect = std::make_shared<gs::effect>(file);
		} catch (std::runtime_error ex) {
			P_LOG_ERROR("<filter-blur> Loading effect '%s' failed with error(s): %s", file, ex.what());
		}
		bfree(file);
	}
	{
		char* file = obs_module_file("effects/mask.effect");
		try {
			mask_effect = std::make_shared<gs::effect>(file);
		} catch (std::runtime_error ex) {
			P_LOG_ERROR("<filter-blur> Loading effect '%s' failed with error(s): %s", file, ex.what());
		}
		bfree(file);
	}
}

void filter::blur::blur_factory::on_list_empty()
{
	auto gctx = gs::context();
	color_converter_effect.reset();
	mask_effect.reset();
}

std::string const& filter::blur::blur_factory::get_translation(std::string const key)
{
	static std::string none("");
	auto               found = translation_map.find(key);
	if (found != translation_map.end()) {
		return found->second;
	}
	return none;
}

void* filter::blur::blur_factory::create(obs_data_t* data, obs_source_t* parent)
{
	if (get()->sources.empty()) {
		get()->on_list_fill();
	}
	filter::blur::blur_instance* ptr = new filter::blur::blur_instance(data, parent);
	get()->sources.push_back(ptr);
	return ptr;
}

void filter::blur::blur_factory::destroy(void* inptr)
{
	filter::blur::blur_instance* ptr = reinterpret_cast<filter::blur::blur_instance*>(inptr);
	get()->sources.remove(ptr);
	if (get()->sources.empty()) {
		get()->on_list_empty();
	}
	delete ptr;
}

void filter::blur::blur_factory::get_defaults(obs_data_t* data)
{
	// Type, Subtype
	obs_data_set_default_string(data, P_TYPE, "box");
	obs_data_set_default_string(data, P_SUBTYPE, "area");

	// Parameters
	obs_data_set_default_int(data, P_SIZE, 5);
	obs_data_set_default_double(data, P_ANGLE, 0.);
	obs_data_set_default_double(data, P_CENTER_X, 50.);
	obs_data_set_default_double(data, P_CENTER_Y, 50.);
	obs_data_set_default_bool(data, P_STEPSCALE, false);
	obs_data_set_default_double(data, P_STEPSCALE_X, 1.);
	obs_data_set_default_double(data, P_STEPSCALE_Y, 1.);

	// Masking
	obs_data_set_default_bool(data, P_MASK, false);
	obs_data_set_default_int(data, P_MASK_TYPE, mask_type::Region);
	obs_data_set_default_double(data, P_MASK_REGION_LEFT, 0.0);
	obs_data_set_default_double(data, P_MASK_REGION_RIGHT, 0.0);
	obs_data_set_default_double(data, P_MASK_REGION_TOP, 0.0);
	obs_data_set_default_double(data, P_MASK_REGION_BOTTOM, 0.0);
	obs_data_set_default_double(data, P_MASK_REGION_FEATHER, 0.0);
	obs_data_set_default_double(data, P_MASK_REGION_FEATHER_SHIFT, 0.0);
	obs_data_set_default_bool(data, P_MASK_REGION_INVERT, false);
	{
		char* default_file = obs_module_file("white.png");
		obs_data_set_default_string(data, P_MASK_IMAGE, default_file);
		bfree(default_file);
	}
	obs_data_set_default_string(data, P_MASK_SOURCE, "");
	obs_data_set_default_int(data, P_MASK_COLOR, 0xFFFFFFFFull);
	obs_data_set_default_double(data, P_MASK_MULTIPLIER, 1.0);
}

obs_properties_t* filter::blur::blur_factory::get_properties(void* inptr)
{
	return reinterpret_cast<filter::blur::blur_instance*>(inptr)->get_properties();
}

void filter::blur::blur_factory::update(void* inptr, obs_data_t* settings)
{
	reinterpret_cast<filter::blur::blur_instance*>(inptr)->update(settings);
}

void filter::blur::blur_factory::load(void* inptr, obs_data_t* settings)
{
	reinterpret_cast<filter::blur::blur_instance*>(inptr)->load(settings);
}

const char* filter::blur::blur_factory::get_name(void*)
{
	return P_TRANSLATE(SOURCE_NAME);
}

uint32_t filter::blur::blur_factory::get_width(void* inptr)
{
	return reinterpret_cast<filter::blur::blur_instance*>(inptr)->get_width();
}

uint32_t filter::blur::blur_factory::get_height(void* inptr)
{
	return reinterpret_cast<filter::blur::blur_instance*>(inptr)->get_height();
}

void filter::blur::blur_factory::activate(void* inptr)
{
	reinterpret_cast<filter::blur::blur_instance*>(inptr)->activate();
}

void filter::blur::blur_factory::deactivate(void* inptr)
{
	reinterpret_cast<filter::blur::blur_instance*>(inptr)->deactivate();
}

void filter::blur::blur_factory::video_tick(void* inptr, float delta)
{
	reinterpret_cast<filter::blur::blur_instance*>(inptr)->video_tick(delta);
}

void filter::blur::blur_factory::video_render(void* inptr, gs_effect_t* effect)
{
	reinterpret_cast<filter::blur::blur_instance*>(inptr)->video_render(effect);
}

std::shared_ptr<gs::effect> filter::blur::blur_factory::get_color_converter_effect()
{
	return color_converter_effect;
}

std::shared_ptr<gs::effect> filter::blur::blur_factory::get_mask_effect()
{
	return mask_effect;
}

filter::blur::blur_instance::blur_instance(obs_data_t* settings, obs_source_t* parent)
	: m_self(parent), m_source_rendered(false), m_output_rendered(false)
{
	m_self = parent;

	// Create RenderTargets
	try {
		this->m_source_rt = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
		this->m_output_rt = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
	} catch (std::exception ex) {
		P_LOG_ERROR("<filter-blur:%s> Failed to create rendertargets, error %s.", obs_source_get_name(m_self),
					ex.what());
	}

	update(settings);
}

filter::blur::blur_instance::~blur_instance()
{
	this->m_mask.source.source_texture.reset();
	this->m_source_rt.reset();
	this->m_output_texture.reset();
}

bool filter::blur::blur_instance::apply_mask_parameters(std::shared_ptr<gs::effect> effect,
														gs_texture_t* original_texture, gs_texture_t* blurred_texture)
{
	if (effect->has_parameter("image_orig")) {
		effect->get_parameter("image_orig").set_texture(original_texture);
	}
	if (effect->has_parameter("image_blur")) {
		effect->get_parameter("image_blur").set_texture(blurred_texture);
	}

	// Region
	if (m_mask.type == mask_type::Region) {
		if (effect->has_parameter("mask_region_left")) {
			effect->get_parameter("mask_region_left").set_float(m_mask.region.left);
		}
		if (effect->has_parameter("mask_region_right")) {
			effect->get_parameter("mask_region_right").set_float(m_mask.region.right);
		}
		if (effect->has_parameter("mask_region_top")) {
			effect->get_parameter("mask_region_top").set_float(m_mask.region.top);
		}
		if (effect->has_parameter("mask_region_bottom")) {
			effect->get_parameter("mask_region_bottom").set_float(m_mask.region.bottom);
		}
		if (effect->has_parameter("mask_region_feather")) {
			effect->get_parameter("mask_region_feather").set_float(m_mask.region.feather);
		}
		if (effect->has_parameter("mask_region_feather_shift")) {
			effect->get_parameter("mask_region_feather_shift").set_float(m_mask.region.feather_shift);
		}
	}

	// Image
	if (m_mask.type == mask_type::Image) {
		if (effect->has_parameter("mask_image")) {
			if (m_mask.image.texture) {
				effect->get_parameter("mask_image").set_texture(m_mask.image.texture);
			} else {
				effect->get_parameter("mask_image").set_texture(nullptr);
			}
		}
	}

	// Source
	if (m_mask.type == mask_type::Source) {
		if (effect->has_parameter("mask_image")) {
			if (m_mask.source.texture) {
				effect->get_parameter("mask_image").set_texture(m_mask.source.texture);
			} else {
				effect->get_parameter("mask_image").set_texture(nullptr);
			}
		}
	}

	// Shared
	if (effect->has_parameter("mask_color")) {
		effect->get_parameter("mask_color").set_float4(m_mask.color.r, m_mask.color.g, m_mask.color.b, m_mask.color.a);
	}
	if (effect->has_parameter("mask_multiplier")) {
		effect->get_parameter("mask_multiplier").set_float(m_mask.multiplier);
	}

	return true;
}

bool filter::blur::blur_instance::modified_properties(void*, obs_properties_t* props, obs_property* prop,
													  obs_data_t* settings)
{
	obs_property_t* p;
	const char*     propname = obs_property_name(prop);
	const char*     vtype    = obs_data_get_string(settings, P_TYPE);
	const char*     vsubtype = obs_data_get_string(settings, P_SUBTYPE);

	// Find new Type
	auto type_found = list_of_types.find(vtype);
	if (type_found == list_of_types.end()) {
		return false;
	}

	// Find new Subtype
	auto subtype_found = list_of_subtypes.find(vsubtype);
	if (subtype_found == list_of_subtypes.end()) {
		return false;
	}

	// Blur Type
	if (strcmp(propname, P_TYPE) == 0) {
		obs_property_t* prop_subtype = obs_properties_get(props, P_SUBTYPE);

		/// Disable unsupported items.
		size_t subvalue_idx = 0;
		for (size_t idx = 0, edx = obs_property_list_item_count(prop_subtype); idx < edx; idx++) {
			const char* subtype  = obs_property_list_item_string(prop_subtype, idx);
			bool        disabled = false;

			auto subtype_found_idx = list_of_subtypes.find(subtype);
			if (subtype_found_idx != list_of_subtypes.end()) {
				disabled = !type_found->second.fn().is_type_supported(subtype_found_idx->second.type);
			} else {
				disabled = true;
			}

			obs_property_list_item_disable(prop_subtype, idx, disabled);
			if (strcmp(subtype, vsubtype) == 0) {
				subvalue_idx = idx;
			}
		}

		/// Ensure that there is a valid item selected.
		if (obs_property_list_item_disabled(prop_subtype, subvalue_idx)) {
			for (size_t idx = 0, edx = obs_property_list_item_count(prop_subtype); idx < edx; idx++) {
				if (!obs_property_list_item_disabled(prop_subtype, idx)) {
					obs_data_set_string(settings, P_SUBTYPE, obs_property_list_item_string(prop_subtype, idx));

					// Find new Subtype
					auto subtype_found2 = list_of_subtypes.find(vsubtype);
					if (subtype_found2 == list_of_subtypes.end()) {
						subtype_found = list_of_subtypes.end();
					} else {
						subtype_found = subtype_found2;
					}

					break;
				}
			}
		}
	}

	// Update hover text with new descriptions.
	if (type_found != list_of_types.end()) {
		if (type_found->first == "box") {
			obs_property_set_long_description(obs_properties_get(props, P_TYPE), P_TRANSLATE(P_DESC(S_BLUR_TYPE_BOX)));
		} else if (type_found->first == "box_linear") {
			obs_property_set_long_description(obs_properties_get(props, P_TYPE),
											  P_TRANSLATE(P_DESC(S_BLUR_TYPE_BOX_LINEAR)));
		} else if (type_found->first == "gaussian") {
			obs_property_set_long_description(obs_properties_get(props, P_TYPE),
											  P_TRANSLATE(P_DESC(S_BLUR_TYPE_GAUSSIAN)));
		} else if (type_found->first == "gaussian_linear") {
			obs_property_set_long_description(obs_properties_get(props, P_TYPE),
											  P_TRANSLATE(P_DESC(S_BLUR_TYPE_GAUSSIAN_LINEAR)));
		}
	} else {
		obs_property_set_long_description(obs_properties_get(props, P_TYPE), P_TRANSLATE(P_DESC(P_TYPE)));
	}
	if (subtype_found != list_of_subtypes.end()) {
		if (subtype_found->first == "area") {
			obs_property_set_long_description(obs_properties_get(props, P_SUBTYPE),
											  P_TRANSLATE(P_DESC(S_BLUR_SUBTYPE_AREA)));
		} else if (subtype_found->first == "directional") {
			obs_property_set_long_description(obs_properties_get(props, P_SUBTYPE),
											  P_TRANSLATE(P_DESC(S_BLUR_SUBTYPE_DIRECTIONAL)));
		} else if (subtype_found->first == "rotational") {
			obs_property_set_long_description(obs_properties_get(props, P_SUBTYPE),
											  P_TRANSLATE(P_DESC(S_BLUR_SUBTYPE_ROTATIONAL)));
		} else if (subtype_found->first == "zoom") {
			obs_property_set_long_description(obs_properties_get(props, P_SUBTYPE),
											  P_TRANSLATE(P_DESC(S_BLUR_SUBTYPE_ZOOM)));
		}
	} else {
		obs_property_set_long_description(obs_properties_get(props, P_SUBTYPE), P_TRANSLATE(P_DESC(P_SUBTYPE)));
	}

	// Blur Sub-Type
	{
		bool has_angle_support = (subtype_found->second.type == ::gfx::blur::type::Directional)
								 || (subtype_found->second.type == ::gfx::blur::type::Rotational);
		bool has_center_support = (subtype_found->second.type == ::gfx::blur::type::Rotational)
								  || (subtype_found->second.type == ::gfx::blur::type::Zoom);
		bool has_stepscale_support = type_found->second.fn().is_step_scale_supported(subtype_found->second.type);
		bool show_scaling          = obs_data_get_bool(settings, P_STEPSCALE) && has_stepscale_support;

		/// Size
		p = obs_properties_get(props, P_SIZE);
		obs_property_float_set_limits(p, type_found->second.fn().get_min_size(subtype_found->second.type),
									  type_found->second.fn().get_max_size(subtype_found->second.type),
									  type_found->second.fn().get_step_size(subtype_found->second.type));

		/// Angle
		p = obs_properties_get(props, P_ANGLE);
		obs_property_set_visible(p, has_angle_support);
		obs_property_float_set_limits(p, type_found->second.fn().get_min_angle(subtype_found->second.type),
									  type_found->second.fn().get_max_angle(subtype_found->second.type),
									  type_found->second.fn().get_step_angle(subtype_found->second.type));

		/// Center, Radius
		obs_property_set_visible(obs_properties_get(props, P_CENTER_X), has_center_support);
		obs_property_set_visible(obs_properties_get(props, P_CENTER_Y), has_center_support);

		/// Step Scaling
		obs_property_set_visible(obs_properties_get(props, P_STEPSCALE), has_stepscale_support);
		p = obs_properties_get(props, P_STEPSCALE_X);
		obs_property_set_visible(p, show_scaling);
		obs_property_float_set_limits(p, type_found->second.fn().get_min_step_scale_x(subtype_found->second.type),
									  type_found->second.fn().get_max_step_scale_x(subtype_found->second.type),
									  type_found->second.fn().get_step_step_scale_x(subtype_found->second.type));
		p = obs_properties_get(props, P_STEPSCALE_Y);
		obs_property_set_visible(p, show_scaling);
		obs_property_float_set_limits(p, type_found->second.fn().get_min_step_scale_x(subtype_found->second.type),
									  type_found->second.fn().get_max_step_scale_x(subtype_found->second.type),
									  type_found->second.fn().get_step_step_scale_x(subtype_found->second.type));
	}

	{ // Masking
		bool      show_mask   = obs_data_get_bool(settings, P_MASK);
		mask_type mtype       = static_cast<mask_type>(obs_data_get_int(settings, P_MASK_TYPE));
		bool      show_region = (mtype == mask_type::Region) && show_mask;
		bool      show_image  = (mtype == mask_type::Image) && show_mask;
		bool      show_source = (mtype == mask_type::Source) && show_mask;
		obs_property_set_visible(obs_properties_get(props, P_MASK_TYPE), show_mask);
		obs_property_set_visible(obs_properties_get(props, P_MASK_REGION_LEFT), show_region);
		obs_property_set_visible(obs_properties_get(props, P_MASK_REGION_TOP), show_region);
		obs_property_set_visible(obs_properties_get(props, P_MASK_REGION_RIGHT), show_region);
		obs_property_set_visible(obs_properties_get(props, P_MASK_REGION_BOTTOM), show_region);
		obs_property_set_visible(obs_properties_get(props, P_MASK_REGION_FEATHER), show_region);
		obs_property_set_visible(obs_properties_get(props, P_MASK_REGION_FEATHER_SHIFT), show_region);
		obs_property_set_visible(obs_properties_get(props, P_MASK_REGION_INVERT), show_region);
		obs_property_set_visible(obs_properties_get(props, P_MASK_IMAGE), show_image);
		obs_property_set_visible(obs_properties_get(props, P_MASK_SOURCE), show_source);
		obs_property_set_visible(obs_properties_get(props, P_MASK_COLOR), show_image || show_source);
		obs_property_set_visible(obs_properties_get(props, P_MASK_ALPHA), show_image || show_source);
		obs_property_set_visible(obs_properties_get(props, P_MASK_MULTIPLIER), show_image || show_source);
	}

	return true;
}

void filter::blur::blur_instance::translate_old_settings(obs_data_t* settings)
{
	obs_data_set_default_int(settings, S_VERSION, -1);
	int64_t version = obs_data_get_int(settings, S_VERSION);

	// If it's the same as the current version, return.
	if (version == PROJECT_VERSION) {
		return;
	}

	// Now we use a fall-through switch to gradually upgrade each known version change.
	switch (version) {
	case -1:
		/// Blur Type
		int64_t old_blur = obs_data_get_int(settings, "Filter.Blur.Type");
		if (old_blur == 0) { // Box
			obs_data_set_string(settings, P_TYPE, "box");
		} else if (old_blur == 1) { // Gaussian
			obs_data_set_string(settings, P_TYPE, "gaussian");
		} else if (old_blur == 2) { // Bilateral, no longer included.
			obs_data_set_string(settings, P_TYPE, "box");
		} else if (old_blur == 3) { // Box Linear
			obs_data_set_string(settings, P_TYPE, "box_linear");
		} else if (old_blur == 4) { // Gaussian Linear
			obs_data_set_string(settings, P_TYPE, "gaussian_linear");
		} else {
			obs_data_set_string(settings, P_TYPE, "box");
		}
		obs_data_unset_user_value(settings, "Filter.Blur.Type");

		/// Directional Blur
		bool directional = obs_data_get_bool(settings, "Filter.Blur.Directional");
		if (directional) {
			obs_data_set_string(settings, P_SUBTYPE, "directional");
		} else {
			obs_data_set_string(settings, P_SUBTYPE, "area");
		}
		obs_data_unset_user_value(settings, "Filter.Blur.Directional");

		/// Directional Blur Angle
		double_t angle = obs_data_get_double(settings, "Filter.Blur.Directional.Angle");
		obs_data_set_double(settings, P_ANGLE, angle);
		obs_data_unset_user_value(settings, "Filter.Blur.Directional.Angle");
	}

	obs_data_set_int(settings, S_VERSION, PROJECT_VERSION);
}

obs_properties_t* filter::blur::blur_instance::get_properties()
{
	obs_properties_t* pr = obs_properties_create();
	obs_property_t*   p  = NULL;

	// Blur Type and Sub-Type
	{
		p = obs_properties_add_list(pr, P_TYPE, P_TRANSLATE(P_TYPE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_TYPE)));
		obs_property_set_modified_callback2(p, modified_properties, this);
		obs_property_list_add_string(p, P_TRANSLATE(S_BLUR_TYPE_BOX), "box");
		obs_property_list_add_string(p, P_TRANSLATE(S_BLUR_TYPE_BOX_LINEAR), "box_linear");
		obs_property_list_add_string(p, P_TRANSLATE(S_BLUR_TYPE_GAUSSIAN), "gaussian");
		obs_property_list_add_string(p, P_TRANSLATE(S_BLUR_TYPE_GAUSSIAN_LINEAR), "gaussian_linear");
		obs_property_list_add_string(p, P_TRANSLATE(S_BLUR_TYPE_DUALFILTERING), "dual_filtering");

		p = obs_properties_add_list(pr, P_SUBTYPE, P_TRANSLATE(P_SUBTYPE), OBS_COMBO_TYPE_LIST,
									OBS_COMBO_FORMAT_STRING);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SUBTYPE)));
		obs_property_set_modified_callback2(p, modified_properties, this);
		obs_property_list_add_string(p, P_TRANSLATE(S_BLUR_SUBTYPE_AREA), "area");
		obs_property_list_add_string(p, P_TRANSLATE(S_BLUR_SUBTYPE_DIRECTIONAL), "directional");
		obs_property_list_add_string(p, P_TRANSLATE(S_BLUR_SUBTYPE_ROTATIONAL), "rotational");
		obs_property_list_add_string(p, P_TRANSLATE(S_BLUR_SUBTYPE_ZOOM), "zoom");
	}

	// Blur Parameters
	{
		p = obs_properties_add_float_slider(pr, P_SIZE, P_TRANSLATE(P_SIZE), 1, 32767, 1);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SIZE)));

		p = obs_properties_add_float_slider(pr, P_ANGLE, P_TRANSLATE(P_ANGLE), -180.0, 180.0, 0.01);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_ANGLE)));

		p = obs_properties_add_float_slider(pr, P_CENTER_X, P_TRANSLATE(P_CENTER_X), 0.00, 100.0, 0.01);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_CENTER_X)));
		p = obs_properties_add_float_slider(pr, P_CENTER_Y, P_TRANSLATE(P_CENTER_Y), 0.00, 100.0, 0.01);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_CENTER_Y)));

		p = obs_properties_add_bool(pr, P_STEPSCALE, P_TRANSLATE(P_STEPSCALE));
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_STEPSCALE)));
		obs_property_set_modified_callback2(p, modified_properties, this);
		p = obs_properties_add_float_slider(pr, P_STEPSCALE_X, P_TRANSLATE(P_STEPSCALE_X), 0.0, 1000.0, 0.01);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_STEPSCALE_X)));
		p = obs_properties_add_float_slider(pr, P_STEPSCALE_Y, P_TRANSLATE(P_STEPSCALE_Y), 0.0, 1000.0, 0.01);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_STEPSCALE_Y)));
	}

	// Masking
	{
		p = obs_properties_add_bool(pr, P_MASK, P_TRANSLATE(P_MASK));
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK)));
		obs_property_set_modified_callback2(p, modified_properties, this);
		p = obs_properties_add_list(pr, P_MASK_TYPE, P_TRANSLATE(P_MASK_TYPE), OBS_COMBO_TYPE_LIST,
									OBS_COMBO_FORMAT_INT);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK_TYPE)));
		obs_property_set_modified_callback2(p, modified_properties, this);
		obs_property_list_add_int(p, P_TRANSLATE(P_MASK_TYPE_REGION), mask_type::Region);
		obs_property_list_add_int(p, P_TRANSLATE(P_MASK_TYPE_IMAGE), mask_type::Image);
		obs_property_list_add_int(p, P_TRANSLATE(P_MASK_TYPE_SOURCE), mask_type::Source);
		/// Region
		p = obs_properties_add_float_slider(pr, P_MASK_REGION_LEFT, P_TRANSLATE(P_MASK_REGION_LEFT), 0.0, 100.0, 0.01);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK_REGION_LEFT)));
		p = obs_properties_add_float_slider(pr, P_MASK_REGION_TOP, P_TRANSLATE(P_MASK_REGION_TOP), 0.0, 100.0, 0.01);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK_REGION_TOP)));
		p = obs_properties_add_float_slider(pr, P_MASK_REGION_RIGHT, P_TRANSLATE(P_MASK_REGION_RIGHT), 0.0, 100.0,
											0.01);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK_REGION_RIGHT)));
		p = obs_properties_add_float_slider(pr, P_MASK_REGION_BOTTOM, P_TRANSLATE(P_MASK_REGION_BOTTOM), 0.0, 100.0,
											0.01);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK_REGION_BOTTOM)));
		p = obs_properties_add_float_slider(pr, P_MASK_REGION_FEATHER, P_TRANSLATE(P_MASK_REGION_FEATHER), 0.0, 50.0,
											0.01);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK_REGION_FEATHER)));
		p = obs_properties_add_float_slider(pr, P_MASK_REGION_FEATHER_SHIFT, P_TRANSLATE(P_MASK_REGION_FEATHER_SHIFT),
											-100.0, 100.0, 0.01);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK_REGION_FEATHER_SHIFT)));
		p = obs_properties_add_bool(pr, P_MASK_REGION_INVERT, P_TRANSLATE(P_MASK_REGION_INVERT));
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK_REGION_INVERT)));
		/// Image
		p = obs_properties_add_path(pr, P_MASK_IMAGE, P_TRANSLATE(P_MASK_IMAGE), OBS_PATH_FILE,
									filter::blur::blur_factory::get()->get_translation("image-filter").c_str(),
									nullptr);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK_IMAGE)));
		/// Source
		p = obs_properties_add_list(pr, P_MASK_SOURCE, P_TRANSLATE(P_MASK_SOURCE), OBS_COMBO_TYPE_LIST,
									OBS_COMBO_FORMAT_STRING);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK_SOURCE)));
		obs_property_list_add_string(p, "", "");
		obs::source_tracker::get()->enumerate(
			[&p](std::string name, obs_source_t*) {
				obs_property_list_add_string(p, std::string(name + " (Source)").c_str(), name.c_str());
				return false;
			},
			obs::source_tracker::filter_video_sources);
		obs::source_tracker::get()->enumerate(
			[&p](std::string name, obs_source_t*) {
				obs_property_list_add_string(p, std::string(name + " (Scene)").c_str(), name.c_str());
				return false;
			},
			obs::source_tracker::filter_scenes);

		/// Shared
		p = obs_properties_add_color(pr, P_MASK_COLOR, P_TRANSLATE(P_MASK_COLOR));
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK_COLOR)));
		p = obs_properties_add_float_slider(pr, P_MASK_ALPHA, P_TRANSLATE(P_MASK_ALPHA), 0.0, 100.0, 0.1);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK_ALPHA)));
		p = obs_properties_add_float_slider(pr, P_MASK_MULTIPLIER, P_TRANSLATE(P_MASK_MULTIPLIER), 0.0, 10.0, 0.01);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK_MULTIPLIER)));
	}

	return pr;
}

void filter::blur::blur_instance::update(obs_data_t* settings)
{
	// Ensure backwards compatibility.
	this->translate_old_settings(settings);

	{ // Blur Type
		const char* blur_type      = obs_data_get_string(settings, P_TYPE);
		const char* blur_subtype   = obs_data_get_string(settings, P_SUBTYPE);
		const char* last_blur_type = obs_data_get_string(settings, P_TYPE ".last");

		auto type_found = list_of_types.find(blur_type);
		if (type_found != list_of_types.end()) {
			auto subtype_found = list_of_subtypes.find(blur_subtype);
			if (subtype_found != list_of_subtypes.end()) {
				if ((strcmp(last_blur_type, blur_type) != 0) || (m_blur->get_type() != subtype_found->second.type)) {
					if (type_found->second.fn().is_type_supported(subtype_found->second.type)) {
						m_blur = type_found->second.fn().create(subtype_found->second.type);
					}
				}
			}
		}
	}

	{ // Blur Parameters
		this->m_blur_size          = obs_data_get_double(settings, P_SIZE);
		this->m_blur_angle         = obs_data_get_double(settings, P_ANGLE);
		this->m_blur_center.first  = obs_data_get_double(settings, P_CENTER_X) / 100.0;
		this->m_blur_center.second = obs_data_get_double(settings, P_CENTER_Y) / 100.0;

		// Scaling
		this->m_blur_step_scaling      = obs_data_get_bool(settings, P_STEPSCALE);
		this->m_blur_step_scale.first  = obs_data_get_double(settings, P_STEPSCALE_X) / 100.0;
		this->m_blur_step_scale.second = obs_data_get_double(settings, P_STEPSCALE_Y) / 100.0;
	}

	{ // Masking
		m_mask.enabled = obs_data_get_bool(settings, P_MASK);
		if (m_mask.enabled) {
			m_mask.type = static_cast<mask_type>(obs_data_get_int(settings, P_MASK_TYPE));
			switch (m_mask.type) {
			case mask_type::Region:
				m_mask.region.left    = float_t(obs_data_get_double(settings, P_MASK_REGION_LEFT) / 100.0);
				m_mask.region.top     = float_t(obs_data_get_double(settings, P_MASK_REGION_TOP) / 100.0);
				m_mask.region.right   = 1.0f - float_t(obs_data_get_double(settings, P_MASK_REGION_RIGHT) / 100.0);
				m_mask.region.bottom  = 1.0f - float_t(obs_data_get_double(settings, P_MASK_REGION_BOTTOM) / 100.0);
				m_mask.region.feather = float_t(obs_data_get_double(settings, P_MASK_REGION_FEATHER) / 100.0);
				m_mask.region.feather_shift =
					float_t(obs_data_get_double(settings, P_MASK_REGION_FEATHER_SHIFT) / 100.0);
				m_mask.region.invert = obs_data_get_bool(settings, P_MASK_REGION_INVERT);
				break;
			case mask_type::Image:
				m_mask.image.path = obs_data_get_string(settings, P_MASK_IMAGE);
				break;
			case mask_type::Source:
				m_mask.source.name = obs_data_get_string(settings, P_MASK_SOURCE);
				break;
			}
			if ((m_mask.type == mask_type::Image) || (m_mask.type == mask_type::Source)) {
				uint32_t color    = static_cast<uint32_t>(obs_data_get_int(settings, P_MASK_COLOR));
				m_mask.color.r    = ((color >> 0) & 0xFF) / 255.0f;
				m_mask.color.g    = ((color >> 8) & 0xFF) / 255.0f;
				m_mask.color.b    = ((color >> 16) & 0xFF) / 255.0f;
				m_mask.color.a    = static_cast<float_t>(obs_data_get_double(settings, P_MASK_ALPHA));
				m_mask.multiplier = float_t(obs_data_get_double(settings, P_MASK_MULTIPLIER));
			}
		}
	}
}

void filter::blur::blur_instance::load(obs_data_t* settings)
{
	update(settings);
}

uint32_t filter::blur::blur_instance::get_width()
{
	return uint32_t(0);
}

uint32_t filter::blur::blur_instance::get_height()
{
	return uint32_t(0);
}

void filter::blur::blur_instance::activate() {}

void filter::blur::blur_instance::deactivate() {}

void filter::blur::blur_instance::video_tick(float)
{
	// Blur
	if (m_blur) {
		m_blur->set_size(m_blur_size);
		if (m_blur_step_scaling) {
			m_blur->set_step_scale(m_blur_step_scale.first, m_blur_step_scale.second);
		} else {
			m_blur->set_step_scale(1.0, 1.0);
		}
		if ((m_blur->get_type() == ::gfx::blur::type::Directional)
			|| (m_blur->get_type() == ::gfx::blur::type::Rotational)) {
			auto obj = std::dynamic_pointer_cast<::gfx::blur::ibase_angle>(m_blur);
			obj->set_angle(m_blur_angle);
		}
		if ((m_blur->get_type() == ::gfx::blur::type::Zoom) || (m_blur->get_type() == ::gfx::blur::type::Rotational)) {
			auto obj = std::dynamic_pointer_cast<::gfx::blur::ibase_center>(m_blur);
			obj->set_center(m_blur_center.first, m_blur_center.second);
		}
	}

	// Load Mask
	if (m_mask.type == mask_type::Image) {
		if (m_mask.image.path_old != m_mask.image.path) {
			try {
				m_mask.image.texture  = std::make_shared<gs::texture>(m_mask.image.path);
				m_mask.image.path_old = m_mask.image.path;
			} catch (...) {
				P_LOG_ERROR("<filter-blur> Instance '%s' failed to load image '%s'.", obs_source_get_name(m_self),
							m_mask.image.path.c_str());
			}
		}
	} else if (m_mask.type == mask_type::Source) {
		if (m_mask.source.name_old != m_mask.source.name) {
			try {
				m_mask.source.source_texture = std::make_shared<gfx::source_texture>(m_mask.source.name, m_self);
				m_mask.source.is_scene = (obs_scene_from_source(m_mask.source.source_texture->get_object()) != nullptr);
				m_mask.source.name_old = m_mask.source.name;
			} catch (...) {
				P_LOG_ERROR("<filter-blur> Instance '%s' failed to grab source '%s'.", obs_source_get_name(m_self),
							m_mask.source.name.c_str());
			}
		}
	}

	m_source_rendered = false;
	m_output_rendered = false;
}

void filter::blur::blur_instance::video_render(gs_effect_t* effect)
{
	obs_source_t* parent        = obs_filter_get_parent(this->m_self);
	obs_source_t* target        = obs_filter_get_target(this->m_self);
	gs_effect_t*  defaultEffect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_DEFAULT);
	uint32_t      baseW         = obs_source_get_base_width(target);
	uint32_t      baseH         = obs_source_get_base_height(target);

	std::shared_ptr<gs::effect> colorConversionEffect = blur_factory::get()->get_color_converter_effect();
	vec4                        black;

	vec4_set(&black, 0, 0, 0, 0);

	// Verify that we can actually run first.
	if (!target || !parent || !this->m_self || !this->m_blur || (baseW == 0) || (baseH == 0)) {
		obs_source_skip_video_filter(this->m_self);
		return;
	}

	if (!m_source_rendered) {
		// Source To Texture
		{
			if (obs_source_process_filter_begin(this->m_self, GS_RGBA, OBS_ALLOW_DIRECT_RENDERING)) {
				{
					auto op = this->m_source_rt->render(baseW, baseH);

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
					gs_stencil_op(GS_STENCIL_BOTH, GS_KEEP, GS_KEEP, GS_KEEP);

					// Orthographic Camera and clear RenderTarget.
					gs_ortho(0, (float)baseW, 0, (float)baseH, -1., 1.);
					//gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &black, 0, 0);

					// Render
					obs_source_process_filter_end(this->m_self, defaultEffect, baseW, baseH);

					gs_blend_state_pop();
				}

				m_source_texture = this->m_source_rt->get_texture();
				if (!m_source_texture) {
					obs_source_skip_video_filter(this->m_self);
					return;
				}
			} else {
				obs_source_skip_video_filter(this->m_self);
				return;
			}
		}

		m_source_rendered = true;
	}

	if (!m_output_rendered) {
		m_blur->set_input(m_source_texture);
		m_output_texture = m_blur->render();

		// Mask
		if (m_mask.enabled) {
			gs_blend_state_push();
			gs_reset_blend_state();
			gs_enable_color(true, true, true, true);
			gs_enable_blending(false);
			gs_enable_depth_test(false);
			gs_enable_stencil_test(false);
			gs_enable_stencil_write(false);
			gs_set_cull_mode(GS_NEITHER);
			gs_depth_function(GS_ALWAYS);
			gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);
			gs_stencil_function(GS_STENCIL_BOTH, GS_ALWAYS);
			gs_stencil_op(GS_STENCIL_BOTH, GS_ZERO, GS_ZERO, GS_ZERO);

			std::string technique = "";
			switch (this->m_mask.type) {
			case Region:
				if (this->m_mask.region.feather > FLT_EPSILON) {
					if (this->m_mask.region.invert) {
						technique = "RegionFeatherInverted";
					} else {
						technique = "RegionFeather";
					}
				} else {
					if (this->m_mask.region.invert) {
						technique = "RegionInverted";
					} else {
						technique = "Region";
					}
				}
				break;
			case Image:
			case Source:
				technique = "Image";
				break;
			}

			if (m_mask.source.source_texture) {
				uint32_t source_width  = obs_source_get_width(this->m_mask.source.source_texture->get_object());
				uint32_t source_height = obs_source_get_height(this->m_mask.source.source_texture->get_object());

				if (source_width == 0) {
					source_width = baseW;
				}
				if (source_height == 0) {
					source_height = baseH;
				}
				if (this->m_mask.source.is_scene) {
					obs_video_info ovi;
					if (obs_get_video_info(&ovi)) {
						source_width  = ovi.base_width;
						source_height = ovi.base_height;
					}
				}

				this->m_mask.source.texture = this->m_mask.source.source_texture->render(source_width, source_height);
			}

			std::shared_ptr<gs::effect> mask_effect = blur_factory::get()->get_mask_effect();
			apply_mask_parameters(mask_effect, m_source_texture->get_object(), m_output_texture->get_object());

			try {
				auto op = this->m_output_rt->render(baseW, baseH);
				gs_ortho(0, (float)baseW, 0, (float)baseH, -1, 1);

				// Render
				while (gs_effect_loop(mask_effect->get_object(), technique.c_str())) {
					gs_draw_sprite(m_output_texture->get_object(), 0, baseW, baseH);
				}
			} catch (const std::exception&) {
				gs_blend_state_pop();
				obs_source_skip_video_filter(this->m_self);
				return;
			}
			gs_blend_state_pop();

			if (!(m_output_texture = this->m_output_rt->get_texture())) {
				obs_source_skip_video_filter(this->m_self);
				return;
			}
		}

		m_output_rendered = true;
	}

	// Draw source
	{
		// It is important that we do not modify the blend state here, as it is set correctly by OBS
		gs_set_cull_mode(GS_NEITHER);
		gs_enable_color(true, true, true, true);
		gs_enable_depth_test(false);
		gs_depth_function(GS_ALWAYS);
		gs_enable_stencil_test(false);
		gs_enable_stencil_write(false);
		gs_stencil_function(GS_STENCIL_BOTH, GS_ALWAYS);
		gs_stencil_op(GS_STENCIL_BOTH, GS_ZERO, GS_ZERO, GS_ZERO);

		gs_effect_t* finalEffect = effect ? effect : defaultEffect;
		const char*  technique   = "Draw";

		gs_eparam_t* param = gs_effect_get_param_by_name(finalEffect, "image");
		if (!param) {
			P_LOG_ERROR("<filter-blur:%s> Failed to set image param.", obs_source_get_name(this->m_self));
			obs_source_skip_video_filter(m_self);
			return;
		} else {
			gs_effect_set_texture(param, m_output_texture->get_object());
		}
		while (gs_effect_loop(finalEffect, technique)) {
			gs_draw_sprite(m_output_texture->get_object(), 0, baseW, baseH);
		}
	}
}
