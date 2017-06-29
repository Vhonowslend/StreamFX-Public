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

static const float PI = 3.1415926535897932384626433832795;

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
	obs_data_set_default_bool(data, P_FILTER_TRANSFORM_ORTHOGRAPHIC, true);
	obs_data_set_default_double(data, P_FILTER_TRANSFORM_POSITION_X, 0);
	obs_data_set_default_double(data, P_FILTER_TRANSFORM_POSITION_Y, 0);
	obs_data_set_default_double(data, P_FILTER_TRANSFORM_POSITION_Z, 0);
	obs_data_set_default_double(data, P_FILTER_TRANSFORM_ROTATION_X, 0);
	obs_data_set_default_double(data, P_FILTER_TRANSFORM_ROTATION_Y, 0);
	obs_data_set_default_double(data, P_FILTER_TRANSFORM_ROTATION_Z, 0);
	obs_data_set_default_double(data, P_FILTER_TRANSFORM_SCALE_X, 1);
	obs_data_set_default_double(data, P_FILTER_TRANSFORM_SCALE_Y, 1);
}

obs_properties_t * Filter::Transform::get_properties(void *) {
	obs_properties_t *pr = obs_properties_create();
	obs_property_t* p = NULL;

	p = obs_properties_add_bool(pr, P_FILTER_TRANSFORM_ORTHOGRAPHIC, P_TRANSLATE(P_FILTER_TRANSFORM_ORTHOGRAPHIC));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_FILTER_TRANSFORM_ORTHOGRAPHIC)));

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
				-100000.00, 100000.00, 0.01);
			obs_property_set_long_description(p, P_TRANSLATE(kv.second));
		}
	}
	{
		std::pair<const char*, const char*> entries[] = {
			std::make_pair(P_FILTER_TRANSFORM_ROTATION_X, P_DESC(P_FILTER_TRANSFORM_ROTATION_X)),
			std::make_pair(P_FILTER_TRANSFORM_ROTATION_Y, P_DESC(P_FILTER_TRANSFORM_ROTATION_Y)),
			std::make_pair(P_FILTER_TRANSFORM_ROTATION_Z, P_DESC(P_FILTER_TRANSFORM_ROTATION_Z)),
		};
		for (auto kv : entries) {
			p = obs_properties_add_float_slider(pr, kv.first, P_TRANSLATE(kv.first),
				-360.00, 360.00, 0.01);
			obs_property_set_long_description(p, P_TRANSLATE(kv.second));
		}
	}

	return pr;
}

bool Filter::Transform::modified_properties(obs_properties_t *pr, obs_property_t *, obs_data_t *data) {

	return true;
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
	m_shapeRender = gs_texrender_create(GS_RGBA, GS_Z32F);
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
	pos.x = obs_data_get_double(data, P_FILTER_TRANSFORM_POSITION_X);
	pos.y = obs_data_get_double(data, P_FILTER_TRANSFORM_POSITION_Y);
	pos.z = obs_data_get_double(data, P_FILTER_TRANSFORM_POSITION_Z);
	rot.x = obs_data_get_double(data, P_FILTER_TRANSFORM_ROTATION_X) / 360.0 * PI;
	rot.y = obs_data_get_double(data, P_FILTER_TRANSFORM_ROTATION_Y) / 360.0 * PI;
	rot.z = obs_data_get_double(data, P_FILTER_TRANSFORM_ROTATION_Z) / 360.0 * PI;
	scale.x = obs_data_get_double(data, P_FILTER_TRANSFORM_SCALE_X);
	scale.y = obs_data_get_double(data, P_FILTER_TRANSFORM_SCALE_Y);
	m_isOrthographic = obs_data_get_bool(data, P_FILTER_TRANSFORM_ORTHOGRAPHIC);
}

uint32_t Filter::Transform::Instance::get_width() {
	return 0;
}

uint32_t Filter::Transform::Instance::get_height() {
	return 0;
}

void Filter::Transform::Instance::activate() {

}

void Filter::Transform::Instance::deactivate() {

}

void Filter::Transform::Instance::show() {

}

void Filter::Transform::Instance::hide() {

}

void Filter::Transform::Instance::video_tick(float) {

}

void Filter::Transform::Instance::video_render(gs_effect_t *effect) {
	obs_source_t *parent = obs_filter_get_parent(context);
	obs_source_t *target = obs_filter_get_target(context);
	uint32_t
		baseW = obs_source_get_base_width(target),
		baseH = obs_source_get_base_height(target);
	float halfW = (float)baseW / 2.0,
		halfH = (float)baseH / 2.0;

	// Skip rendering if our target, parent or context is not valid.
	if (!target || !parent || !context) {
		obs_source_skip_video_filter(context);
		return;
	}

	gs_effect_t* eff = obs_get_base_effect(OBS_EFFECT_OPAQUE);

	// Draw previous filters to texture.
	gs_texrender_reset(m_texRender);
	if (gs_texrender_begin(m_texRender, baseW, baseH)) {
		if (obs_source_process_filter_begin(context, GS_RGBA, OBS_NO_DIRECT_RENDERING)) {
			obs_source_process_filter_end(context, obs_get_base_effect(OBS_EFFECT_OPAQUE), baseW, baseH);
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
		gs_rect vp;
		gs_get_viewport(&vp);

		gs_projection_push();
		gs_matrix_push();
		gs_viewport_push();

		gs_set_viewport(0, 0, baseW, baseH);
		if (m_isOrthographic) {
			gs_ortho(
				-halfW, halfW,
				-halfH, halfH,
				-65535.0, 65535.0);
		} else {
			gs_perspective(90 / 360.0 * PI, (float)baseW / (float)baseH, -65535.0, 65535.0);
		}

		gs_set_cull_mode(GS_NEITHER);
		gs_enable_blending(false);
		gs_enable_depth_test(false);
		gs_enable_stencil_test(false);
		gs_enable_stencil_write(false);
		gs_enable_color(true, true, true, true);
		gs_enable_depth_test(false);

		// Data
		vec3 sourceSize;
		sourceSize.x = baseW;
		sourceSize.y = baseH;
		sourceSize.z = 1.0;

		// Matrix
		matrix4 ident;
		matrix4_identity(&ident);
		matrix4_scale(&ident, &ident, &scale);
		//matrix4_scale(&ident, &ident, &sourceSize);
		matrix4_rotate_aa4f(&ident, &ident, 1, 0, 0, rot.x);
		matrix4_rotate_aa4f(&ident, &ident, 0, 1, 0, rot.y);
		matrix4_rotate_aa4f(&ident, &ident, 0, 0, 1, rot.z);
		gs_matrix_set(&ident);

		// Mesh
		{
			Helper::Vertex& v = m_vertexHelper->at(0);
			v.position.x = -halfW;
			v.position.y = -halfH;
			v.position.z = 1.0;
			v.color = 0xFFFFFFFF;
			v.uv[0].x = 0;
			v.uv[0].y = 0;
		}
		{
			Helper::Vertex& v = m_vertexHelper->at(1);
			v.position.x = halfW;
			v.position.y = -halfH;
			v.position.z = 1.0;
			v.color = 0xFFFFFFFF;
			v.uv[0].x = 1;
			v.uv[0].y = 0;
		}
		{
			Helper::Vertex& v = m_vertexHelper->at(2);
			v.position.x = -halfW;
			v.position.y = halfH;
			v.position.z = 1.0;
			v.color = 0xFFFFFFFF;
			v.uv[0].x = 0;
			v.uv[0].y = 1;
		}
		{
			Helper::Vertex& v = m_vertexHelper->at(3);
			v.position.x = halfW;
			v.position.y = halfH;
			v.position.z = 1.0;
			v.color = 0xFFFFFFFF;
			v.uv[0].x = 1;
			v.uv[0].y = 1;
		}
		m_vertexBuffer = m_vertexHelper->update();

		// Rendering
		vec4 black;
		vec4_zero(&black);
		gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &black, 0, 0);

		while (gs_effect_loop(eff, "Draw")) {
			gs_effect_set_texture(gs_effect_get_param_by_name(eff, "image"), filterTexture);
			//gs_draw_sprite(filterTexture, 0, 0, 0);
			gs_load_vertexbuffer(m_vertexBuffer);
			gs_load_indexbuffer(NULL);
			gs_draw(GS_LINESTRIP, 0, 4);
		}

		gs_projection_pop();
		gs_matrix_pop();
		gs_viewport_pop();

		gs_texrender_end(m_shapeRender);
	} else {
		obs_source_skip_video_filter(context);
	}
	gs_texture* shapeTexture = gs_texrender_get_texture(m_shapeRender);

	// Draw final shape
	while (gs_effect_loop(eff, "Draw")) {
		gs_effect_set_texture(gs_effect_get_param_by_name(eff, "image"), shapeTexture);
		gs_draw_sprite(shapeTexture, 0, 0, 0);
	}
}


