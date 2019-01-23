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
#include <util/threading.h>
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
			snprintf(buf.data(), buf.size(), "%" PRIu32 "x%" PRIu32, obs_source_get_width(target),
					 obs_source_get_height(target));
			obs_data_set_string(data, P_SOURCE_SIZE, buf.data());
		} else {
			obs_data_set_string(data, P_SOURCE_SIZE, "0x0");
		}
		obs_source_release(target);
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
	obs_property_list_add_string(p, "", "");
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

void Source::Mirror::release_input()
{
	// Clear any references to the previous source.
	if (this->m_source_item) {
		obs_sceneitem_remove(this->m_source_item);
		this->m_source_item = nullptr;
	}
	{
		std::unique_lock<std::mutex> audio_lock(this->m_audio_lock);
		this->m_source_audio.reset();
	}
	this->m_source.reset();
}

void Source::Mirror::acquire_input(std::string source_name)
{
	// Acquire new reference to current source.
	obs_source_t* ref_source = obs_get_source_by_name(source_name.c_str());
	if (!ref_source) {
		// Early-Exit: Unable to find a source with this name, likely has been released.
#ifdef _DEBUG
		P_LOG_DEBUG("<Source Mirror:%s> Unable to find target source '%s'.", obs_source_get_name(this->m_self),
					source_name.c_str());
#endif
		return;
	} else if (ref_source == this->m_self) {
		// Early-Exit: Attempted self-mirror (recursion).
#ifdef _DEBUG
		P_LOG_DEBUG("<Source Mirror:%s> Attempted to mirror self.", obs_source_get_name(this->m_self));
#endif
		obs_source_release(ref_source);
		return;
	}

	std::shared_ptr<obs::source> new_source = std::make_shared<obs::source>(ref_source, true, false);

	// It looks like everything is in order, so continue now.
	this->m_source_item = obs_scene_add(obs_scene_from_source(this->m_scene->get()), new_source->get());
	if (!this->m_source_item) {
		// Late-Exit: OBS detected something bad, so no further action will be taken.
#ifdef _DEBUG
		P_LOG_DEBUG("<Source Mirror:%s> Attempted recursion with scene '%s'.", obs_source_get_name(this->m_self),
					source_name.c_str());
#endif
		return;
	}

	// If everything worked fine, we now set everything up.
	this->m_source = std::move(new_source);
	this->m_source->events.rename += std::bind(&Source::Mirror::on_source_rename, this, std::placeholders::_1,
											   std::placeholders::_2, std::placeholders::_3);
	try {
		// Audio
		this->m_source_audio = std::make_shared<obs::audio_capture>(this->m_source);
		this->m_source_audio->on.data += std::bind(&Source::Mirror::audio_capture_cb, this, std::placeholders::_1,
												   std::placeholders::_2, std::placeholders::_3);
	} catch (...) {
		P_LOG_ERROR("<Source Mirror:%s> Unexpected error during registering audio callback for '%s'.",
					source_name.c_str());
	}
}

Source::Mirror::Mirror(obs_data_t* data, obs_source_t* src)
	: m_self(src), m_active(false), m_tick(0), m_rescale_enabled(false), m_rescale_effect(nullptr),
	  m_rescale_keep_orig_size(false), m_width(1), m_height(1), m_audio_enabled(false), m_audio_kill_thread(false),
	  m_audio_have_output(false), m_source_item(nullptr)
{
	m_active = true;
}

Source::Mirror::~Mirror()
{
	release_input();

	// Finalize Audio Rendering
	this->m_audio_kill_thread = true;
	this->m_audio_notify.notify_all();
	if (this->m_audio_thread.joinable()) {
		this->m_audio_thread.join();
	}

	// Finalize Rescaling
	this->m_rescale_effect = nullptr;
	this->m_sampler.reset();
	this->m_rescale_rt.reset();

	// Finalize Video Rendering
	this->m_scene_texture.reset();
	this->m_scene.reset();
}

uint32_t Source::Mirror::get_width()
{
	if (this->m_rescale_enabled && this->m_width > 0 && !this->m_rescale_keep_orig_size) {
		return this->m_width;
	}
	obs_source_t* source = obs_sceneitem_get_source(this->m_source_item);
	if (source) {
		return obs_source_get_width(source);
	}
	return 1;
}

uint32_t Source::Mirror::get_height()
{
	if (this->m_rescale_enabled && this->m_height > 0 && !this->m_rescale_keep_orig_size)
		return this->m_height;
	obs_source_t* source = obs_sceneitem_get_source(this->m_source_item);
	if (source) {
		return obs_source_get_height(source);
	}
	return 1;
}

void Source::Mirror::update(obs_data_t* data)
{
	if (!this->m_scene) {
		// Initialize Video Rendering
		this->m_scene = std::make_shared<obs::source>(
			obs_scene_get_source(obs_scene_create_private("Source Mirror Internal Scene")));
		this->m_scene_texture = std::make_shared<gfx::source_texture>(
			this->m_scene, std::make_shared<obs::source>(this->m_self, false, false));

		// Initialize Rescaling
		this->m_rescale_rt     = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
		this->m_sampler        = std::make_shared<gs::sampler>();
		this->m_rescale_effect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_DEFAULT);

		// Initialize Audio Rendering
		this->m_audio_data.resize(MAX_AUDIO_CHANNELS);
		for (size_t idx = 0; idx < this->m_audio_data.size(); idx++) {
			this->m_audio_data[idx].resize(AUDIO_OUTPUT_FRAMES);
		}
		this->m_audio_thread = std::thread(std::bind(&Source::Mirror::audio_output_cb, this));
	}

	{ // User changed the source we are tracking.
		release_input();
		this->m_source_name = obs_data_get_string(data, P_SOURCE);
		if (this->m_source_name.length() > 0) {
			acquire_input(this->m_source_name);
		}
	}

	// Audio
	this->m_audio_enabled = obs_data_get_bool(data, P_SOURCE_AUDIO);

	// Rescaling
	this->m_rescale_enabled = obs_data_get_bool(data, P_SCALING);
	if (this->m_rescale_enabled) { // Parse rescaling settings.
		uint32_t width, height;

		// Read value.
		const char* size = obs_data_get_string(data, P_SCALING_SIZE);
		const char* xpos = strchr(size, 'x');
		if (xpos != nullptr) {
			// Width
			width = strtoul(size, nullptr, 10);
			if (errno == ERANGE || width == 0) {
				this->m_rescale_enabled = false;
				this->m_width           = 1;
			} else {
				this->m_width = width;
			}

			height = strtoul(xpos + 1, nullptr, 10);
			if (errno == ERANGE || height == 0) {
				this->m_rescale_enabled = false;
				this->m_height          = 1;
			} else {
				this->m_height = height;
			}
		} else {
			this->m_rescale_enabled = false;
			this->m_width           = 1;
			this->m_height          = 1;
		}

		ScalingMethod scaler = (ScalingMethod)obs_data_get_int(data, P_SCALING_METHOD);
		switch (scaler) {
		case ScalingMethod::Point:
		default:
			this->m_rescale_effect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_DEFAULT);
			this->m_sampler->set_filter(GS_FILTER_POINT);
			break;
		case ScalingMethod::Bilinear:
			this->m_rescale_effect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_DEFAULT);
			this->m_sampler->set_filter(GS_FILTER_LINEAR);
			break;
		case ScalingMethod::BilinearLowRes:
			this->m_rescale_effect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_BILINEAR_LOWRES);
			this->m_sampler->set_filter(GS_FILTER_LINEAR);
			break;
		case ScalingMethod::Bicubic:
			this->m_rescale_effect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_BICUBIC);
			this->m_sampler->set_filter(GS_FILTER_LINEAR);
			break;
		case ScalingMethod::Lanczos:
			this->m_rescale_effect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_LANCZOS);
			this->m_sampler->set_filter(GS_FILTER_LINEAR);
			break;
		}

		this->m_rescale_keep_orig_size = obs_data_get_bool(data, P_SCALING_TRANSFORMKEEPORIGINAL);
	}
}

void Source::Mirror::activate()
{
	this->m_active = true;

	// No source, delayed acquire.
	if (!this->m_source_item) {
		this->acquire_input(this->m_source_name.c_str());
	}
}

void Source::Mirror::deactivate()
{
	this->m_active = false;
}

static inline void mix_audio(float* p_out, float* p_in, size_t pos, size_t count)
{
	float* out = p_out;
	float* in  = p_in + pos;
	float* end = in + count;

	while (in < end)
		*(out++) += *(in++);
}

void Source::Mirror::video_tick(float time)
{
	this->m_tick += time;
	if (this->m_tick > 0.1f) {
		this->m_tick -= 0.1f;

		// No source, delayed acquire.
		if (!this->m_source_item) {
			this->acquire_input(this->m_source_name.c_str());
		}
	}
}

void Source::Mirror::video_render(gs_effect_t*)
{
	if ((this->m_width == 0) || (this->m_height == 0) || !this->m_source_item) {
		return;
	}

	if (this->m_rescale_enabled && this->m_width > 0 && this->m_height > 0 && this->m_rescale_effect) {
		// Get Size of source.
		uint32_t sw, sh;
		sw = this->m_source->width();
		sh = this->m_source->height();

		vec2 bounds;
		bounds.x = float_t(sw);
		bounds.y = float_t(sh);
		obs_sceneitem_set_bounds(this->m_source_item, &bounds);

		// Store original Source Texture
		std::shared_ptr<gs::texture> tex;
		try {
			tex = this->m_scene_texture->render(sw, sh);
		} catch (...) {
			return;
		}

		gs_eparam_t* scale_param = gs_effect_get_param_by_name(this->m_rescale_effect, "base_dimension_i");
		if (scale_param) {
			vec2 base_res_i;
			vec2_set(&base_res_i, 1.0f / float_t(sw), 1.0f / float_t(sh));
			gs_effect_set_vec2(scale_param, &base_res_i);
		}

		if (this->m_rescale_keep_orig_size) {
			{
				auto op = m_rescale_rt->render(this->m_width, this->m_height);
				gs_ortho(0, float_t(this->m_width), 0, float_t(this->m_height), 0, 1);

				vec4 black;
				vec4_zero(&black);
				gs_clear(GS_CLEAR_COLOR, &black, 0, 0);

				while (gs_effect_loop(this->m_rescale_effect, "Draw")) {
					gs_eparam_t* image = gs_effect_get_param_by_name(this->m_rescale_effect, "image");
					gs_effect_set_next_sampler(image, this->m_sampler->get_object());
					obs_source_draw(tex->get_object(), 0, 0, this->m_width, this->m_height, false);
				}
			}
			while (gs_effect_loop(obs_get_base_effect(OBS_EFFECT_DEFAULT), "Draw")) {
				gs_eparam_t* image = gs_effect_get_param_by_name(obs_get_base_effect(OBS_EFFECT_DEFAULT), "image");
				gs_effect_set_next_sampler(image, this->m_sampler->get_object());
				obs_source_draw(this->m_rescale_rt->get_object(), 0, 0, sw, sh, false);
			}
		} else {
			while (gs_effect_loop(this->m_rescale_effect, "Draw")) {
				gs_eparam_t* image = gs_effect_get_param_by_name(this->m_rescale_effect, "image");
				gs_effect_set_next_sampler(image, this->m_sampler->get_object());
				obs_source_draw(tex->get_object(), 0, 0, this->m_width, this->m_height, false);
			}
		}
	} else {
		obs_source_video_render(this->m_scene_texture->get_object());
	}
}

void Source::Mirror::audio_capture_cb(std::shared_ptr<obs::source> source, audio_data const* const audio, bool)
{
	std::unique_lock<std::mutex> ulock(this->m_audio_lock);
	if (!this->m_audio_enabled) {
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

	this->m_audio_have_output = true;
	this->m_audio_notify.notify_all();
}

void Source::Mirror::audio_output_cb()
{
	std::unique_lock<std::mutex> ulock(this->m_audio_lock);
	os_set_thread_name("Source Mirror Audio Thread");

	while (!this->m_audio_kill_thread) {
		if (this->m_audio_have_output) {
			std::unique_lock<std::mutex> ulock(this->m_audio_lock);
			obs_source_output_audio(this->m_self, &this->m_audio_output);
			this->m_audio_have_output = false;
		}
		this->m_audio_notify.wait(ulock, [this]() { return this->m_audio_have_output || this->m_audio_kill_thread; });
	}
}

void Source::Mirror::enum_active_sources(obs_source_enum_proc_t enum_callback, void* param)
{
	if (this->m_scene) {
		enum_callback(this->m_self, this->m_scene->get(), param);
	}
}

void Source::Mirror::load(obs_data_t* data)
{
	this->update(data);
}

void Source::Mirror::save(obs_data_t* data)
{
	if (this->m_source_item) {
		obs_data_set_string(data, P_SOURCE, obs_source_get_name(m_source->get()));
	}
}

void Source::Mirror::on_source_rename(obs::source* source, std::string, std::string)
{
	obs_data_t* ref = obs_source_get_settings(this->m_self);
	obs_data_set_string(ref, P_SOURCE, obs_source_get_name(source->get()));
	obs_source_update(this->m_self, ref);
	obs_data_release(ref);
}
