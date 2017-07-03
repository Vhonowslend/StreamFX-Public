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

extern "C" {
#pragma warning (push)
#pragma warning (disable: 4201)
#include "libobs/util/platform.h"
#include "libobs/graphics/graphics.h"
#include "libobs/graphics/matrix4.h"
#pragma warning (pop)
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
	sourceInfo.show = show;
	sourceInfo.hide = hide;
	sourceInfo.video_tick = video_tick;
	sourceInfo.video_render = video_render;

	obs_register_source(&sourceInfo);
}

Filter::Blur::~Blur() {}

const char * Filter::Blur::get_name(void *) {
	return P_TRANSLATE(P_FILTER_BLUR);
}

void Filter::Blur::get_defaults(obs_data_t *data) {
	obs_data_set_default_int(data, P_FILTER_BLUR_TYPE, Filter::Blur::Type::Box);
	obs_data_set_default_int(data, P_FILTER_BLUR_SIZE, 1);
}

obs_properties_t * Filter::Blur::get_properties(void *) {
	obs_properties_t *pr = obs_properties_create();
	obs_property_t* p = NULL;

	p = obs_properties_add_list(pr, P_FILTER_BLUR_TYPE, P_TRANSLATE(P_FILTER_BLUR_TYPE),
		obs_combo_type::OBS_COMBO_TYPE_LIST, obs_combo_format::OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_FILTER_BLUR_TYPE)));
	obs_property_list_add_int(p, P_TRANSLATE(P_FILTER_BLUR_TYPE_BOX), Filter::Blur::Type::Box);
	obs_property_list_add_int(p, P_TRANSLATE(P_FILTER_BLUR_TYPE_GAUSSIAN),
		Filter::Blur::Type::Gaussian);

	p = obs_properties_add_int_slider(pr, P_FILTER_BLUR_SIZE, P_TRANSLATE(P_FILTER_BLUR_SIZE),
		1, 100, 1);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_FILTER_BLUR_SIZE)));

	return pr;
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

void Filter::Blur::show(void *ptr) {
	reinterpret_cast<Instance*>(ptr)->show();
}

void Filter::Blur::hide(void *ptr) {
	reinterpret_cast<Instance*>(ptr)->hide();
}

void Filter::Blur::video_tick(void *ptr, float time) {
	reinterpret_cast<Instance*>(ptr)->video_tick(time);
}

void Filter::Blur::video_render(void *ptr, gs_effect_t *effect) {
	reinterpret_cast<Instance*>(ptr)->video_render(effect);
}

Filter::Blur::Instance::Instance(obs_data_t *data, obs_source_t *context) : m_source(context) {
	obs_enter_graphics();
	{
		char* loadError = nullptr;
		char* file = obs_module_file("filter-blur/filter.effect");
		m_effect = gs_effect_create_from_file(file, &loadError);
		bfree(file);
		if (loadError != nullptr) {
			PLOG_ERROR("<filter-blur> Loading effect failed with error(s): %s", loadError);
			bfree(loadError);
		}
	}
	m_primaryRT = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	m_secondaryRT = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	obs_leave_graphics();

	update(data);
}

Filter::Blur::Instance::~Instance() {
	obs_enter_graphics();
	gs_effect_destroy(m_effect);
	gs_texrender_destroy(m_primaryRT);
	gs_texrender_destroy(m_secondaryRT);
	obs_leave_graphics();
}

void Filter::Blur::Instance::update(obs_data_t *data) {
	switch (obs_data_get_int(data, P_FILTER_BLUR_TYPE)) {
		case Filter::Blur::Type::Box:
			m_technique = gs_effect_get_technique(m_effect, "Box");
			break;
		case Filter::Blur::Type::Gaussian:
			m_technique = gs_effect_get_technique(m_effect, "Gaussian");
			break;
	}
	m_filterWidth = (int)obs_data_get_int(data, P_FILTER_BLUR_SIZE);
}

uint32_t Filter::Blur::Instance::get_width() {
	return 0;
}

uint32_t Filter::Blur::Instance::get_height() {
	return 0;
}

void Filter::Blur::Instance::activate() {}

void Filter::Blur::Instance::deactivate() {}

void Filter::Blur::Instance::show() {}

void Filter::Blur::Instance::hide() {}

void Filter::Blur::Instance::video_tick(float) {}

void Filter::Blur::Instance::video_render(gs_effect_t *effect) {
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
	gs_texture_t *sourceTexture = nullptr,
		*horizontalTexture = nullptr;

#pragma region Source To Texture
	gs_texrender_reset(m_primaryRT);
	if (!gs_texrender_begin(m_primaryRT, baseW, baseH)) {
		PLOG_ERROR("<filter-blur> Failed to set up base texture.");
		obs_source_skip_video_filter(m_source);
		return;
	} else {
		gs_ortho(0, baseW, 0, baseH, -1, 1);

		// Clear to Black
		vec4 black;
		vec4_zero(&black);
		gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &black, 0, 0);

		// Set up camera stuff
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

		// Render
		if (obs_source_process_filter_begin(m_source, GS_RGBA, OBS_NO_DIRECT_RENDERING)) {
			obs_source_process_filter_end(m_source, effect ? effect : defaultEffect, baseW, baseH);
		} else {
			PLOG_ERROR("<filter-blur> Unable to render source.");
			obs_source_skip_video_filter(m_source);
			return;
		}
		gs_texrender_end(m_primaryRT);
	}

	sourceTexture = gs_texrender_get_texture(m_primaryRT);
	if (!sourceTexture) {
		PLOG_ERROR("<filter-blur> Failed to get source texture.");
		obs_source_skip_video_filter(m_source);
		return;
	}
#pragma endregion Source To Texture

#pragma region Horizontal Pass
	gs_texrender_reset(m_secondaryRT);
	if (!gs_texrender_begin(m_secondaryRT, baseW, baseH)) {
		PLOG_ERROR("<filter-blur> Failed set up horizontal pass.");
		obs_source_skip_video_filter(m_source);
		return;
	} else {
		gs_ortho(0, baseW, 0, baseH, -1, 1);

		// Clear to Black
		vec4 black;
		vec4_zero(&black);
		gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &black, 0, 0);

		// Set up camera stuff
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

		// Prepare Effect
		gs_eparam_t *param = gs_effect_get_param_by_name(m_effect, "texel");
		if (param) {
			vec2 texel;
			vec2_set(&texel, (float)(1.0 / baseW), 0);
			gs_effect_set_vec2(param, &texel);
		} else {
			PLOG_ERROR("<filter-blur> Failed to set texel param.");
			obs_source_skip_video_filter(m_source);
			return;
		}
		param = gs_effect_get_param_by_name(m_effect, "size");
		if (param) {
			gs_effect_set_int(param, m_filterWidth);
		} else {
			PLOG_ERROR("<filter-blur> Failed to set size param.");
			obs_source_skip_video_filter(m_source);
			return;
		}
		param = gs_effect_get_param_by_name(m_effect, "image");
		if (param) {
			gs_effect_set_texture(param, sourceTexture);
		} else {
			PLOG_ERROR("<filter-blur> Failed to set image param.");
			obs_source_skip_video_filter(m_source);
			return;
		}

		// Render
		if (gs_technique_begin(m_technique)) {
			if (gs_technique_begin_pass_by_name(m_technique, "Horizontal")) {
				gs_draw_sprite(sourceTexture, 0, baseW, baseH);
				gs_technique_end_pass(m_technique);
			}
			gs_technique_end(m_technique);
		}
		gs_texrender_end(m_secondaryRT);
	}

	horizontalTexture = gs_texrender_get_texture(m_secondaryRT);
	if (!horizontalTexture) {
		PLOG_ERROR("<filter-blur> Failed to get horizontal pass texture.");
		obs_source_skip_video_filter(m_source);
		return;
	}
#pragma endregion Horizontal Pass

#pragma region Horizontal Pass
	// Prepare Effect
	gs_eparam_t *param = gs_effect_get_param_by_name(m_effect, "texel");
	if (param) {
		vec2 texel;
		vec2_set(&texel, 0, (float)(1.0 / baseH));
		gs_effect_set_vec2(param, &texel);
	} else {
		PLOG_ERROR("<filter-blur> Failed to set texel param.");
		obs_source_skip_video_filter(m_source);
		return;
	}
	param = gs_effect_get_param_by_name(m_effect, "size");
	if (param) {
		gs_effect_set_int(param, m_filterWidth);
	} else {
		PLOG_ERROR("<filter-blur> Failed to set size param.");
		obs_source_skip_video_filter(m_source);
		return;
	}
	param = gs_effect_get_param_by_name(m_effect, "image");
	if (param) {
		gs_effect_set_texture(param, horizontalTexture);
	} else {
		PLOG_ERROR("<filter-blur> Failed to set image param.");
		obs_source_skip_video_filter(m_source);
		return;
	}

	// Render
	if (gs_technique_begin(m_technique)) {
		if (gs_technique_begin_pass_by_name(m_technique, "Vertical")) {
			gs_draw_sprite(horizontalTexture, 0, baseW, baseH);
			gs_technique_end_pass(m_technique);
		}
		gs_technique_end(m_technique);
	}
#pragma endregion Horizontal Pass




	// Draw final shape
	//gs_reset_blend_state();
	//gs_enable_depth_test(false);
	//while (gs_effect_loop(defaultEffect, "Draw")) {
	//	gs_effect_set_texture(gs_effect_get_param_by_name(defaultEffect, "image"), horizontalTexture);
	//	gs_draw_sprite(horizontalTexture, 0, 0, 0);
	//}
}
