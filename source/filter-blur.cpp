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

#include "filter-blur.h"
#include "strings.h"
#include <math.h>
#include <map>

extern "C" {
#pragma warning (push)
#pragma warning (disable: 4201)
#include "libobs/util/platform.h"
#include "libobs/graphics/graphics.h"
#include "libobs/graphics/matrix4.h"
#pragma warning (pop)
}

// Initialize & Finalizer
static Filter::Blur* handler;
INITIALIZER(HandlerInit) {
	initializerFunctions.push_back([] {
		handler = new Filter::Blur();
	});
	finalizerFunctions.push_back([] {
		delete handler;
	});
}


enum ColorFormat : uint64_t {
	RGB,
	YUV, // 701
};


struct g_blurEffect {
	gs_effect_t* effect;
	std::vector<gs_texture_t*> kernels;
};
static g_blurEffect g_gaussianBlur, g_bilateralBlur;
static gs_effect_t* g_boxBlurEffect, *g_colorConversionEffect;

static size_t g_maxKernelSize = 25;

const double_t pi = 3.1415926535897932384626433832795;
const double_t twopi = 6.283185307179586476925286766559;
const double_t twopisqr = 2.506628274631000502415765284811;

double_t gaussian1D(double_t x, double_t o) {
	double_t c = (x / o);
	double_t b = exp(-0.5 * c * c);
	double_t a = (1.0 / (o * twopisqr));
	return a * b;
}
double_t bilateral(double_t x, double_t o) {
	return 0.39894 * exp(-0.5 * (x * x) / (o * o)) / o;
}

static void makeGaussianKernels() {
	g_gaussianBlur.kernels.resize(g_maxKernelSize);

	std::vector<double_t> mathBuffer;
	std::vector<float_t> textureBuffer;

	for (size_t n = 1; n <= g_maxKernelSize; n++) {
		size_t width = n + 1;
		mathBuffer.resize(width);
		textureBuffer.resize(width);

		// Calculate
		double_t gaussianSum = 0.0;
		for (size_t p = 0; p < width; p++) {
			double_t g = gaussian1D(double_t(p), double_t(n));
			mathBuffer[p] = g;
			gaussianSum += g;
			if (p != 0)
				gaussianSum += g;
		}

		// Normalize
		double_t gaussianMult = 1.0 / gaussianSum;
		for (size_t p = 0; p < width; p++) {
			textureBuffer[p] = (float_t)(mathBuffer[p] * gaussianMult);
		}

		// Create Texture
		uint8_t* data = reinterpret_cast<uint8_t*>(textureBuffer.data());
		const uint8_t** pdata = const_cast<const uint8_t**>(&data);

		gs_texture_t* tex = gs_texture_create(uint32_t(width), 1, gs_color_format::GS_R32F, 1, pdata, 0);
		if (!tex) {
			P_LOG_ERROR("<filter-blur> Failed to create gaussian kernel for %d width.", n);
		} else {
			g_gaussianBlur.kernels[n - 1] = tex;
		}
	}
}

Filter::Blur::Blur() {
	memset(&sourceInfo, 0, sizeof(obs_source_info));
	sourceInfo.id = "obs-stream-effects-filter-blur";
	sourceInfo.type = OBS_SOURCE_TYPE_FILTER;
	sourceInfo.output_flags = OBS_SOURCE_VIDEO;
	sourceInfo.get_name = get_name;
	sourceInfo.get_defaults = get_defaults;
	sourceInfo.get_properties = get_properties;

	sourceInfo.create = create;
	sourceInfo.destroy = destroy;
	sourceInfo.update = update;
	sourceInfo.activate = activate;
	sourceInfo.deactivate = deactivate;
	sourceInfo.video_tick = video_tick;
	sourceInfo.video_render = video_render;

	obs_enter_graphics();

	// Blur Effects
	/// Box Blur
	{
		char* loadError = nullptr;
		char* file = obs_module_file("effects/box-blur.effect");
		g_boxBlurEffect = gs_effect_create_from_file(file, &loadError);
		bfree(file);
		if (loadError != nullptr) {
			P_LOG_ERROR("<filter-blur> Loading box-blur effect failed with error(s): %s", loadError);
			bfree(loadError);
		} else if (!g_boxBlurEffect) {
			P_LOG_ERROR("<filter-blur> Loading box-blur effect failed with unspecified error.");
		}
	}
	/// Gaussian Blur
	{
		gs_effect_t* effect;
		char* loadError = nullptr;
		char* file = obs_module_file("effects/gaussian-blur.effect");
		effect = gs_effect_create_from_file(file, &loadError);
		bfree(file);
		if (loadError != nullptr) {
			P_LOG_ERROR("<filter-blur> Loading gaussian blur effect failed with error(s): %s", loadError);
			bfree(loadError);
		} else if (!effect) {
			P_LOG_ERROR("<filter-blur> Loading gaussian blur effect failed with unspecified error.");
		} else {
			g_gaussianBlur.effect = effect;

		}
	}
	/// Bilateral Blur
	{
		gs_effect_t* effect;
		char* loadError = nullptr;
		char* file = obs_module_file("effects/bilateral-blur.effect");
		effect = gs_effect_create_from_file(file, &loadError);
		bfree(file);
		if (loadError != nullptr) {
			P_LOG_ERROR("<filter-blur> Loading bilateral blur effect failed with error(s): %s", loadError);
			bfree(loadError);
		} else if (!effect) {
			P_LOG_ERROR("<filter-blur> Loading bilateral blur effect failed with unspecified error.");
		} else {
			g_bilateralBlur.effect = effect;
			makeGaussianKernels();
		}
	}

	// Color Conversion
	{
		char* loadError = nullptr;
		char* file = obs_module_file("effects/color-conversion.effect");
		g_colorConversionEffect = gs_effect_create_from_file(file, &loadError);
		bfree(file);
		if (loadError != nullptr) {
			P_LOG_ERROR("<filter-blur> Loading color conversion effect failed with error(s): %s", loadError);
			bfree(loadError);
		} else if (!g_colorConversionEffect) {
			P_LOG_ERROR("<filter-blur> Loading color conversion effect failed with unspecified error.");
		}
	}

	obs_leave_graphics();

	if (g_boxBlurEffect && g_gaussianBlur.effect && g_bilateralBlur.effect && g_colorConversionEffect)
		obs_register_source(&sourceInfo);
}

Filter::Blur::~Blur() {
	obs_enter_graphics();
	gs_effect_destroy(g_colorConversionEffect);
	gs_effect_destroy(g_bilateralBlur.effect);
	gs_effect_destroy(g_gaussianBlur.effect);
	for (size_t n = 1; n <= g_maxKernelSize; n++) {
		if (g_gaussianBlur.kernels.size() > 0)
			gs_texture_destroy(g_gaussianBlur.kernels[n - 1]);
	}
	gs_effect_destroy(g_boxBlurEffect);
	obs_leave_graphics();
}

const char * Filter::Blur::get_name(void *) {
	return P_TRANSLATE(S_FILTER_BLUR);
}

void Filter::Blur::get_defaults(obs_data_t *data) {
	obs_data_set_default_int(data, S_FILTER_BLUR_TYPE, Filter::Blur::Type::Box);
	obs_data_set_default_int(data, S_FILTER_BLUR_SIZE, 5);

	// Bilateral Only
	obs_data_set_default_double(data, S_FILTER_BLUR_BILATERAL_SMOOTHING, 50.0);
	obs_data_set_default_double(data, S_FILTER_BLUR_BILATERAL_SHARPNESS, 90.0);

	// Advanced
	obs_data_set_default_bool(data, S_ADVANCED, false);
	obs_data_set_default_int(data, S_FILTER_BLUR_COLORFORMAT, ColorFormat::RGB);
}

obs_properties_t * Filter::Blur::get_properties(void *) {
	obs_properties_t *pr = obs_properties_create();
	obs_property_t* p = NULL;

	p = obs_properties_add_list(pr, S_FILTER_BLUR_TYPE, P_TRANSLATE(S_FILTER_BLUR_TYPE),
		obs_combo_type::OBS_COMBO_TYPE_LIST, obs_combo_format::OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(S_FILTER_BLUR_TYPE)));
	obs_property_set_modified_callback(p, modified_properties);
	obs_property_list_add_int(p, P_TRANSLATE(S_FILTER_BLUR_TYPE_BOX),
		Filter::Blur::Type::Box);
	obs_property_list_add_int(p, P_TRANSLATE(S_FILTER_BLUR_TYPE_GAUSSIAN),
		Filter::Blur::Type::Gaussian);
	obs_property_list_add_int(p, P_TRANSLATE(S_FILTER_BLUR_TYPE_BILATERAL),
		Filter::Blur::Type::Bilateral);

	p = obs_properties_add_int_slider(pr, S_FILTER_BLUR_SIZE,
		P_TRANSLATE(S_FILTER_BLUR_SIZE), 1, 25, 1);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(S_FILTER_BLUR_SIZE)));
	//obs_property_set_modified_callback(p, modified_properties);

	// Bilateral Only
	p = obs_properties_add_float_slider(pr, S_FILTER_BLUR_BILATERAL_SMOOTHING,
		P_TRANSLATE(S_FILTER_BLUR_BILATERAL_SMOOTHING), 0.01, 100.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(S_FILTER_BLUR_BILATERAL_SMOOTHING)));
	p = obs_properties_add_float_slider(pr, S_FILTER_BLUR_BILATERAL_SHARPNESS,
		P_TRANSLATE(S_FILTER_BLUR_BILATERAL_SHARPNESS), 0, 99.99, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(S_FILTER_BLUR_BILATERAL_SHARPNESS)));

	// Advanced
	p = obs_properties_add_bool(pr, S_ADVANCED, P_TRANSLATE(S_ADVANCED));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(S_ADVANCED)));
	obs_property_set_modified_callback(p, modified_properties);

	p = obs_properties_add_list(pr, S_FILTER_BLUR_COLORFORMAT,
		P_TRANSLATE(S_FILTER_BLUR_COLORFORMAT),
		obs_combo_type::OBS_COMBO_TYPE_LIST, obs_combo_format::OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p,
		P_TRANSLATE(P_DESC(S_FILTER_BLUR_COLORFORMAT)));
	obs_property_list_add_int(p, "RGB",
		ColorFormat::RGB);
	obs_property_list_add_int(p, "YUV",
		ColorFormat::YUV);

	return pr;
}

bool Filter::Blur::modified_properties(obs_properties_t *pr, obs_property_t *, obs_data_t *d) {
	bool showBilateral = false;

	switch (obs_data_get_int(d, S_FILTER_BLUR_TYPE)) {
		case Filter::Blur::Type::Box:
			break;
		case Filter::Blur::Type::Gaussian:
			break;
		case Filter::Blur::Type::Bilateral:
			showBilateral = true;
			break;
	}

	// Bilateral Blur
	obs_property_set_visible(obs_properties_get(pr, S_FILTER_BLUR_BILATERAL_SMOOTHING),
		showBilateral);
	obs_property_set_visible(obs_properties_get(pr, S_FILTER_BLUR_BILATERAL_SHARPNESS),
		showBilateral);

	// Advanced
	bool showAdvanced = false;
	if (obs_data_get_bool(d, S_ADVANCED))
		showAdvanced = true;

	obs_property_set_visible(obs_properties_get(pr, S_FILTER_BLUR_COLORFORMAT),
		showAdvanced);

	return true;
}

void * Filter::Blur::create(obs_data_t *data, obs_source_t *source) {
	return new Instance(data, source);
}

void Filter::Blur::destroy(void *ptr) {
	delete reinterpret_cast<Instance*>(ptr);
}

uint32_t Filter::Blur::get_width(void *ptr) {
	return reinterpret_cast<Instance*>(ptr)->get_width();
}

uint32_t Filter::Blur::get_height(void *ptr) {
	return reinterpret_cast<Instance*>(ptr)->get_height();
}

void Filter::Blur::update(void *ptr, obs_data_t *data) {
	reinterpret_cast<Instance*>(ptr)->update(data);
}

void Filter::Blur::activate(void *ptr) {
	reinterpret_cast<Instance*>(ptr)->activate();
}

void Filter::Blur::deactivate(void *ptr) {
	reinterpret_cast<Instance*>(ptr)->deactivate();
}

void Filter::Blur::video_tick(void *ptr, float time) {
	reinterpret_cast<Instance*>(ptr)->video_tick(time);
}

void Filter::Blur::video_render(void *ptr, gs_effect_t *effect) {
	reinterpret_cast<Instance*>(ptr)->video_render(effect);
}

Filter::Blur::Instance::Instance(obs_data_t *data, obs_source_t *context) : m_source(context) {
	obs_enter_graphics();
	m_effect = g_boxBlurEffect;
	m_primaryRT = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	m_secondaryRT = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	m_rtHorizontal = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	m_rtVertical = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	obs_leave_graphics();

	update(data);
}

Filter::Blur::Instance::~Instance() {
	obs_enter_graphics();
	gs_texrender_destroy(m_primaryRT);
	gs_texrender_destroy(m_secondaryRT);
	gs_texrender_destroy(m_rtHorizontal);
	gs_texrender_destroy(m_rtVertical);
	obs_leave_graphics();
}

void Filter::Blur::Instance::update(obs_data_t *data) {
	m_type = (Type)obs_data_get_int(data, S_FILTER_BLUR_TYPE);
	switch (m_type) {
		case Filter::Blur::Type::Box:
			m_effect = g_boxBlurEffect;
			break;
		case Filter::Blur::Type::Gaussian:
			m_effect = g_gaussianBlur.effect;
			break;
		case Filter::Blur::Type::Bilateral:
			m_effect = g_bilateralBlur.effect;
			break;
	}
	m_size = (uint64_t)obs_data_get_int(data, S_FILTER_BLUR_SIZE);

	// Bilateral Blur
	m_bilateralSmoothing = obs_data_get_double(data, S_FILTER_BLUR_BILATERAL_SMOOTHING) / 100.0;
	m_bilateralSharpness = obs_data_get_double(data, S_FILTER_BLUR_BILATERAL_SHARPNESS) / 100.0;

	// Advanced
	m_colorFormat = obs_data_get_int(data, S_FILTER_BLUR_COLORFORMAT);
}

uint32_t Filter::Blur::Instance::get_width() {
	return 0;
}

uint32_t Filter::Blur::Instance::get_height() {
	return 0;
}

void Filter::Blur::Instance::activate() {}

void Filter::Blur::Instance::deactivate() {}

void Filter::Blur::Instance::video_tick(float) {}

void Filter::Blur::Instance::video_render(gs_effect_t *effect) {
	bool failed = false;
	vec4 black; vec4_zero(&black);
	obs_source_t
		*parent = obs_filter_get_parent(m_source),
		*target = obs_filter_get_target(m_source);
	uint32_t
		baseW = obs_source_get_base_width(target),
		baseH = obs_source_get_base_height(target);

	// Skip rendering if our target, parent or context is not valid.
	if (!target || !parent || !m_source || !m_effect || !m_primaryRT || !m_technique
		|| !baseW || !baseH) {
		obs_source_skip_video_filter(m_source);
		return;
	}

	gs_effect_t* defaultEffect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_DEFAULT);
	gs_texture_t *sourceTexture = nullptr;

#pragma region Source To Texture
	gs_texrender_reset(m_primaryRT);
	if (!gs_texrender_begin(m_primaryRT, baseW, baseH)) {
		P_LOG_ERROR("<filter-blur> Failed to set up base texture.");
		obs_source_skip_video_filter(m_source);
		return;
	} else {
		gs_ortho(0, (float)baseW, 0, (float)baseH, -1, 1);

		// Clear to Black
		vec4 black;
		vec4_zero(&black);
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

		// Render
		if (obs_source_process_filter_begin(m_source, GS_RGBA, OBS_NO_DIRECT_RENDERING)) {
			obs_source_process_filter_end(m_source, effect ? effect : defaultEffect, baseW, baseH);
		} else {
			P_LOG_ERROR("<filter-blur> Unable to render source.");
			failed = true;
		}
		gs_texrender_end(m_primaryRT);
	}

	if (failed) {
		obs_source_skip_video_filter(m_source);
		return;
	}

	sourceTexture = gs_texrender_get_texture(m_primaryRT);
	if (!sourceTexture) {
		P_LOG_ERROR("<filter-blur> Failed to get source texture.");
		obs_source_skip_video_filter(m_source);
		return;
	}
#pragma endregion Source To Texture

	// Conversion
#pragma region RGB -> YUV
	if (m_colorFormat == ColorFormat::YUV) {
		gs_texrender_reset(m_secondaryRT);
		if (!gs_texrender_begin(m_secondaryRT, baseW, baseH)) {
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

			gs_eparam_t* param = gs_effect_get_param_by_name(g_colorConversionEffect, "image");
			if (!param) {
				P_LOG_ERROR("<filter-blur:Final> Failed to set image param.");
				failed = true;
			} else {
				gs_effect_set_texture(param, sourceTexture);
			}
			while (gs_effect_loop(g_colorConversionEffect, "RGBToYUV")) {
				gs_draw_sprite(sourceTexture, 0, baseW, baseH);
			}
			gs_texrender_end(m_secondaryRT);
		}

		if (failed) {
			obs_source_skip_video_filter(m_source);
			return;
		}

		sourceTexture = gs_texrender_get_texture(m_secondaryRT);
		if (!sourceTexture) {
			P_LOG_ERROR("<filter-blur> Failed to get source texture.");
			obs_source_skip_video_filter(m_source);
			return;
		}
	}
#pragma endregion RGB -> YUV

#pragma region Blur
	// Set up camera stuff
	gs_set_cull_mode(GS_NEITHER);
	gs_reset_blend_state();
	gs_enable_blending(true);
	gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);
	gs_enable_depth_test(false);
	gs_enable_stencil_test(false);
	gs_enable_stencil_write(false);
	gs_enable_color(true, true, true, true);

	gs_texture_t* blurred = nullptr, *intermediate = sourceTexture;
	std::tuple<const char*, gs_texrender_t*, float, float> kvs[] = {
		std::make_tuple("Horizontal", m_rtHorizontal, 1.0f / baseW, 0.0f),
		std::make_tuple("Vertical", m_rtVertical, 0.0f, 1.0f / baseH),
	};
	for (auto v : kvs) {
		const char* name = std::get<0>(v);
		gs_texrender_t* rt = std::get<1>(v);
		float xpel = std::get<2>(v),
			ypel = std::get<3>(v);

		if (!apply_shared_param(intermediate, xpel, ypel))
			break;
		switch (m_type) {
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
		while (gs_effect_loop(m_effect, "Draw")) {
			gs_draw_sprite(intermediate, 0, baseW, baseH);
		}

		gs_texrender_end(rt);
		intermediate = gs_texrender_get_texture(rt);
		if (!intermediate) {
			P_LOG_ERROR("<filter-blur:%s> Failed to get intermediate texture.",
				name);
			break;
		}
		blurred = intermediate;
	}
	if (blurred == nullptr) {
		obs_source_skip_video_filter(m_source);
		return;
	}
#pragma endregion Blur

#pragma region YUV -> RGB or straight draw
	// Draw final effect
	{
		gs_effect_t* finalEffect = defaultEffect;
		const char* technique = "Draw";

		if (m_colorFormat == ColorFormat::YUV) {
			finalEffect = g_colorConversionEffect;
			technique = "YUVToRGB";
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
#pragma endregion YUV -> RGB or straight draw

	if (failed) {
		obs_source_skip_video_filter(m_source);
		return;
	}
}

bool Filter::Blur::Instance::apply_shared_param(gs_texture_t* input, float texelX, float texelY) {
	bool result = true;

	result = result && gs_set_param_texture(m_effect, "u_image", input);

	vec2 imageSize;
	vec2_set(&imageSize,
		(float)gs_texture_get_width(input),
		(float)gs_texture_get_height(input));
	result = result && gs_set_param_float2(m_effect, "u_imageSize", &imageSize);

	vec2 imageTexelDelta;
	vec2_set(&imageTexelDelta, 1.0f, 1.0f);
	vec2_div(&imageTexelDelta, &imageTexelDelta, &imageSize);
	result = result && gs_set_param_float2(m_effect, "u_imageTexel", &imageTexelDelta);

	vec2 texel; vec2_set(&texel, texelX, texelY);
	result = result && gs_set_param_float2(m_effect, "u_texelDelta", &texel);

	result = result && gs_set_param_int(m_effect, "u_radius", (int)m_size);
	result = result && gs_set_param_int(m_effect, "u_diameter", (int)(1 + (m_size * 2)));

	return result;
}

bool Filter::Blur::Instance::apply_bilateral_param() {
	gs_eparam_t *param;

	if (m_type != Type::Bilateral)
		return false;

	// Bilateral Blur
	param = gs_effect_get_param_by_name(m_effect, "bilateralSmoothing");
	if (!param) {
		P_LOG_ERROR("<filter-blur> Failed to set bilateralSmoothing param.");
		return false;
	} else {
		P_LOG_DEBUG("<filter-blur> Applying bilateralSmoothing parameter.");
		gs_effect_set_float(param,
			(float)(m_bilateralSmoothing * (1 + m_size * 2)));
	}

	param = gs_effect_get_param_by_name(m_effect, "bilateralSharpness");
	if (!param) {
		P_LOG_ERROR("<filter-blur> Failed to set bilateralSmoothing param.");
		return false;
	} else {
		P_LOG_DEBUG("<filter-blur> Applying bilateralSharpness parameter.");
		gs_effect_set_float(param, (float)(1.0 - m_bilateralSharpness));
	}

	return true;
}

bool Filter::Blur::Instance::apply_gaussian_param() {
	bool result = true;

	if (m_type != Type::Gaussian)
		return false;

	if (g_gaussianBlur.kernels.size() >= m_size) {
		result = result && gs_set_param_texture(m_effect, "kernel",
			g_gaussianBlur.kernels[m_size - 1]);
	}

	vec2 kerneltexel;
	vec2_set(&kerneltexel, 1.0f / (m_size + 1), 0);
	result = result && gs_set_param_float2(m_effect, "kernelTexel", &kerneltexel);

	return result;
}
