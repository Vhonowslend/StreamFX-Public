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

#include "filter-transform.h"
#include "strings.h"

extern "C" {
#pragma warning (push)
#pragma warning (disable: 4201)
#include "libobs/util/platform.h"
#include "libobs/graphics/graphics.h"
#include "libobs/graphics/matrix4.h"
#pragma warning (pop)
}

static const float PI = 3.1415926535897932384626433832795f;
static const float farZ = 2097152.0f; // 2 pow 21
static const float nearZ = 1.0f / farZ;
static const float valueLimit = 65536.0f;

enum RotationOrder : int64_t {
	XYZ,
	XZY,
	YXZ,
	YZX,
	ZXY,
	ZYX,
};

Filter::Transform::Transform() {
	memset(&sourceInfo, 0, sizeof(obs_source_info));
	sourceInfo.id = "obs-stream-effects-filter-transform";
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
	sourceInfo.show = show;
	sourceInfo.hide = hide;
	sourceInfo.video_tick = video_tick;
	sourceInfo.video_render = video_render;

	obs_register_source(&sourceInfo);
}

Filter::Transform::~Transform() {}

const char* Filter::Transform::get_name(void *) {
	return P_TRANSLATE(S_FILTER_TRANSFORM);
}

void Filter::Transform::get_defaults(obs_data_t *data) {
	obs_data_set_default_int(data, S_FILTER_TRANSFORM_CAMERA, 0);
	obs_data_set_default_double(data, S_FILTER_TRANSFORM_CAMERA_FIELDOFVIEW,
		90.0);
	obs_data_set_default_double(data, S_FILTER_TRANSFORM_POSITION_X, 0);
	obs_data_set_default_double(data, S_FILTER_TRANSFORM_POSITION_Y, 0);
	obs_data_set_default_double(data, S_FILTER_TRANSFORM_POSITION_Z, 0);
	obs_data_set_default_double(data, S_FILTER_TRANSFORM_SCALE_X, 100);
	obs_data_set_default_double(data, S_FILTER_TRANSFORM_SCALE_Y, 100);
	obs_data_set_default_int(data, S_FILTER_TRANSFORM_ROTATION_ORDER,
		RotationOrder::ZXY); //ZXY
	obs_data_set_default_double(data, S_FILTER_TRANSFORM_ROTATION_X, 0);
	obs_data_set_default_double(data, S_FILTER_TRANSFORM_ROTATION_Y, 0);
	obs_data_set_default_double(data, S_FILTER_TRANSFORM_ROTATION_Z, 0);
}

obs_properties_t * Filter::Transform::get_properties(void *) {
	obs_properties_t *pr = obs_properties_create();
	obs_property_t* p = NULL;

	p = obs_properties_add_list(pr, S_FILTER_TRANSFORM_CAMERA,
		P_TRANSLATE(S_FILTER_TRANSFORM_CAMERA),
		OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p,
		P_TRANSLATE(P_DESC(S_FILTER_TRANSFORM_CAMERA)));
	obs_property_list_add_int(p,
		P_TRANSLATE(S_FILTER_TRANSFORM_CAMERA_ORTHOGRAPHIC), 0);
	obs_property_list_add_int(p,
		P_TRANSLATE(S_FILTER_TRANSFORM_CAMERA_PERSPECTIVE), 1);
	obs_property_set_modified_callback(p, modified_properties);

	p = obs_properties_add_float_slider(pr,
		S_FILTER_TRANSFORM_CAMERA_FIELDOFVIEW,
		P_TRANSLATE(S_FILTER_TRANSFORM_CAMERA_FIELDOFVIEW),
		1.0, 179.0, 0.01);
	obs_property_set_long_description(p,
		P_TRANSLATE(P_DESC(S_FILTER_TRANSFORM_CAMERA_FIELDOFVIEW)));

	// Position, Scale, Rotation
	{
		std::pair<const char*, const char*> entries[] = {
			std::make_pair(S_FILTER_TRANSFORM_POSITION_X,
			P_DESC(S_FILTER_TRANSFORM_POSITION_X)),
			std::make_pair(S_FILTER_TRANSFORM_POSITION_Y,
			P_DESC(S_FILTER_TRANSFORM_POSITION_Y)),
			std::make_pair(S_FILTER_TRANSFORM_POSITION_Z,
			P_DESC(S_FILTER_TRANSFORM_POSITION_Z)),
		};
		for (auto kv : entries) {
			p = obs_properties_add_float(pr, kv.first,
				P_TRANSLATE(kv.first), -10000, 10000, 0.01);
			obs_property_set_long_description(p,
				P_TRANSLATE(kv.second));
		}
	}{
		std::pair<const char*, const char*> entries[] = {
			std::make_pair(S_FILTER_TRANSFORM_ROTATION_X,
			P_DESC(S_FILTER_TRANSFORM_ROTATION_X)),
			std::make_pair(S_FILTER_TRANSFORM_ROTATION_Y,
			P_DESC(S_FILTER_TRANSFORM_ROTATION_Y)),
			std::make_pair(S_FILTER_TRANSFORM_ROTATION_Z,
			P_DESC(S_FILTER_TRANSFORM_ROTATION_Z)),
		};
		for (auto kv : entries) {
			p = obs_properties_add_float_slider(pr, kv.first,
				P_TRANSLATE(kv.first), -180, 180, 0.01);
			obs_property_set_long_description(p,
				P_TRANSLATE(kv.second));
		}
	}{
		std::pair<const char*, const char*> entries[] = {
			std::make_pair(S_FILTER_TRANSFORM_SCALE_X,
			P_DESC(S_FILTER_TRANSFORM_SCALE_X)),
			std::make_pair(S_FILTER_TRANSFORM_SCALE_Y,
			P_DESC(S_FILTER_TRANSFORM_SCALE_Y)),
		};
		for (auto kv : entries) {
			p = obs_properties_add_float_slider(pr, kv.first,
				P_TRANSLATE(kv.first), -1000, 1000, 0.01);
			obs_property_set_long_description(p,
				P_TRANSLATE(kv.second));
		}
	}

	p = obs_properties_add_bool(pr, S_ADVANCED, P_TRANSLATE(S_ADVANCED));
	obs_property_set_modified_callback(p, modified_properties);

	p = obs_properties_add_list(pr, S_FILTER_TRANSFORM_ROTATION_ORDER,
		P_TRANSLATE(S_FILTER_TRANSFORM_ROTATION_ORDER),
		OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p,
		P_TRANSLATE(P_DESC(S_FILTER_TRANSFORM_ROTATION_ORDER)));
	obs_property_list_add_int(p,
		P_TRANSLATE(S_FILTER_TRANSFORM_ROTATION_ORDER_XYZ),
		RotationOrder::XYZ);
	obs_property_list_add_int(p,
		P_TRANSLATE(S_FILTER_TRANSFORM_ROTATION_ORDER_XZY),
		RotationOrder::XZY);
	obs_property_list_add_int(p,
		P_TRANSLATE(S_FILTER_TRANSFORM_ROTATION_ORDER_YXZ),
		RotationOrder::YXZ);
	obs_property_list_add_int(p,
		P_TRANSLATE(S_FILTER_TRANSFORM_ROTATION_ORDER_YZX),
		RotationOrder::YZX);
	obs_property_list_add_int(p,
		P_TRANSLATE(S_FILTER_TRANSFORM_ROTATION_ORDER_ZXY),
		RotationOrder::ZXY);
	obs_property_list_add_int(p,
		P_TRANSLATE(S_FILTER_TRANSFORM_ROTATION_ORDER_ZYX),
		RotationOrder::ZYX);


	return pr;
}

bool Filter::Transform::modified_properties(obs_properties_t *pr,
	obs_property_t *, obs_data_t *d) {
	switch (obs_data_get_int(d, S_FILTER_TRANSFORM_CAMERA)) {
		case 0:
			obs_property_set_visible(obs_properties_get(pr,
				S_FILTER_TRANSFORM_CAMERA_FIELDOFVIEW), false);
			obs_property_set_visible(obs_properties_get(pr,
				S_FILTER_TRANSFORM_POSITION_Z), false);
			break;
		case 1:
			obs_property_set_visible(obs_properties_get(pr,
				S_FILTER_TRANSFORM_CAMERA_FIELDOFVIEW), true);
			obs_property_set_visible(obs_properties_get(pr,
				S_FILTER_TRANSFORM_POSITION_Z), true);
			break;
	}

	bool advancedVisible = obs_data_get_bool(d, S_ADVANCED);
	obs_property_set_visible(obs_properties_get(pr,
		S_FILTER_TRANSFORM_ROTATION_ORDER), advancedVisible);

	return false;
}

void * Filter::Transform::create(obs_data_t *data, obs_source_t *source) {
	return new Instance(data, source);
}

void Filter::Transform::destroy(void *ptr) {
	delete reinterpret_cast<Instance*>(ptr);
}

uint32_t Filter::Transform::get_width(void *ptr) {
	return reinterpret_cast<Instance*>(ptr)->get_width();
}

uint32_t Filter::Transform::get_height(void *ptr) {
	return reinterpret_cast<Instance*>(ptr)->get_height();
}

void Filter::Transform::update(void *ptr, obs_data_t *data) {
	reinterpret_cast<Instance*>(ptr)->update(data);
}

void Filter::Transform::activate(void *ptr) {
	reinterpret_cast<Instance*>(ptr)->activate();
}

void Filter::Transform::deactivate(void *ptr) {
	reinterpret_cast<Instance*>(ptr)->deactivate();
}

void Filter::Transform::show(void *ptr) {
	reinterpret_cast<Instance*>(ptr)->show();
}

void Filter::Transform::hide(void *ptr) {
	reinterpret_cast<Instance*>(ptr)->hide();
}

void Filter::Transform::video_tick(void *ptr, float time) {
	reinterpret_cast<Instance*>(ptr)->video_tick(time);
}

void Filter::Transform::video_render(void *ptr, gs_effect_t *effect) {
	reinterpret_cast<Instance*>(ptr)->video_render(effect);
}

Filter::Transform::Instance::Instance(obs_data_t *data, obs_source_t *context) :
m_sourceContext(context), m_vertexHelper(nullptr), m_vertexBuffer(nullptr),
m_texRender(nullptr), m_shapeRender(nullptr), m_isCameraOrthographic(true),
m_cameraFieldOfView(90.0), m_isInactive(false), m_isHidden(false),
m_isMeshUpdateRequired(false), m_rotationOrder(RotationOrder::ZXY) {
	vec3_set(&m_position, 0, 0, 0);
	vec3_set(&m_rotation, 0, 0, 0);
	vec3_set(&m_scale, 1, 1, 1);

	obs_enter_graphics();
	m_texRender = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	m_shapeRender = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	m_vertexHelper = new Helper::VertexBuffer(4);
	m_vertexHelper->set_uv_layers(1);
	m_vertexHelper->resize(4);
	obs_leave_graphics();

	update(data);
}

Filter::Transform::Instance::~Instance() {
	obs_enter_graphics();
	delete m_vertexHelper;
	gs_texrender_destroy(m_texRender);
	gs_texrender_destroy(m_shapeRender);
	obs_leave_graphics();
}

void Filter::Transform::Instance::update(obs_data_t *data) {
	// Camera
	m_isCameraOrthographic = obs_data_get_int(data,
		S_FILTER_TRANSFORM_CAMERA) == 0;
	m_cameraFieldOfView = (float)obs_data_get_double(data,
		S_FILTER_TRANSFORM_CAMERA_FIELDOFVIEW);

	// Source
	m_position.x = (float)obs_data_get_double(data,
		S_FILTER_TRANSFORM_POSITION_X) / 100.0f;
	m_position.y = (float)obs_data_get_double(data,
		S_FILTER_TRANSFORM_POSITION_Y) / 100.0f;
	m_position.z = (float)obs_data_get_double(data,
		S_FILTER_TRANSFORM_POSITION_Z) / 100.0f;
	m_scale.x = (float)obs_data_get_double(data,
		S_FILTER_TRANSFORM_SCALE_X) / 100.0f;
	m_scale.y = (float)obs_data_get_double(data,
		S_FILTER_TRANSFORM_SCALE_Y) / 100.0f;
	m_scale.z = 1.0;
	m_rotationOrder = (int)obs_data_get_int(data,
		S_FILTER_TRANSFORM_ROTATION_ORDER);
	m_rotation.x = (float)obs_data_get_double(data,
		S_FILTER_TRANSFORM_ROTATION_X) / 180.0f * PI;
	m_rotation.y = (float)obs_data_get_double(data,
		S_FILTER_TRANSFORM_ROTATION_Y) / 180.0f * PI;
	m_rotation.z = (float)obs_data_get_double(data,
		S_FILTER_TRANSFORM_ROTATION_Z) / 180.0f * PI;
	m_isMeshUpdateRequired = true;
}

uint32_t Filter::Transform::Instance::get_width() {
	return 0;
}

uint32_t Filter::Transform::Instance::get_height() {
	return 0;
}

void Filter::Transform::Instance::activate() {
	m_isInactive = false;
}

void Filter::Transform::Instance::deactivate() {
	m_isInactive = true;
}

void Filter::Transform::Instance::show() {
	m_isHidden = false;
}

void Filter::Transform::Instance::hide() {
	m_isHidden = true;
}

void Filter::Transform::Instance::video_tick(float) {}

void Filter::Transform::Instance::video_render(gs_effect_t *paramEffect) {
	obs_source_t *parent = obs_filter_get_parent(m_sourceContext);
	obs_source_t *target = obs_filter_get_target(m_sourceContext);
	uint32_t
		baseW = obs_source_get_base_width(target),
		baseH = obs_source_get_base_height(target);

	// Skip rendering if our target, parent or context is not valid.
	if (!target || !parent || !m_sourceContext
		|| !baseW || !baseH
		|| !m_texRender || !m_shapeRender
		|| m_isInactive || m_isHidden) {
		obs_source_skip_video_filter(m_sourceContext);
		return;
	}

	gs_effect_t *alphaEffect = obs_get_base_effect(OBS_EFFECT_DEFAULT);

	// Draw previous filters to texture.
	gs_texrender_reset(m_texRender);
	if (!gs_texrender_begin(m_texRender, baseW, baseH)) {
		obs_source_skip_video_filter(m_sourceContext);
		return;
	}
	gs_ortho(0, (float)baseW, 0, (float)baseH, -1, 1);

	/// Set up the Scene
	vec4 black;
	vec4_zero(&black);
	gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &black, 0, 0);
	gs_set_cull_mode(GS_NEITHER);
	gs_reset_blend_state();
	gs_blend_function_separate(
		gs_blend_type::GS_BLEND_ONE,
		gs_blend_type::GS_BLEND_ZERO,
		gs_blend_type::GS_BLEND_ONE,
		gs_blend_type::GS_BLEND_ZERO);
	gs_enable_depth_test(false);
	gs_enable_stencil_test(false);
	gs_enable_stencil_write(false);
	gs_enable_color(true, true, true, true);

	/// Render original source
	if (obs_source_process_filter_begin(m_sourceContext, GS_RGBA,
		OBS_NO_DIRECT_RENDERING)) {
		obs_source_process_filter_end(m_sourceContext,
			paramEffect ? paramEffect : alphaEffect,
			baseW, baseH);
	} else {
		obs_source_skip_video_filter(m_sourceContext);
	}

	gs_texrender_end(m_texRender);
	gs_texture* filterTexture = gs_texrender_get_texture(m_texRender);

	// Update Mesh
	if (m_isMeshUpdateRequired) {
		double_t aspectRatioX = double_t(baseW) / double_t(baseH),
			aspectRatioY = double_t(baseH) / double_t(baseW);
		if (baseW > baseH) {
			aspectRatioY = 1.0;
		} else {
			aspectRatioX = 1.0;
		}
		if (m_isCameraOrthographic)
			aspectRatioX = aspectRatioY = 1.0;

		// Mesh
		matrix4 ident;
		matrix4_identity(&ident);
		matrix4_scale3f(&ident, &ident, (float)aspectRatioX, (float)aspectRatioY, 1.0);
		matrix4_scale(&ident, &ident, &m_scale);
		switch (m_rotationOrder) {
			case RotationOrder::XYZ: // XYZ
				matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, m_rotation.x);
				matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, m_rotation.y);
				matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, m_rotation.z);
				break;
			case RotationOrder::XZY: // XZY
				matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, m_rotation.x);
				matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, m_rotation.z);
				matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, m_rotation.y);
				break;
			case RotationOrder::YXZ: // YXZ
				matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, m_rotation.y);
				matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, m_rotation.x);
				matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, m_rotation.z);
				break;
			case RotationOrder::YZX: // YZX
				matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, m_rotation.y);
				matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, m_rotation.z);
				matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, m_rotation.x);
				break;
			case RotationOrder::ZXY: // ZXY
				matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, m_rotation.z);
				matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, m_rotation.x);
				matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, m_rotation.y);
				break;
			case RotationOrder::ZYX: // ZYX
				matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, m_rotation.z);
				matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, m_rotation.y);
				matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, m_rotation.x);
				break;
		}
		matrix4_translate3f(&ident, &ident, m_position.x, m_position.y, m_position.z);

		{
			Helper::Vertex& v = m_vertexHelper->at(0);
			v.uv[0].x = 0; v.uv[0].y = 0;
			v.color = 0xFFFFFFFF;
			v.position.x = -1.0;
			v.position.y = -1.0;
			v.position.z = 0.0f;
			vec3_transform(&v.position, &v.position, &ident);
		}
		{
			Helper::Vertex& v = m_vertexHelper->at(1);
			v.uv[0].x = 1; v.uv[0].y = 0;
			v.color = 0xFFFFFFFF;
			v.position.x = 1.0;
			v.position.y = -1.0;
			v.position.z = 0.0f;
			vec3_transform(&v.position, &v.position, &ident);
		}
		{
			Helper::Vertex& v = m_vertexHelper->at(2);
			v.uv[0].x = 0; v.uv[0].y = 1;
			v.color = 0xFFFFFFFF;
			v.position.x = -1.0;
			v.position.y = 1.0;
			v.position.z = 0.0f;
			vec3_transform(&v.position, &v.position, &ident);
		}
		{
			Helper::Vertex& v = m_vertexHelper->at(3);
			v.uv[0].x = 1; v.uv[0].y = 1;
			v.color = 0xFFFFFFFF;
			v.position.x = 1.0;
			v.position.y = 1.0;
			v.position.z = 0.0f;
			vec3_transform(&v.position, &v.position, &ident);
		}
		m_vertexBuffer = m_vertexHelper->update();
		if (!m_vertexBuffer) {
			obs_source_skip_video_filter(m_sourceContext);
			return;
		}
		m_isMeshUpdateRequired = false;
	}

	// Draw shape to texture
	gs_texrender_reset(m_shapeRender);
	if (gs_texrender_begin(m_shapeRender, baseW, baseH)) {
		if (m_isCameraOrthographic) {
			gs_ortho(-1.0, 1.0, -1.0, 1.0, -farZ, farZ);
		} else {
			gs_perspective(m_cameraFieldOfView,
				float_t(baseW) / float_t(baseH), nearZ, farZ);
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
		while (gs_effect_loop(alphaEffect, "Draw")) {
			gs_effect_set_texture(
				gs_effect_get_param_by_name(alphaEffect,
				"image"), filterTexture);
			gs_load_vertexbuffer(m_vertexBuffer);
			gs_load_indexbuffer(NULL);
			gs_draw(GS_TRISTRIP, 0, 4);
		}

		gs_texrender_end(m_shapeRender);
	} else {
		obs_source_skip_video_filter(m_sourceContext);
		return;
	}
	gs_texture* shapeTexture = gs_texrender_get_texture(m_shapeRender);

	// Draw final shape
	gs_reset_blend_state();
	gs_enable_depth_test(false);
	while (gs_effect_loop(alphaEffect, "Draw")) {
		gs_effect_set_texture(gs_effect_get_param_by_name(alphaEffect,
			"image"), shapeTexture);
		gs_draw_sprite(shapeTexture, 0, 0, 0);
	}
}
