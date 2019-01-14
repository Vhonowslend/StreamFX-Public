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

#include "source-mirror.hpp"
#include <bitset>
#include <cstring>
#include <functional>
#include <memory>
#include <vector>
#include "obs-tools.hpp"
#include "strings.hpp"

// OBS
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <media-io/audio-io.h>
#include <obs-config.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#define S_SOURCE_MIRROR "Source.Mirror"
#define P_SOURCE "Source.Mirror.Source"
#define P_SOURCE_SIZE "Source.Mirror.Source.Size"
#define P_SOURCE_AUDIO "Source.Mirror.Source.Audio"
#define P_SCALING "Source.Mirror.Scaling"
#define P_SCALING_METHOD "Source.Mirror.Scaling.Method"
#define P_SCALING_METHOD_POINT "Source.Mirror.Scaling.Method.Point"
#define P_SCALING_METHOD_BILINEAR "Source.Mirror.Scaling.Method.Bilinear"
#define P_SCALING_METHOD_BILINEARLOWRES "Source.Mirror.Scaling.Method.BilinearLowRes"
#define P_SCALING_METHOD_BICUBIC "Source.Mirror.Scaling.Method.Bicubic"
#define P_SCALING_METHOD_LANCZOS "Source.Mirror.Scaling.Method.Lanczos"
#define P_SCALING_SIZE "Source.Mirror.Scaling.Size"
#define P_SCALING_TRANSFORMKEEPORIGINAL "Source.Mirror.Scaling.TransformKeepOriginal"

enum class ScalingMethod : int64_t {
	Point,
	Bilinear,
	BilinearLowRes,
	Bicubic,
	Lanczos,
};

// Initializer & Finalizer
Source::MirrorAddon* sourceMirrorInstance;
INITIALIZER(SourceMirrorInit)
{
	initializerFunctions.push_back([] { sourceMirrorInstance = new Source::MirrorAddon(); });
	finalizerFunctions.push_back([] { delete sourceMirrorInstance; });
}

Source::MirrorAddon::MirrorAddon()
{
	memset(&osi, 0, sizeof(obs_source_info));
	osi.id           = "obs-stream-effects-source-mirror";
	osi.type         = OBS_SOURCE_TYPE_INPUT;
	osi.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_AUDIO | OBS_SOURCE_CUSTOM_DRAW;

	osi.get_name            = get_name;
	osi.get_defaults        = get_defaults;
	osi.get_properties      = get_properties;
	osi.get_width           = get_width;
	osi.get_height          = get_height;
	osi.create              = create;
	osi.destroy             = destroy;
	osi.update              = update;
	osi.activate            = activate;
	osi.deactivate          = deactivate;
	osi.video_tick          = video_tick;
	osi.video_render        = video_render;
	osi.enum_active_sources = enum_active_sources;
	osi.load                = load;
	osi.save                = save;

	obs_register_source(&osi);
}

Source::MirrorAddon::~MirrorAddon() {}

const char* Source::MirrorAddon::get_name(void*)
{
	return P_TRANSLATE(S_SOURCE_MIRROR);
}

void Source::MirrorAddon::get_defaults(obs_data_t* data)
{
	obs_data_set_default_string(data, P_SOURCE, "");
	obs_data_set_default_bool(data, P_SOURCE_AUDIO, false);
	obs_data_set_default_bool(data, P_SCALING, false);
	obs_data_set_default_string(data, P_SCALING_SIZE, "100x100");
	obs_data_set_default_int(data, P_SCALING_METHOD, (int64_t)ScalingMethod::Bilinear);
}

bool Source::MirrorAddon::modified_properties(obs_properties_t* pr, obs_property_t* p, obs_data_t* data)
{
	if (obs_properties_get(pr, P_SOURCE) == p) {
		obs_source_t* target = obs_get_source_by_name(obs_data_get_string(data, P_SOURCE));
		if (target) {
			std::vector<char> buf(256);
			snprintf(buf.data(), buf.size(), "%ldx%ld\0", obs_source_get_width(target), obs_source_get_height(target));
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

static bool UpdateSourceListCB(void* ptr, obs_source_t* src)
{
	obs_property_t* p = (obs_property_t*)ptr;
	obs_property_list_add_string(p, obs_source_get_name(src), obs_source_get_name(src));
	return true;
}

static void UpdateSourceList(obs_property_t* p)
{
	obs_property_list_clear(p);
	obs_enum_sources(UpdateSourceListCB, p);
#if LIBOBS_API_MAJOR_VER >= 23
	obs_enum_scenes(UpdateSourceListCB, p);
#endif
}

obs_properties_t* Source::MirrorAddon::get_properties(void*)
{
	obs_properties_t* pr = obs_properties_create();
	obs_property_t*   p  = nullptr;

	p = obs_properties_add_list(pr, P_SOURCE, P_TRANSLATE(P_SOURCE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
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

	p = obs_properties_add_list(pr, P_SCALING_METHOD, P_TRANSLATE(P_SCALING_METHOD), OBS_COMBO_TYPE_LIST,
								OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SCALING_METHOD)));
	obs_property_list_add_int(p, P_TRANSLATE(P_SCALING_METHOD_POINT), (int64_t)ScalingMethod::Point);
	obs_property_list_add_int(p, P_TRANSLATE(P_SCALING_METHOD_BILINEAR), (int64_t)ScalingMethod::Bilinear);
	obs_property_list_add_int(p, P_TRANSLATE(P_SCALING_METHOD_BILINEARLOWRES), (int64_t)ScalingMethod::BilinearLowRes);
	obs_property_list_add_int(p, P_TRANSLATE(P_SCALING_METHOD_BICUBIC), (int64_t)ScalingMethod::Bicubic);
	obs_property_list_add_int(p, P_TRANSLATE(P_SCALING_METHOD_LANCZOS), (int64_t)ScalingMethod::Lanczos);

	p = obs_properties_add_text(pr, P_SCALING_SIZE, P_TRANSLATE(P_SCALING_SIZE), OBS_TEXT_DEFAULT);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SCALING_SIZE)));

	p = obs_properties_add_bool(pr, P_SCALING_TRANSFORMKEEPORIGINAL, P_TRANSLATE(P_SCALING_TRANSFORMKEEPORIGINAL));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SCALING_TRANSFORMKEEPORIGINAL)));

	return pr;
}

void* Source::MirrorAddon::create(obs_data_t* data, obs_source_t* source)
{
	return new Source::Mirror(data, source);
}

void Source::MirrorAddon::destroy(void* p)
{
	if (p) {
		delete static_cast<Source::Mirror*>(p);
	}
}

uint32_t Source::MirrorAddon::get_width(void* p)
{
	if (p) {
		return static_cast<Source::Mirror*>(p)->get_width();
	}
	return 0;
}

uint32_t Source::MirrorAddon::get_height(void* p)
{
	if (p) {
		return static_cast<Source::Mirror*>(p)->get_height();
	}
	return 0;
}

void Source::MirrorAddon::update(void* p, obs_data_t* data)
{
	if (p) {
		static_cast<Source::Mirror*>(p)->update(data);
	}
}

void Source::MirrorAddon::activate(void* p)
{
	if (p) {
		static_cast<Source::Mirror*>(p)->activate();
	}
}

void Source::MirrorAddon::deactivate(void* p)
{
	if (p) {
		static_cast<Source::Mirror*>(p)->deactivate();
	}
}

void Source::MirrorAddon::video_tick(void* p, float t)
{
	if (p) {
		static_cast<Source::Mirror*>(p)->video_tick(t);
	}
}

void Source::MirrorAddon::video_render(void* p, gs_effect_t* ef)
{
	if (p) {
		static_cast<Source::Mirror*>(p)->video_render(ef);
	}
}

void Source::MirrorAddon::enum_active_sources(void* p, obs_source_enum_proc_t enum_callback, void* param)
{
	if (p) {
		static_cast<Source::Mirror*>(p)->enum_active_sources(enum_callback, param);
	}
}

void Source::MirrorAddon::load(void* p, obs_data_t* d)
{
	if (p) {
		static_cast<Source::Mirror*>(p)->load(d);
	}
}

void Source::MirrorAddon::save(void* p, obs_data_t* d)
{
	if (p) {
		static_cast<Source::Mirror*>(p)->save(d);
	}
}

Source::Mirror::Mirror(obs_data_t* data, obs_source_t* src)
{
	this->m_source              = src;
	this->m_rescale             = false;
	this->m_width               = 1;
	this->m_height              = this->m_width;
	this->m_render_target_scale = std::make_unique<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
	this->m_sampler             = std::make_shared<gs::sampler>();
	this->m_scaling_effect      = obs_get_base_effect(obs_base_effect::OBS_EFFECT_DEFAULT);

	this->m_audio_data.resize(MAX_AUDIO_CHANNELS);
	for (size_t idx = 0; idx < this->m_audio_data.size(); idx++) {
		this->m_audio_data[idx].resize(AUDIO_OUTPUT_FRAMES);
	}
	this->m_audio_thread = std::thread(std::bind(&Source::Mirror::audio_output_cb, this));

	update(data);
	m_active = true;
}

Source::Mirror::~Mirror()
{
	if (this->m_audio_capture) {
		this->m_audio_capture.reset();
	}
	this->m_kill_audio_thread = true;
	this->m_audio_notify.notify_all();
	if (this->m_audio_thread.joinable()) {
		this->m_audio_thread.join();
	}

	if (this->m_source_texture) {
		this->m_source_texture.reset();
	}
	if (this->m_scene) {
		obs_scene_release(this->m_scene);
	}
}

uint32_t Source::Mirror::get_width()
{
	if (this->m_rescale && this->m_width > 0 && !this->m_keep_original_size) {
		return this->m_width;
	}
	obs_source_t* source = obs_sceneitem_get_source(this->m_sceneitem);
	if (source) {
		return obs_source_get_width(source);
	}
	return 1;
}

uint32_t Source::Mirror::get_height()
{
	if (this->m_rescale && this->m_height > 0 && !this->m_keep_original_size)
		return this->m_height;
	obs_source_t* source = obs_sceneitem_get_source(this->m_sceneitem);
	if (source) {
		return obs_source_get_height(source);
	}
	return 1;
}

void Source::Mirror::update(obs_data_t* data)
{
	if (!this->m_scene && this->m_active) {
		this->m_scene          = obs_scene_create_private("localscene");
		this->m_scene_source   = std::make_shared<obs::source>(obs_scene_get_source(this->m_scene), false, false);
		this->m_source_texture = std::make_unique<gfx::source_texture>(
			this->m_scene_source, std::make_shared<obs::source>(this->m_source, false, false));
	}

	// Update selected source.
	const char* new_source_name = obs_data_get_string(data, P_SOURCE);
	if (new_source_name != m_source_name) {
		if (m_scene) {
			if (this->m_sceneitem) {
				this->m_audio_capture.reset();
				this->m_source_target.reset();
				obs_sceneitem_remove(this->m_sceneitem);
				this->m_sceneitem = nullptr;
			}
			obs_source_t* source = obs_get_source_by_name(new_source_name);
			if (source) {
				bool allow = true;
				if (source == m_source) {
					allow = false;
				}
				if (strcmp(obs_source_get_id(source), "scene") == 0) {
					if (obs::tools::scene_contains_source(obs_scene_from_source(source), this->m_source)) {
						allow = false;
					}
				}
				if (allow) {
					this->m_sceneitem = obs_scene_add(m_scene, source);
					if (this->m_sceneitem) {
						m_source_target = std::make_shared<obs::source>(source, true, true);
						m_source_target->events.rename.add(std::bind(&Source::Mirror::on_source_rename, this,
																	 std::placeholders::_1, std::placeholders::_2,
																	 std::placeholders::_3));
						m_source_target->events.destroy.add(
							std::bind(&Source::Mirror::on_source_destroy, this, std::placeholders::_1));
						try {
							this->m_audio_capture = std::make_unique<obs::audio_capture>(source);
							this->m_audio_capture->set_callback(std::bind(&Source::Mirror::audio_capture_cb, this,
																		  std::placeholders::_1, std::placeholders::_2,
																		  std::placeholders::_3));
						} catch (...) {
						}
						this->m_source_name = new_source_name;
					}
				}
				obs_source_release(source);
			}
		}
	}
	this->m_enable_audio = obs_data_get_bool(data, P_SOURCE_AUDIO);

	// Rescaling
	this->m_rescale = obs_data_get_bool(data, P_SCALING);
	if (this->m_rescale) { // Parse rescaling settings.
		uint32_t width, height;

		// Read value.
		const char* size = obs_data_get_string(data, P_SCALING_SIZE);
		const char* xpos = strchr(size, 'x');
		if (xpos != nullptr) {
			// Width
			width = strtoul(size, nullptr, 10);
			if (errno == ERANGE || width == 0) {
				this->m_rescale = false;
				this->m_width   = 1;
			} else {
				this->m_width = width;
			}

			height = strtoul(xpos + 1, nullptr, 10);
			if (errno == ERANGE || height == 0) {
				this->m_rescale = false;
				this->m_height  = 1;
			} else {
				this->m_height = height;
			}
		} else {
			this->m_rescale = false;
			this->m_width   = 1;
			this->m_height  = 1;
		}

		ScalingMethod scaler = (ScalingMethod)obs_data_get_int(data, P_SCALING_METHOD);
		switch (scaler) {
		case ScalingMethod::Point:
		default:
			this->m_scaling_effect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_DEFAULT);
			this->m_sampler->set_filter(GS_FILTER_POINT);
			break;
		case ScalingMethod::Bilinear:
			this->m_scaling_effect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_DEFAULT);
			this->m_sampler->set_filter(GS_FILTER_LINEAR);
			break;
		case ScalingMethod::BilinearLowRes:
			this->m_scaling_effect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_BILINEAR_LOWRES);
			this->m_sampler->set_filter(GS_FILTER_LINEAR);
			break;
		case ScalingMethod::Bicubic:
			this->m_scaling_effect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_BICUBIC);
			this->m_sampler->set_filter(GS_FILTER_LINEAR);
			break;
		case ScalingMethod::Lanczos:
			this->m_scaling_effect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_LANCZOS);
			this->m_sampler->set_filter(GS_FILTER_LINEAR);
			break;
		}

		this->m_keep_original_size = obs_data_get_bool(data, P_SCALING_TRANSFORMKEEPORIGINAL);
	}
}

void Source::Mirror::activate()
{
	this->m_active = true;
	if (!this->m_sceneitem) {
		obs_data_t* ref = obs_source_get_settings(this->m_source);
		update(ref);
		obs_data_release(ref);
	}
}

void Source::Mirror::deactivate()
{
	this->m_active = false;
}

static inline void mix_audio(float* p_out, float* p_in, size_t pos, size_t count)
{
	register float* out = p_out;
	register float* in  = p_in + pos;
	register float* end = in + count;

	while (in < end)
		*(out++) += *(in++);
}

void Source::Mirror::video_tick(float time)
{
	this->m_tick += time;

	if (!this->m_sceneitem) {
		if (this->m_tick > 0.1f) {
			obs_data_t* ref = obs_source_get_settings(this->m_source);
			update(ref);
			obs_data_release(ref);
			this->m_tick -= 0.1f;
		}
	}
}

void Source::Mirror::video_render(gs_effect_t*)
{
	if ((this->m_width == 0) || (this->m_height == 0) || !this->m_source_texture || !this->m_scene
		|| !this->m_sceneitem) {
		return;
	}

	if (this->m_rescale && this->m_width > 0 && this->m_height > 0 && this->m_scaling_effect) {
		// Get Size of source.
		obs_source_t* source = obs_sceneitem_get_source(this->m_sceneitem);

		uint32_t sw, sh;
		sw = obs_source_get_width(source);
		sh = obs_source_get_height(source);

		vec2 bounds;
		bounds.x = sw;
		bounds.y = sh;
		obs_sceneitem_set_bounds(this->m_sceneitem, &bounds);

		// Store original Source Texture
		std::shared_ptr<gs::texture> tex;
		try {
			tex = this->m_source_texture->render(sw, sh);
		} catch (...) {
			return;
		}

		gs_eparam_t* scale_param = gs_effect_get_param_by_name(this->m_scaling_effect, "base_dimension_i");
		if (scale_param) {
			struct vec2 base_res_i = {1.0f / float_t(sw), 1.0f / float_t(sh)};
			gs_effect_set_vec2(scale_param, &base_res_i);
		}

		if (this->m_keep_original_size) {
			{
				auto op = m_render_target_scale->render(this->m_width, this->m_height);
				gs_ortho(0, float_t(this->m_width), 0, float_t(this->m_height), 0, 1);

				vec4 black;
				vec4_zero(&black);
				gs_clear(GS_CLEAR_COLOR, &black, 0, 0);

				while (gs_effect_loop(this->m_scaling_effect, "Draw")) {
					gs_eparam_t* image = gs_effect_get_param_by_name(this->m_scaling_effect, "image");
					gs_effect_set_next_sampler(image, this->m_sampler->get_object());
					obs_source_draw(tex->get_object(), 0, 0, this->m_width, this->m_height, false);
				}
			}
			while (gs_effect_loop(obs_get_base_effect(OBS_EFFECT_DEFAULT), "Draw")) {
				gs_eparam_t* image = gs_effect_get_param_by_name(obs_get_base_effect(OBS_EFFECT_DEFAULT), "image");
				gs_effect_set_next_sampler(image, this->m_sampler->get_object());
				obs_source_draw(this->m_render_target_scale->get_object(), 0, 0, sw, sh, false);
			}
		} else {
			while (gs_effect_loop(this->m_scaling_effect, "Draw")) {
				gs_eparam_t* image = gs_effect_get_param_by_name(this->m_scaling_effect, "image");
				gs_effect_set_next_sampler(image, this->m_sampler->get_object());
				obs_source_draw(tex->get_object(), 0, 0, this->m_width, this->m_height, false);
			}
		}
	} else {
		obs_source_video_render(this->m_source_texture->get_object());
	}
}

void Source::Mirror::audio_capture_cb(void*, const audio_data* audio, bool)
{
	std::unique_lock<std::mutex> ulock(this->m_audio_lock);
	if (!this->m_enable_audio) {
		return;
	}

	audio_t* aud = obs_get_audio();
	if (!aud) {
		return;
	}
	audio_output_info const* aoi = audio_output_get_info(aud);
	if (!aoi) {
		return;
	}

	std::bitset<8> layout;
	for (size_t plane = 0; plane < MAX_AV_PLANES; plane++) {
		float* samples = (float*)audio->data[plane];
		if (!samples) {
			this->m_audio_output.data[plane] = nullptr;
			continue;
		}
		layout.set(plane);

		memcpy(this->m_audio_data[plane].data(), audio->data[plane], audio->frames * sizeof(float_t));
		this->m_audio_output.data[plane] = reinterpret_cast<uint8_t*>(this->m_audio_data[plane].data());
	}
	this->m_audio_output.format          = aoi->format;
	this->m_audio_output.frames          = audio->frames;
	this->m_audio_output.timestamp       = audio->timestamp;
	this->m_audio_output.samples_per_sec = aoi->samples_per_sec;
	this->m_audio_output.speakers        = aoi->speakers;

	this->m_have_audio_output = true;
	this->m_audio_notify.notify_all();
}

void Source::Mirror::audio_output_cb()
{
	std::unique_lock<std::mutex> ulock(this->m_audio_lock);

	while (!this->m_kill_audio_thread) {
		if (this->m_have_audio_output) {
			obs_source_output_audio(this->m_source, &this->m_audio_output);
			this->m_have_audio_output = false;
		}
		this->m_audio_notify.wait(ulock, [this]() { return this->m_have_audio_output || this->m_kill_audio_thread; });
	}
}

void Source::Mirror::enum_active_sources(obs_source_enum_proc_t enum_callback, void* param)
{
	if (this->m_sceneitem) {
		enum_callback(this->m_source, obs_sceneitem_get_source(this->m_sceneitem), param);
	}
	if (this->m_scene) {
		enum_callback(this->m_source, obs_scene_get_source(this->m_scene), param);
	}
}

void Source::Mirror::load(obs_data_t* data) {}

void Source::Mirror::save(obs_data_t* data)
{
	if (this->m_sceneitem) {
		obs_data_set_string(data, P_SOURCE, obs_source_get_name(obs_sceneitem_get_source(this->m_sceneitem)));
	}
}

void Source::Mirror::on_source_rename(obs::source* source, std::string new_name, std::string old_name) {
	obs_data_t* ref = obs_source_get_settings(this->m_source);
	obs_data_set_string(ref, P_SOURCE, obs_source_get_name(source->get()));
	obs_source_update(this->m_source, ref);
	obs_data_release(ref);
}

void Source::Mirror::on_source_destroy(obs::source* source)
{
	// This is an odd case. If you hit this, be prepared for all kinds of broken things.
	this->m_source_target->clear();
	this->m_source_target.reset();
	this->m_audio_capture.reset();
	obs_sceneitem_remove(this->m_sceneitem);
	this->m_sceneitem = nullptr;

	obs_data_t* ref = obs_source_get_settings(this->m_source);
	obs_data_set_string(ref, P_SOURCE, "");
	obs_source_update(this->m_source, ref);
	obs_data_release(ref);
}
