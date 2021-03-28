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

#include "filter-color-grade.hpp"
#include "strings.hpp"
#include <stdexcept>
#include "obs/gs/gs-helper.hpp"

// OBS
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <graphics/graphics.h>
#include <graphics/matrix4.h>
#include <util/platform.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#define ST "Filter.ColorGrade"
#define ST_TOOL ST ".Tool"
#define ST_LIFT ST ".Lift"
#define ST_LIFT_(x) ST_LIFT "." D_VSTR(x)
#define ST_GAMMA ST ".Gamma"
#define ST_GAMMA_(x) ST_GAMMA "." D_VSTR(x)
#define ST_GAIN ST ".Gain"
#define ST_GAIN_(x) ST_GAIN "." D_VSTR(x)
#define ST_OFFSET ST ".Offset"
#define ST_OFFSET_(x) ST_OFFSET "." D_VSTR(x)
#define ST_TINT ST ".Tint"
#define ST_TINT_DETECTION ST_TINT ".Detection"
#define ST_TINT_DETECTION_(x) ST_TINT_DETECTION "." D_VSTR(x)
#define ST_TINT_MODE ST_TINT ".Mode"
#define ST_TINT_MODE_(x) ST_TINT_MODE "." D_VSTR(x)
#define ST_TINT_EXPONENT ST_TINT ".Exponent"
#define ST_TINT_(x, y) ST_TINT "." D_VSTR(x) "." D_VSTR(y)
#define ST_CORRECTION ST ".Correction"
#define ST_CORRECTION_(x) ST_CORRECTION "." D_VSTR(x)

#define ST_RENDERMODE ST ".RenderMode"
#define ST_RENDERMODE_DIRECT ST_RENDERMODE ".Direct"
#define ST_RENDERMODE_LUT_2BIT ST_RENDERMODE ".LUT.2Bit"
#define ST_RENDERMODE_LUT_4BIT ST_RENDERMODE ".LUT.4Bit"
#define ST_RENDERMODE_LUT_6BIT ST_RENDERMODE ".LUT.6Bit"
#define ST_RENDERMODE_LUT_8BIT ST_RENDERMODE ".LUT.8Bit"
#define ST_RENDERMODE_LUT_10BIT ST_RENDERMODE ".LUT.10Bit"

#define RED Red
#define GREEN Green
#define BLUE Blue
#define ALL All
#define HUE Hue
#define SATURATION Saturation
#define LIGHTNESS Lightness
#define CONTRAST Contrast
#define TONE_LOW Shadow
#define TONE_MID Midtone
#define TONE_HIGH Highlight
#define DETECTION_HSV HSV
#define DETECTION_HSL HSL
#define DETECTION_YUV_SDR YUV.SDR
#define MODE_LINEAR Linear
#define MODE_EXP Exp
#define MODE_EXP2 Exp2
#define MODE_LOG Log
#define MODE_LOG10 Log10

using namespace streamfx::filter::color_grade;

// TODO: Figure out a way to merge _lut_rt, _lut_texture, _rt_source, _rt_grad, _tex_source, _tex_grade, _source_updated and _grade_updated.
// Seriously this is too much GPU space wasted on unused trash.

#define LOCAL_PREFIX "<filter::color-grade> "

color_grade_instance::~color_grade_instance() {}

color_grade_instance::color_grade_instance(obs_data_t* data, obs_source_t* self)
	: obs::source_instance(data, self), _effect(),

	  _lift(), _gamma(), _gain(), _offset(), _tint_detection(), _tint_luma(), _tint_exponent(), _tint_low(),
	  _tint_mid(), _tint_hig(), _correction(), _lut_enabled(true), _lut_depth(),

	  _cache_rt(), _cache_texture(), _cache_fresh(false),

	  _lut_initialized(false), _lut_dirty(true), _lut_producer(), _lut_consumer()
{
	// Load the color grading effect.
	auto path = streamfx::data_file_path("effects/color-grade.effect");
	if (!std::filesystem::exists(path)) {
		DLOG_ERROR(LOCAL_PREFIX "Failed to locate effect file '%s'.", path.u8string().c_str());
		throw std::runtime_error("Failed to load color grade effect.");
	} else {
		try {
			_effect = gs::effect::create(path.u8string());
		} catch (std::exception const& ex) {
			DLOG_ERROR(LOCAL_PREFIX "Failed to load effect '%s': %s", path.u8string().c_str(), ex.what());
			throw;
		}
	}

	// Initialize LUT work flow.
	try {
		_lut_producer    = std::make_shared<gfx::lut::producer>();
		_lut_consumer    = std::make_shared<gfx::lut::consumer>();
		_lut_initialized = true;
	} catch (std::exception const& ex) {
		DLOG_WARNING(LOCAL_PREFIX "Failed to initialize LUT rendering, falling back to direct rendering.\n%s",
					 ex.what());
		_lut_initialized = false;
	}

	// Allocate render target for rendering.
	try {
		allocate_rendertarget(GS_RGBA);
	} catch (std::exception const& ex) {
		DLOG_ERROR(LOCAL_PREFIX "Failed to acquire render target for rendering: %s", ex.what());
		throw;
	}

	update(data);
}

void color_grade_instance::allocate_rendertarget(gs_color_format format)
{
	_cache_rt = std::make_unique<gs::rendertarget>(format, GS_ZS_NONE);
}

float_t fix_gamma_value(double_t v)
{
	if (v < 0.0) {
		return static_cast<float_t>(-v + 1.0);
	} else {
		return static_cast<float_t>(1.0 / (v + 1.0));
	}
}

void color_grade_instance::load(obs_data_t* data)
{
	update(data);
}

void color_grade_instance::migrate(obs_data_t* data, uint64_t version) {}

void color_grade_instance::update(obs_data_t* data)
{
	_lift.x         = static_cast<float_t>(obs_data_get_double(data, ST_LIFT_(RED)) / 100.0);
	_lift.y         = static_cast<float_t>(obs_data_get_double(data, ST_LIFT_(GREEN)) / 100.0);
	_lift.z         = static_cast<float_t>(obs_data_get_double(data, ST_LIFT_(BLUE)) / 100.0);
	_lift.w         = static_cast<float_t>(obs_data_get_double(data, ST_LIFT_(ALL)) / 100.0);
	_gamma.x        = fix_gamma_value(obs_data_get_double(data, ST_GAMMA_(RED)) / 100.0);
	_gamma.y        = fix_gamma_value(obs_data_get_double(data, ST_GAMMA_(GREEN)) / 100.0);
	_gamma.z        = fix_gamma_value(obs_data_get_double(data, ST_GAMMA_(BLUE)) / 100.0);
	_gamma.w        = fix_gamma_value(obs_data_get_double(data, ST_GAMMA_(ALL)) / 100.0);
	_gain.x         = static_cast<float_t>(obs_data_get_double(data, ST_GAIN_(RED)) / 100.0);
	_gain.y         = static_cast<float_t>(obs_data_get_double(data, ST_GAIN_(GREEN)) / 100.0);
	_gain.z         = static_cast<float_t>(obs_data_get_double(data, ST_GAIN_(BLUE)) / 100.0);
	_gain.w         = static_cast<float_t>(obs_data_get_double(data, ST_GAIN_(ALL)) / 100.0);
	_offset.x       = static_cast<float_t>(obs_data_get_double(data, ST_OFFSET_(RED)) / 100.0);
	_offset.y       = static_cast<float_t>(obs_data_get_double(data, ST_OFFSET_(GREEN)) / 100.0);
	_offset.z       = static_cast<float_t>(obs_data_get_double(data, ST_OFFSET_(BLUE)) / 100.0);
	_offset.w       = static_cast<float_t>(obs_data_get_double(data, ST_OFFSET_(ALL)) / 100.0);
	_tint_detection = static_cast<detection_mode>(obs_data_get_int(data, ST_TINT_DETECTION));
	_tint_luma      = static_cast<luma_mode>(obs_data_get_int(data, ST_TINT_MODE));
	_tint_exponent  = static_cast<float_t>(obs_data_get_double(data, ST_TINT_EXPONENT));
	_tint_low.x     = static_cast<float_t>(obs_data_get_double(data, ST_TINT_(TONE_LOW, RED)) / 100.0);
	_tint_low.y     = static_cast<float_t>(obs_data_get_double(data, ST_TINT_(TONE_LOW, GREEN)) / 100.0);
	_tint_low.z     = static_cast<float_t>(obs_data_get_double(data, ST_TINT_(TONE_LOW, BLUE)) / 100.0);
	_tint_mid.x     = static_cast<float_t>(obs_data_get_double(data, ST_TINT_(TONE_MID, RED)) / 100.0);
	_tint_mid.y     = static_cast<float_t>(obs_data_get_double(data, ST_TINT_(TONE_MID, GREEN)) / 100.0);
	_tint_mid.z     = static_cast<float_t>(obs_data_get_double(data, ST_TINT_(TONE_MID, BLUE)) / 100.0);
	_tint_hig.x     = static_cast<float_t>(obs_data_get_double(data, ST_TINT_(TONE_HIGH, RED)) / 100.0);
	_tint_hig.y     = static_cast<float_t>(obs_data_get_double(data, ST_TINT_(TONE_HIGH, GREEN)) / 100.0);
	_tint_hig.z     = static_cast<float_t>(obs_data_get_double(data, ST_TINT_(TONE_HIGH, BLUE)) / 100.0);
	_correction.x   = static_cast<float_t>(obs_data_get_double(data, ST_CORRECTION_(HUE)) / 360.0);
	_correction.y   = static_cast<float_t>(obs_data_get_double(data, ST_CORRECTION_(SATURATION)) / 100.0);
	_correction.z   = static_cast<float_t>(obs_data_get_double(data, ST_CORRECTION_(LIGHTNESS)) / 100.0);
	_correction.w   = static_cast<float_t>(obs_data_get_double(data, ST_CORRECTION_(CONTRAST)) / 100.0);

	{
		int64_t v = obs_data_get_int(data, ST_RENDERMODE);

		// LUT status depends on selected option.
		_lut_enabled = v != 0; // 0 (Direct)

		if (v == -1) {
			_lut_depth = gfx::lut::color_depth::_8;
		} else if (v > 0) {
			_lut_depth = static_cast<gfx::lut::color_depth>(v);
		}
	}

	if (_lut_enabled && _lut_initialized)
		_lut_dirty = true;
}

void color_grade_instance::prepare_effect()
{
	if (auto p = _effect.get_parameter("pLift"); p) {
		p.set_float4(_lift);
	}

	if (auto p = _effect.get_parameter("pGamma"); p) {
		p.set_float4(_gamma);
	}

	if (auto p = _effect.get_parameter("pGain"); p) {
		p.set_float4(_gain);
	}

	if (auto p = _effect.get_parameter("pOffset"); p) {
		p.set_float4(_offset);
	}

	if (auto p = _effect.get_parameter("pLift"); p) {
		p.set_float4(_lift);
	}

	if (auto p = _effect.get_parameter("pTintDetection"); p) {
		p.set_int(static_cast<int32_t>(_tint_detection));
	}

	if (auto p = _effect.get_parameter("pTintMode"); p) {
		p.set_int(static_cast<int32_t>(_tint_luma));
	}

	if (auto p = _effect.get_parameter("pTintExponent"); p) {
		p.set_float(_tint_exponent);
	}

	if (auto p = _effect.get_parameter("pTintLow"); p) {
		p.set_float3(_tint_low);
	}

	if (auto p = _effect.get_parameter("pTintMid"); p) {
		p.set_float3(_tint_mid);
	}

	if (auto p = _effect.get_parameter("pTintHig"); p) {
		p.set_float3(_tint_hig);
	}

	if (auto p = _effect.get_parameter("pCorrection"); p) {
		p.set_float4(_correction);
	}
}

void color_grade_instance::rebuild_lut()
{
#ifdef ENABLE_PROFILING
	gs::debug_marker gdm{gs::debug_color_cache, "Rebuild LUT"};
#endif

	// Generate a fresh LUT texture.
	auto lut_texture = _lut_producer->produce(_lut_depth);

	// Modify the LUT with our color grade.
	if (lut_texture) {
		// Check if we have a render target to work with and if it's the correct format.
		if (!_lut_rt || (lut_texture->get_color_format() != _lut_rt->get_color_format())) {
			// Create a new render target with new format.
			_lut_rt = std::make_unique<gs::rendertarget>(lut_texture->get_color_format(), GS_ZS_NONE);
		}

		// Prepare our color grade effect.
		prepare_effect();

		// Assign texture.
		if (auto p = _effect.get_parameter("image"); p) {
			p.set_texture(lut_texture);
		}

		{ // Begin rendering.
			auto op = _lut_rt->render(lut_texture->get_width(), lut_texture->get_height());

			// Set up graphics context.
			gs_ortho(0, 1, 0, 1, 0, 1);
			gs_blend_state_push();
			gs_enable_blending(false);
			gs_enable_color(true, true, true, true);
			gs_enable_stencil_test(false);
			gs_enable_stencil_write(false);

			while (gs_effect_loop(_effect.get_object(), "Draw")) {
				streamfx::gs_draw_fullscreen_tri();
			}

			gs_blend_state_pop();
		}

		_lut_rt->get_texture(_lut_texture);
		if (!_lut_texture) {
			throw std::runtime_error("Failed to produce modified LUT texture.");
		}
	} else {
		throw std::runtime_error("Failed to produce LUT texture.");
	}

	_lut_dirty = false;
}

void color_grade_instance::video_tick(float)
{
	_ccache_fresh = false;
	_cache_fresh  = false;
}

void color_grade_instance::video_render(gs_effect_t* shader)
{
	// Grab initial values.
	obs_source_t* parent = obs_filter_get_parent(_self);
	obs_source_t* target = obs_filter_get_target(_self);
	uint32_t      width  = obs_source_get_base_width(target);
	uint32_t      height = obs_source_get_base_height(target);
	vec4          blank  = vec4{0, 0, 0, 0};
	shader               = shader ? shader : obs_get_base_effect(OBS_EFFECT_DEFAULT);

	// Skip filter if anything is wrong.
	if (!parent || !target || !width || !height) {
		obs_source_skip_video_filter(_self);
		return;
	}

#ifdef ENABLE_PROFILING
	gs::debug_marker gdmp{gs::debug_color_source, "Color Grading '%s'", obs_source_get_name(_self)};
#endif

	// TODO: Optimize this once (https://github.com/obsproject/obs-studio/pull/4199) is merged.
	// - We can skip the original capture and reduce the overall impact of this.

	// 1. Capture the filter/source rendered above this.
	if (!_ccache_fresh || !_ccache_texture) {
#ifdef ENABLE_PROFILING
		gs::debug_marker gdmp{gs::debug_color_cache, "Cache '%s'", obs_source_get_name(target)};
#endif
		// If the input cache render target doesn't exist, create it.
		if (!_ccache_rt) {
			_ccache_rt = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
		}

		{
			auto op = _ccache_rt->render(width, height);
			gs_ortho(0, static_cast<float_t>(width), 0, static_cast<float_t>(height), 0, 1);

			// Blank out the input cache.
			gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &blank, 0., 0);

			// Begin rendering the actual input source.
			obs_source_process_filter_begin(_self, GS_RGBA, OBS_ALLOW_DIRECT_RENDERING);

			// Enable all colors for rendering.
			gs_enable_color(true, true, true, true);

			// Prevent blending with existing content, even if it is cleared.
			gs_blend_state_push();
			gs_enable_blending(false);
			gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);

			// Disable depth testing.
			gs_enable_depth_test(false);

			// Disable stencil testing.
			gs_enable_stencil_test(false);

			// Disable culling.
			gs_set_cull_mode(GS_NEITHER);

			// End rendering the actual input source.
			obs_source_process_filter_end(_self, obs_get_base_effect(OBS_EFFECT_DEFAULT), width, height);

			// Restore original blend mode.
			gs_blend_state_pop();
		}

		// Try and retrieve the input cache as a texture for later use.
		_ccache_rt->get_texture(_ccache_texture);
		if (!_ccache_texture) {
			throw std::runtime_error("Failed to cache original source.");
		}

		// Mark the input cache as valid.
		_ccache_fresh = true;
	}

	// 2. Apply one of the two rendering methods (LUT or Direct).
	if (_lut_initialized && _lut_enabled) { // Try to apply with the LUT based method.
		try {
#ifdef ENABLE_PROFILING
			gs::debug_marker gdm{gs::debug_color_convert, "LUT Rendering"};
#endif
			// If the LUT was changed, rebuild the LUT first.
			if (_lut_dirty) {
				rebuild_lut();

				// Mark the cache as invalid, since the LUT has been changed.
				_cache_fresh = false;
			}

			// Reallocate the rendertarget if necessary.
			if (_cache_rt->get_color_format() != GS_RGBA) {
				allocate_rendertarget(GS_RGBA);
			}

			if (!_cache_fresh) {
				{ // Render the source to the cache.
					auto op = _cache_rt->render(width, height);
					gs_ortho(0, 1., 0, 1., 0, 1);

					// Blank out the input cache.
					gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &blank, 0., 0);

					// Enable all colors for rendering.
					gs_enable_color(true, true, true, true);

					// Prevent blending with existing content, even if it is cleared.
					gs_blend_state_push();
					gs_enable_blending(false);
					gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);

					// Disable depth testing.
					gs_enable_depth_test(false);

					// Disable stencil testing.
					gs_enable_stencil_test(false);

					// Disable culling.
					gs_set_cull_mode(GS_NEITHER);

					auto effect = _lut_consumer->prepare(_lut_depth, _lut_texture);
					effect->get_parameter("image").set_texture(_ccache_texture);
					while (gs_effect_loop(effect->get_object(), "Draw")) {
						streamfx::gs_draw_fullscreen_tri();
					}

					// Restore original blend mode.
					gs_blend_state_pop();
				}

				// Try and retrieve the render cache as a texture.
				_cache_rt->get_texture(_cache_texture);

				// Mark the render cache as valid.
				_cache_fresh = true;
			}
		} catch (std::exception const& ex) {
			// If anything happened, revert to direct rendering.
			_lut_rt.reset();
			_lut_texture.reset();
			_lut_enabled = false;
			DLOG_WARNING(LOCAL_PREFIX "Reverting to direct rendering due to error: %s", ex.what());
		}
	}
	if ((!_lut_initialized || !_lut_enabled) && !_cache_fresh) {
#ifdef ENABLE_PROFILING
		gs::debug_marker gdm{gs::debug_color_convert, "Direct Rendering"};
#endif
		// Reallocate the rendertarget if necessary.
		if (_cache_rt->get_color_format() != GS_RGBA) {
			allocate_rendertarget(GS_RGBA);
		}

		{ // Render the source to the cache.
			auto op = _cache_rt->render(width, height);
			gs_ortho(0, 1, 0, 1, 0, 1);

			prepare_effect();

			// Blank out the input cache.
			gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &blank, 0., 0);

			// Enable all colors for rendering.
			gs_enable_color(true, true, true, true);

			// Prevent blending with existing content, even if it is cleared.
			gs_blend_state_push();
			gs_enable_blending(false);
			gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);

			// Disable depth testing.
			gs_enable_depth_test(false);

			// Disable stencil testing.
			gs_enable_stencil_test(false);

			// Disable culling.
			gs_set_cull_mode(GS_NEITHER);

			// Render the effect.
			_effect.get_parameter("image").set_texture(_ccache_texture);
			while (gs_effect_loop(_effect.get_object(), "Draw")) {
				streamfx::gs_draw_fullscreen_tri();
			}

			// Restore original blend mode.
			gs_blend_state_pop();
		}

		// Try and retrieve the render cache as a texture.
		_cache_rt->get_texture(_cache_texture);

		// Mark the render cache as valid.
		_cache_fresh = true;
	}
	if (!_cache_texture) {
		throw std::runtime_error("Failed to cache processed source.");
	}

	// 3. Render the output cache.
	{
#ifdef ENABLE_PROFILING
		gs::debug_marker gdm{gs::debug_color_cache_render, "Draw Cache"};
#endif
		// Revert GPU status to what OBS Studio expects.
		gs_enable_depth_test(false);
		gs_enable_color(true, true, true, true);
		gs_set_cull_mode(GS_NEITHER);

		// Draw the render cache.
		while (gs_effect_loop(shader, "Draw")) {
			gs_effect_set_texture(gs_effect_get_param_by_name(shader, "image"),
								  _cache_texture ? _cache_texture->get_object() : nullptr);
			gs_draw_sprite(nullptr, 0, width, height);
		}
	}
}

color_grade_factory::color_grade_factory()
{
	_info.id           = PREFIX "filter-color-grade";
	_info.type         = OBS_SOURCE_TYPE_FILTER;
	_info.output_flags = OBS_SOURCE_VIDEO;

	set_resolution_enabled(false);
	finish_setup();
	register_proxy("obs-stream-effects-filter-color-grade");
}

color_grade_factory::~color_grade_factory() {}

const char* color_grade_factory::get_name()
{
	return D_TRANSLATE(ST);
}

void color_grade_factory::get_defaults2(obs_data_t* data)
{
	obs_data_set_default_string(data, ST_TOOL, ST_CORRECTION);
	obs_data_set_default_double(data, ST_LIFT_(RED), 0);
	obs_data_set_default_double(data, ST_LIFT_(GREEN), 0);
	obs_data_set_default_double(data, ST_LIFT_(BLUE), 0);
	obs_data_set_default_double(data, ST_LIFT_(ALL), 0);
	obs_data_set_default_double(data, ST_GAMMA_(RED), 0);
	obs_data_set_default_double(data, ST_GAMMA_(GREEN), 0);
	obs_data_set_default_double(data, ST_GAMMA_(BLUE), 0);
	obs_data_set_default_double(data, ST_GAMMA_(ALL), 0);
	obs_data_set_default_double(data, ST_GAIN_(RED), 100.0);
	obs_data_set_default_double(data, ST_GAIN_(GREEN), 100.0);
	obs_data_set_default_double(data, ST_GAIN_(BLUE), 100.0);
	obs_data_set_default_double(data, ST_GAIN_(ALL), 100.0);
	obs_data_set_default_double(data, ST_OFFSET_(RED), 0.0);
	obs_data_set_default_double(data, ST_OFFSET_(GREEN), 0.0);
	obs_data_set_default_double(data, ST_OFFSET_(BLUE), 0.0);
	obs_data_set_default_double(data, ST_OFFSET_(ALL), 0.0);
	obs_data_set_default_int(data, ST_TINT_MODE, static_cast<int64_t>(luma_mode::Linear));
	obs_data_set_default_int(data, ST_TINT_DETECTION, static_cast<int64_t>(detection_mode::YUV_SDR));
	obs_data_set_default_double(data, ST_TINT_EXPONENT, 1.5);
	obs_data_set_default_double(data, ST_TINT_(TONE_LOW, RED), 100.0);
	obs_data_set_default_double(data, ST_TINT_(TONE_LOW, GREEN), 100.0);
	obs_data_set_default_double(data, ST_TINT_(TONE_LOW, BLUE), 100.0);
	obs_data_set_default_double(data, ST_TINT_(TONE_MID, RED), 100.0);
	obs_data_set_default_double(data, ST_TINT_(TONE_MID, GREEN), 100.0);
	obs_data_set_default_double(data, ST_TINT_(TONE_MID, BLUE), 100.0);
	obs_data_set_default_double(data, ST_TINT_(TONE_HIGH, RED), 100.0);
	obs_data_set_default_double(data, ST_TINT_(TONE_HIGH, GREEN), 100.0);
	obs_data_set_default_double(data, ST_TINT_(TONE_HIGH, BLUE), 100.0);
	obs_data_set_default_double(data, ST_CORRECTION_(HUE), 0.0);
	obs_data_set_default_double(data, ST_CORRECTION_(SATURATION), 100.0);
	obs_data_set_default_double(data, ST_CORRECTION_(LIGHTNESS), 100.0);
	obs_data_set_default_double(data, ST_CORRECTION_(CONTRAST), 100.0);

	obs_data_set_default_int(data, ST_RENDERMODE, -1);
}

obs_properties_t* color_grade_factory::get_properties2(color_grade_instance* data)
{
	obs_properties_t* pr = obs_properties_create();

	{
		obs_properties_t* grp = obs_properties_create();
		obs_properties_add_group(pr, ST_LIFT, D_TRANSLATE(ST_LIFT), OBS_GROUP_NORMAL, grp);

		obs_properties_add_float_slider(grp, ST_LIFT_(RED), D_TRANSLATE(ST_LIFT_(RED)), -1000.0, 1000.0, 0.01);
		obs_properties_add_float_slider(grp, ST_LIFT_(GREEN), D_TRANSLATE(ST_LIFT_(GREEN)), -1000.0, 1000.0, 0.01);
		obs_properties_add_float_slider(grp, ST_LIFT_(BLUE), D_TRANSLATE(ST_LIFT_(BLUE)), -1000.0, 1000.0, 0.01);
		obs_properties_add_float_slider(grp, ST_LIFT_(ALL), D_TRANSLATE(ST_LIFT_(ALL)), -1000.0, 1000.0, 0.01);
	}

	{
		obs_properties_t* grp = obs_properties_create();
		obs_properties_add_group(pr, ST_GAMMA, D_TRANSLATE(ST_GAMMA), OBS_GROUP_NORMAL, grp);

		obs_properties_add_float_slider(grp, ST_GAMMA_(RED), D_TRANSLATE(ST_GAMMA_(RED)), -1000.0, 1000.0, 0.01);
		obs_properties_add_float_slider(grp, ST_GAMMA_(GREEN), D_TRANSLATE(ST_GAMMA_(GREEN)), -1000.0, 1000.0, 0.01);
		obs_properties_add_float_slider(grp, ST_GAMMA_(BLUE), D_TRANSLATE(ST_GAMMA_(BLUE)), -1000.0, 1000.0, 0.01);
		obs_properties_add_float_slider(grp, ST_GAMMA_(ALL), D_TRANSLATE(ST_GAMMA_(ALL)), -1000.0, 1000.0, 0.01);
	}

	{
		obs_properties_t* grp = obs_properties_create();
		obs_properties_add_group(pr, ST_GAIN, D_TRANSLATE(ST_GAIN), OBS_GROUP_NORMAL, grp);

		obs_properties_add_float_slider(grp, ST_GAIN_(RED), D_TRANSLATE(ST_GAIN_(RED)), 0.01, 1000.0, 0.01);
		obs_properties_add_float_slider(grp, ST_GAIN_(GREEN), D_TRANSLATE(ST_GAIN_(GREEN)), 0.01, 1000.0, 0.01);
		obs_properties_add_float_slider(grp, ST_GAIN_(BLUE), D_TRANSLATE(ST_GAIN_(BLUE)), 0.01, 1000.0, 0.01);
		obs_properties_add_float_slider(grp, ST_GAIN_(ALL), D_TRANSLATE(ST_GAIN_(ALL)), 0.01, 1000.0, 0.01);
	}

	{
		obs_properties_t* grp = obs_properties_create();
		obs_properties_add_group(pr, ST_OFFSET, D_TRANSLATE(ST_OFFSET), OBS_GROUP_NORMAL, grp);

		obs_properties_add_float_slider(grp, ST_OFFSET_(RED), D_TRANSLATE(ST_OFFSET_(RED)), -1000.0, 1000.0, 0.01);
		obs_properties_add_float_slider(grp, ST_OFFSET_(GREEN), D_TRANSLATE(ST_OFFSET_(GREEN)), -1000.0, 1000.0, 0.01);
		obs_properties_add_float_slider(grp, ST_OFFSET_(BLUE), D_TRANSLATE(ST_OFFSET_(BLUE)), -1000.0, 1000.0, 0.01);
		obs_properties_add_float_slider(grp, ST_OFFSET_(ALL), D_TRANSLATE(ST_OFFSET_(ALL)), -1000.0, 1000.0, 0.01);
	}

	{
		obs_properties_t* grp = obs_properties_create();
		obs_properties_add_group(pr, ST_TINT, D_TRANSLATE(ST_TINT), OBS_GROUP_NORMAL, grp);

		obs_properties_add_float_slider(grp, ST_TINT_(TONE_LOW, RED), D_TRANSLATE(ST_TINT_(TONE_LOW, RED)), 0, 1000.0,
										0.01);
		obs_properties_add_float_slider(grp, ST_TINT_(TONE_LOW, GREEN), D_TRANSLATE(ST_TINT_(TONE_LOW, GREEN)), 0,
										1000.0, 0.01);
		obs_properties_add_float_slider(grp, ST_TINT_(TONE_LOW, BLUE), D_TRANSLATE(ST_TINT_(TONE_LOW, BLUE)), 0, 1000.0,
										0.01);

		obs_properties_add_float_slider(grp, ST_TINT_(TONE_MID, RED), D_TRANSLATE(ST_TINT_(TONE_MID, RED)), 0, 1000.0,
										0.01);
		obs_properties_add_float_slider(grp, ST_TINT_(TONE_MID, GREEN), D_TRANSLATE(ST_TINT_(TONE_MID, GREEN)), 0,
										1000.0, 0.01);
		obs_properties_add_float_slider(grp, ST_TINT_(TONE_MID, BLUE), D_TRANSLATE(ST_TINT_(TONE_MID, BLUE)), 0, 1000.0,
										0.01);

		obs_properties_add_float_slider(grp, ST_TINT_(TONE_HIGH, RED), D_TRANSLATE(ST_TINT_(TONE_HIGH, RED)), 0, 1000.0,
										0.01);
		obs_properties_add_float_slider(grp, ST_TINT_(TONE_HIGH, GREEN), D_TRANSLATE(ST_TINT_(TONE_HIGH, GREEN)), 0,
										1000.0, 0.01);
		obs_properties_add_float_slider(grp, ST_TINT_(TONE_HIGH, BLUE), D_TRANSLATE(ST_TINT_(TONE_HIGH, BLUE)), 0,
										1000.0, 0.01);
	}

	{
		obs_properties_t* grp = obs_properties_create();
		obs_properties_add_group(pr, ST_CORRECTION, D_TRANSLATE(ST_CORRECTION), OBS_GROUP_NORMAL, grp);

		obs_properties_add_float_slider(grp, ST_CORRECTION_(HUE), D_TRANSLATE(ST_CORRECTION_(HUE)), -180, 180.0, 0.01);
		obs_properties_add_float_slider(grp, ST_CORRECTION_(SATURATION), D_TRANSLATE(ST_CORRECTION_(SATURATION)), 0.0,
										1000.0, 0.01);
		obs_properties_add_float_slider(grp, ST_CORRECTION_(LIGHTNESS), D_TRANSLATE(ST_CORRECTION_(LIGHTNESS)), 0.0,
										1000.0, 0.01);
		obs_properties_add_float_slider(grp, ST_CORRECTION_(CONTRAST), D_TRANSLATE(ST_CORRECTION_(CONTRAST)), 0.0,
										1000.0, 0.01);
	}

	{
		obs_properties_t* grp = obs_properties_create();
		obs_properties_add_group(pr, S_ADVANCED, D_TRANSLATE(S_ADVANCED), OBS_GROUP_NORMAL, grp);

		{
			auto p = obs_properties_add_list(grp, ST_TINT_MODE, D_TRANSLATE(ST_TINT_MODE), OBS_COMBO_TYPE_LIST,
											 OBS_COMBO_FORMAT_INT);
			std::pair<const char*, luma_mode> els[] = {{ST_TINT_MODE_(MODE_LINEAR), luma_mode::Linear},
													   {ST_TINT_MODE_(MODE_EXP), luma_mode::Exp},
													   {ST_TINT_MODE_(MODE_EXP2), luma_mode::Exp2},
													   {ST_TINT_MODE_(MODE_LOG), luma_mode::Log},
													   {ST_TINT_MODE_(MODE_LOG10), luma_mode::Log10}};
			for (auto kv : els) {
				obs_property_list_add_int(p, D_TRANSLATE(kv.first), static_cast<int64_t>(kv.second));
			}
		}

		{
			auto p = obs_properties_add_list(grp, ST_TINT_DETECTION, D_TRANSLATE(ST_TINT_DETECTION),
											 OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
			std::pair<const char*, detection_mode> els[] = {
				{ST_TINT_DETECTION_(DETECTION_HSV), detection_mode::HSV},
				{ST_TINT_DETECTION_(DETECTION_HSL), detection_mode::HSL},
				{ST_TINT_DETECTION_(DETECTION_YUV_SDR), detection_mode::YUV_SDR}};
			for (auto kv : els) {
				obs_property_list_add_int(p, D_TRANSLATE(kv.first), static_cast<int64_t>(kv.second));
			}
		}

		obs_properties_add_float_slider(grp, ST_TINT_EXPONENT, D_TRANSLATE(ST_TINT_EXPONENT), 0., 10., 0.01);

		{
			auto p = obs_properties_add_list(grp, ST_RENDERMODE, D_TRANSLATE(ST_RENDERMODE), OBS_COMBO_TYPE_LIST,
											 OBS_COMBO_FORMAT_INT);
			obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_RENDERMODE)));
			std::pair<const char*, int64_t> els[] = {
				{S_STATE_AUTOMATIC, -1},
				{ST_RENDERMODE_DIRECT, 0},
				{ST_RENDERMODE_LUT_2BIT, static_cast<int64_t>(gfx::lut::color_depth::_2)},
				{ST_RENDERMODE_LUT_4BIT, static_cast<int64_t>(gfx::lut::color_depth::_4)},
				{ST_RENDERMODE_LUT_6BIT, static_cast<int64_t>(gfx::lut::color_depth::_6)},
				{ST_RENDERMODE_LUT_8BIT, static_cast<int64_t>(gfx::lut::color_depth::_8)},
				//{ST_RENDERMODE_LUT_10BIT, static_cast<int64_t>(gfx::lut::color_depth::_10)},
			};
			for (auto kv : els) {
				obs_property_list_add_int(p, D_TRANSLATE(kv.first), kv.second);
			}
		}
	}

	return pr;
}

std::shared_ptr<color_grade_factory> _color_grade_factory_instance = nullptr;

void streamfx::filter::color_grade::color_grade_factory::initialize()
{
	if (!_color_grade_factory_instance)
		_color_grade_factory_instance = std::make_shared<color_grade_factory>();
}

void streamfx::filter::color_grade::color_grade_factory::finalize()
{
	_color_grade_factory_instance.reset();
}

std::shared_ptr<color_grade_factory> streamfx::filter::color_grade::color_grade_factory::get()
{
	return _color_grade_factory_instance;
}
