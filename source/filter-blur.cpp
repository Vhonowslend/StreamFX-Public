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

#include "filter-blur.h"
#include <inttypes.h>
#include <map>
#include <math.h>
#include "strings.h"
#include "util-math.h"

extern "C" {
#pragma warning(push)
#pragma warning(disable : 4201)
#include "graphics/graphics.h"
#include "graphics/matrix4.h"
#include "util/platform.h"
#pragma warning(pop)
}

// Translation Strings
#define SOURCE_NAME "Filter.Blur"

#define P_TYPE "Filter.Blur.Type"
#define P_TYPE_BOX "Filter.Blur.Type.Box"
#define P_TYPE_GAUSSIAN "Filter.Blur.Type.Gaussian"
#define P_TYPE_BILATERAL "Filter.Blur.Type.Bilateral"
#define P_SIZE "Filter.Blur.Size"
#define P_BILATERAL_SMOOTHING "Filter.Blur.Bilateral.Smoothing"
#define P_BILATERAL_SHARPNESS "Filter.Blur.Bilateral.Sharpness"
#define P_REGION "Filter.Blur.Region"
#define P_REGION_LEFT "Filter.Blur.Region.Left"
#define P_REGION_RIGHT "Filter.Blur.Region.Right"
#define P_REGION_TOP "Filter.Blur.Region.Top"
#define P_REGION_BOTTOM "Filter.Blur.Region.Bottom"
#define P_REGION_FEATHER "Filter.Blur.Region.Feather"
#define P_REGION_FEATHER_SHIFT "Filter.Blur.Region.Feather.Shift"
#define P_REGION_INVERT "Filter.Blur.Region.Invert"
#define P_COLORFORMAT "Filter.Blur.ColorFormat"

// Initializer & Finalizer
INITIALIZER(filterBlurFactoryInitializer)
{
	initializerFunctions.push_back([] { filter::blur::factory::initialize(); });
	finalizerFunctions.push_back([] { filter::blur::factory::finalize(); });
}

enum ColorFormat : uint64_t { // ToDo: Refactor into full class.
	RGB,
	YUV, // 701
};

static uint8_t const max_kernel_size = 25;

bool filter::blur::instance::apply_shared_param(gs_texture_t* input, float texelX, float texelY)
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

	if (region.enabled) {
		if (blur_effect->has_parameter("regionLeft")) {
			blur_effect->get_parameter("regionLeft").set_float(region.left);
		}
		if (blur_effect->has_parameter("regionTop")) {
			blur_effect->get_parameter("regionTop").set_float(region.top);
		}
		if (blur_effect->has_parameter("regionRight")) {
			blur_effect->get_parameter("regionRight").set_float(region.right);
		}
		if (blur_effect->has_parameter("regionBottom")) {
			blur_effect->get_parameter("regionBottom").set_float(region.bottom);
		}
		if (blur_effect->has_parameter("regionFeather")) {
			blur_effect->get_parameter("regionFeather").set_float(region.feather);
		}
		if (blur_effect->has_parameter("regionFeatherShift")) {
			blur_effect->get_parameter("regionFeatherShift").set_float(region.feather_shift);
		}
	}

	return result;
}

bool filter::blur::instance::apply_bilateral_param()
{
	gs_eparam_t* param;

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

bool filter::blur::instance::apply_gaussian_param()
{
	std::shared_ptr<gs::texture> kernel = filter::blur::factory::get()->get_kernel(filter::blur::type::Gaussian);

	if (blur_effect->has_parameter("kernel")) {
		blur_effect->get_parameter("kernel").set_texture(kernel);
	} else {
		return false;
	}

	if (blur_effect->has_parameter("kernelTexel")) {
		float_t wb = 1.0f / kernel->get_width();
		float_t hb = 1.0f / kernel->get_height();
		blur_effect->get_parameter("kernelTexel").set_float2(wb, hb);
	}

	return true;
}

bool filter::blur::instance::modified_properties(void* ptr, obs_properties_t* props, obs_property* prop,
												 obs_data_t* settings)
{
	bool showBilateral = (obs_data_get_int(settings, P_TYPE) == type::Bilateral);

	// bilateral blur
	obs_property_set_visible(obs_properties_get(props, P_BILATERAL_SMOOTHING), showBilateral);
	obs_property_set_visible(obs_properties_get(props, P_BILATERAL_SHARPNESS), showBilateral);

	// region
	bool showRegion = obs_data_get_bool(settings, P_REGION);
	obs_property_set_visible(obs_properties_get(props, P_REGION_LEFT), showRegion);
	obs_property_set_visible(obs_properties_get(props, P_REGION_TOP), showRegion);
	obs_property_set_visible(obs_properties_get(props, P_REGION_RIGHT), showRegion);
	obs_property_set_visible(obs_properties_get(props, P_REGION_BOTTOM), showRegion);
	obs_property_set_visible(obs_properties_get(props, P_REGION_FEATHER), showRegion);
	obs_property_set_visible(obs_properties_get(props, P_REGION_FEATHER_SHIFT), showRegion);
	obs_property_set_visible(obs_properties_get(props, P_REGION_INVERT), showRegion);

	// advanced
	bool showAdvanced = obs_data_get_bool(settings, S_ADVANCED);
	obs_property_set_visible(obs_properties_get(props, P_COLORFORMAT), showAdvanced);

	return true;
}

filter::blur::instance::instance(obs_data_t* settings, obs_source_t* parent)
{
	m_source = parent;

	obs_enter_graphics();
	blur_effect             = filter::blur::factory::get()->get_effect(filter::blur::type::Box);
	primary_rendertarget    = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	secondary_rendertarget  = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	horizontal_rendertarget = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	vertical_rendertarget   = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	obs_leave_graphics();

	if (!primary_rendertarget) {
		P_LOG_ERROR("<filter-blur> Instance '%s' failed to create primary rendertarget.",
					obs_source_get_name(m_source));
	}

	if (!secondary_rendertarget) {
		P_LOG_ERROR("<filter-blur> Instance '%s' failed to create secondary rendertarget.",
					obs_source_get_name(m_source));
	}

	if (!horizontal_rendertarget) {
		P_LOG_ERROR("<filter-blur> Instance '%s' failed to create horizontal rendertarget.",
					obs_source_get_name(m_source));
	}

	if (!vertical_rendertarget) {
		P_LOG_ERROR("<filter-blur> Instance '%s' failed to create vertical rendertarget.",
					obs_source_get_name(m_source));
	}

	update(settings);
}

filter::blur::instance::~instance()
{
	obs_enter_graphics();
	gs_texrender_destroy(primary_rendertarget);
	gs_texrender_destroy(secondary_rendertarget);
	gs_texrender_destroy(horizontal_rendertarget);
	gs_texrender_destroy(vertical_rendertarget);
	obs_leave_graphics();
}

obs_properties_t* filter::blur::instance::get_properties()
{
	obs_properties_t* pr = obs_properties_create();
	obs_property_t*   p  = NULL;

	p = obs_properties_add_list(pr, P_TYPE, P_TRANSLATE(P_TYPE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_TYPE)));
	obs_property_set_modified_callback2(p, modified_properties, this);
	obs_property_list_add_int(p, P_TRANSLATE(P_TYPE_BOX), filter::blur::type::Box);
	obs_property_list_add_int(p, P_TRANSLATE(P_TYPE_GAUSSIAN), filter::blur::type::Gaussian);
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

	// region
	p = obs_properties_add_bool(pr, P_REGION, P_TRANSLATE(P_REGION));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_REGION)));
	obs_property_set_modified_callback2(p, modified_properties, this);
	p = obs_properties_add_float_slider(pr, P_REGION_LEFT, P_TRANSLATE(P_REGION_LEFT), 0.0, 100.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_REGION_LEFT)));
	p = obs_properties_add_float_slider(pr, P_REGION_TOP, P_TRANSLATE(P_REGION_TOP), 0.0, 100.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_REGION_TOP)));
	p = obs_properties_add_float_slider(pr, P_REGION_RIGHT, P_TRANSLATE(P_REGION_RIGHT), 0.0, 100.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_REGION_RIGHT)));
	p = obs_properties_add_float_slider(pr, P_REGION_BOTTOM, P_TRANSLATE(P_REGION_BOTTOM), 0.0, 100.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_REGION_BOTTOM)));
	p = obs_properties_add_float_slider(pr, P_REGION_FEATHER, P_TRANSLATE(P_REGION_FEATHER), 0.0, 50.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_REGION_FEATHER)));
	p = obs_properties_add_float_slider(pr, P_REGION_FEATHER_SHIFT, P_TRANSLATE(P_REGION_FEATHER_SHIFT), -100.0, 100.0,
										0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_REGION_FEATHER_SHIFT)));
	p = obs_properties_add_bool(pr, P_REGION_INVERT, P_TRANSLATE(P_REGION_INVERT));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_REGION_INVERT)));

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

void filter::blur::instance::update(obs_data_t* settings)
{
	type        = (blur::type)obs_data_get_int(settings, P_TYPE);
	blur_effect = factory::get()->get_effect(type);
	size        = (uint64_t)obs_data_get_int(settings, P_SIZE);

	// bilateral blur
	bilateral_smoothing = obs_data_get_double(settings, P_BILATERAL_SMOOTHING) / 100.0;
	bilateral_sharpness = obs_data_get_double(settings, P_BILATERAL_SHARPNESS) / 100.0;

	// region
	region.enabled = obs_data_get_bool(settings, P_REGION);
	if (region.enabled) {
		region.left          = float_t(obs_data_get_double(settings, P_REGION_LEFT) / 100.0);
		region.top           = float_t(obs_data_get_double(settings, P_REGION_TOP) / 100.0);
		region.right         = 1.0 - float_t(obs_data_get_double(settings, P_REGION_RIGHT) / 100.0);
		region.bottom        = 1.0 - float_t(obs_data_get_double(settings, P_REGION_BOTTOM) / 100.0);
		region.feather       = float_t(obs_data_get_double(settings, P_REGION_FEATHER) / 100.0);
		region.feather_shift = float_t(obs_data_get_double(settings, P_REGION_FEATHER_SHIFT) / 100.0);
		region.invert        = obs_data_get_bool(settings, P_REGION_INVERT);
	}

	// advanced
	if (obs_data_get_bool(settings, S_ADVANCED)) {
		color_format = obs_data_get_int(settings, P_COLORFORMAT);
	} else {
		color_format = obs_data_get_default_int(settings, P_COLORFORMAT);
	}
}

uint32_t filter::blur::instance::get_width()
{
	return uint32_t();
}

uint32_t filter::blur::instance::get_height()
{
	return uint32_t();
}

void filter::blur::instance::activate() {}

void filter::blur::instance::deactivate() {}

void filter::blur::instance::video_tick(float) {}

void filter::blur::instance::video_render(gs_effect_t* effect)
{
	obs_source_t* parent = obs_filter_get_parent(m_source);
	obs_source_t* target = obs_filter_get_target(m_source);
	uint32_t      baseW  = obs_source_get_base_width(target);
	uint32_t      baseH  = obs_source_get_base_height(target);
	vec4          black  = {0};

	bool failed = false;

	std::shared_ptr<gs::effect> colorConversionEffect = factory::get()->get_color_converter_effect();

	// Skip rendering if our target, parent or context is not valid.
	if (!target || !parent || !m_source) {
		obs_source_skip_video_filter(m_source);
		return;
	}
	if ((baseW <= 0) || (baseH <= 0)) {
		if (!have_logged_error)
			P_LOG_ERROR("<filter-blur> Instance '%s' has invalid size source '%s'.", obs_source_get_name(m_source),
						obs_source_get_name(target));
		have_logged_error = true;
		obs_source_skip_video_filter(m_source);
		return;
	}
	if (!primary_rendertarget || !blur_effect) {
		if (!have_logged_error)
			P_LOG_ERROR("<filter-blur> Instance '%s' is unable to render.", obs_source_get_name(m_source),
						obs_source_get_name(target));
		have_logged_error = true;
		obs_source_skip_video_filter(m_source);
		return;
	}
	have_logged_error = false;

	gs_effect_t*  defaultEffect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_DEFAULT);
	gs_texture_t* sourceTexture = nullptr;

#pragma region Source To Texture
	gs_texrender_reset(primary_rendertarget);
	if (!gs_texrender_begin(primary_rendertarget, baseW, baseH)) {
		P_LOG_ERROR("<filter-blur> Failed to set up base texture.");
		obs_source_skip_video_filter(m_source);
		return;
	} else {
		gs_ortho(0, (float)baseW, 0, (float)baseH, -1, 1);

		// Clear to Black
		gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &black, 0, 0);

		// Render
		if (obs_source_process_filter_begin(m_source, GS_RGBA, OBS_NO_DIRECT_RENDERING)) {
			obs_source_process_filter_end(m_source, effect ? effect : defaultEffect, baseW, baseH);
		} else {
			P_LOG_ERROR("<filter-blur> Unable to render source.");
			failed = true;
		}
		gs_texrender_end(primary_rendertarget);
	}

	if (failed) {
		obs_source_skip_video_filter(m_source);
		return;
	}

	sourceTexture = gs_texrender_get_texture(primary_rendertarget);
	if (!sourceTexture) {
		P_LOG_ERROR("<filter-blur> Failed to get source texture.");
		obs_source_skip_video_filter(m_source);
		return;
	}
#pragma endregion Source To Texture

	// Conversion
#pragma region RGB->YUV
	if ((color_format == ColorFormat::YUV) && colorConversionEffect) {
		gs_texrender_reset(secondary_rendertarget);
		if (!gs_texrender_begin(secondary_rendertarget, baseW, baseH)) {
			P_LOG_ERROR("<filter-blur> Failed to set up base texture.");
			obs_source_skip_video_filter(m_source);
			return;
		} else {
			gs_ortho(0, (float)baseW, 0, (float)baseH, -1, 1);

			// Clear to Black
			gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &black, 0, 0);

			// Set up camera stuff
			gs_set_cull_mode(GS_NEITHER);
			gs_reset_blend_state();
			gs_enable_blending(false);
			gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);
			gs_enable_depth_test(false);
			gs_enable_stencil_test(false);
			gs_enable_stencil_write(false);
			gs_enable_color(true, true, true, true);

			if (colorConversionEffect->has_parameter("image")) {
				colorConversionEffect->get_parameter("image").set_texture(sourceTexture);
			}
			while (gs_effect_loop(colorConversionEffect->get_object(), "RGBToYUV")) {
				gs_draw_sprite(sourceTexture, 0, baseW, baseH);
			}
			gs_texrender_end(secondary_rendertarget);
		}

		if (failed) {
			obs_source_skip_video_filter(m_source);
			return;
		}

		sourceTexture = gs_texrender_get_texture(secondary_rendertarget);
		if (!sourceTexture) {
			P_LOG_ERROR("<filter-blur> Failed to get source texture.");
			obs_source_skip_video_filter(m_source);
			return;
		}
	}
#pragma endregion RGB->YUV

#pragma region blur
	// Set up camera stuff
	gs_set_cull_mode(GS_NEITHER);
	gs_reset_blend_state();
	gs_enable_blending(true);
	gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);
	gs_enable_depth_test(false);
	gs_enable_stencil_test(false);
	gs_enable_stencil_write(false);
	gs_enable_color(true, true, true, true);

	gs_texture_t* blurred      = nullptr;
	gs_texture_t* intermediate = sourceTexture;

	std::tuple<const char*, gs_texrender_t*, float, float> kvs[] = {
		std::make_tuple("Horizontal", horizontal_rendertarget, 1.0f / baseW, 0.0f),
		std::make_tuple("Vertical", vertical_rendertarget, 0.0f, 1.0f / baseH),
	};

	std::string pass = "Draw";
	if (region.enabled) {
		if (region.feather > 0) {
			pass = "DrawRegionFeather";
		} else {
			pass = "DrawRegion";
		}
		if (region.invert) {
			pass += "Invert";
		}
	}
	for (auto v : kvs) {
		const char*     name = std::get<0>(v);
		gs_texrender_t* rt   = std::get<1>(v);
		float           xpel = std::get<2>(v), ypel = std::get<3>(v);

		if (!apply_shared_param(intermediate, xpel, ypel))
			break;
		switch (type) {
		case Gaussian:
			apply_gaussian_param();
			break;
		case Bilateral:
			apply_bilateral_param();
			break;
		}

		gs_texrender_reset(rt);
		if (!gs_texrender_begin(rt, baseW, baseH)) {
			P_LOG_ERROR("<filter-blur:%s> Failed to begin rendering.", name);
			break;
		}

		// Camera
		gs_ortho(0, (float)baseW, 0, (float)baseH, -1, 1);
		gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &black, 0, 0);

		// Render
		while (gs_effect_loop(blur_effect->get_object(), pass.c_str())) {
			gs_draw_sprite(intermediate, 0, baseW, baseH);
		}

		gs_texrender_end(rt);
		intermediate = gs_texrender_get_texture(rt);
		if (!intermediate) {
			P_LOG_ERROR("<filter-blur:%s> Failed to get intermediate texture.", name);
			break;
		}
		blurred = intermediate;
	}
	if (blurred == nullptr) {
		obs_source_skip_video_filter(m_source);
		return;
	}
#pragma endregion blur

#pragma region YUV->RGB or straight draw
	// Draw final effect
	{
		gs_effect_t* finalEffect = defaultEffect;
		const char*  technique   = "Draw";

		if ((color_format == ColorFormat::YUV) && colorConversionEffect) {
			finalEffect = colorConversionEffect->get_object();
			technique   = "YUVToRGB";
		}

		// Set up camera stuff
		gs_set_cull_mode(GS_NEITHER);
		gs_reset_blend_state();
		gs_enable_blending(true);
		gs_blend_function(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA);
		gs_enable_depth_test(false);
		gs_enable_stencil_test(false);
		gs_enable_stencil_write(false);
		gs_enable_color(true, true, true, true);

		gs_eparam_t* param = gs_effect_get_param_by_name(finalEffect, "image");
		if (!param) {
			P_LOG_ERROR("<filter-blur:Final> Failed to set image param.");
			failed = true;
		} else {
			gs_effect_set_texture(param, blurred);
		}
		while (gs_effect_loop(finalEffect, technique)) {
			gs_draw_sprite(blurred, 0, baseW, baseH);
		}
	}
#pragma endregion YUV->RGB or straight draw

	if (failed) {
		obs_source_skip_video_filter(m_source);
		return;
	}
}

filter::blur::factory::factory()
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
}

filter::blur::factory::~factory() {}

void filter::blur::factory::on_list_fill()
{
	obs_enter_graphics();
	std::pair<blur::type, std::string> shader_list[] = {
		{type::Box, obs_module_file("effects/box-blur.effect")},
		{type::Gaussian, obs_module_file("effects/gaussian-blur.effect")},
		{type::Bilateral, obs_module_file("effects/bilateral-blur.effect")},
	};
	for (auto& kv : shader_list) {
		try {
			std::shared_ptr<gs::effect> effect = std::make_shared<gs::effect>(kv.second);
			effects.insert(std::make_pair(kv.first, effect));
		} catch (std::runtime_error ex) {
			P_LOG_ERROR("<filter-blur> Loading effect '%s' failed with error(s): %s", kv.second.c_str(), ex.what());
			obs_leave_graphics();
			return;
		}
	}
	color_converter_effect = std::make_shared<gs::effect>(obs_module_file("effects/color-conversion.effect")),

	generate_kernel_textures();
	obs_leave_graphics();
}

void filter::blur::factory::on_list_empty()
{
	obs_enter_graphics();
	effects.clear();
	kernels.clear();
	obs_leave_graphics();
}

void filter::blur::factory::generate_gaussian_kernels()
{
	// 2D texture, horizontal is value, vertical is kernel size.
	size_t size_power_of_two = pow(2, util::math::get_power_of_two_exponent_ceil(max_kernel_size));

	std::vector<float_t> texture_Data(size_power_of_two * size_power_of_two);
	std::vector<float_t> math_data(size_power_of_two);

	for (size_t width = 1; width <= max_kernel_size; width++) {
		size_t v = (width - 1) * size_power_of_two;

		// Calculate and normalize
		float_t sum = 0;
		for (size_t p = 0; p <= width; p++) {
			math_data[p] = float_t(Gaussian1D(double_t(p), double_t(width)));
			sum += math_data[p] * (p > 0 ? 2 : 1);
		}

		// Normalize to Texture Buffer
		double_t inverse_sum = 1.0 / sum;
		for (size_t p = 0; p <= width; p++) {
			texture_Data[v + p] = float_t(math_data[p] * inverse_sum);
		}
	}

	// Create Texture
	try {
		auto texture_buffer = reinterpret_cast<uint8_t*>(texture_Data.data());
		auto unsafe_buffer  = const_cast<const uint8_t**>(&texture_buffer);

		kernels.insert_or_assign(filter::blur::type::Gaussian,
								 std::make_shared<gs::texture>(uint32_t(size_power_of_two), uint32_t(size_power_of_two),
															   GS_R32F, 1, unsafe_buffer, gs::texture::flags::None));
	} catch (std::runtime_error ex) {
		P_LOG_ERROR("<filter-blur> Failed to create gaussian kernel texture.");
	}
}

void filter::blur::factory::generate_kernel_textures()
{
	generate_gaussian_kernels();
}

void* filter::blur::factory::create(obs_data_t* data, obs_source_t* parent)
{
	if (get()->sources.empty()) {
		get()->on_list_fill();
	}
	filter::blur::instance* ptr = new filter::blur::instance(data, parent);
	get()->sources.push_back(ptr);
	return ptr;
}

void filter::blur::factory::destroy(void* inptr)
{
	filter::blur::instance* ptr = reinterpret_cast<filter::blur::instance*>(inptr);
	get()->sources.remove(ptr);
	if (get()->sources.empty()) {
		get()->on_list_empty();
	}
}

void filter::blur::factory::get_defaults(obs_data_t* data)
{
	obs_data_set_default_int(data, P_TYPE, filter::blur::type::Box);
	obs_data_set_default_int(data, P_SIZE, 5);

	// bilateral Only
	obs_data_set_default_double(data, P_BILATERAL_SMOOTHING, 50.0);
	obs_data_set_default_double(data, P_BILATERAL_SHARPNESS, 90.0);

	// region
	obs_data_set_default_bool(data, P_REGION, false);
	obs_data_set_default_double(data, P_REGION_LEFT, 0.0f);
	obs_data_set_default_double(data, P_REGION_TOP, 0.0f);
	obs_data_set_default_double(data, P_REGION_RIGHT, 0.0f);
	obs_data_set_default_double(data, P_REGION_BOTTOM, 0.0f);
	obs_data_set_default_double(data, P_REGION_FEATHER, 0.0f);
	obs_data_set_default_double(data, P_REGION_FEATHER_SHIFT, 0.0f);
	obs_data_set_default_bool(data, P_REGION_INVERT, false);

	// advanced
	obs_data_set_default_bool(data, S_ADVANCED, false);
	obs_data_set_default_int(data, P_COLORFORMAT, ColorFormat::RGB);
}

obs_properties_t* filter::blur::factory::get_properties(void* inptr)
{
	return reinterpret_cast<filter::blur::instance*>(inptr)->get_properties();
}

void filter::blur::factory::update(void* inptr, obs_data_t* settings)
{
	reinterpret_cast<filter::blur::instance*>(inptr)->update(settings);
}

const char* filter::blur::factory::get_name(void* inptr)
{
	return P_TRANSLATE(SOURCE_NAME);
}

uint32_t filter::blur::factory::get_width(void* inptr)
{
	return reinterpret_cast<filter::blur::instance*>(inptr)->get_width();
}

uint32_t filter::blur::factory::get_height(void* inptr)
{
	return reinterpret_cast<filter::blur::instance*>(inptr)->get_height();
}

void filter::blur::factory::activate(void* inptr)
{
	reinterpret_cast<filter::blur::instance*>(inptr)->activate();
}

void filter::blur::factory::deactivate(void* inptr)
{
	reinterpret_cast<filter::blur::instance*>(inptr)->deactivate();
}

void filter::blur::factory::video_tick(void* inptr, float delta)
{
	reinterpret_cast<filter::blur::instance*>(inptr)->video_tick(delta);
}

void filter::blur::factory::video_render(void* inptr, gs_effect_t* effect)
{
	reinterpret_cast<filter::blur::instance*>(inptr)->video_render(effect);
}

std::shared_ptr<gs::effect> filter::blur::factory::get_effect(filter::blur::type type)
{
	return effects.at(type);
}

std::shared_ptr<gs::effect> filter::blur::factory::get_color_converter_effect()
{
	return color_converter_effect;
}

std::shared_ptr<gs::texture> filter::blur::factory::get_kernel(filter::blur::type type)
{
	return kernels.at(type);
}

static filter::blur::factory* factory_instance = nullptr;

void filter::blur::factory::initialize()
{
	factory_instance = new filter::blur::factory();
}

void filter::blur::factory::finalize()
{
	delete factory_instance;
}

filter::blur::factory* filter::blur::factory::get()
{
	return factory_instance;
}
