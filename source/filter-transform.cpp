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

#include "libobs/util/platform.h"
#include "libobs/graphics/graphics.h"
#include "libobs/graphics/matrix4.h"

static const float PI = 3.1415926535897932384626433832795f;
static const float nearZ = 0.0001f;
static const float farZ = 65536.0f;
static const float valueLimit = 65536.0f;

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

Filter::Transform::~Transform() {

}

const char* Filter::Transform::get_name(void *) {
	return P_TRANSLATE(P_FILTER_TRANSFORM);
}

void Filter::Transform::get_defaults(obs_data_t *data) {
	obs_data_set_default_int(data, P_FILTER_TRANSFORM_CAMERA, 0);
	obs_data_set_default_double(data, P_FILTER_TRANSFORM_CAMERA_FIELDOFVIEW, 90.0);
	obs_data_set_default_double(data, P_FILTER_TRANSFORM_POSITION_X, 0);
	obs_data_set_default_double(data, P_FILTER_TRANSFORM_POSITION_Y, 0);
	obs_data_set_default_double(data, P_FILTER_TRANSFORM_POSITION_Z, -100.0);
	obs_data_set_default_double(data, P_FILTER_TRANSFORM_SCALE_X, 100);
	obs_data_set_default_double(data, P_FILTER_TRANSFORM_SCALE_Y, 100);
	obs_data_set_default_int(data, P_FILTER_TRANSFORM_ROTATION_ORDER, 4); //ZXY
	obs_data_set_default_double(data, P_FILTER_TRANSFORM_ROTATION_X, 0);
	obs_data_set_default_double(data, P_FILTER_TRANSFORM_ROTATION_Y, 0);
	obs_data_set_default_double(data, P_FILTER_TRANSFORM_ROTATION_Z, 0);
}

obs_properties_t * Filter::Transform::get_properties(void *) {
	obs_properties_t *pr = obs_properties_create();
	obs_property_t* p = NULL;

	p = obs_properties_add_list(pr, P_FILTER_TRANSFORM_CAMERA, P_TRANSLATE(P_FILTER_TRANSFORM_CAMERA),
		OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_FILTER_TRANSFORM_CAMERA)));
	obs_property_list_add_int(p, P_TRANSLATE(P_FILTER_TRANSFORM_CAMERA_ORTHOGRAPHIC), 0);
	obs_property_list_add_int(p, P_TRANSLATE(P_FILTER_TRANSFORM_CAMERA_PERSPECTIVE), 1);

	p = obs_properties_add_float_slider(pr, P_FILTER_TRANSFORM_CAMERA_FIELDOFVIEW, P_TRANSLATE(P_FILTER_TRANSFORM_CAMERA_FIELDOFVIEW),
		1.0, 180.0, 0.01);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_FILTER_TRANSFORM_CAMERA_FIELDOFVIEW)));

	{
		std::pair<const char*, const char*> entries[] = {
			std::make_pair(P_FILTER_TRANSFORM_POSITION_X, P_DESC(P_FILTER_TRANSFORM_POSITION_X)),
			std::make_pair(P_FILTER_TRANSFORM_POSITION_Y, P_DESC(P_FILTER_TRANSFORM_POSITION_Y)),
			std::make_pair(P_FILTER_TRANSFORM_POSITION_Z, P_DESC(P_FILTER_TRANSFORM_POSITION_Z)),
			std::make_pair(P_FILTER_TRANSFORM_SCALE_X, P_DESC(P_FILTER_TRANSFORM_SCALE_X)),
			std::make_pair(P_FILTER_TRANSFORM_SCALE_Y, P_DESC(P_FILTER_TRANSFORM_SCALE_Y)),
		};
		for (auto kv : entries) {
			p = obs_properties_add_float(pr, kv.first, P_TRANSLATE(kv.first),
				-valueLimit, valueLimit, 0.01);
			obs_property_set_long_description(p, P_TRANSLATE(kv.second));
		}
	}

	p = obs_properties_add_list(pr, P_FILTER_TRANSFORM_ROTATION_ORDER, P_TRANSLATE(P_FILTER_TRANSFORM_ROTATION_ORDER),
		OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_FILTER_TRANSFORM_ROTATION_ORDER)));
	obs_property_list_add_int(p, P_TRANSLATE(P_FILTER_TRANSFORM_ROTATION_ORDER_XYZ), 0);
	obs_property_list_add_int(p, P_TRANSLATE(P_FILTER_TRANSFORM_ROTATION_ORDER_XZY), 1);
	obs_property_list_add_int(p, P_TRANSLATE(P_FILTER_TRANSFORM_ROTATION_ORDER_YXZ), 2);
	obs_property_list_add_int(p, P_TRANSLATE(P_FILTER_TRANSFORM_ROTATION_ORDER_YZX), 3);
	obs_property_list_add_int(p, P_TRANSLATE(P_FILTER_TRANSFORM_ROTATION_ORDER_ZXY), 4);
	obs_property_list_add_int(p, P_TRANSLATE(P_FILTER_TRANSFORM_ROTATION_ORDER_ZYX), 5);

	{
		std::pair<const char*, const char*> entries[] = {
			std::make_pair(P_FILTER_TRANSFORM_ROTATION_X, P_DESC(P_FILTER_TRANSFORM_ROTATION_X)),
			std::make_pair(P_FILTER_TRANSFORM_ROTATION_Y, P_DESC(P_FILTER_TRANSFORM_ROTATION_Y)),
			std::make_pair(P_FILTER_TRANSFORM_ROTATION_Z, P_DESC(P_FILTER_TRANSFORM_ROTATION_Z)),
		};
		for (auto kv : entries) {
			p = obs_properties_add_float_slider(pr, kv.first, P_TRANSLATE(kv.first),
				-180, 180, 0.01);
			obs_property_set_long_description(p, P_TRANSLATE(kv.second));
		}
	}

	return pr;
}

bool Filter::Transform::modified_properties(obs_properties_t *pr, obs_property_t *, obs_data_t *d) {
	switch (obs_data_get_int(d, P_FILTER_TRANSFORM_CAMERA)) {
		case 0:
			obs_property_set_visible(obs_properties_get(pr, P_FILTER_TRANSFORM_CAMERA_FIELDOFVIEW), false);
			obs_property_set_visible(obs_properties_get(pr, P_FILTER_TRANSFORM_POSITION_Z), false);
			break;
		case 1:
			obs_property_set_visible(obs_properties_get(pr, P_FILTER_TRANSFORM_CAMERA_FIELDOFVIEW), true);
			obs_property_set_visible(obs_properties_get(pr, P_FILTER_TRANSFORM_POSITION_Z), true);
			break;
	}

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

Filter::Transform::Instance::Instance(obs_data_t *data, obs_source_t *context)
	: context(context) {
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
	pos.x = (float)obs_data_get_double(data, P_FILTER_TRANSFORM_POSITION_X) / 100.0f;
	pos.y = (float)obs_data_get_double(data, P_FILTER_TRANSFORM_POSITION_Y) / 100.0f;
	pos.z = (float)obs_data_get_double(data, P_FILTER_TRANSFORM_POSITION_Z) / 100.0f;
	rot.x = (float)obs_data_get_double(data, P_FILTER_TRANSFORM_ROTATION_X) / 180.0f * PI;
	rot.y = (float)obs_data_get_double(data, P_FILTER_TRANSFORM_ROTATION_Y) / 180.0f * PI;
	rot.z = (float)obs_data_get_double(data, P_FILTER_TRANSFORM_ROTATION_Z) / 180.0f * PI;
	scale.x = (float)obs_data_get_double(data, P_FILTER_TRANSFORM_SCALE_X) / 100.0f;
	scale.y = (float)obs_data_get_double(data, P_FILTER_TRANSFORM_SCALE_Y) / 100.0f;
	scale.z = 1.0;
	m_isOrthographic = obs_data_get_int(data, P_FILTER_TRANSFORM_CAMERA) == 0;
	fov = (float)obs_data_get_double(data, P_FILTER_TRANSFORM_CAMERA_FIELDOFVIEW);

	// Matrix
	matrix4 ident;
	matrix4_identity(&ident);
	matrix4_scale(&ident, &ident, &scale);
	switch (obs_data_get_int(data, P_FILTER_TRANSFORM_ROTATION_ORDER)) {
		case 0: // XYZ
			matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, rot.x);
			matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, rot.y);
			matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, rot.z);
			break;
		case 1: // XZY
			matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, rot.x);
			matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, rot.z);
			matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, rot.y);
			break;
		case 2: // YXZ
			matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, rot.y);
			matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, rot.x);
			matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, rot.z);
			break;
		case 3: // YZX
			matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, rot.y);
			matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, rot.z);
			matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, rot.x);
			break;
		case 4: // ZXY
			matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, rot.z);
			matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, rot.x);
			matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, rot.y);
			break;
		case 5: // ZYX
			matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, rot.z);
			matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, rot.y);
			matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, rot.x);
			break;
	}
	matrix4_translate3f(&ident, &ident, pos.x, pos.y, pos.z);

	// Mesh
	{
		Helper::Vertex& v = m_vertexHelper->at(0);
		v.position.x = -0.5;
		v.position.y = -0.5;
		v.position.z = 0.0f;
		vec3_transform(&v.position, &v.position, &ident);
		v.color = 0xFFFFFFFF;
		v.uv[0].x = 0;
		v.uv[0].y = 0;
	}
	{
		Helper::Vertex& v = m_vertexHelper->at(1);
		v.position.x = 0.5;
		v.position.y = -0.5;
		v.position.z = 0.0f;
		vec3_transform(&v.position, &v.position, &ident);
		v.color = 0xFFFFFFFF;
		v.uv[0].x = 1;
		v.uv[0].y = 0;
	}
	{
		Helper::Vertex& v = m_vertexHelper->at(2);
		v.position.x = -0.5;
		v.position.y = 0.5;
		v.position.z = 0.0f;
		vec3_transform(&v.position, &v.position, &ident);
		v.color = 0xFFFFFFFF;
		v.uv[0].x = 0;
		v.uv[0].y = 1;
	}
	{
		Helper::Vertex& v = m_vertexHelper->at(3);
		v.position.x = 0.5;
		v.position.y = 0.5;
		v.position.z = 0.0f;
		vec3_transform(&v.position, &v.position, &ident);
		v.color = 0xFFFFFFFF;
		v.uv[0].x = 1;
		v.uv[0].y = 1;
	}
	obs_enter_graphics();
	m_vertexBuffer = m_vertexHelper->update();
	obs_leave_graphics();
}

uint32_t Filter::Transform::Instance::get_width() {
	return 0;
}

uint32_t Filter::Transform::Instance::get_height() {
	return 0;
}

void Filter::Transform::Instance::activate() {}

void Filter::Transform::Instance::deactivate() {}

void Filter::Transform::Instance::show() {}

void Filter::Transform::Instance::hide() {}

void Filter::Transform::Instance::video_tick(float) {}

void Filter::Transform::Instance::video_render(gs_effect_t *paramEffect) {
	obs_source_t *parent = obs_filter_get_parent(context);
	obs_source_t *target = obs_filter_get_target(context);

	// Skip rendering if our target, parent or context is not valid.
	if (!target || !parent || !context || !m_vertexBuffer || !m_texRender || !m_shapeRender) {
		obs_source_skip_video_filter(context);
		return;
	}

	uint32_t
		baseW = obs_source_get_base_width(target),
		baseH = obs_source_get_base_height(target);

	gs_effect_t *alphaEffect = obs_get_base_effect(OBS_EFFECT_DEFAULT);

	// Draw previous filters to texture.
	gs_texrender_reset(m_texRender);
	if (gs_texrender_begin(m_texRender, baseW, baseH)) {
		gs_ortho(0, (float)baseW, 0, (float)baseH, -1, 1);

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

		if (obs_source_process_filter_begin(context, GS_RGBA, OBS_NO_DIRECT_RENDERING)) {
			obs_source_process_filter_end(context, paramEffect ? paramEffect : alphaEffect, baseW, baseH);
		} else {
			obs_source_skip_video_filter(context);
		}

		gs_texrender_end(m_texRender);
	} else {
		obs_source_skip_video_filter(context);
	}
	gs_texture* filterTexture = gs_texrender_get_texture(m_texRender);

	// Draw shape to texture
	gs_texrender_reset(m_shapeRender);
	if (gs_texrender_begin(m_shapeRender, baseW, baseH)) {
		if (m_isOrthographic) {
			gs_ortho(
				-0.5, 0.5,
				-0.5, 0.5,
				-farZ, farZ);
		} else {
			float aspect = (float)baseW / (float)baseH;
			gs_perspective(fov, aspect, nearZ, farZ);

			//if (baseW > baseH) {
			//	gs_matrix_scale3f(1.0, (float)baseH / (float)baseW, 1.0);
			//} else if (baseH > baseW) {
			//	gs_matrix_scale3f((float)baseW / (float)baseH, 1.0, 1.0);
			//}
		}

		// Rendering
		vec4 black;
		vec4_zero(&black);
		gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &black, 0, 0);

		gs_set_cull_mode(GS_NEITHER);
		gs_enable_blending(false);
		gs_enable_depth_test(false);
		gs_depth_function(gs_depth_test::GS_ALWAYS);
		gs_enable_stencil_test(false);
		gs_enable_stencil_write(false);
		gs_enable_color(true, true, true, true);

		while (gs_effect_loop(alphaEffect, "Draw")) {
			gs_effect_set_texture(gs_effect_get_param_by_name(alphaEffect, "image"), filterTexture);
			gs_load_vertexbuffer(m_vertexBuffer);
			gs_load_indexbuffer(NULL);
			gs_draw(GS_TRISTRIP, 0, 4);
		}

		gs_texrender_end(m_shapeRender);
	} else {
		obs_source_skip_video_filter(context);
	}
	gs_texture* shapeTexture = gs_texrender_get_texture(m_shapeRender);

	// Draw final shape
	gs_reset_blend_state();
	gs_enable_depth_test(false);
	while (gs_effect_loop(alphaEffect, "Draw")) {
		gs_effect_set_texture(gs_effect_get_param_by_name(alphaEffect, "image"), shapeTexture);
		gs_draw_sprite(shapeTexture, 0, 0, 0);
	}
}


