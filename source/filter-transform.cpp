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
#include "util-math.hpp"

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

// Initializer & Finalizer
static filter::TransformAddon* filterTransformInstance;
INITIALIZER(FilterTransformInit)
{
	initializerFunctions.push_back([] { filterTransformInstance = new filter::TransformAddon(); });
	finalizerFunctions.push_back([] { delete filterTransformInstance; });
}

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

static const float farZ       = 2097152.0f; // 2 pow 21
static const float nearZ      = 1.0f / farZ;

enum class CameraMode : int32_t { Orthographic, Perspective };

enum RotationOrder : int64_t {
	XYZ,
	XZY,
	YXZ,
	YZX,
	ZXY,
	ZYX,
};

filter::TransformAddon::TransformAddon()
{
	memset(&sourceInfo, 0, sizeof(obs_source_info));
	sourceInfo.id             = "obs-stream-effects-filter-transform";
	sourceInfo.type           = OBS_SOURCE_TYPE_FILTER;
	sourceInfo.output_flags   = OBS_SOURCE_VIDEO;
	sourceInfo.get_name       = get_name;
	sourceInfo.get_defaults   = get_defaults;
	sourceInfo.get_properties = get_properties;

	sourceInfo.create       = create;
	sourceInfo.destroy      = destroy;
	sourceInfo.update       = update;
	sourceInfo.activate     = activate;
	sourceInfo.deactivate   = deactivate;
	sourceInfo.video_tick   = video_tick;
	sourceInfo.video_render = video_render;

	obs_register_source(&sourceInfo);
}

filter::TransformAddon::~TransformAddon() {}

const char* filter::TransformAddon::get_name(void*)
{
	return P_TRANSLATE(ST);
}

void filter::TransformAddon::get_defaults(obs_data_t* data)
{
	obs_data_set_default_int(data, ST_CAMERA, (int64_t)CameraMode::Orthographic);
	obs_data_set_default_double(data, ST_CAMERA_FIELDOFVIEW, 90.0);
	obs_data_set_default_double(data, ST_POSITION_X, 0);
	obs_data_set_default_double(data, ST_POSITION_Y, 0);
	obs_data_set_default_double(data, ST_POSITION_Z, 0);
	obs_data_set_default_double(data, ST_ROTATION_X, 0);
	obs_data_set_default_double(data, ST_ROTATION_Y, 0);
	obs_data_set_default_double(data, ST_ROTATION_Z, 0);
	obs_data_set_default_double(data, ST_SCALE_X, 100);
	obs_data_set_default_double(data, ST_SCALE_Y, 100);
	obs_data_set_default_double(data, ST_SHEAR_X, 0);
	obs_data_set_default_double(data, ST_SHEAR_Y, 0);
	obs_data_set_default_bool(data, S_ADVANCED, false);
	obs_data_set_default_int(data, ST_ROTATION_ORDER,
							 RotationOrder::ZXY); //ZXY
}

obs_properties_t* filter::TransformAddon::get_properties(void*)
{
	obs_properties_t* pr = obs_properties_create();
	obs_property_t*   p  = NULL;

	p = obs_properties_add_list(pr, ST_CAMERA, P_TRANSLATE(ST_CAMERA), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(ST_CAMERA)));
	obs_property_list_add_int(p, P_TRANSLATE(ST_CAMERA_ORTHOGRAPHIC), (int64_t)CameraMode::Orthographic);
	obs_property_list_add_int(p, P_TRANSLATE(ST_CAMERA_PERSPECTIVE), (int64_t)CameraMode::Perspective);
	obs_property_set_modified_callback(p, modified_properties);

	p = obs_properties_add_float_slider(pr, ST_CAMERA_FIELDOFVIEW, P_TRANSLATE(ST_CAMERA_FIELDOFVIEW), 1.0, 179.0,
										0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(ST_CAMERA_FIELDOFVIEW)));

	// Position, Scale, Rotation
	{
		std::pair<const char*, const char*> entries[] = {
			std::make_pair(ST_POSITION_X, P_DESC(ST_POSITION_X)),
			std::make_pair(ST_POSITION_Y, P_DESC(ST_POSITION_Y)),
			std::make_pair(ST_POSITION_Z, P_DESC(ST_POSITION_Z)),
		};
		for (auto kv : entries) {
			p = obs_properties_add_float(pr, kv.first, P_TRANSLATE(kv.first), -10000, 10000, 0.01);
			obs_property_set_long_description(p, P_TRANSLATE(kv.second));
		}
	}
	{
		std::pair<const char*, const char*> entries[] = {
			std::make_pair(ST_ROTATION_X, P_DESC(ST_ROTATION_X)),
			std::make_pair(ST_ROTATION_Y, P_DESC(ST_ROTATION_Y)),
			std::make_pair(ST_ROTATION_Z, P_DESC(ST_ROTATION_Z)),
		};
		for (auto kv : entries) {
			p = obs_properties_add_float_slider(pr, kv.first, P_TRANSLATE(kv.first), -180, 180, 0.01);
			obs_property_set_long_description(p, P_TRANSLATE(kv.second));
		}
	}
	{
		std::pair<const char*, const char*> entries[] = {
			std::make_pair(ST_SCALE_X, P_DESC(ST_SCALE_X)),
			std::make_pair(ST_SCALE_Y, P_DESC(ST_SCALE_Y)),
		};
		for (auto kv : entries) {
			p = obs_properties_add_float_slider(pr, kv.first, P_TRANSLATE(kv.first), -1000, 1000, 0.01);
			obs_property_set_long_description(p, P_TRANSLATE(kv.second));
		}
	}
	{
		std::pair<const char*, const char*> entries[] = {
			std::make_pair(ST_SHEAR_X, P_DESC(ST_SHEAR_X)),
			std::make_pair(ST_SHEAR_Y, P_DESC(ST_SHEAR_Y)),
		};
		for (auto kv : entries) {
			p = obs_properties_add_float_slider(pr, kv.first, P_TRANSLATE(kv.first), -100.0, 100.0, 0.01);
			obs_property_set_long_description(p, P_TRANSLATE(kv.second));
		}
	}

	p = obs_properties_add_bool(pr, S_ADVANCED, P_TRANSLATE(S_ADVANCED));
	obs_property_set_modified_callback(p, modified_properties);

	p = obs_properties_add_list(pr, ST_ROTATION_ORDER, P_TRANSLATE(ST_ROTATION_ORDER), OBS_COMBO_TYPE_LIST,
								OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(ST_ROTATION_ORDER)));
	obs_property_list_add_int(p, P_TRANSLATE(ST_ROTATION_ORDER_XYZ), RotationOrder::XYZ);
	obs_property_list_add_int(p, P_TRANSLATE(ST_ROTATION_ORDER_XZY), RotationOrder::XZY);
	obs_property_list_add_int(p, P_TRANSLATE(ST_ROTATION_ORDER_YXZ), RotationOrder::YXZ);
	obs_property_list_add_int(p, P_TRANSLATE(ST_ROTATION_ORDER_YZX), RotationOrder::YZX);
	obs_property_list_add_int(p, P_TRANSLATE(ST_ROTATION_ORDER_ZXY), RotationOrder::ZXY);
	obs_property_list_add_int(p, P_TRANSLATE(ST_ROTATION_ORDER_ZYX), RotationOrder::ZYX);

	p = obs_properties_add_bool(pr, ST_MIPMAPPING, P_TRANSLATE(ST_MIPMAPPING));
	obs_property_set_modified_callback(p, modified_properties);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(ST_MIPMAPPING)));

	p = obs_properties_add_list(pr, strings::MipGenerator::Name, P_TRANSLATE(strings::MipGenerator::Name),
								OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, P_TRANSLATE(strings::MipGenerator::Description));

	obs_property_list_add_int(p, P_TRANSLATE(strings::MipGenerator::Point), (long long)gs::mipmapper::generator::Point);
	obs_property_list_add_int(p, P_TRANSLATE(strings::MipGenerator::Linear),
							  (long long)gs::mipmapper::generator::Linear);
	obs_property_list_add_int(p, P_TRANSLATE(strings::MipGenerator::Sharpen),
							  (long long)gs::mipmapper::generator::Sharpen);
	obs_property_list_add_int(p, P_TRANSLATE(strings::MipGenerator::Smoothen),
							  (long long)gs::mipmapper::generator::Smoothen);
	obs_property_list_add_int(p, P_TRANSLATE(strings::MipGenerator::Bicubic),
							  (long long)gs::mipmapper::generator::Bicubic);
	obs_property_list_add_int(p, P_TRANSLATE(strings::MipGenerator::Lanczos),
							  (long long)gs::mipmapper::generator::Lanczos);

	p = obs_properties_add_float(pr, strings::MipGenerator::Strength, P_TRANSLATE(strings::MipGenerator::Strength), 0.0,
								 100.0, 0.01);

	return pr;
}

bool filter::TransformAddon::modified_properties(obs_properties_t* pr, obs_property_t*, obs_data_t* d)
{
	switch ((CameraMode)obs_data_get_int(d, ST_CAMERA)) {
	case CameraMode::Orthographic:
		obs_property_set_visible(obs_properties_get(pr, ST_CAMERA_FIELDOFVIEW), false);
		obs_property_set_visible(obs_properties_get(pr, ST_POSITION_Z), false);
		break;
	case CameraMode::Perspective:
		obs_property_set_visible(obs_properties_get(pr, ST_CAMERA_FIELDOFVIEW), true);
		obs_property_set_visible(obs_properties_get(pr, ST_POSITION_Z), true);
		break;
	}

	bool advancedVisible = obs_data_get_bool(d, S_ADVANCED);
	obs_property_set_visible(obs_properties_get(pr, ST_ROTATION_ORDER), advancedVisible);
	obs_property_set_visible(obs_properties_get(pr, ST_MIPMAPPING), advancedVisible);

	bool mipmappingVisible = obs_data_get_bool(d, ST_MIPMAPPING) && advancedVisible;
	obs_property_set_visible(obs_properties_get(pr, strings::MipGenerator::Name), mipmappingVisible);
	obs_property_set_visible(obs_properties_get(pr, strings::MipGenerator::Strength), mipmappingVisible);

	return true;
}

void* filter::TransformAddon::create(obs_data_t* data, obs_source_t* source)
{
	return new Transform(data, source);
}

void filter::TransformAddon::destroy(void* ptr)
{
	delete reinterpret_cast<Transform*>(ptr);
}

uint32_t filter::TransformAddon::get_width(void* ptr)
{
	return reinterpret_cast<Transform*>(ptr)->get_width();
}

uint32_t filter::TransformAddon::get_height(void* ptr)
{
	return reinterpret_cast<Transform*>(ptr)->get_height();
}

void filter::TransformAddon::update(void* ptr, obs_data_t* data)
{
	reinterpret_cast<Transform*>(ptr)->update(data);
}

void filter::TransformAddon::activate(void* ptr)
{
	reinterpret_cast<Transform*>(ptr)->activate();
}

void filter::TransformAddon::deactivate(void* ptr)
{
	reinterpret_cast<Transform*>(ptr)->deactivate();
}

void filter::TransformAddon::video_tick(void* ptr, float time)
{
	reinterpret_cast<Transform*>(ptr)->video_tick(time);
}

void filter::TransformAddon::video_render(void* ptr, gs_effect_t* effect)
{
	reinterpret_cast<Transform*>(ptr)->video_render(effect);
}

filter::Transform::~Transform()
{
	obs_enter_graphics();
	shape_rt.reset();
	source_rt.reset();
	vertex_buffer.reset();
	obs_leave_graphics();
}

filter::Transform::Transform(obs_data_t* data, obs_source_t* context)
	: source_context(context), is_orthographic(true), field_of_view(90.0), is_inactive(false), is_hidden(false),
	  is_mesh_update_required(false), rotation_order(RotationOrder::ZXY)
{
	position = std::make_unique<util::vec3a>();
	rotation = std::make_unique<util::vec3a>();
	scale    = std::make_unique<util::vec3a>();
	shear    = std::make_unique<util::vec3a>();

	vec3_set(position.get(), 0, 0, 0);
	vec3_set(rotation.get(), 0, 0, 0);
	vec3_set(scale.get(), 1, 1, 1);

	enable_mipmapping  = false;
	generator          = gs::mipmapper::generator::Linear;
	generator_strength = 50.0;

	obs_enter_graphics();
	source_rt     = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
	shape_rt      = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
	vertex_buffer = std::make_shared<gs::vertex_buffer>(4u, 1u);
	obs_leave_graphics();

	update(data);
}

void filter::Transform::update(obs_data_t* data)
{
	// Camera
	is_orthographic = obs_data_get_int(data, ST_CAMERA) == 0;
	field_of_view   = (float)obs_data_get_double(data, ST_CAMERA_FIELDOFVIEW);

	// Source
	position->x    = (float)obs_data_get_double(data, ST_POSITION_X) / 100.0f;
	position->y    = (float)obs_data_get_double(data, ST_POSITION_Y) / 100.0f;
	position->z    = (float)obs_data_get_double(data, ST_POSITION_Z) / 100.0f;
	scale->x       = (float)obs_data_get_double(data, ST_SCALE_X) / 100.0f;
	scale->y       = (float)obs_data_get_double(data, ST_SCALE_Y) / 100.0f;
	scale->z       = 1.0f;
	rotation_order = (int)obs_data_get_int(data, ST_ROTATION_ORDER);
	rotation->x    = (float)(obs_data_get_double(data, ST_ROTATION_X) / 180.0f * PI);
	rotation->y    = (float)(obs_data_get_double(data, ST_ROTATION_Y) / 180.0f * PI);
	rotation->z    = (float)(obs_data_get_double(data, ST_ROTATION_Z) / 180.0f * PI);
	shear->x       = (float)obs_data_get_double(data, ST_SHEAR_X) / 100.0f;
	shear->y       = (float)obs_data_get_double(data, ST_SHEAR_Y) / 100.0f;
	shear->z       = 0.0f;

	// Mipmapping
	enable_mipmapping  = obs_data_get_bool(data, ST_MIPMAPPING);
	generator_strength = obs_data_get_double(data, strings::MipGenerator::Strength);
	generator          = (gs::mipmapper::generator)obs_data_get_int(data, strings::MipGenerator::Name);

	is_mesh_update_required = true;
}

uint32_t filter::Transform::get_width()
{
	return 0;
}

uint32_t filter::Transform::get_height()
{
	return 0;
}

void filter::Transform::activate()
{
	is_inactive = false;
}

void filter::Transform::deactivate()
{
	is_inactive = true;
}

void filter::Transform::video_tick(float) {}

void filter::Transform::video_render(gs_effect_t* paramEffect)
{
	std::shared_ptr<gs::texture> source_tex;
	std::shared_ptr<gs::texture> shape_tex;

	obs_source_t* parent = obs_filter_get_parent(source_context);
	obs_source_t* target = obs_filter_get_target(source_context);

	uint32_t width       = obs_source_get_base_width(target);
	uint32_t height      = obs_source_get_base_height(target);
	uint32_t real_width  = width;
	uint32_t real_height = height;

	gs_effect_t* default_effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);

	// Skip rendering if our target, parent or context is not valid.
	if (!target || !parent || !source_context || !width || !height || is_inactive || is_hidden) {
		obs_source_skip_video_filter(source_context);
		return;
	}

	// Update Mesh
	if (is_mesh_update_required) {
		float_t aspectRatioX = float_t(width) / float_t(height);
		if (is_orthographic)
			aspectRatioX = 1.0;

		// Mesh
		matrix4 ident;
		matrix4_identity(&ident);
		switch (rotation_order) {
		case RotationOrder::XYZ: // XYZ
			matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, rotation->x);
			matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, rotation->y);
			matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, rotation->z);
			break;
		case RotationOrder::XZY: // XZY
			matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, rotation->x);
			matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, rotation->z);
			matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, rotation->y);
			break;
		case RotationOrder::YXZ: // YXZ
			matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, rotation->y);
			matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, rotation->x);
			matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, rotation->z);
			break;
		case RotationOrder::YZX: // YZX
			matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, rotation->y);
			matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, rotation->z);
			matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, rotation->x);
			break;
		case RotationOrder::ZXY: // ZXY
			matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, rotation->z);
			matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, rotation->x);
			matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, rotation->y);
			break;
		case RotationOrder::ZYX: // ZYX
			matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, rotation->z);
			matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, rotation->y);
			matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, rotation->x);
			break;
		}
		matrix4_translate3f(&ident, &ident, position->x, position->y, position->z);

		/// Calculate vertex position once only.
		float_t p_x = aspectRatioX * scale->x;
		float_t p_y = 1.0f * scale->y;

		/// Generate mesh
		{
			auto vtx   = vertex_buffer->at(0);
			*vtx.color = 0xFFFFFFFF;
			vec4_set(vtx.uv[0], 0, 0, 0, 0);
			vec3_set(vtx.position, -p_x + shear->x, -p_y - shear->y, 0);
			vec3_transform(vtx.position, vtx.position, &ident);
		}
		{
			auto vtx   = vertex_buffer->at(1);
			*vtx.color = 0xFFFFFFFF;
			vec4_set(vtx.uv[0], 1, 0, 0, 0);
			vec3_set(vtx.position, p_x + shear->x, -p_y + shear->y, 0);
			vec3_transform(vtx.position, vtx.position, &ident);
		}
		{
			auto vtx   = vertex_buffer->at(2);
			*vtx.color = 0xFFFFFFFF;
			vec4_set(vtx.uv[0], 0, 1, 0, 0);
			vec3_set(vtx.position, -p_x - shear->x, p_y - shear->y, 0);
			vec3_transform(vtx.position, vtx.position, &ident);
		}
		{
			auto vtx   = vertex_buffer->at(3);
			*vtx.color = 0xFFFFFFFF;
			vec4_set(vtx.uv[0], 1, 1, 0, 0);
			vec3_set(vtx.position, p_x - shear->x, p_y + shear->y, 0);
			vec3_transform(vtx.position, vtx.position, &ident);
		}

		vertex_buffer->update(true);
		is_mesh_update_required = false;
	}

	// Make texture a power of two compatible texture if mipmapping is enabled.
	if (enable_mipmapping) {
		real_width  = uint32_t(pow(2, util::math::get_power_of_two_exponent_ceil(width)));
		real_height = uint32_t(pow(2, util::math::get_power_of_two_exponent_ceil(height)));
		if ((real_width >= 8192) || (real_height >= 8192)) {
			// Most GPUs cap out here, so let's not go higher.
			double_t aspect = double_t(width) / double_t(height);
			if (aspect > 1.0) { // height < width
				real_width  = 8192;
				real_height = uint32_t(real_width / aspect);
			} else if (aspect < 1.0) { // width > height
				real_height = 8192;
				real_width  = uint32_t(real_height * aspect);
			}
		}
	}

	// Draw previous filters to texture.
	try {
		auto op = source_rt->render(real_width, real_height);

		gs_set_cull_mode(GS_NEITHER);
		gs_reset_blend_state();
		gs_blend_function_separate(gs_blend_type::GS_BLEND_ONE, gs_blend_type::GS_BLEND_ZERO,
								   gs_blend_type::GS_BLEND_ONE, gs_blend_type::GS_BLEND_ZERO);
		gs_enable_depth_test(false);
		gs_enable_stencil_test(false);
		gs_enable_stencil_write(false);
		gs_enable_color(true, true, true, true);
		gs_ortho(0, (float)width, 0, (float)height, -1, 1);

		vec4 black;
		vec4_zero(&black);
		gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &black, 0, 0);

		/// Render original source
		if (obs_source_process_filter_begin(source_context, GS_RGBA, OBS_NO_DIRECT_RENDERING)) {
			obs_source_process_filter_end(source_context, paramEffect ? paramEffect : default_effect, width, height);
		} else {
			obs_source_skip_video_filter(source_context);
		}
	} catch (...) {
		obs_source_skip_video_filter(source_context);
		return;
	}
	source_rt->get_texture(source_tex);

	if (enable_mipmapping) {
		if ((!source_texture) || (source_texture->get_width() != real_width)
			|| (source_texture->get_height() != real_height)) {
			size_t mip_levels = 0;
			if (util::math::is_power_of_two(real_width) && util::math::is_power_of_two(real_height)) {
				size_t w_level = util::math::get_power_of_two_exponent_ceil(real_width);
				size_t h_level = util::math::get_power_of_two_exponent_ceil(real_height);
				if (h_level > w_level) {
					mip_levels = h_level;
				} else {
					mip_levels = w_level;
				}
			}

			source_texture = std::make_shared<gs::texture>(real_width, real_height, GS_RGBA, uint32_t(1u + mip_levels),
														   nullptr, gs::texture::flags::BuildMipMaps);
		}

		mipmapper.rebuild(source_tex, source_texture, generator, float_t(generator_strength));
	}

	// Draw shape to texture
	try {
		auto op = shape_rt->render(width, height);

		if (is_orthographic) {
			gs_ortho(-1.0, 1.0, -1.0, 1.0, -farZ, farZ);
		} else {
			gs_perspective(field_of_view, float_t(width) / float_t(height), nearZ, farZ);
			// Fix camera pointing at -Z instead of +Z.
			gs_matrix_scale3f(1.0, 1.0, -1.0);
			// Move backwards so we can actually see stuff.
			gs_matrix_translate3f(0, 0, 1.0);
		}

		// Rendering
		vec4 black;
		vec4_zero(&black);
		gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &black, farZ, 0);
		gs_set_cull_mode(GS_NEITHER);
		gs_enable_blending(false);
		gs_enable_depth_test(false);
		gs_depth_function(gs_depth_test::GS_ALWAYS);
		gs_enable_stencil_test(false);
		gs_enable_stencil_write(false);
		gs_enable_color(true, true, true, true);
		gs_load_vertexbuffer(vertex_buffer->update(false));
		gs_load_indexbuffer(nullptr);
		while (gs_effect_loop(default_effect, "Draw")) {
			gs_effect_set_texture(gs_effect_get_param_by_name(default_effect, "image"),
								  enable_mipmapping ? source_texture->get_object() : source_tex->get_object());
			gs_draw(GS_TRISTRIP, 0, 4);
		}
		gs_load_vertexbuffer(nullptr);
	} catch (...) {
		obs_source_skip_video_filter(source_context);
		return;
	}
	shape_rt->get_texture(shape_tex);

	// Draw final shape
	gs_reset_blend_state();
	gs_enable_depth_test(false);
	while (gs_effect_loop(default_effect, "Draw")) {
		gs_effect_set_texture(gs_effect_get_param_by_name(default_effect, "image"), shape_tex->get_object());
		gs_draw_sprite(shape_tex->get_object(), 0, 0, 0);
	}
}
