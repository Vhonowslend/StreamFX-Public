// Modern effects for a modern Streamer
// Copyright (C) 2019 Michael Fabian Dirks
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

#include "gfx-blur-dual-filtering.hpp"
#include "obs/gs/gs-helper.hpp"
#include "plugin.hpp"
#include "util-math.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <obs-module.h>
#include <obs.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

// Dual Filtering Blur
//
// This type of Blur uses downsampling and upsampling and clever math. That makes it less
//  controllable compared to other blur, but it can still be worked with. The distance for
//  sampling texels has to be adjusted to match the correct value so that lower levels of
//  blur than 2^n are possible.
//
// That means that for a blur size of:
//   0: No Iterations, straight copy.
//   1: 1 Iteration (2x), Arm Size 2, Offset Scale 1.0
//   2: 2 Iteration (4x), Arm Size 3, Offset Scale 0.5
//   3: 2 Iteration (4x), Arm Size 4, Offset Scale 1.0
//   4: 3 Iteration (8x), Arm Size 5, Offset Scale 0.25
//   5: 3 Iteration (8x), Arm Size 6, Offset Scale 0.5
//   6: 3 Iteration (8x), Arm Size 7, Offset Scale 0.75
//   7: 3 Iteration (8x), Arm Size 8, Offset Scale 1.0
//   ...

#define MAX_LEVELS 16

gfx::blur::dual_filtering_data::dual_filtering_data()
{
	auto gctx = gs::context();
	try {
		char* file = obs_module_file("effects/blur/dual-filtering._effect");
		_effect   = std::make_shared<::gs::effect>(file);
		bfree(file);
	} catch (...) {
		P_LOG_ERROR("<gfx::blur::box_linear> Failed to load _effect.");
	}
}

gfx::blur::dual_filtering_data::~dual_filtering_data()
{
	auto gctx = gs::context();
	_effect.reset();
}

std::shared_ptr<::gs::effect> gfx::blur::dual_filtering_data::get_effect()
{
	return _effect;
}

gfx::blur::dual_filtering_factory::dual_filtering_factory() {}

gfx::blur::dual_filtering_factory::~dual_filtering_factory() {}

bool gfx::blur::dual_filtering_factory::is_type_supported(::gfx::blur::type type)
{
	switch (type) {
	case ::gfx::blur::type::Area:
		return true;
	default:
		return false;
	}
}

std::shared_ptr<::gfx::blur::base> gfx::blur::dual_filtering_factory::create(::gfx::blur::type type)
{
	switch (type) {
	case ::gfx::blur::type::Area:
		return std::make_shared<::gfx::blur::dual_filtering>();
	default:
		throw std::runtime_error("Invalid type.");
	}
}

double_t gfx::blur::dual_filtering_factory::get_min_size(::gfx::blur::type)
{
	return double_t(1.);
}

double_t gfx::blur::dual_filtering_factory::get_step_size(::gfx::blur::type)
{
	return double_t(1.);
}

double_t gfx::blur::dual_filtering_factory::get_max_size(::gfx::blur::type)
{
	return double_t(MAX_LEVELS);
}

double_t gfx::blur::dual_filtering_factory::get_min_angle(::gfx::blur::type)
{
	return double_t(0);
}

double_t gfx::blur::dual_filtering_factory::get_step_angle(::gfx::blur::type)
{
	return double_t(0);
}

double_t gfx::blur::dual_filtering_factory::get_max_angle(::gfx::blur::type)
{
	return double_t(0);
}

bool gfx::blur::dual_filtering_factory::is_step_scale_supported(::gfx::blur::type)
{
	return false;
}

double_t gfx::blur::dual_filtering_factory::get_min_step_scale_x(::gfx::blur::type)
{
	return double_t(0);
}

double_t gfx::blur::dual_filtering_factory::get_step_step_scale_x(::gfx::blur::type)
{
	return double_t(0);
}

double_t gfx::blur::dual_filtering_factory::get_max_step_scale_x(::gfx::blur::type)
{
	return double_t(0);
}

double_t gfx::blur::dual_filtering_factory::get_min_step_scale_y(::gfx::blur::type)
{
	return double_t(0);
}

double_t gfx::blur::dual_filtering_factory::get_step_step_scale_y(::gfx::blur::type)
{
	return double_t(0);
}

double_t gfx::blur::dual_filtering_factory::get_max_step_scale_y(::gfx::blur::type)
{
	return double_t(0);
}

std::shared_ptr<::gfx::blur::dual_filtering_data> gfx::blur::dual_filtering_factory::data()
{
	std::unique_lock<std::mutex>                      ulock(_data_lock);
	std::shared_ptr<::gfx::blur::dual_filtering_data> data = _data.lock();
	if (!data) {
		data   = std::make_shared<::gfx::blur::dual_filtering_data>();
		_data = data;
	}
	return data;
}

::gfx::blur::dual_filtering_factory& gfx::blur::dual_filtering_factory::get()
{
	static ::gfx::blur::dual_filtering_factory instance;
	return instance;
}

gfx::blur::dual_filtering::dual_filtering()
	: _data(::gfx::blur::dual_filtering_factory::get().data()), _size(0), _size_iterations(0)
{
	auto gctx = gs::context();
	_rendertargets.resize(MAX_LEVELS + 1);
	for (size_t n = 0; n <= MAX_LEVELS; n++) {
		_rendertargets[n] = std::make_shared<gs::rendertarget>(GS_RGBA32F, GS_ZS_NONE);
	}
}

gfx::blur::dual_filtering::~dual_filtering() {}

void gfx::blur::dual_filtering::set_input(std::shared_ptr<::gs::texture> texture)
{
	_input_texture = texture;
}

::gfx::blur::type gfx::blur::dual_filtering::get_type()
{
	return ::gfx::blur::type::Area;
}

double_t gfx::blur::dual_filtering::get_size()
{
	return _size;
}

void gfx::blur::dual_filtering::set_size(double_t width)
{
	_size            = width;
	_size_iterations = size_t(round(width));
	if (_size_iterations >= MAX_LEVELS) {
		_size_iterations = MAX_LEVELS;
	}
}

void gfx::blur::dual_filtering::set_step_scale(double_t, double_t) {}

void gfx::blur::dual_filtering::get_step_scale(double_t&, double_t&) {}

std::shared_ptr<::gs::texture> gfx::blur::dual_filtering::render()
{
	auto gctx   = gs::context();
	auto effect = _data->get_effect();
	if (!effect) {
		return _input_texture;
	}

	size_t actual_iterations = _size_iterations;

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

	// Downsample
	for (size_t n = 1; n <= actual_iterations; n++) {
		// Idx 0 is a simply considered as a straight copy of the original and not rendered to.

		// Select Texture
		std::shared_ptr<gs::texture> tex_cur;
		if (n > 1) {
			tex_cur = _rendertargets[n - 1]->get_texture();
		} else {
			tex_cur = _input_texture;
		}

		// Reduce Size
		uint32_t width  = tex_cur->get_width() / 2;
		uint32_t height = tex_cur->get_height() / 2;
		if ((width <= 0) || (height <= 0)) {
			actual_iterations = n - 1;
			break;
		}

		// Apply
		effect->get_parameter("pImage")->set_texture(tex_cur);
		effect->get_parameter("pImageSize")->set_float2(float_t(width), float_t(height));
		effect->get_parameter("pImageTexel")->set_float2(1.0f / width, 1.0f / height);
		effect->get_parameter("pImageHalfTexel")->set_float2(0.5f / width, 0.5f / height);

		{
			auto op = _rendertargets[n]->render(width, height);
			gs_ortho(0., 1., 0., 1., 0., 1.);
			while (gs_effect_loop(effect->get_object(), "Down")) {
				gs_draw_sprite(tex_cur->get_object(), 0, 1, 1);
			}
		}
	}

	// Upsample
	for (size_t n = actual_iterations; n > 0; n--) {
		// Select Texture
		std::shared_ptr<gs::texture> tex_cur = _rendertargets[n]->get_texture();

		// Get Size
		uint32_t width  = tex_cur->get_width();
		uint32_t height = tex_cur->get_height();

		// Apply
		effect->get_parameter("pImage")->set_texture(tex_cur);
		effect->get_parameter("pImageSize")->set_float2(float_t(width), float_t(height));
		effect->get_parameter("pImageTexel")->set_float2(1.0f / width, 1.0f / height);
		effect->get_parameter("pImageHalfTexel")->set_float2(0.5f / width, 0.5f / height);

		// Increase Size
		width *= 2;
		height *= 2;

		{
			auto op = _rendertargets[n - 1]->render(width, height);
			gs_ortho(0., 1., 0., 1., 0., 1.);
			while (gs_effect_loop(effect->get_object(), "Up")) {
				gs_draw_sprite(tex_cur->get_object(), 0, 1, 1);
			}
		}
	}

	gs_blend_state_pop();

	return _rendertargets[0]->get_texture();
}

std::shared_ptr<::gs::texture> gfx::blur::dual_filtering::get()
{
	return _rendertargets[0]->get_texture();
}
