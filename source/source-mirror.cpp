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
#include <bitset>
#include <media-io/audio-io.h>
#include <functional>
#include "obs-tools.hpp"

#define S_SOURCE_MIRROR					"Source.Mirror"
#define P_SOURCE					"Source.Mirror.Source"
#define P_SOURCE_SIZE					"Source.Mirror.Source.Size"
#define P_SOURCE_AUDIO					"Source.Mirror.Source.Audio"
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
	osi.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_AUDIO | OBS_SOURCE_CUSTOM_DRAW;

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
	osi.enum_active_sources = enum_active_sources;

	obs_register_source(&osi);
}

Source::MirrorAddon::~MirrorAddon() {}

const char * Source::MirrorAddon::get_name(void *) {
	return P_TRANSLATE(S_SOURCE_MIRROR);
}

void Source::MirrorAddon::get_defaults(obs_data_t *data) {
	obs_data_set_default_string(data, P_SOURCE, "");
	obs_data_set_default_bool(data, P_SOURCE_AUDIO, false);
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

static bool UpdateSourceListCB(void *ptr, obs_source_t* src) {
	obs_property_t* p = (obs_property_t*)ptr;
	obs_property_list_add_string(p, obs_source_get_name(src), obs_source_get_name(src));
	return true;
}

static void UpdateSourceList(obs_property_t* p) {
	obs_property_list_clear(p);
	obs_enum_sources(UpdateSourceListCB, p);
	obs_enum_scenes(UpdateSourceListCB, p);
}

obs_properties_t * Source::MirrorAddon::get_properties(void *) {
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

	p = obs_properties_add_bool(pr, P_SOURCE_AUDIO, P_TRANSLATE(P_SOURCE_AUDIO));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SOURCE_AUDIO)));

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
	if (p) {
		delete static_cast<Source::Mirror*>(p);
	}
}

uint32_t Source::MirrorAddon::get_width(void *p) {
	if (p) {
		return static_cast<Source::Mirror*>(p)->get_width();
	}
	return 0;
}

uint32_t Source::MirrorAddon::get_height(void *p) {
	if (p) {
		return static_cast<Source::Mirror*>(p)->get_height();
	}
	return 0;
}

void Source::MirrorAddon::update(void *p, obs_data_t *data) {
	if (p) {
		static_cast<Source::Mirror*>(p)->update(data);
	}
}

void Source::MirrorAddon::activate(void *p) {
	if (p) {
		static_cast<Source::Mirror*>(p)->activate();
	}
}

void Source::MirrorAddon::deactivate(void *p) {
	if (p) {
		static_cast<Source::Mirror*>(p)->deactivate();
	}
}

void Source::MirrorAddon::video_tick(void *p, float t) {
	if (p) {
		static_cast<Source::Mirror*>(p)->video_tick(t);
	}
}

void Source::MirrorAddon::video_render(void *p, gs_effect_t *ef) {
	if (p) {
		static_cast<Source::Mirror*>(p)->video_render(ef);
	}
}


void Source::MirrorAddon::enum_active_sources(void *p, obs_source_enum_proc_t enum_callback, void *param) {
	if (p) {
		static_cast<Source::Mirror*>(p)->enum_active_sources(enum_callback, param);
	}
}

Source::Mirror::Mirror(obs_data_t* data, obs_source_t* src) {
	m_source = src;

	m_rescale = false;
	m_width = m_height = 1;
	m_renderTargetScale = std::make_unique<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
	m_sampler = std::make_shared<gs::sampler>();
	m_scalingEffect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_DEFAULT);

	m_audioData.resize(MAX_AUDIO_CHANNELS);
	for (size_t idx = 0; idx < m_audioData.size(); idx++) {
		m_audioData[idx].resize(AUDIO_OUTPUT_FRAMES);
	}
	m_audioThread = std::thread(std::bind(&Source::Mirror::audio_output_cb, this));

	update(data);
	m_active = true;
}

Source::Mirror::~Mirror() {
	if (m_audioCapture) {
		m_audioCapture.reset();
	}
	if (m_source_texture) {
		m_source_texture.reset();
	}
	if (m_scene) {
		obs_scene_release(m_scene);
	}

	m_killAudioThread = true;
	m_audioNotify.notify_all();
	if (m_audioThread.joinable())
		m_audioThread.join();
}

uint32_t Source::Mirror::get_width() {
	if (m_rescale && m_width > 0 && !m_keepOriginalSize) {
		return m_width;
	}
	obs_source_t* source = obs_sceneitem_get_source(m_sceneitem);
	if (source) {
		return obs_source_get_width(source);
	}
	return 1;
}

uint32_t Source::Mirror::get_height() {
	if (m_rescale && m_height > 0 && !m_keepOriginalSize)
		return m_height;
	obs_source_t* source = obs_sceneitem_get_source(m_sceneitem);
	if (source) {
		return obs_source_get_height(source);
	}
	return 1;
}

void Source::Mirror::update(obs_data_t* data) {
	if (!this->m_scene && m_active) {
		m_scene          = obs_scene_create_private("localscene");
		m_scene_source   = std::make_shared<obs::source>(obs_scene_get_source(m_scene), false, false);
		m_source_texture = std::make_unique<gfx::source_texture>(m_scene_source, m_source);
	}

	// Update selected source.
	const char* sourceName = obs_data_get_string(data, P_SOURCE);
	if (sourceName != m_mirrorName) {
		if (m_scene) {
			if (m_sceneitem) {
				obs_sceneitem_remove(m_sceneitem);
				m_sceneitem = nullptr;
			}
			obs_source_t* source = obs_get_source_by_name(sourceName);
			if (source) {
				bool allow  = true;
				if (strcmp(obs_source_get_id(source), "scene") == 0) {
					if (obs::tools::scene_contains_source(obs_scene_from_source(source), m_source)) {
						allow = false;
					}
				}
				if (allow) {				
					m_sceneitem    = obs_scene_add(m_scene, source);
					try {
						m_audioCapture = std::make_unique<obs::audio_capture>(source);
						m_audioCapture->set_callback(std::bind(&Source::Mirror::audio_capture_cb, this,
															   std::placeholders::_1, std::placeholders::_2,
															   std::placeholders::_3));
					} catch(...) {
					}
				}
				obs_source_release(source);
			}
		}
	}
	m_enableAudio = obs_data_get_bool(data, P_SOURCE_AUDIO);

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
				m_sampler->set_filter(GS_FILTER_POINT);
				break;
			case ScalingMethod::Bilinear:
				m_scalingEffect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_DEFAULT);
				m_sampler->set_filter(GS_FILTER_LINEAR);
				break;
			case ScalingMethod::BilinearLowRes:
				m_scalingEffect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_BILINEAR_LOWRES);
				m_sampler->set_filter(GS_FILTER_LINEAR);
				break;
			case ScalingMethod::Bicubic:
				m_scalingEffect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_BICUBIC);
				m_sampler->set_filter(GS_FILTER_LINEAR);
				break;
			case ScalingMethod::Lanczos:
				m_scalingEffect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_LANCZOS);
				m_sampler->set_filter(GS_FILTER_LINEAR);
				break;
		}

		m_keepOriginalSize = obs_data_get_bool(data, P_SCALING_TRANSFORMKEEPORIGINAL);
	}
}

void Source::Mirror::activate() {
	m_active = true;
	if (!m_sceneitem) {
		obs_data_t* ref = obs_source_get_settings(m_source);
		update(ref);
		obs_data_release(ref);
	}
}

void Source::Mirror::deactivate() {
	m_active = false;
}

static inline void mix_audio(float *p_out, float *p_in,
	size_t pos, size_t count) {
	register float *out = p_out;
	register float *in = p_in + pos;
	register float *end = in + count;

	while (in < end)
		*(out++) += *(in++);
}

void Source::Mirror::video_tick(float time) {
	m_tick += time;

	if (m_sceneitem) {
		m_mirrorName = obs_source_get_name(obs_sceneitem_get_source(m_sceneitem));
	} else {
		if (m_tick > 0.1f) {
			obs_data_t* ref = obs_source_get_settings(m_source);
			update(ref);
			obs_data_release(ref);
			m_tick -= 0.1f;
		}
	}
}

void Source::Mirror::video_render(gs_effect_t*) {
	if ((m_width == 0) || (m_height == 0) || !m_source_texture || (m_source_texture->get_object() == m_source)
		|| !m_scene || !m_sceneitem) {
		return;
	}

	if (m_rescale && m_width > 0 && m_height > 0 && m_scalingEffect) {
		// Get Size of source.
		obs_source_t* source = obs_sceneitem_get_source(m_sceneitem);

		uint32_t sw, sh;
		sw = obs_source_get_width(source);
		sh = obs_source_get_height(source);

		vec2 bounds;
		bounds.x = sw;
		bounds.y = sh;
		obs_sceneitem_set_bounds(m_sceneitem, &bounds);

		// Store original Source Texture
		std::shared_ptr<gs::texture> tex;
		try {
			tex = m_source_texture->render(sw, sh);
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
				auto op = m_renderTargetScale->render(m_width, m_height);
				gs_ortho(0, (float_t)m_width, 0, (float_t)m_height, 0, 1);
				gs_clear(GS_CLEAR_COLOR, &black, 0, 0);
				while (gs_effect_loop(m_scalingEffect, "Draw")) {
					gs_eparam_t* image = gs_effect_get_param_by_name(m_scalingEffect, "image");
					gs_effect_set_next_sampler(image, m_sampler->get_object());
					obs_source_draw(tex->get_object(), 0, 0, m_width, m_height, false);
				}
			}
			while (gs_effect_loop(obs_get_base_effect(OBS_EFFECT_DEFAULT), "Draw")) {
				gs_eparam_t* image = gs_effect_get_param_by_name(obs_get_base_effect(OBS_EFFECT_DEFAULT), "image");
				gs_effect_set_next_sampler(image, m_sampler->get_object());
				obs_source_draw(m_renderTargetScale->get_object(), 0, 0, sw, sh, false);
			}
		} else {
			while (gs_effect_loop(m_scalingEffect, "Draw")) {
				gs_eparam_t* image = gs_effect_get_param_by_name(m_scalingEffect, "image");
				gs_effect_set_next_sampler(image, m_sampler->get_object());
				obs_source_draw(tex->get_object(), 0, 0, m_width, m_height, false);
			}
		}
	} else {
		obs_source_video_render(m_source_texture->get_object());
	}
}

void Source::Mirror::audio_capture_cb(void*, const audio_data* audio, bool) {
	std::unique_lock<std::mutex> ulock(m_audioLock);
	if (!m_enableAudio) {
		return;
	}

	audio_t* aud = obs_get_audio();
	audio_output_info const* aoi = audio_output_get_info(aud);

	std::bitset<8> layout;
	for (size_t plane = 0; plane < MAX_AV_PLANES; plane++) {
		float *samples = (float*)audio->data[plane];
		if (!samples) {
			m_audioOutput.data[plane] = nullptr;
			continue;
		}
		layout.set(plane);

		memcpy(m_audioData[plane].data(), audio->data[plane], audio->frames * sizeof(float_t));
		m_audioOutput.data[plane] = (uint8_t*)m_audioData[plane].data();
	}
	m_audioOutput.format = aoi->format;
	m_audioOutput.frames = audio->frames;
	m_audioOutput.timestamp = audio->timestamp;
	m_audioOutput.samples_per_sec = aoi->samples_per_sec;
	m_audioOutput.speakers = aoi->speakers;

	m_haveAudioOutput = true;
	m_audioNotify.notify_all();
}

void Source::Mirror::audio_output_cb() {
	std::unique_lock<std::mutex> ulock(m_audioLock);

	while (!m_killAudioThread) {
		if (m_haveAudioOutput) {
			obs_source_output_audio(m_source, &m_audioOutput);
			m_haveAudioOutput = false;
		}
		m_audioNotify.wait(ulock, [this]() { return m_haveAudioOutput || m_killAudioThread; });
	}
}

void Source::Mirror::enum_active_sources(obs_source_enum_proc_t enum_callback, void *param) {
	if (m_sceneitem) {
		enum_callback(m_source, obs_sceneitem_get_source(m_sceneitem), param);
	}
}
