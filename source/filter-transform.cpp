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

static const float farZ  = 2097152.0f; // 2 pow 21
static const float nearZ = 1.0f / farZ;

enum class CameraMode : int32_t { Orthographic, Perspective };

enum RotationOrder : int64_t {
	XYZ,
	XZY,
	YXZ,
	YZX,
	ZXY,
	ZYX,
};

// Initializer & Finalizer
INITIALIZER(FilterTransformInit)
{
	initializerFunctions.push_back([] { filter::transform::transform_factory::initialize(); });
	finalizerFunctions.push_back([] { filter::transform::transform_factory::finalize(); });
}

static std::shared_ptr<filter::transform::transform_factory> factory_instance = nullptr;

void filter::transform::transform_factory::initialize()
{
	factory_instance = std::make_shared<filter::transform::transform_factory>();
}

void filter::transform::transform_factory::finalize()
{
	factory_instance.reset();
}

std::shared_ptr<filter::transform::transform_factory> filter::transform::transform_factory::get()
{
	return factory_instance;
}

filter::transform::transform_factory::transform_factory()
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

filter::transform::transform_factory::~transform_factory() {}

const char* filter::transform::transform_factory::get_name(void*)
{
	return P_TRANSLATE(ST);
}

void filter::transform::transform_factory::get_defaults(obs_data_t* data)
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
	obs_data_set_default_int(data, ST_ROTATION_ORDER, RotationOrder::ZXY);
}

obs_properties_t* filter::transform::transform_factory::get_properties(void*)
{
	obs_properties_t* pr = obs_properties_create();
	obs_property_t*   p  = NULL;

	// Camera
	/// Projection Mode
	p = obs_properties_add_list(pr, ST_CAMERA, P_TRANSLATE(ST_CAMERA), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(ST_CAMERA)));
	obs_property_list_add_int(p, P_TRANSLATE(ST_CAMERA_ORTHOGRAPHIC), (int64_t)CameraMode::Orthographic);
	obs_property_list_add_int(p, P_TRANSLATE(ST_CAMERA_PERSPECTIVE), (int64_t)CameraMode::Perspective);
	obs_property_set_modified_callback(p, modified_properties);
	/// Field Of View
	p = obs_properties_add_float_slider(pr, ST_CAMERA_FIELDOFVIEW, P_TRANSLATE(ST_CAMERA_FIELDOFVIEW), 1.0, 179.0,
										0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(ST_CAMERA_FIELDOFVIEW)));

	// Mesh
	/// Position
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
	/// Rotation
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
	/// Scale
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
	/// Shear
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

	p = obs_properties_add_list(pr, S_MIPGENERATOR, P_TRANSLATE(S_MIPGENERATOR), OBS_COMBO_TYPE_LIST,
								OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(S_MIPGENERATOR)));

	obs_property_list_add_int(p, P_TRANSLATE(S_MIPGENERATOR_POINT), (long long)gs::mipmapper::generator::Point);
	obs_property_list_add_int(p, P_TRANSLATE(S_MIPGENERATOR_LINEAR), (long long)gs::mipmapper::generator::Linear);
	obs_property_list_add_int(p, P_TRANSLATE(S_MIPGENERATOR_SHARPEN), (long long)gs::mipmapper::generator::Sharpen);
	obs_property_list_add_int(p, P_TRANSLATE(S_MIPGENERATOR_SMOOTHEN), (long long)gs::mipmapper::generator::Smoothen);
	obs_property_list_add_int(p, P_TRANSLATE(S_MIPGENERATOR_BICUBIC), (long long)gs::mipmapper::generator::Bicubic);
	obs_property_list_add_int(p, P_TRANSLATE(S_MIPGENERATOR_LANCZOS), (long long)gs::mipmapper::generator::Lanczos);

	p = obs_properties_add_float_slider(pr, S_MIPGENERATOR_INTENSITY, P_TRANSLATE(S_MIPGENERATOR_INTENSITY), 0.0,
										1000.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(S_MIPGENERATOR_INTENSITY)));

	return pr;
}

bool filter::transform::transform_factory::modified_properties(obs_properties_t* pr, obs_property_t*, obs_data_t* d)
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
	obs_property_set_visible(obs_properties_get(pr, S_MIPGENERATOR), mipmappingVisible);
	obs_property_set_visible(obs_properties_get(pr, S_MIPGENERATOR_INTENSITY), mipmappingVisible);

	return true;
}

void* filter::transform::transform_factory::create(obs_data_t* data, obs_source_t* source)
{
	return new transform_instance(data, source);
}

void filter::transform::transform_factory::destroy(void* ptr)
{
	delete reinterpret_cast<transform_instance*>(ptr);
}

uint32_t filter::transform::transform_factory::get_width(void* ptr)
{
	return reinterpret_cast<transform_instance*>(ptr)->get_width();
}

uint32_t filter::transform::transform_factory::get_height(void* ptr)
{
	return reinterpret_cast<transform_instance*>(ptr)->get_height();
}

void filter::transform::transform_factory::update(void* ptr, obs_data_t* data)
{
	reinterpret_cast<transform_instance*>(ptr)->update(data);
}

void filter::transform::transform_factory::activate(void* ptr)
{
	reinterpret_cast<transform_instance*>(ptr)->activate();
}

void filter::transform::transform_factory::deactivate(void* ptr)
{
	reinterpret_cast<transform_instance*>(ptr)->deactivate();
}

void filter::transform::transform_factory::video_tick(void* ptr, float time)
{
	reinterpret_cast<transform_instance*>(ptr)->video_tick(time);
}

void filter::transform::transform_factory::video_render(void* ptr, gs_effect_t* effect)
{
	reinterpret_cast<transform_instance*>(ptr)->video_render(effect);
}

filter::transform::transform_instance::~transform_instance()
{
	m_shear.reset();
	m_scale.reset();
	m_rotation.reset();
	m_position.reset();
	m_vertex_buffer.reset();
	m_shape_texture.reset();
	m_shape_rendertarget.reset();
	m_source_texture.reset();
	m_source_rendertarget.reset();
}

filter::transform::transform_instance::transform_instance(obs_data_t* data, obs_source_t* context)
	: m_active(true), m_self(context), m_source_rendered(false), m_mipmap_enabled(false), m_mipmap_strength(50.0),
	  m_mipmap_generator(gs::mipmapper::generator::Linear), m_update_mesh(false), m_rotation_order(RotationOrder::ZXY),
	  m_camera_orthographic(true), m_camera_fov(90.0)
{
	m_source_rendertarget = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
	m_shape_rendertarget  = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
	m_vertex_buffer       = std::make_shared<gs::vertex_buffer>(4u, 1u);

	m_position = std::make_unique<util::vec3a>();
	m_rotation = std::make_unique<util::vec3a>();
	m_scale    = std::make_unique<util::vec3a>();
	m_shear    = std::make_unique<util::vec3a>();

	vec3_set(m_position.get(), 0, 0, 0);
	vec3_set(m_rotation.get(), 0, 0, 0);
	vec3_set(m_scale.get(), 1, 1, 1);

	update(data);
}

uint32_t filter::transform::transform_instance::get_width()
{
	return 0;
}

uint32_t filter::transform::transform_instance::get_height()
{
	return 0;
}

void filter::transform::transform_instance::update(obs_data_t* data)
{
	// Camera
	m_camera_orthographic = obs_data_get_int(data, ST_CAMERA) == 0;
	m_camera_fov          = (float)obs_data_get_double(data, ST_CAMERA_FIELDOFVIEW);

	// Source
	m_position->x    = (float)obs_data_get_double(data, ST_POSITION_X) / 100.0f;
	m_position->y    = (float)obs_data_get_double(data, ST_POSITION_Y) / 100.0f;
	m_position->z    = (float)obs_data_get_double(data, ST_POSITION_Z) / 100.0f;
	m_scale->x       = (float)obs_data_get_double(data, ST_SCALE_X) / 100.0f;
	m_scale->y       = (float)obs_data_get_double(data, ST_SCALE_Y) / 100.0f;
	m_scale->z       = 1.0f;
	m_rotation_order = (int)obs_data_get_int(data, ST_ROTATION_ORDER);
	m_rotation->x    = (float)(obs_data_get_double(data, ST_ROTATION_X) / 180.0f * PI);
	m_rotation->y    = (float)(obs_data_get_double(data, ST_ROTATION_Y) / 180.0f * PI);
	m_rotation->z    = (float)(obs_data_get_double(data, ST_ROTATION_Z) / 180.0f * PI);
	m_shear->x       = (float)obs_data_get_double(data, ST_SHEAR_X) / 100.0f;
	m_shear->y       = (float)obs_data_get_double(data, ST_SHEAR_Y) / 100.0f;
	m_shear->z       = 0.0f;

	// Mipmapping
	m_mipmap_enabled   = obs_data_get_bool(data, ST_MIPMAPPING);
	m_mipmap_strength  = obs_data_get_double(data, S_MIPGENERATOR_INTENSITY);
	m_mipmap_generator = (gs::mipmapper::generator)obs_data_get_int(data, S_MIPGENERATOR);

	m_update_mesh = true;
}

void filter::transform::transform_instance::activate()
{
	m_active = true;
}

void filter::transform::transform_instance::deactivate()
{
	m_active = false;
}

void filter::transform::transform_instance::video_tick(float)
{
	// Update Mesh
	if (m_update_mesh) {
		uint32_t width  = 0;
		uint32_t height = 0;

		// Grab parent and target.
		obs_source_t* target = obs_filter_get_target(m_self);
		if (target) {
			// Grab width an height of the target source (child filter or source).
			width  = obs_source_get_base_width(target);
			height = obs_source_get_base_height(target);
		}
		if (width == 0) {
			width = 1;
		}
		if (height == 0) {
			height = 1;
		}

		// Calculate Aspect Ratio
		float_t aspectRatioX = float_t(width) / float_t(height);
		if (m_camera_orthographic)
			aspectRatioX = 1.0;

		// Mesh
		matrix4 ident;
		matrix4_identity(&ident);
		switch (m_rotation_order) {
		case RotationOrder::XYZ: // XYZ
			matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, m_rotation->x);
			matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, m_rotation->y);
			matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, m_rotation->z);
			break;
		case RotationOrder::XZY: // XZY
			matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, m_rotation->x);
			matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, m_rotation->z);
			matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, m_rotation->y);
			break;
		case RotationOrder::YXZ: // YXZ
			matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, m_rotation->y);
			matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, m_rotation->x);
			matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, m_rotation->z);
			break;
		case RotationOrder::YZX: // YZX
			matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, m_rotation->y);
			matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, m_rotation->z);
			matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, m_rotation->x);
			break;
		case RotationOrder::ZXY: // ZXY
			matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, m_rotation->z);
			matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, m_rotation->x);
			matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, m_rotation->y);
			break;
		case RotationOrder::ZYX: // ZYX
			matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, m_rotation->z);
			matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, m_rotation->y);
			matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, m_rotation->x);
			break;
		}
		matrix4_translate3f(&ident, &ident, m_position->x, m_position->y, m_position->z);

		/// Calculate vertex position once only.
		float_t p_x = aspectRatioX * m_scale->x;
		float_t p_y = 1.0f * m_scale->y;

		/// Generate mesh
		{
			auto vtx   = m_vertex_buffer->at(0);
			*vtx.color = 0xFFFFFFFF;
			vec4_set(vtx.uv[0], 0, 0, 0, 0);
			vec3_set(vtx.position, -p_x + m_shear->x, -p_y - m_shear->y, 0);
			vec3_transform(vtx.position, vtx.position, &ident);
		}
		{
			auto vtx   = m_vertex_buffer->at(1);
			*vtx.color = 0xFFFFFFFF;
			vec4_set(vtx.uv[0], 1, 0, 0, 0);
			vec3_set(vtx.position, p_x + m_shear->x, -p_y + m_shear->y, 0);
			vec3_transform(vtx.position, vtx.position, &ident);
		}
		{
			auto vtx   = m_vertex_buffer->at(2);
			*vtx.color = 0xFFFFFFFF;
			vec4_set(vtx.uv[0], 0, 1, 0, 0);
			vec3_set(vtx.position, -p_x - m_shear->x, p_y - m_shear->y, 0);
			vec3_transform(vtx.position, vtx.position, &ident);
		}
		{
			auto vtx   = m_vertex_buffer->at(3);
			*vtx.color = 0xFFFFFFFF;
			vec4_set(vtx.uv[0], 1, 1, 0, 0);
			vec3_set(vtx.position, p_x - m_shear->x, p_y + m_shear->y, 0);
			vec3_transform(vtx.position, vtx.position, &ident);
		}

		m_vertex_buffer->update(true);
		m_update_mesh = false;
	}

	this->m_source_rendered = false;
}

void filter::transform::transform_instance::video_render(gs_effect_t* paramEffect)
{
	if (!m_active) {
		obs_source_skip_video_filter(m_self);
		return;
	}

	// Grab parent and target.
	obs_source_t* parent = obs_filter_get_parent(m_self);
	obs_source_t* target = obs_filter_get_target(m_self);
	if (!parent || !target) {
		obs_source_skip_video_filter(m_self);
		return;
	}

	// Grab width an height of the target source (child filter or source).
	uint32_t width  = obs_source_get_base_width(target);
	uint32_t height = obs_source_get_base_height(target);
	if ((width == 0) || (height == 0)) {
		obs_source_skip_video_filter(m_self);
		return;
	}

	gs_effect_t* default_effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);

	// Only render if we didn't already render.
	if (!this->m_source_rendered) {
		std::shared_ptr<gs::texture> source_tex;
		uint32_t                     real_width  = width;
		uint32_t                     real_height = height;

		// If MipMapping is enabled, resize Render Target to be a Power of Two.
		if (m_mipmap_enabled) {
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
			auto op = m_source_rendertarget->render(real_width, real_height);

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
			if (obs_source_process_filter_begin(m_self, GS_RGBA, OBS_NO_DIRECT_RENDERING)) {
				obs_source_process_filter_end(m_self, paramEffect ? paramEffect : default_effect, width, height);
			} else {
				obs_source_skip_video_filter(m_self);
			}
		} catch (...) {
			obs_source_skip_video_filter(m_self);
			return;
		}
		m_source_rendertarget->get_texture(source_tex);

		if (m_mipmap_enabled) {
			if ((!m_source_texture) || (m_source_texture->get_width() != real_width)
				|| (m_source_texture->get_height() != real_height)) {
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

				m_source_texture =
					std::make_shared<gs::texture>(real_width, real_height, GS_RGBA, uint32_t(1u + mip_levels), nullptr,
												  gs::texture::flags::BuildMipMaps);
			}

			m_mipmapper.rebuild(source_tex, m_source_texture, m_mipmap_generator, float_t(m_mipmap_strength));
		}

		// Draw shape to texture
		try {
			auto op = m_shape_rendertarget->render(width, height);

			if (m_camera_orthographic) {
				gs_ortho(-1.0, 1.0, -1.0, 1.0, -farZ, farZ);
			} else {
				gs_perspective(m_camera_fov, float_t(width) / float_t(height), nearZ, farZ);
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
			gs_load_vertexbuffer(m_vertex_buffer->update(false));
			gs_load_indexbuffer(nullptr);
			while (gs_effect_loop(default_effect, "Draw")) {
				gs_effect_set_texture(gs_effect_get_param_by_name(default_effect, "image"),
									  m_mipmap_enabled ? m_source_texture->get_object() : source_tex->get_object());
				gs_draw(GS_TRISTRIP, 0, 4);
			}
			gs_load_vertexbuffer(nullptr);
		} catch (...) {
			obs_source_skip_video_filter(m_self);
			return;
		}
		m_shape_rendertarget->get_texture(m_shape_texture);

		this->m_source_rendered = true;
	}

	// Draw final shape
	gs_reset_blend_state();
	gs_enable_depth_test(false);
	while (gs_effect_loop(default_effect, "Draw")) {
		gs_effect_set_texture(gs_effect_get_param_by_name(default_effect, "image"), m_shape_texture->get_object());
		gs_draw_sprite(m_shape_texture->get_object(), 0, 0, 0);
	}
}
