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

color_grade_instance::~color_grade_instance() {}

color_grade_instance::color_grade_instance(obs_data_t* data, obs_source_t* self) : obs::source_instance(data, self)
{
	{
		char* file = obs_module_file("effects/color-grade.effect");
		if (file) {
			try {
				_effect = gs::effect::create(file);
				bfree(file);
			} catch (std::runtime_error& ex) {
				DLOG_ERROR("<filter-color-grade> Loading _effect '%s' failed with error(s): %s", file, ex.what());
				bfree(file);
				throw ex;
			}
		} else {
			throw std::runtime_error("Missing file color-grade.effect.");
		}
	}
	{
		_rt_source = std::make_unique<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
		{
			auto op = _rt_source->render(1, 1);
		}
		_tex_source = _rt_source->get_texture();
	}
	{
		_rt_grade = std::make_unique<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
		{
			auto op = _rt_grade->render(1, 1);
		}
		_tex_grade = _rt_grade->get_texture();
	}

	update(data);
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

void color_grade_instance::migrate(obs_data_t* data, std::uint64_t version) {}

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
}

void color_grade_instance::video_tick(float)
{
	_source_updated = false;
	_grade_updated  = false;
}

void color_grade_instance::video_render(gs_effect_t* effect)
{
	// Grab initial values.
	obs_source_t* parent         = obs_filter_get_parent(_self);
	obs_source_t* target         = obs_filter_get_target(_self);
	std::uint32_t width          = obs_source_get_base_width(target);
	std::uint32_t height         = obs_source_get_base_height(target);
	gs_effect_t*  effect_default = obs_get_base_effect(obs_base_effect::OBS_EFFECT_DEFAULT);

	// Skip filter if anything is wrong.
	if (!parent || !target || !width || !height || !effect_default) {
		obs_source_skip_video_filter(_self);
		return;
	}

#ifdef ENABLE_PROFILING
	gs::debug_marker gdmp{gs::debug_color_source, "Color Grading '%s'", obs_source_get_name(_self)};
#endif

	if (!_source_updated) {
#ifdef ENABLE_PROFILING
		gs::debug_marker gdm{gs::debug_color_cache, "Cache"};
#endif

		if (obs_source_process_filter_begin(_self, GS_RGBA, OBS_ALLOW_DIRECT_RENDERING)) {
			auto op = _rt_source->render(width, height);
			gs_blend_state_push();
			gs_reset_blend_state();
			gs_set_cull_mode(GS_NEITHER);
			gs_enable_color(true, true, true, true);
			gs_enable_blending(false);
			gs_enable_depth_test(false);
			gs_enable_stencil_test(false);
			gs_enable_stencil_write(false);
			gs_ortho(0, static_cast<float_t>(width), 0, static_cast<float_t>(height), -1., 1.);
			obs_source_process_filter_end(_self, effect ? effect : effect_default, width, height);
			gs_blend_state_pop();
		}

		_tex_source     = _rt_source->get_texture();
		_source_updated = true;
	}

	if (!_grade_updated) {
#ifdef ENABLE_PROFILING
		gs::debug_marker gdm{gs::debug_color_convert, "Calculate"};
#endif

		{
			auto op = _rt_grade->render(width, height);
			gs_blend_state_push();
			gs_reset_blend_state();
			gs_set_cull_mode(GS_NEITHER);
			gs_enable_color(true, true, true, true);
			gs_enable_blending(false);
			gs_enable_depth_test(false);
			gs_enable_stencil_test(false);
			gs_enable_stencil_write(false);
			gs_ortho(0, 1, 0, 1, -1., 1.);

			_effect.get_parameter("image").set_texture(_tex_source);
			_effect.get_parameter("pLift").set_float4(_lift);
			_effect.get_parameter("pGamma").set_float4(_gamma);
			_effect.get_parameter("pGain").set_float4(_gain);
			_effect.get_parameter("pOffset").set_float4(_offset);
			_effect.get_parameter("pTintDetection").set_int(static_cast<int32_t>(_tint_detection));
			_effect.get_parameter("pTintMode").set_int(static_cast<int32_t>(_tint_luma));
			_effect.get_parameter("pTintExponent").set_float(_tint_exponent);
			_effect.get_parameter("pTintLow").set_float3(_tint_low);
			_effect.get_parameter("pTintMid").set_float3(_tint_mid);
			_effect.get_parameter("pTintHig").set_float3(_tint_hig);
			_effect.get_parameter("pCorrection").set_float4(_correction);

			while (gs_effect_loop(_effect.get_object(), "Draw")) {
				streamfx::gs_draw_fullscreen_tri();
			}

			gs_blend_state_pop();
		}

		_tex_grade      = _rt_grade->get_texture();
		_source_updated = true;
	}

	// Render final result.
	{
#ifdef ENABLE_PROFILING
		gs::debug_marker gdm{gs::debug_color_render, "Render"};
#endif

		auto shader = obs_get_base_effect(OBS_EFFECT_DEFAULT);
		gs_enable_depth_test(false);
		while (gs_effect_loop(shader, "Draw")) {
			gs_effect_set_texture(gs_effect_get_param_by_name(shader, "image"),
								  _tex_grade ? _tex_grade->get_object() : nullptr);
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
