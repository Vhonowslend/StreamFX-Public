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
#define P_TYPE_BOX "Filter.Blur.Type.Box"
#define P_TYPE_BOXLINEAR "Filter.Blur.Type.BoxLinear"
#define P_TYPE_GAUSSIAN "Filter.Blur.Type.Gaussian"
#define P_TYPE_GAUSSIANLINEAR "Filter.Blur.Type.GaussianLinear"
#define P_TYPE_BILATERAL "Filter.Blur.Type.Bilateral"
#define P_SIZE "Filter.Blur.Size"
#define P_BILATERAL_SMOOTHING "Filter.Blur.Bilateral.Smoothing"
#define P_BILATERAL_SHARPNESS "Filter.Blur.Bilateral.Sharpness"
#define P_DIRECTIONAL "Filter.Blur.Directional"
#define P_DIRECTIONAL_ANGLE "Filter.Blur.Directional.Angle"
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
#define P_COLORFORMAT "Filter.Blur.ColorFormat"

// Initializer & Finalizer
INITIALIZER(filterBlurFactoryInitializer)
{
	initializerFunctions.push_back([] { filter::blur::blur_factory::initialize(); });
	finalizerFunctions.push_back([] { filter::blur::blur_factory::finalize(); });
}

enum ColorFormat : uint64_t { // ToDo: Refactor into full class.
	RGB,
	YUV, // 701
};

static uint8_t const max_kernel_size = 25;

bool filter::blur::blur_instance::apply_shared_param(gs_texture_t* input, float texelX, float texelY)
{
	bool result = true;

	result = result && gs_set_param_texture(blur_effect->get_object(), "u_image", input);

	vec2 imageSize;
	vec2_set(&imageSize, (float)gs_texture_get_width(input), (float)gs_texture_get_height(input));
	result = result && gs_set_param_float2(blur_effect->get_object(), "u_imageSize", &imageSize);

	vec2 imageTexelDelta;
	vec2_set(&imageTexelDelta, 1.0f, 1.0f);
	vec2_div(&imageTexelDelta, &imageTexelDelta, &imageSize);
	result = result && gs_set_param_float2(blur_effect->get_object(), "u_imageTexel", &imageTexelDelta);

	vec2 texel;
	vec2_set(&texel, texelX, texelY);
	result = result && gs_set_param_float2(blur_effect->get_object(), "u_texelDelta", &texel);

	result = result && gs_set_param_int(blur_effect->get_object(), "u_radius", (int)size);
	result = result && gs_set_param_int(blur_effect->get_object(), "u_diameter", (int)(1 + (size * 2)));

	return result;
}

bool filter::blur::blur_instance::apply_bilateral_param()
{
	if (type != type::Bilateral)
		return false;

	if (blur_effect->has_parameter("bilateralSmoothing")) {
		blur_effect->get_parameter("bilateralSmoothing").set_float((float)(bilateral_smoothing * (1 + size * 2)));
	}

	if (blur_effect->has_parameter("bilateralSharpness")) {
		blur_effect->get_parameter("bilateralSharpness").set_float((float)(1.0 - bilateral_sharpness));
	}

	return true;
}

bool filter::blur::blur_instance::apply_gaussian_param(uint8_t width)
{
	auto kernel = filter::blur::blur_factory::get()->get_gaussian_kernel(width);

	if (blur_effect->has_parameter("kernel")) {
		blur_effect->get_parameter("kernel").set_float_array(&(kernel->front()), kernel->size());
	}

	return true;
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
	if (mask.type == mask_type::Region) {
		if (effect->has_parameter("mask_region_left")) {
			effect->get_parameter("mask_region_left").set_float(mask.region.left);
		}
		if (effect->has_parameter("mask_region_right")) {
			effect->get_parameter("mask_region_right").set_float(mask.region.right);
		}
		if (effect->has_parameter("mask_region_top")) {
			effect->get_parameter("mask_region_top").set_float(mask.region.top);
		}
		if (effect->has_parameter("mask_region_bottom")) {
			effect->get_parameter("mask_region_bottom").set_float(mask.region.bottom);
		}
		if (effect->has_parameter("mask_region_feather")) {
			effect->get_parameter("mask_region_feather").set_float(mask.region.feather);
		}
		if (effect->has_parameter("mask_region_feather_shift")) {
			effect->get_parameter("mask_region_feather_shift").set_float(mask.region.feather_shift);
		}
	}

	// Image
	if (mask.type == mask_type::Image) {
		if (effect->has_parameter("mask_image")) {
			if (mask.image.texture) {
				effect->get_parameter("mask_image").set_texture(mask.image.texture);
			} else {
				effect->get_parameter("mask_image").set_texture(nullptr);
			}
		}
	}

	// Source
	if (mask.type == mask_type::Source) {
		if (effect->has_parameter("mask_image")) {
			if (mask.source.texture) {
				effect->get_parameter("mask_image").set_texture(mask.source.texture);
			} else {
				effect->get_parameter("mask_image").set_texture(nullptr);
			}
		}
	}

	// Shared
	if (effect->has_parameter("mask_color")) {
		effect->get_parameter("mask_color").set_float4(mask.color.r, mask.color.g, mask.color.b, mask.color.a);
	}
	if (effect->has_parameter("mask_multiplier")) {
		effect->get_parameter("mask_multiplier").set_float(mask.multiplier);
	}

	return true;
}

bool filter::blur::blur_instance::modified_properties(void*, obs_properties_t* props, obs_property*,
													  obs_data_t* settings)
{
	// bilateral blur
	bool show_bilateral = (obs_data_get_int(settings, P_TYPE) == type::Bilateral);
	obs_property_set_visible(obs_properties_get(props, P_BILATERAL_SMOOTHING), show_bilateral);
	obs_property_set_visible(obs_properties_get(props, P_BILATERAL_SHARPNESS), show_bilateral);

	// region
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

	// Directional Blur
	bool show_directional = obs_data_get_bool(settings, P_DIRECTIONAL);
	obs_property_set_visible(obs_properties_get(props, P_DIRECTIONAL_ANGLE), show_directional);

	// Scaling
	bool show_scaling = obs_data_get_bool(settings, P_STEPSCALE);
	obs_property_set_visible(obs_properties_get(props, P_STEPSCALE_X), show_scaling);
	obs_property_set_visible(obs_properties_get(props, P_STEPSCALE_Y), show_scaling);

	// advanced
	bool showAdvanced = obs_data_get_bool(settings, S_ADVANCED);
	obs_property_set_visible(obs_properties_get(props, P_COLORFORMAT), showAdvanced);

	return true;
}

bool filter::blur::blur_instance::can_log()
{
	// Only allow logging errors every 200ms.
	auto now   = std::chrono::high_resolution_clock::now();
	auto delta = (now - this->last_log);
	std::swap(this->last_log, now);
	return std::chrono::duration_cast<std::chrono::milliseconds>(delta) > std::chrono::milliseconds(200);
}

filter::blur::blur_instance::blur_instance(obs_data_t* settings, obs_source_t* parent)
{
	m_source = parent;

	// Create RenderTargets
	try {
		this->rt_source    = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
		this->rt_primary   = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
		this->rt_secondary = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
	} catch (std::exception ex) {
		P_LOG_ERROR("<filter-blur:%s> Failed to create rendertargets, error %s.", obs_source_get_name(m_source),
					ex.what());
	}

	// Get initial Blur effect.
	blur_effect = filter::blur::blur_factory::get()->get_effect(filter::blur::type::Box);

	update(settings);
}

filter::blur::blur_instance::~blur_instance()
{
	this->rt_primary.reset();
	this->rt_secondary.reset();
}

obs_properties_t* filter::blur::blur_instance::get_properties()
{
	obs_properties_t* pr = obs_properties_create();
	obs_property_t*   p  = NULL;

	p = obs_properties_add_list(pr, P_TYPE, P_TRANSLATE(P_TYPE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_TYPE)));
	obs_property_set_modified_callback2(p, modified_properties, this);
	obs_property_list_add_int(p, P_TRANSLATE(P_TYPE_BOX), filter::blur::type::Box);
	obs_property_list_add_int(p, P_TRANSLATE(P_TYPE_BOXLINEAR), filter::blur::type::BoxLinear);
	obs_property_list_add_int(p, P_TRANSLATE(P_TYPE_GAUSSIAN), filter::blur::type::Gaussian);
	obs_property_list_add_int(p, P_TRANSLATE(P_TYPE_GAUSSIANLINEAR), filter::blur::type::GaussianLinear);
	obs_property_list_add_int(p, P_TRANSLATE(P_TYPE_BILATERAL), filter::blur::type::Bilateral);

	p = obs_properties_add_int_slider(pr, P_SIZE, P_TRANSLATE(P_SIZE), 1, 25, 1);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SIZE)));
	//obs_property_set_modified_callback(p, modified_properties);

	// bilateral Only
	p = obs_properties_add_float_slider(pr, P_BILATERAL_SMOOTHING, P_TRANSLATE(P_BILATERAL_SMOOTHING), 0.0, 100.0,
										0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_BILATERAL_SMOOTHING)));
	p = obs_properties_add_float_slider(pr, P_BILATERAL_SHARPNESS, P_TRANSLATE(P_BILATERAL_SHARPNESS), 0.0, 100.0,
										0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_BILATERAL_SHARPNESS)));

	// Mask
	p = obs_properties_add_bool(pr, P_MASK, P_TRANSLATE(P_MASK));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK)));
	obs_property_set_modified_callback2(p, modified_properties, this);
	p = obs_properties_add_list(pr, P_MASK_TYPE, P_TRANSLATE(P_MASK_TYPE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
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
	p = obs_properties_add_float_slider(pr, P_MASK_REGION_RIGHT, P_TRANSLATE(P_MASK_REGION_RIGHT), 0.0, 100.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK_REGION_RIGHT)));
	p = obs_properties_add_float_slider(pr, P_MASK_REGION_BOTTOM, P_TRANSLATE(P_MASK_REGION_BOTTOM), 0.0, 100.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK_REGION_BOTTOM)));
	p = obs_properties_add_float_slider(pr, P_MASK_REGION_FEATHER, P_TRANSLATE(P_MASK_REGION_FEATHER), 0.0, 50.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK_REGION_FEATHER)));
	p = obs_properties_add_float_slider(pr, P_MASK_REGION_FEATHER_SHIFT, P_TRANSLATE(P_MASK_REGION_FEATHER_SHIFT),
										-100.0, 100.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK_REGION_FEATHER_SHIFT)));
	p = obs_properties_add_bool(pr, P_MASK_REGION_INVERT, P_TRANSLATE(P_MASK_REGION_INVERT));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK_REGION_INVERT)));
	/// Image
	p = obs_properties_add_path(pr, P_MASK_IMAGE, P_TRANSLATE(P_MASK_IMAGE), OBS_PATH_FILE, P_TRANSLATE(""),
								nullptr);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK_IMAGE)));
	/// Source
	p = obs_properties_add_list(pr, P_MASK_SOURCE, P_TRANSLATE(P_MASK_SOURCE), OBS_COMBO_TYPE_LIST,
								OBS_COMBO_FORMAT_STRING);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK_SOURCE)));
	obs_enum_sources(
		[](void* ptr, obs_source_t* source) {
			obs_property_t* p = reinterpret_cast<obs_property_t*>(ptr);
			obs_property_list_add_string(p, std::string(std::string(obs_source_get_name(source)) + " (Source)").c_str(),
										 obs_source_get_name(source));
			return true;
		},
		p);
	blur_factory::get()->enum_scenes([this, p](obs_scene_t* scene) {
		struct data {
			blur_instance*  self;
			obs_property_t* prop;
			std::string     parent_name;
		};

		obs_source_t* scene_source = obs_scene_get_source(scene);
		P_LOG_DEBUG("<filter-blur> Instance '%s' adding scene '%s'.", obs_source_get_name(m_source),
					obs_source_get_name(scene_source));
		obs_property_list_add_string(p,
									 std::string(std::string(obs_source_get_name(scene_source)) + " (Scene)").c_str(),
									 obs_source_get_name(scene_source));
		return true;
	});

	/// Shared
	p = obs_properties_add_color(pr, P_MASK_COLOR, P_TRANSLATE(P_MASK_COLOR));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK_COLOR)));
	p = obs_properties_add_float_slider(pr, P_MASK_ALPHA, P_TRANSLATE(P_MASK_ALPHA), 0.0, 100.0, 0.1);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK_ALPHA)));
	p = obs_properties_add_float_slider(pr, P_MASK_MULTIPLIER, P_TRANSLATE(P_MASK_MULTIPLIER), 0.0, 10.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_MASK_MULTIPLIER)));

	// Directional Blur
	p = obs_properties_add_bool(pr, P_DIRECTIONAL, P_TRANSLATE(P_DIRECTIONAL));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_DIRECTIONAL)));
	obs_property_set_modified_callback2(p, modified_properties, this);
	p = obs_properties_add_float_slider(pr, P_DIRECTIONAL_ANGLE, P_TRANSLATE(P_DIRECTIONAL_ANGLE), -180.0, 180.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_DIRECTIONAL_ANGLE)));

	// Scaling
	p = obs_properties_add_bool(pr, P_STEPSCALE, P_TRANSLATE(P_STEPSCALE));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_STEPSCALE)));
	obs_property_set_modified_callback2(p, modified_properties, this);
	p = obs_properties_add_float_slider(pr, P_STEPSCALE_X, P_TRANSLATE(P_STEPSCALE_X), 0.0, 1000.0, 0.01);
	p = obs_properties_add_float_slider(pr, P_STEPSCALE_Y, P_TRANSLATE(P_STEPSCALE_Y), 0.0, 1000.0, 0.01);

	// advanced
	p = obs_properties_add_bool(pr, S_ADVANCED, P_TRANSLATE(S_ADVANCED));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(S_ADVANCED)));
	obs_property_set_modified_callback2(p, modified_properties, this);

	p = obs_properties_add_list(pr, P_COLORFORMAT, P_TRANSLATE(P_COLORFORMAT), OBS_COMBO_TYPE_LIST,
								OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_COLORFORMAT)));
	obs_property_list_add_int(p, "RGB", ColorFormat::RGB);
	obs_property_list_add_int(p, "YUV", ColorFormat::YUV);

	return pr;
}

void filter::blur::blur_instance::update(obs_data_t* settings)
{
	type           = (blur::type)obs_data_get_int(settings, P_TYPE);
	blur_effect    = blur_factory::get()->get_effect(type);
	blur_technique = blur_factory::get()->get_technique(type);
	size           = (uint64_t)obs_data_get_int(settings, P_SIZE);

	// bilateral blur
	bilateral_smoothing = obs_data_get_double(settings, P_BILATERAL_SMOOTHING) / 100.0;
	bilateral_sharpness = obs_data_get_double(settings, P_BILATERAL_SHARPNESS) / 100.0;

	// region
	mask.enabled = obs_data_get_bool(settings, P_MASK);
	if (mask.enabled) {
		mask.type = static_cast<mask_type>(obs_data_get_int(settings, P_MASK_TYPE));
		switch (mask.type) {
		case mask_type::Region:
			mask.region.left          = float_t(obs_data_get_double(settings, P_MASK_REGION_LEFT) / 100.0);
			mask.region.top           = float_t(obs_data_get_double(settings, P_MASK_REGION_TOP) / 100.0);
			mask.region.right         = 1.0f - float_t(obs_data_get_double(settings, P_MASK_REGION_RIGHT) / 100.0);
			mask.region.bottom        = 1.0f - float_t(obs_data_get_double(settings, P_MASK_REGION_BOTTOM) / 100.0);
			mask.region.feather       = float_t(obs_data_get_double(settings, P_MASK_REGION_FEATHER) / 100.0);
			mask.region.feather_shift = float_t(obs_data_get_double(settings, P_MASK_REGION_FEATHER_SHIFT) / 100.0);
			mask.region.invert        = obs_data_get_bool(settings, P_MASK_REGION_INVERT);
			break;
		case mask_type::Image:
			mask.image.path = obs_data_get_string(settings, P_MASK_IMAGE);
			break;
		case mask_type::Source:
			mask.source.name = obs_data_get_string(settings, P_MASK_SOURCE);
			break;
		}
		if ((mask.type == mask_type::Image) || (mask.type == mask_type::Source)) {
			uint32_t color  = static_cast<uint32_t>(obs_data_get_int(settings, P_MASK_COLOR));
			mask.color.r    = ((color >> 0) & 0xFF) / 255.0f;
			mask.color.g    = ((color >> 8) & 0xFF) / 255.0f;
			mask.color.b    = ((color >> 16) & 0xFF) / 255.0f;
			mask.color.a    = static_cast<float_t>(obs_data_get_double(settings, P_MASK_ALPHA));
			mask.multiplier = float_t(obs_data_get_double(settings, P_MASK_MULTIPLIER));
		}
	}

	// Directional Blur
	this->directional = obs_data_get_bool(settings, P_DIRECTIONAL);
	this->angle       = obs_data_get_double(settings, P_DIRECTIONAL_ANGLE);

	// Scaling
	this->scaling      = obs_data_get_bool(settings, P_STEPSCALE);
	this->scale.first  = obs_data_get_double(settings, P_STEPSCALE_X) / 100.0;
	this->scale.second = obs_data_get_double(settings, P_STEPSCALE_Y) / 100.0;

	// advanced
	if (obs_data_get_bool(settings, S_ADVANCED)) {
		color_format = obs_data_get_int(settings, P_COLORFORMAT);
	} else {
		color_format = obs_data_get_default_int(settings, P_COLORFORMAT);
	}
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
	if (mask.type == mask_type::Image) {
		if (mask.image.path_old != mask.image.path) {
			try {
				mask.image.texture  = std::make_shared<gs::texture>(mask.image.path);
				mask.image.path_old = mask.image.path;
			} catch (...) {
				P_LOG_ERROR("<filter-blur> Instance '%s' failed to load image '%s'.", obs_source_get_name(m_source),
							mask.image.path.c_str());
			}
		}
	} else if (mask.type == mask_type::Source) {
		if (mask.source.name_old != mask.source.name) {
			try {
				mask.source.source_texture = std::make_shared<gfx::source_texture>(mask.source.name, m_source);
				mask.source.is_scene = (obs_scene_from_source(mask.source.source_texture->get_object()) != nullptr);
				mask.source.name_old = mask.source.name;
			} catch (...) {
				P_LOG_ERROR("<filter-blur> Instance '%s' failed to grab source '%s'.", obs_source_get_name(m_source),
							mask.source.name.c_str());
			}
		}
	}
}

void filter::blur::blur_instance::video_render(gs_effect_t* effect)
{
	obs_source_t* parent = obs_filter_get_parent(this->m_source);
	obs_source_t* target = obs_filter_get_target(this->m_source);
	uint32_t      baseW  = obs_source_get_base_width(target);
	uint32_t      baseH  = obs_source_get_base_height(target);
	vec4          black;
	vec4_set(&black, 0, 0, 0, 0);

	bool failed = false;

	std::shared_ptr<gs::effect> colorConversionEffect = blur_factory::get()->get_color_converter_effect();

	// Verify that we can actually run first.
	if (!target || !parent || !this->m_source) {
		if (this->can_log()) {
			P_LOG_ERROR("<filter-blur:%s> Invalid context, memory corruption may have happened.",
						obs_source_get_name(this->m_source));
		}
		obs_source_skip_video_filter(this->m_source);
		return;
	}
	if ((baseW == 0) || (baseH == 0)) {
		if (this->can_log()) {
			P_LOG_ERROR("<filter-blur:%s> Invalid base size from '%s': %" PRIu32 "x%" PRIu32 ".",
						obs_source_get_name(this->m_source), obs_source_get_name(target), baseW, baseH);
		}
		obs_source_skip_video_filter(this->m_source);
		return;
	}
	if (!this->rt_primary || !this->rt_secondary || !this->blur_effect) {
		if (this->can_log()) {
			P_LOG_ERROR("<filter-blur:%s> Missing RenderTarget or Effect.", obs_source_get_name(this->m_source));
		}
		obs_source_skip_video_filter(this->m_source);
		return;
	}

	gs_effect_t*                 defaultEffect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_DEFAULT);
	std::shared_ptr<gs::texture> tex_source;
	std::shared_ptr<gs::texture> tex_intermediate;

	// Source To Texture
	{
		gs_blend_state_push();
		gs_reset_blend_state();
		gs_enable_color(true, true, true, true);
		gs_enable_blending(true);
		gs_enable_depth_test(false);
		gs_enable_stencil_test(false);
		gs_enable_stencil_write(false);
		gs_set_cull_mode(GS_NEITHER);
		gs_depth_function(GS_ALWAYS);
		gs_blend_function(GS_BLEND_ONE, GS_BLEND_INVSRCALPHA);
		gs_stencil_function(GS_STENCIL_BOTH, GS_ALWAYS);
		gs_stencil_op(GS_STENCIL_BOTH, GS_ZERO, GS_ZERO, GS_ZERO);

		try {
			auto op = this->rt_source->render(baseW, baseH);

			// Orthographic Camera and clear RenderTarget.
			gs_ortho(0, (float)baseW, 0, (float)baseH, -1., 1.);
			gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &black, 0, 0);

			// Render
			if (obs_source_process_filter_begin(this->m_source, GS_RGBA, OBS_NO_DIRECT_RENDERING)) {
				obs_source_process_filter_end(this->m_source, defaultEffect, baseW, baseH);
			} else {
				throw std::runtime_error("Failed to render source");
			}
		} catch (std::exception ex) {
			if (this->can_log()) {
				P_LOG_ERROR("<filter-blur:%s> Rendering parent source to texture failed with error: %s.",
							obs_source_get_name(this->m_source), ex.what());
			}
			gs_blend_state_pop();
			obs_source_skip_video_filter(this->m_source);
			return;
		}
		gs_blend_state_pop();

		if (!(tex_source = this->rt_source->get_texture())) {
			if (this->can_log()) {
				P_LOG_ERROR("<filter-blur> Failed to get source texture.");
			}
			obs_source_skip_video_filter(m_source);
			return;
		}
	}

	// Color Conversion RGB-YUV
	if ((color_format == ColorFormat::YUV) && colorConversionEffect) {
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

		try {
			auto op = this->rt_primary->render(baseW, baseH);
			gs_ortho(0, (float)baseW, 0, (float)baseH, -1, 1);

			if (colorConversionEffect->has_parameter("image")) {
				colorConversionEffect->get_parameter("image").set_texture(tex_source->get_object());
			}
			while (gs_effect_loop(colorConversionEffect->get_object(), "RGBToYUV")) {
				gs_draw_sprite(tex_source->get_object(), 0, baseW, baseH);
			}
		} catch (std::exception ex) {
			if (this->can_log()) {
				P_LOG_ERROR("<filter-blur:%s> RGB-YUV conversion failed with error: %s.",
							obs_source_get_name(this->m_source), ex.what());
			}
			gs_blend_state_pop();
			obs_source_skip_video_filter(m_source);
			return;
		}
		gs_blend_state_pop();

		if (!(tex_source = this->rt_primary->get_texture())) {
			if (this->can_log()) {
				P_LOG_ERROR("<filter-blur:%s> Failed to get color conversion texture.",
							obs_source_get_name(this->m_source));
			}
			obs_source_skip_video_filter(m_source);
			return;
		}

		// Swap RTs for further rendering.
		std::swap(this->rt_primary, this->rt_secondary);
	}

	// Blur
	{
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

		std::pair<float, float> kvs[] = {{1.0f / baseW, 0.0f}, {0.0f, 1.0f / baseH}};
		tex_intermediate              = tex_source; // We need the original to work.

		// Directional Blur
		if (this->directional) {
			// Directional Blur changes how

			kvs[0].first  = (1.0f / baseW);
			kvs[0].second = (1.0f / baseH);
			kvs[1].first  = kvs[0].first;
			kvs[1].second = kvs[0].second;

			double_t rad = this->angle * PI / 180.0;
			double_t c0  = cos(rad);
			double_t s0  = sin(rad);

			kvs[0].first *= c0;
			kvs[0].second *= s0;

			if (!this->scaling) {
				kvs[1].first *= 0.0;
				kvs[1].second *= 0.0;
			} else {
				kvs[1].first *= s0;
				kvs[1].second *= c0;
			}
		}

		// Apply scaling
		if (this->scaling) {
			if (!this->directional) {
				kvs[0].first *= float_t(this->scale.first);
				kvs[0].second *= float_t(this->scale.second);
				kvs[1].first *= float_t(this->scale.first);
				kvs[1].second *= float_t(this->scale.second);
			} else {
				// Directional Blur changes how scaling works as it rotates and needs to be relative to the axis of rotation.
				kvs[0].first *= float_t(this->scale.first);
				kvs[0].second *= float_t(this->scale.first);
				kvs[1].first *= float_t(this->scale.second);
				kvs[1].second *= float_t(this->scale.second);
			}
		}

		try {
			for (auto v : kvs) {
				float xpel = std::get<0>(v);
				float ypel = std::get<1>(v);
				if ((abs(xpel) <= FLT_EPSILON) && (abs(ypel) <= FLT_EPSILON)) {
					// Ignore passes that have a 0 texel modifier.
					continue;
				}

				{
					auto op = this->rt_primary->render(baseW, baseH);
					gs_ortho(0, (float)baseW, 0, (float)baseH, -1, 1);

					apply_shared_param(tex_intermediate->get_object(), xpel, ypel);
					apply_gaussian_param(uint8_t(this->size));
					apply_bilateral_param();

					// Render
					while (gs_effect_loop(this->blur_effect->get_object(), this->blur_technique.c_str())) {
						gs_draw_sprite(tex_intermediate->get_object(), 0, baseW, baseH);
					}
				}

				if (!(tex_intermediate = this->rt_primary->get_texture())) {
					throw("Failed to get blur texture.");
				}

				// Swap RTs for further rendering.
				std::swap(this->rt_primary, this->rt_secondary);
			}
		} catch (std::exception ex) {
			if (this->can_log()) {
				P_LOG_ERROR("<filter-blur:%s> Blur failed with error: %s.", obs_source_get_name(this->m_source),
							ex.what());
			}
			gs_blend_state_pop();
			obs_source_skip_video_filter(this->m_source);
			return;
		}
		gs_blend_state_pop();
	}

	// Mask
	if (mask.enabled) {
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
		switch (this->mask.type) {
		case Region:
			if (this->mask.region.feather > 0.001) {
				if (this->mask.region.invert) {
					technique = "RegionFeatherInverted";
				} else {
					technique = "RegionFeather";
				}
			} else {
				if (this->mask.region.invert) {
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

		if (mask.source.source_texture) {
			uint32_t source_width  = obs_source_get_width(this->mask.source.source_texture->get_object());
			uint32_t source_height = obs_source_get_height(this->mask.source.source_texture->get_object());

			if (source_width == 0) {
				source_width = baseW;
			}
			if (source_height == 0) {
				source_height = baseH;
			}
			if (this->mask.source.is_scene) {
				obs_video_info ovi;
				if (obs_get_video_info(&ovi)) {
					source_width  = ovi.base_width;
					source_height = ovi.base_height;
				}
			}

			this->mask.source.texture = this->mask.source.source_texture->render(source_width, source_height);
		}

		std::shared_ptr<gs::effect> mask_effect = blur_factory::get()->get_mask_effect();
		apply_mask_parameters(mask_effect, tex_source->get_object(), tex_intermediate->get_object());

		try {
			auto op = this->rt_primary->render(baseW, baseH);
			gs_ortho(0, (float)baseW, 0, (float)baseH, -1, 1);

			// Render
			while (gs_effect_loop(mask_effect->get_object(), technique.c_str())) {
				gs_draw_sprite(tex_intermediate->get_object(), 0, baseW, baseH);
			}

		} catch (std::exception ex) {
			if (this->can_log()) {
				P_LOG_ERROR("<filter-blur:%s> Masking failed with error: %s.", obs_source_get_name(this->m_source),
							ex.what());
			}
			gs_blend_state_pop();
			obs_source_skip_video_filter(this->m_source);
			return;
		}
		gs_blend_state_pop();

		if (!(tex_intermediate = this->rt_primary->get_texture())) {
			if (this->can_log()) {
				P_LOG_ERROR("<filter-blur:%s> Failed to get masked texture.", obs_source_get_name(this->m_source));
			}
			obs_source_skip_video_filter(this->m_source);
			return;
		}

		// Swap RTs for further rendering.
		std::swap(this->rt_primary, this->rt_secondary);
	}

	// Color Conversion RGB-YUV or Straight Draw
	{
		// It is important that we do not modify the blend state here, as it is set correctly by OBS
		gs_enable_color(true, true, true, true);
		gs_enable_depth_test(false);
		gs_enable_stencil_test(false);
		gs_enable_stencil_write(false);
		gs_set_cull_mode(GS_NEITHER);
		gs_depth_function(GS_ALWAYS);
		gs_stencil_function(GS_STENCIL_BOTH, GS_ALWAYS);
		gs_stencil_op(GS_STENCIL_BOTH, GS_ZERO, GS_ZERO, GS_ZERO);

		gs_effect_t* finalEffect = effect ? effect : defaultEffect;
		const char*  technique   = "Draw";

		if ((color_format == ColorFormat::YUV) && colorConversionEffect) {
			finalEffect = colorConversionEffect->get_object();
			technique   = "YUVToRGB";
		}

		gs_eparam_t* param = gs_effect_get_param_by_name(finalEffect, "image");
		if (!param) {
			P_LOG_ERROR("<filter-blur:%s> Failed to set image param.", obs_source_get_name(this->m_source));
			failed = true;
		} else {
			gs_effect_set_texture(param, tex_intermediate->get_object());
		}
		while (gs_effect_loop(finalEffect, technique)) {
			gs_draw_sprite(tex_intermediate->get_object(), 0, baseW, baseH);
		}
	}

	if (failed) {
		obs_source_skip_video_filter(m_source);
		return;
	}
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

	obs_register_source(&source_info);

	auto osi = obs_get_signal_handler();
	signal_handler_connect(osi, "source_create", scene_create_handler, this);
	signal_handler_connect(osi, "source_destroy", scene_destroy_handler, this);
}

filter::blur::blur_factory::~blur_factory()
{
	auto osi = obs_get_signal_handler();
	signal_handler_disconnect(osi, "source_create", scene_create_handler, this);
	signal_handler_disconnect(osi, "source_destroy", scene_destroy_handler, this);
}

void filter::blur::blur_factory::on_list_fill()
{
	obs_enter_graphics();

	{
		char* file = obs_module_file("effects/blur.effect");
		try {
			blur_effect = std::make_shared<gs::effect>(file);
		} catch (std::runtime_error ex) {
			P_LOG_ERROR("<filter-blur> Loading effect '%s' failed with error(s): %s", file, ex.what());
		}
		bfree(file);
	}
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

	generate_kernel_textures();
	obs_leave_graphics();
}

void filter::blur::blur_factory::on_list_empty()
{
	obs_enter_graphics();
	blur_effect.reset();
	kernels.clear();
	color_converter_effect.reset();
	mask_effect.reset();
	obs_leave_graphics();
}

void filter::blur::blur_factory::generate_gaussian_kernels()
{
	// 2D texture, horizontal is value, vertical is kernel size.
	size_t size_power_of_two = size_t(pow(2, util::math::get_power_of_two_exponent_ceil(max_kernel_size)));

	std::vector<float_t>                  texture_data(size_power_of_two * size_power_of_two);
	std::vector<float_t>                  math_data(size_power_of_two);
	std::shared_ptr<std::vector<float_t>> kernel_data;

	for (size_t width = 1; width <= max_kernel_size; width++) {
		size_t v    = (width - 1) * size_power_of_two;
		kernel_data = std::make_shared<std::vector<float_t>>(size_power_of_two);

		// Calculate and normalize
		float_t sum = 0;
		for (size_t p = 0; p <= width; p++) {
			math_data[p] = float_t(Gaussian1D(double_t(p), double_t(width)));
			sum += math_data[p] * (p > 0 ? 2 : 1);
		}

		// Normalize to Texture Buffer
		double_t inverse_sum = 1.0 / sum;
		for (size_t p = 0; p <= width; p++) {
			texture_data[v + p] = float_t(math_data[p] * inverse_sum);
			kernel_data->at(p)  = texture_data[v + p];
		}

		gaussian_kernels.insert({uint8_t(width), kernel_data});
	}

	// Create Texture
	try {
		auto texture_buffer = reinterpret_cast<uint8_t*>(texture_data.data());
		auto unsafe_buffer  = const_cast<const uint8_t**>(&texture_buffer);

		kernels.insert_or_assign(filter::blur::type::Gaussian,
								 std::make_shared<gs::texture>(uint32_t(size_power_of_two), uint32_t(size_power_of_two),
															   GS_R32F, 1, unsafe_buffer, gs::texture::flags::None));
	} catch (std::runtime_error ex) {
		P_LOG_ERROR("<filter-blur> Failed to create gaussian kernel texture.");
	}
}

void filter::blur::blur_factory::generate_kernel_textures()
{
	generate_gaussian_kernels();
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
}

void filter::blur::blur_factory::get_defaults(obs_data_t* data)
{
	obs_data_set_default_int(data, P_TYPE, filter::blur::type::Box);
	obs_data_set_default_int(data, P_SIZE, 5);

	// Bilateral Only
	obs_data_set_default_double(data, P_BILATERAL_SMOOTHING, 50.0);
	obs_data_set_default_double(data, P_BILATERAL_SHARPNESS, 90.0);

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
	char* default_file = obs_module_file("white.png");
	obs_data_set_default_string(data, P_MASK_IMAGE, default_file);
	bfree(default_file);
	obs_data_set_default_string(data, P_MASK_SOURCE, "");
	obs_data_set_default_int(data, P_MASK_COLOR, 0xFFFFFFFFull);
	obs_data_set_default_double(data, P_MASK_MULTIPLIER, 1.0);

	// Directional Blur
	obs_data_set_default_bool(data, P_DIRECTIONAL, false);
	obs_data_set_default_double(data, P_DIRECTIONAL_ANGLE, 0.0);

	// Scaling
	obs_data_set_default_bool(data, P_STEPSCALE, false);
	obs_data_set_default_double(data, P_STEPSCALE_X, 100.0);
	obs_data_set_default_double(data, P_STEPSCALE_Y, 100.0);

	// Advanced
	obs_data_set_default_bool(data, S_ADVANCED, false);
	obs_data_set_default_int(data, P_COLORFORMAT, ColorFormat::RGB);
}

obs_properties_t* filter::blur::blur_factory::get_properties(void* inptr)
{
	return reinterpret_cast<filter::blur::blur_instance*>(inptr)->get_properties();
}

void filter::blur::blur_factory::update(void* inptr, obs_data_t* settings)
{
	reinterpret_cast<filter::blur::blur_instance*>(inptr)->update(settings);
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

void filter::blur::blur_factory::scene_create_handler(void* ptr, calldata_t* data)
{
	filter::blur::blur_factory* self   = reinterpret_cast<filter::blur::blur_factory*>(ptr);
	obs_source_t*               source = nullptr;
	calldata_get_ptr(data, "source", &source);
	obs_scene_t* scene = obs_scene_from_source(source);
	if (scene) {
		self->scenes.insert_or_assign(std::string(obs_source_get_name(source)), scene);
	}
}

void filter::blur::blur_factory::scene_destroy_handler(void* ptr, calldata_t* data)
{
	filter::blur::blur_factory* self   = reinterpret_cast<filter::blur::blur_factory*>(ptr);
	obs_source_t*               source = nullptr;
	calldata_get_ptr(data, "source", &source);
	obs_scene_t* scene = obs_scene_from_source(source);
	if (scene) {
		self->scenes.erase(std::string(obs_source_get_name(source)));
	}
}

std::shared_ptr<gs::effect> filter::blur::blur_factory::get_effect(filter::blur::type)
{
	return blur_effect;
}

std::string filter::blur::blur_factory::get_technique(filter::blur::type type)
{
	switch (type) {
	case type::Box:
		return "Box";
	case type::Gaussian:
		return "Gaussian";
	case type::Bilateral:
		return "Bilateral";
	case type::BoxLinear:
		return "BoxLinear";
	case type::GaussianLinear:
		return "GaussianLinear";
	}
	return "";
}

std::shared_ptr<gs::effect> filter::blur::blur_factory::get_color_converter_effect()
{
	return color_converter_effect;
}

std::shared_ptr<gs::effect> filter::blur::blur_factory::get_mask_effect()
{
	return mask_effect;
}

std::shared_ptr<gs::texture> filter::blur::blur_factory::get_kernel(filter::blur::type type)
{
	return kernels.at(type);
}

std::shared_ptr<std::vector<float_t>> filter::blur::blur_factory::get_gaussian_kernel(uint8_t size)
{
	return gaussian_kernels.at(size);
}

obs_scene_t* filter::blur::blur_factory::get_scene(std::string name)
{
	auto kv = scenes.find(name);
	if (kv != scenes.end()) {
		return kv->second;
	}
	return nullptr;
}

void filter::blur::blur_factory::enum_scenes(std::function<bool(obs_scene_t*)> fnc)
{
	for (auto kv : scenes) {
		if (!fnc(kv.second)) {
			break;
		}
	}
}

static filter::blur::blur_factory* factory_instance = nullptr;

void filter::blur::blur_factory::initialize()
{
	factory_instance = new filter::blur::blur_factory();
}

void filter::blur::blur_factory::finalize()
{
	delete factory_instance;
}

filter::blur::blur_factory* filter::blur::blur_factory::get()
{
	return factory_instance;
}
