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

#include "filter-transform.hpp"
#include "strings.hpp"
#include <algorithm>
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

#define ST "Filter.Transform"
#define ST_CAMERA "Filter.Transform.Camera"
#define ST_CAMERA_ORTHOGRAPHIC "Filter.Transform.Camera.Orthographic"
#define ST_CAMERA_PERSPECTIVE "Filter.Transform.Camera.Perspective"
#define ST_CAMERA_FIELDOFVIEW "Filter.Transform.Camera.FieldOfView"
#define ST_POSITION "Filter.Transform.Position"
#define ST_POSITION_X "Filter.Transform.Position.X"
#define ST_POSITION_Y "Filter.Transform.Position.Y"
#define ST_POSITION_Z "Filter.Transform.Position.Z"
#define ST_ROTATION "Filter.Transform.Rotation"
#define ST_ROTATION_X "Filter.Transform.Rotation.X"
#define ST_ROTATION_Y "Filter.Transform.Rotation.Y"
#define ST_ROTATION_Z "Filter.Transform.Rotation.Z"
#define ST_SCALE "Filter.Transform.Scale"
#define ST_SCALE_X "Filter.Transform.Scale.X"
#define ST_SCALE_Y "Filter.Transform.Scale.Y"
#define ST_SHEAR "Filter.Transform.Shear"
#define ST_SHEAR_X "Filter.Transform.Shear.X"
#define ST_SHEAR_Y "Filter.Transform.Shear.Y"
#define ST_ROTATION_ORDER "Filter.Transform.Rotation.Order"
#define ST_ROTATION_ORDER_XYZ "Filter.Transform.Rotation.Order.XYZ"
#define ST_ROTATION_ORDER_XZY "Filter.Transform.Rotation.Order.XZY"
#define ST_ROTATION_ORDER_YXZ "Filter.Transform.Rotation.Order.YXZ"
#define ST_ROTATION_ORDER_YZX "Filter.Transform.Rotation.Order.YZX"
#define ST_ROTATION_ORDER_ZXY "Filter.Transform.Rotation.Order.ZXY"
#define ST_ROTATION_ORDER_ZYX "Filter.Transform.Rotation.Order.ZYX"
#define ST_MIPMAPPING "Filter.Transform.Mipmapping"

using namespace streamfx::filter::transform;

static constexpr std::string_view HELP_URL = "https://github.com/Xaymar/obs-StreamFX/wiki/Filter-3D-Transform";

static const float_t farZ  = 2097152.0f; // 2 pow 21
static const float_t nearZ = 1.0f / farZ;

enum class CameraMode : int64_t { Orthographic, Perspective };

enum RotationOrder : int64_t {
	XYZ,
	XZY,
	YXZ,
	YZX,
	ZXY,
	ZYX,
};

transform_instance::transform_instance(obs_data_t* data, obs_source_t* context)
	: obs::source_instance(data, context), _cache_rendered(), _mipmap_enabled(), _source_rendered(), _source_size(),
	  _update_mesh(), _rotation_order(), _camera_orthographic(), _camera_fov()
{
	_cache_rt      = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
	_source_rt     = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
	_vertex_buffer = std::make_shared<gs::vertex_buffer>(uint32_t(4u), uint8_t(1u));

	_position = std::make_unique<util::vec3a>();
	_rotation = std::make_unique<util::vec3a>();
	_scale    = std::make_unique<util::vec3a>();
	_shear    = std::make_unique<util::vec3a>();

	vec3_set(_position.get(), 0, 0, 0);
	vec3_set(_rotation.get(), 0, 0, 0);
	vec3_set(_scale.get(), 1, 1, 1);

	update(data);
}

transform_instance::~transform_instance()
{
	_shear.reset();
	_scale.reset();
	_rotation.reset();
	_position.reset();
	_vertex_buffer.reset();
	_cache_rt.reset();
	_cache_texture.reset();
	_mipmap_texture.reset();
}

void transform_instance::load(obs_data_t* settings)
{
	update(settings);
}

void transform_instance::migrate(obs_data_t* data, uint64_t version)
{
	switch (version & STREAMFX_MASK_COMPAT) {
	case 0:
		obs_data_set_double(data, ST_ROTATION_X, -obs_data_get_double(data, ST_ROTATION_X));
		obs_data_set_double(data, ST_ROTATION_Y, -obs_data_get_double(data, ST_ROTATION_Y));
	}
}

void transform_instance::update(obs_data_t* settings)
{
	// Camera
	_camera_orthographic = obs_data_get_int(settings, ST_CAMERA) == 0;
	_camera_fov          = static_cast<float_t>(obs_data_get_double(settings, ST_CAMERA_FIELDOFVIEW));

	// Source
	_position->x    = static_cast<float_t>(obs_data_get_double(settings, ST_POSITION_X) / 100.0);
	_position->y    = static_cast<float_t>(obs_data_get_double(settings, ST_POSITION_Y) / 100.0);
	_position->z    = static_cast<float_t>(obs_data_get_double(settings, ST_POSITION_Z) / 100.0);
	_scale->x       = static_cast<float_t>(obs_data_get_double(settings, ST_SCALE_X) / 100.0);
	_scale->y       = static_cast<float_t>(obs_data_get_double(settings, ST_SCALE_Y) / 100.0);
	_scale->z       = 1.0f;
	_rotation_order = static_cast<uint32_t>(obs_data_get_int(settings, ST_ROTATION_ORDER));
	_rotation->x    = static_cast<float_t>(obs_data_get_double(settings, ST_ROTATION_X) / 180.0 * S_PI);
	_rotation->y    = static_cast<float_t>(obs_data_get_double(settings, ST_ROTATION_Y) / 180.0 * S_PI);
	_rotation->z    = static_cast<float_t>(obs_data_get_double(settings, ST_ROTATION_Z) / 180.0 * S_PI);
	_shear->x       = static_cast<float_t>(obs_data_get_double(settings, ST_SHEAR_X) / 100.0);
	_shear->y       = static_cast<float_t>(obs_data_get_double(settings, ST_SHEAR_Y) / 100.0);
	_shear->z       = 0.0f;

	// Mipmapping
	_mipmap_enabled = obs_data_get_bool(settings, ST_MIPMAPPING);

	_update_mesh = true;
}

void transform_instance::video_tick(float_t)
{
	uint32_t width  = 0;
	uint32_t height = 0;

	// Grab parent and target.
	obs_source_t* target = obs_filter_get_target(_self);
	if (target) {
		// Grab width an height of the target source (child filter or source).
		width  = obs_source_get_base_width(target);
		height = obs_source_get_base_height(target);
	}

	// If size mismatch, force an update.
	if (width != _source_size.first) {
		_update_mesh = true;
	} else if (height != _source_size.second) {
		_update_mesh = true;
	}

	// Update Mesh
	if (_update_mesh) {
		_source_size.first  = width;
		_source_size.second = height;

		if (width == 0) {
			width = 1;
		}
		if (height == 0) {
			height = 1;
		}

		// Calculate Aspect Ratio
		float_t aspectRatioX = float_t(width) / float_t(height);
		if (_camera_orthographic)
			aspectRatioX = 1.0;

		// Mesh
		matrix4 ident;
		matrix4_identity(&ident);
		switch (_rotation_order) {
		case RotationOrder::XYZ: // XYZ
			matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, _rotation->x);
			matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, _rotation->y);
			matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, _rotation->z);
			break;
		case RotationOrder::XZY: // XZY
			matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, _rotation->x);
			matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, _rotation->z);
			matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, _rotation->y);
			break;
		case RotationOrder::YXZ: // YXZ
			matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, _rotation->y);
			matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, _rotation->x);
			matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, _rotation->z);
			break;
		case RotationOrder::YZX: // YZX
			matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, _rotation->y);
			matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, _rotation->z);
			matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, _rotation->x);
			break;
		case RotationOrder::ZXY: // ZXY
			matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, _rotation->z);
			matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, _rotation->x);
			matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, _rotation->y);
			break;
		case RotationOrder::ZYX: // ZYX
			matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, _rotation->z);
			matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, _rotation->y);
			matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, _rotation->x);
			break;
		}
		matrix4_translate3f(&ident, &ident, _position->x, _position->y, _position->z);
		//matrix4_scale3f(&ident, &ident, _source_size.first / 2.f, _source_size.second / 2.f, 1.f);

		/// Calculate vertex position once only.
		float_t p_x = aspectRatioX * _scale->x;
		float_t p_y = 1.0f * _scale->y;

		/// Generate mesh
		{
			auto vtx   = _vertex_buffer->at(0);
			*vtx.color = 0xFFFFFFFF;
			vec4_set(vtx.uv[0], 0, 0, 0, 0);
			vec3_set(vtx.position, -p_x + _shear->x, -p_y - _shear->y, 0);
			vec3_transform(vtx.position, vtx.position, &ident);
		}
		{
			auto vtx   = _vertex_buffer->at(1);
			*vtx.color = 0xFFFFFFFF;
			vec4_set(vtx.uv[0], 1, 0, 0, 0);
			vec3_set(vtx.position, p_x + _shear->x, -p_y + _shear->y, 0);
			vec3_transform(vtx.position, vtx.position, &ident);
		}
		{
			auto vtx   = _vertex_buffer->at(2);
			*vtx.color = 0xFFFFFFFF;
			vec4_set(vtx.uv[0], 0, 1, 0, 0);
			vec3_set(vtx.position, -p_x - _shear->x, p_y - _shear->y, 0);
			vec3_transform(vtx.position, vtx.position, &ident);
		}
		{
			auto vtx   = _vertex_buffer->at(3);
			*vtx.color = 0xFFFFFFFF;
			vec4_set(vtx.uv[0], 1, 1, 0, 0);
			vec3_set(vtx.position, p_x - _shear->x, p_y + _shear->y, 0);
			vec3_transform(vtx.position, vtx.position, &ident);
		}

		_vertex_buffer->update(true);
		_update_mesh = false;
	}

	_cache_rendered  = false;
	_mipmap_rendered = false;
	_source_rendered = false;
}

void transform_instance::video_render(gs_effect_t* effect)
{
	obs_source_t* parent         = obs_filter_get_parent(_self);
	obs_source_t* target         = obs_filter_get_target(_self);
	uint32_t      base_width     = obs_source_get_base_width(target);
	uint32_t      base_height    = obs_source_get_base_height(target);
	gs_effect_t*  default_effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);
	if (!effect)
		effect = default_effect;

	if (!base_width || !base_height || !parent || !target) { // Skip if something is wrong.
		obs_source_skip_video_filter(_self);
		return;
	}

#ifdef ENABLE_PROFILING
	gs::debug_marker gdmp{gs::debug_color_source, "3D Transform '%s' on '%s'", obs_source_get_name(_self),
						  obs_source_get_name(obs_filter_get_parent(_self))};
#endif

	uint32_t cache_width  = base_width;
	uint32_t cache_height = base_height;

	if (_mipmap_enabled) {
		double_t aspect  = double_t(base_width) / double_t(base_height);
		double_t aspect2 = 1.0 / aspect;
		cache_width = std::clamp(uint32_t(pow(2, util::math::get_power_of_two_exponent_ceil(cache_width))), 1u, 16384u);
		cache_height =
			std::clamp(uint32_t(pow(2, util::math::get_power_of_two_exponent_ceil(cache_height))), 1u, 16384u);

		if (aspect > 1.0) {
			cache_height = std::clamp(
				uint32_t(pow(2, util::math::get_power_of_two_exponent_ceil(uint64_t(cache_width * aspect2)))), 1u,
				16384u);
		} else if (aspect < 1.0) {
			cache_width = std::clamp(
				uint32_t(pow(2, util::math::get_power_of_two_exponent_ceil(uint64_t(cache_height * aspect)))), 1u,
				16384u);
		}
	}

	if (!_cache_rendered) {
#ifdef ENABLE_PROFILING
		gs::debug_marker gdm{gs::debug_color_cache, "Cache"};
#endif

		auto op = _cache_rt->render(cache_width, cache_height);

		gs_ortho(0, static_cast<float_t>(base_width), 0, static_cast<float_t>(base_height), -1, 1);

		vec4 clear_color = {0, 0, 0, 0};
		gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &clear_color, 0, 0);

		/// Render original source
		if (obs_source_process_filter_begin(_self, GS_RGBA, OBS_NO_DIRECT_RENDERING)) {
			gs_blend_state_push();
			gs_reset_blend_state();
			gs_enable_blending(false);
			gs_blend_function_separate(GS_BLEND_ONE, GS_BLEND_ZERO, GS_BLEND_SRCALPHA, GS_BLEND_ZERO);
			gs_enable_depth_test(false);
			gs_enable_stencil_test(false);
			gs_enable_stencil_write(false);
			gs_enable_color(true, true, true, true);
			gs_set_cull_mode(GS_NEITHER);

			obs_source_process_filter_end(_self, default_effect, base_width, base_height);

			gs_blend_state_pop();
		} else {
			obs_source_skip_video_filter(_self);
			return;
		}

		_cache_rendered = true;
	}
	_cache_rt->get_texture(_cache_texture);
	if (!_cache_texture) {
		obs_source_skip_video_filter(_self);
		return;
	}

	if (_mipmap_enabled) {
#ifdef ENABLE_PROFILING
		gs::debug_marker gdm{gs::debug_color_convert, "Mipmap"};
#endif

		if (!_mipmap_texture || (_mipmap_texture->get_width() != cache_width)
			|| (_mipmap_texture->get_height() != cache_height)) {
#ifdef ENABLE_PROFILING
			gs::debug_marker gdr{gs::debug_color_allocate, "Allocate Mipmapped Texture"};
#endif

			std::size_t mip_levels = std::max(util::math::get_power_of_two_exponent_ceil(cache_width),
											  util::math::get_power_of_two_exponent_ceil(cache_height));
			_mipmap_texture =
				std::make_shared<gs::texture>(cache_width, cache_height, GS_RGBA, static_cast<uint32_t>(mip_levels),
											  nullptr, gs::texture::flags::None);
		}
		_mipmapper.rebuild(_cache_texture, _mipmap_texture);

		_mipmap_rendered = true;
		if (!_mipmap_texture) {
			obs_source_skip_video_filter(_self);
			return;
		}
	}

	{
#ifdef ENABLE_PROFILING
		gs::debug_marker gdm{gs::debug_color_convert, "Transform"};
#endif

		auto op = _source_rt->render(base_width, base_height);

		gs_blend_state_push();
		gs_reset_blend_state();
		gs_enable_blending(false);
		gs_blend_function_separate(GS_BLEND_ONE, GS_BLEND_ZERO, GS_BLEND_ONE, GS_BLEND_ZERO);

		gs_enable_depth_test(false);
		gs_enable_stencil_test(false);
		gs_enable_stencil_write(false);
		gs_enable_color(true, true, true, true);
		gs_set_cull_mode(GS_NEITHER);

		if (_camera_orthographic) {
			gs_ortho(-1., 1., -1., 1., -farZ, farZ);
		} else {
			gs_perspective(_camera_fov, float_t(base_width) / float_t(base_height), nearZ, farZ);
			gs_matrix_scale3f(1.0, 1.0, 1.0);
			gs_matrix_translate3f(0., 0., -1.0);
		}

		vec4 clear_color = {0, 0, 0, 0};
		gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &clear_color, 0, 0);

		gs_load_vertexbuffer(_vertex_buffer->update(false));
		gs_load_indexbuffer(nullptr);
		gs_effect_set_texture(gs_effect_get_param_by_name(default_effect, "image"),
							  _mipmap_enabled
								  ? (_mipmap_texture ? _mipmap_texture->get_object() : _cache_texture->get_object())
								  : _cache_texture->get_object());
		while (gs_effect_loop(default_effect, "Draw")) {
			gs_draw(GS_TRISTRIP, 0, 4);
		}
		gs_load_vertexbuffer(nullptr);

		gs_blend_state_pop();
	}
	_source_rt->get_texture(_source_texture);
	if (!_source_texture) {
		obs_source_skip_video_filter(_self);
		return;
	}

	{
#ifdef ENABLE_PROFILING
		gs::debug_marker gdm{gs::debug_color_render, "Render"};
#endif

		gs_effect_set_texture(gs_effect_get_param_by_name(effect, "image"), _source_texture->get_object());
		while (gs_effect_loop(effect, "Draw")) {
			gs_draw_sprite(nullptr, 0, base_width, base_height);
		}
	}
}

transform_factory::transform_factory()
{
	_info.id           = PREFIX "filter-transform";
	_info.type         = OBS_SOURCE_TYPE_FILTER;
	_info.output_flags = OBS_SOURCE_VIDEO;

	set_resolution_enabled(false);
	finish_setup();
	register_proxy("obs-stream-effects-filter-transform");
}

transform_factory::~transform_factory() {}

const char* transform_factory::get_name()
{
	return D_TRANSLATE(ST);
}

void transform_factory::get_defaults2(obs_data_t* settings)
{
	obs_data_set_default_int(settings, ST_CAMERA, static_cast<int64_t>(CameraMode::Orthographic));
	obs_data_set_default_double(settings, ST_CAMERA_FIELDOFVIEW, 90.0);
	obs_data_set_default_double(settings, ST_POSITION_X, 0);
	obs_data_set_default_double(settings, ST_POSITION_Y, 0);
	obs_data_set_default_double(settings, ST_POSITION_Z, 0);
	obs_data_set_default_double(settings, ST_ROTATION_X, 0);
	obs_data_set_default_double(settings, ST_ROTATION_Y, 0);
	obs_data_set_default_double(settings, ST_ROTATION_Z, 0);
	obs_data_set_default_int(settings, ST_ROTATION_ORDER, static_cast<int64_t>(RotationOrder::ZXY));
	obs_data_set_default_double(settings, ST_SCALE_X, 100);
	obs_data_set_default_double(settings, ST_SCALE_Y, 100);
	obs_data_set_default_double(settings, ST_SHEAR_X, 0);
	obs_data_set_default_double(settings, ST_SHEAR_Y, 0);
	obs_data_set_default_bool(settings, ST_MIPMAPPING, false);
}

static bool modified_properties(obs_properties_t* pr, obs_property_t*, obs_data_t* d) noexcept
try {
	switch (static_cast<CameraMode>(obs_data_get_int(d, ST_CAMERA))) {
	case CameraMode::Orthographic:
		obs_property_set_visible(obs_properties_get(pr, ST_CAMERA_FIELDOFVIEW), false);
		obs_property_set_visible(obs_properties_get(pr, ST_POSITION_Z), false);
		break;
	case CameraMode::Perspective:
		obs_property_set_visible(obs_properties_get(pr, ST_CAMERA_FIELDOFVIEW), true);
		obs_property_set_visible(obs_properties_get(pr, ST_POSITION_Z), true);
		break;
	}

	return true;
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
	return true;
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
	return true;
}

obs_properties_t* transform_factory::get_properties2(transform_instance* data)
{
	obs_properties_t* pr = obs_properties_create();

#ifdef ENABLE_FRONTEND
	{
		obs_properties_add_button2(pr, S_MANUAL_OPEN, D_TRANSLATE(S_MANUAL_OPEN),
								   streamfx::filter::transform::transform_factory::on_manual_open, nullptr);
	}
#endif

	// Camera
	{
		auto grp = obs_properties_create();

		{ // Projection Mode
			auto p = obs_properties_add_list(grp, ST_CAMERA, D_TRANSLATE(ST_CAMERA), OBS_COMBO_TYPE_LIST,
											 OBS_COMBO_FORMAT_INT);
			obs_property_list_add_int(p, D_TRANSLATE(ST_CAMERA_ORTHOGRAPHIC),
									  static_cast<int64_t>(CameraMode::Orthographic));
			obs_property_list_add_int(p, D_TRANSLATE(ST_CAMERA_PERSPECTIVE),
									  static_cast<int64_t>(CameraMode::Perspective));
			obs_property_set_modified_callback(p, modified_properties);
		}
		{ // Field Of View
			auto p = obs_properties_add_float_slider(grp, ST_CAMERA_FIELDOFVIEW, D_TRANSLATE(ST_CAMERA_FIELDOFVIEW),
													 1.0, 179.0, 0.01);
		}

		obs_properties_add_group(pr, ST_CAMERA, D_TRANSLATE(ST_CAMERA), OBS_GROUP_NORMAL, grp);
	}

	// Mesh
	{ // Position
		auto grp = obs_properties_create();

		const char* opts[] = {ST_POSITION_X, ST_POSITION_Y, ST_POSITION_Z};
		for (auto opt : opts) {
			auto p = obs_properties_add_float(grp, opt, D_TRANSLATE(opt), std::numeric_limits<float_t>::lowest(),
											  std::numeric_limits<float_t>::max(), 0.01);
		}

		obs_properties_add_group(pr, ST_POSITION, D_TRANSLATE(ST_POSITION), OBS_GROUP_NORMAL, grp);
	}
	{ // Rotation
		auto grp = obs_properties_create();

		{
			const char* opts[] = {ST_ROTATION_X, ST_ROTATION_Y, ST_ROTATION_Z};
			for (auto opt : opts) {
				auto p = obs_properties_add_float_slider(grp, opt, D_TRANSLATE(opt), -180.0, 180.0, 0.01);
				obs_property_float_set_suffix(p, "° Deg");
			}
		}

		obs_properties_add_group(pr, ST_ROTATION, D_TRANSLATE(ST_ROTATION), OBS_GROUP_NORMAL, grp);
	}
	{ // Scale
		auto grp = obs_properties_create();

		const char* opts[] = {ST_SCALE_X, ST_SCALE_Y};
		for (auto opt : opts) {
			auto p = obs_properties_add_float_slider(grp, opt, D_TRANSLATE(opt), -1000, 1000, 0.01);
			obs_property_float_set_suffix(p, "%");
		}

		obs_properties_add_group(pr, ST_SCALE, D_TRANSLATE(ST_SCALE), OBS_GROUP_NORMAL, grp);
	}
	{ // Shear
		auto grp = obs_properties_create();

		const char* opts[] = {ST_SHEAR_X, ST_SHEAR_Y};
		for (auto opt : opts) {
			auto p = obs_properties_add_float_slider(grp, opt, D_TRANSLATE(opt), -200.0, 200.0, 0.01);
			obs_property_float_set_suffix(p, "%");
		}

		obs_properties_add_group(pr, ST_SHEAR, D_TRANSLATE(ST_SHEAR), OBS_GROUP_NORMAL, grp);
	}

	{
		auto grp = obs_properties_create();
		obs_properties_add_group(pr, S_ADVANCED, D_TRANSLATE(S_ADVANCED), OBS_GROUP_NORMAL, grp);

		{ // Mipmapping
			auto p = obs_properties_add_bool(grp, ST_MIPMAPPING, D_TRANSLATE(ST_MIPMAPPING));
		}

		{ // Order
			auto p = obs_properties_add_list(grp, ST_ROTATION_ORDER, D_TRANSLATE(ST_ROTATION_ORDER),
											 OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
			obs_property_list_add_int(p, D_TRANSLATE(ST_ROTATION_ORDER_XYZ), RotationOrder::XYZ);
			obs_property_list_add_int(p, D_TRANSLATE(ST_ROTATION_ORDER_XZY), RotationOrder::XZY);
			obs_property_list_add_int(p, D_TRANSLATE(ST_ROTATION_ORDER_YXZ), RotationOrder::YXZ);
			obs_property_list_add_int(p, D_TRANSLATE(ST_ROTATION_ORDER_YZX), RotationOrder::YZX);
			obs_property_list_add_int(p, D_TRANSLATE(ST_ROTATION_ORDER_ZXY), RotationOrder::ZXY);
			obs_property_list_add_int(p, D_TRANSLATE(ST_ROTATION_ORDER_ZYX), RotationOrder::ZYX);
		}
	}

	return pr;
}

#ifdef ENABLE_FRONTEND
bool transform_factory::on_manual_open(obs_properties_t* props, obs_property_t* property, void* data)
{
	streamfx::open_url(HELP_URL);
	return false;
}
#endif

std::shared_ptr<transform_factory> _filter_transform_factory_instance = nullptr;

void transform_factory::initialize()
{
	if (!_filter_transform_factory_instance)
		_filter_transform_factory_instance = std::make_shared<transform_factory>();
}

void transform_factory::finalize()
{
	_filter_transform_factory_instance.reset();
}

std::shared_ptr<transform_factory> transform_factory::get()
{
	return _filter_transform_factory_instance;
}
