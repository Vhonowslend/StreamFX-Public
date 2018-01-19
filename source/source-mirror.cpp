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

#include "source-mirror.h"
#include "strings.h"
#include <memory>
#include <cstring>
#include <vector>

#define S_SOURCE_MIRROR					"Source.Mirror"
#define P_SOURCE					"Source.Mirror.Source"
#define P_SOURCE_SIZE					"Source.Mirror.Source.Size"
#define P_SCALING					"Source.Mirror.Scaling"
#define P_SCALING_METHOD				"Source.Mirror.Scaling.Method"
#define P_SCALING_METHOD_POINT				"Source.Mirror.Scaling.Method.Point"
#define P_SCALING_METHOD_BILINEAR			"Source.Mirror.Scaling.Method.Bilinear"
#define P_SCALING_METHOD_BILINEARLOWRES			"Source.Mirror.Scaling.Method.BilinearLowRes"
#define P_SCALING_METHOD_BICUBIC			"Source.Mirror.Scaling.Method.Bicubic"
#define P_SCALING_METHOD_LANCZOS			"Source.Mirror.Scaling.Method.Lanczos"
#define P_SCALING_SIZE					"Source.Mirror.Scaling.Size"
#define P_SCALING_TRANSFORMKEEPORIGINAL			"Source.Mirror.Scaling.TransformKeepOriginal"

enum class ScalingMethod : int64_t {
	Point,
	Bilinear,
	BilinearLowRes,
	Bicubic,
	Lanczos,
};

// Initializer & Finalizer
Source::MirrorAddon* sourceMirrorInstance;
INITIALIZER(SourceMirrorInit) {
	initializerFunctions.push_back([] {
		sourceMirrorInstance = new Source::MirrorAddon();
	});
	finalizerFunctions.push_back([] {
		delete sourceMirrorInstance;
	});
}

Source::MirrorAddon::MirrorAddon() {
	memset(&osi, 0, sizeof(obs_source_info));
	osi.id = "obs-stream-effects-source-mirror";
	osi.type = OBS_SOURCE_TYPE_INPUT;
	osi.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW;

	osi.get_name = get_name;
	osi.get_defaults = get_defaults;
	osi.get_properties = get_properties;
	osi.get_width = get_width;
	osi.get_height = get_height;
	osi.create = create;
	osi.destroy = destroy;
	osi.update = update;
	osi.activate = activate;
	osi.deactivate = deactivate;
	osi.video_tick = video_tick;
	osi.video_render = video_render;

	obs_register_source(&osi);
}

Source::MirrorAddon::~MirrorAddon() {}

const char * Source::MirrorAddon::get_name(void *) {
	return P_TRANSLATE(S_SOURCE_MIRROR);
}

void Source::MirrorAddon::get_defaults(obs_data_t *data) {
	obs_data_set_default_string(data, P_SOURCE, "");
	obs_data_set_default_bool(data, P_SCALING, false);
	obs_data_set_default_string(data, P_SCALING_SIZE, "100x100");
	obs_data_set_default_int(data, P_SCALING_METHOD, (int64_t)ScalingMethod::Bilinear);
}

bool Source::MirrorAddon::modified_properties(obs_properties_t *pr, obs_property_t *p, obs_data_t *data) {
	if (obs_properties_get(pr, P_SOURCE) == p) {
		obs_source_t* target = obs_get_source_by_name(obs_data_get_string(data, P_SOURCE));
		if (target) {
			std::vector<char> buf(256);
			sprintf_s(buf.data(), buf.size(), "%ldx%ld\0",
				obs_source_get_width(target), obs_source_get_height(target));
			obs_data_set_string(data, P_SOURCE_SIZE, buf.data());
		} else {
			obs_data_set_string(data, P_SOURCE_SIZE, "0x0");
		}
	}

	if (obs_properties_get(pr, P_SCALING) == p) {
		bool show = obs_data_get_bool(data, P_SCALING);
		obs_property_set_visible(obs_properties_get(pr, P_SCALING_METHOD), show);
		obs_property_set_visible(obs_properties_get(pr, P_SCALING_SIZE), show);
		return true;
	}

	return false;
}

bool UpdateSourceListCB(void *ptr, obs_source_t* src) {
	obs_property_t* p = (obs_property_t*)ptr;
	obs_property_list_add_string(p, obs_source_get_name(src), obs_source_get_name(src));
	return true;
}

void UpdateSourceList(obs_property_t* p) {
	obs_property_list_clear(p);
	obs_enum_sources(UpdateSourceListCB, p);
}

obs_properties_t * Source::MirrorAddon::get_properties(void *ptr) {
	obs_properties_t* pr = obs_properties_create();
	obs_property_t* p = nullptr;

	p = obs_properties_add_list(pr, P_SOURCE, P_TRANSLATE(P_SOURCE),
		OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SOURCE)));
	obs_property_set_modified_callback(p, modified_properties);
	UpdateSourceList(p);

	p = obs_properties_add_text(pr, P_SOURCE_SIZE, P_TRANSLATE(P_SOURCE_SIZE), OBS_TEXT_DEFAULT);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SOURCE_SIZE)));
	obs_property_set_enabled(p, false);

	p = obs_properties_add_bool(pr, P_SCALING, P_TRANSLATE(P_SCALING));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SCALING)));
	obs_property_set_modified_callback(p, modified_properties);

	p = obs_properties_add_list(pr, P_SCALING_METHOD, P_TRANSLATE(P_SCALING_METHOD),
		OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SCALING_METHOD)));
	obs_property_list_add_int(p, P_TRANSLATE(P_SCALING_METHOD_POINT), (int64_t)ScalingMethod::Point);
	obs_property_list_add_int(p, P_TRANSLATE(P_SCALING_METHOD_BILINEAR), (int64_t)ScalingMethod::Bilinear);
	obs_property_list_add_int(p, P_TRANSLATE(P_SCALING_METHOD_BILINEARLOWRES), (int64_t)ScalingMethod::BilinearLowRes);
	obs_property_list_add_int(p, P_TRANSLATE(P_SCALING_METHOD_BICUBIC), (int64_t)ScalingMethod::Bicubic);
	obs_property_list_add_int(p, P_TRANSLATE(P_SCALING_METHOD_LANCZOS), (int64_t)ScalingMethod::Lanczos);

	p = obs_properties_add_text(pr, P_SCALING_SIZE, P_TRANSLATE(P_SCALING_SIZE),
		OBS_TEXT_DEFAULT);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SCALING_SIZE)));

	p = obs_properties_add_bool(pr, P_SCALING_TRANSFORMKEEPORIGINAL, P_TRANSLATE(P_SCALING_TRANSFORMKEEPORIGINAL));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SCALING_TRANSFORMKEEPORIGINAL)));

	return pr;
}

void * Source::MirrorAddon::create(obs_data_t *data, obs_source_t *source) {
	return new Source::Mirror(data, source);
}

void Source::MirrorAddon::destroy(void *p) {
	delete static_cast<Source::Mirror*>(p);
}

uint32_t Source::MirrorAddon::get_width(void *p) {
	return static_cast<Source::Mirror*>(p)->get_width();
}

uint32_t Source::MirrorAddon::get_height(void *p) {
	return static_cast<Source::Mirror*>(p)->get_height();
}

void Source::MirrorAddon::update(void *p, obs_data_t *data) {
	static_cast<Source::Mirror*>(p)->update(data);
}

void Source::MirrorAddon::activate(void *p) {
	static_cast<Source::Mirror*>(p)->activate();
}

void Source::MirrorAddon::deactivate(void *p) {
	static_cast<Source::Mirror*>(p)->deactivate();
}

void Source::MirrorAddon::video_tick(void *p, float t) {
	static_cast<Source::Mirror*>(p)->video_tick(t);
}

void Source::MirrorAddon::video_render(void *p, gs_effect_t *ef) {
	static_cast<Source::Mirror*>(p)->video_render(ef);
}

Source::Mirror::Mirror(obs_data_t* data, obs_source_t* src) {
	m_active = true;
	m_source = src;
	m_target = nullptr;

	m_rescale = false;
	m_width = m_height = 1;
	m_renderTarget = std::make_unique<GS::RenderTarget>(GS_RGBA, GS_ZS_NONE);
	m_renderTargetScale = std::make_unique<GS::RenderTarget>(GS_RGBA, GS_ZS_NONE);
	m_sampler = std::make_shared<GS::Sampler>();
	m_scalingEffect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_DEFAULT);

	update(data);
}

Source::Mirror::~Mirror() {

}

uint32_t Source::Mirror::get_width() {
	if (m_rescale && m_width > 0 && !m_keepOriginalSize)
		return m_width;
	if (m_target && m_target != m_source)
		return obs_source_get_width(m_target);
	return 1;
}

uint32_t Source::Mirror::get_height() {
	if (m_rescale && m_height > 0 && !m_keepOriginalSize)
		return m_height;
	if (m_target && m_target != m_source)
		return obs_source_get_height(m_target);
	return 1;
}

void Source::Mirror::update(obs_data_t* data) {
	// Update selected source.
	const char* source = obs_data_get_string(data, P_SOURCE);
	m_target = obs_get_source_by_name(source);
	m_targetName = source;

	// Rescaling
	m_rescale = obs_data_get_bool(data, P_SCALING);
	if (m_rescale) { // Parse rescaling settings.
		uint32_t width, height;

		// Read value.
		const char* size = obs_data_get_string(data, P_SCALING_SIZE);
		const char* xpos = strchr(size, 'x');
		if (xpos != nullptr) {
			// Width
			width = strtoul(size, nullptr, 10);
			if (errno == ERANGE || width == 0) {
				m_rescale = false;
				m_width = 1;
			} else {
				m_width = width;
			}

			height = strtoul(xpos + 1, nullptr, 10);
			if (errno == ERANGE || height == 0) {
				m_rescale = false;
				m_height = 1;
			} else {
				m_height = height;
			}
		} else {
			m_rescale = false;
			m_width = 1;
			m_height = 1;
		}

		ScalingMethod scaler = (ScalingMethod)obs_data_get_int(data, P_SCALING_METHOD);
		switch (scaler) {
			case ScalingMethod::Point:
			default:
				m_scalingEffect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_DEFAULT);
				m_sampler->SetSampleFilter(GS_FILTER_POINT);
				break;
			case ScalingMethod::Bilinear:
				m_scalingEffect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_DEFAULT);
				m_sampler->SetSampleFilter(GS_FILTER_LINEAR);
				break;
			case ScalingMethod::BilinearLowRes:
				m_scalingEffect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_BILINEAR_LOWRES);
				m_sampler->SetSampleFilter(GS_FILTER_LINEAR);
				break;
			case ScalingMethod::Bicubic:
				m_scalingEffect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_BICUBIC);
				m_sampler->SetSampleFilter(GS_FILTER_LINEAR);
				break;
			case ScalingMethod::Lanczos:
				m_scalingEffect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_LANCZOS);
				m_sampler->SetSampleFilter(GS_FILTER_LINEAR);
				break;
		}

		m_keepOriginalSize = obs_data_get_bool(data, P_SCALING_TRANSFORMKEEPORIGINAL);
	}
}

void Source::Mirror::activate() {
	m_active = true;
}

void Source::Mirror::deactivate() {
	m_active = false;
}

void Source::Mirror::video_tick(float) {
	if (!m_target)
		m_target = obs_get_source_by_name(m_targetName.c_str());
}

void Source::Mirror::video_render(gs_effect_t* effect) {
	if (!m_active || (m_source == m_target) || (m_width == 0 || m_height == 0))
		return;

	if (m_rescale && m_width > 0 && m_height > 0 && m_scalingEffect) {
		uint32_t sw, sh;
		sw = obs_source_get_width(m_target);
		sh = obs_source_get_height(m_target);

		// Store original Source Texture
		try {
			vec4 black; vec4_zero(&black);
			auto op = m_renderTarget->Render(sw, sh);
			gs_ortho(0, sw, 0, sh, 0, 1);
			gs_clear(GS_CLEAR_COLOR, &black, 0, 0);

			obs_source_video_render(m_target);
		} catch (...) {
			return;
		}

		gs_eparam_t *scale_param = gs_effect_get_param_by_name(m_scalingEffect, "base_dimension_i");
		if (scale_param) {
			struct vec2 base_res_i = {
				1.0f / (float)sw,
				1.0f / (float)sh
			};
			gs_effect_set_vec2(scale_param, &base_res_i);
		}

		if (m_keepOriginalSize) {
			{
				vec4 black; vec4_zero(&black);
				auto op = m_renderTargetScale->Render(m_width, m_height);
				gs_ortho(0, m_width, 0, m_height, 0, 1);
				gs_clear(GS_CLEAR_COLOR, &black, 0, 0);
				while (gs_effect_loop(m_scalingEffect, "Draw")) {
					gs_eparam_t* image = gs_effect_get_param_by_name(m_scalingEffect, "image");
					gs_effect_set_next_sampler(image, m_sampler->GetObject());
					obs_source_draw(m_renderTarget->GetTextureObject(), 0, 0, m_width, m_height, false);
				}
			}
			while (gs_effect_loop(obs_get_base_effect(OBS_EFFECT_DEFAULT), "Draw")) {
				gs_eparam_t* image = gs_effect_get_param_by_name(obs_get_base_effect(OBS_EFFECT_DEFAULT), "image");
				gs_effect_set_next_sampler(image, m_sampler->GetObject());
				obs_source_draw(m_renderTargetScale->GetTextureObject(), 0, 0, sw, sh, false);
			}
		} else {
			while (gs_effect_loop(m_scalingEffect, "Draw")) {
				gs_eparam_t* image = gs_effect_get_param_by_name(m_scalingEffect, "image");
				gs_effect_set_next_sampler(image, m_sampler->GetObject());
				obs_source_draw(m_renderTarget->GetTextureObject(), 0, 0, m_width, m_height, false);
			}
		}
	} else {
		obs_source_video_render(m_target);
	}
}
