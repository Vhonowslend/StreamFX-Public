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
#define P_SCALING_METHOD_BICUBIC "Source.Mirror.Scaling.Method.Bicubic"
#define P_SCALING_METHOD_LANCZOS "Source.Mirror.Scaling.Method.Lanczos"
#define P_SCALING_SIZE "Source.Mirror.Scaling.Size"
#define P_SCALING_TRANSFORMKEEPORIGINAL "Source.Mirror.Scaling.TransformKeepOriginal"
#define P_SCALING_BOUNDS "Source.Mirror.Scaling.Bounds"
#define P_SCALING_BOUNDS_STRETCH "Source.Mirror.Scaling.Bounds.Stretch"
#define P_SCALING_BOUNDS_FIT "Source.Mirror.Scaling.Bounds.Fit"
#define P_SCALING_BOUNDS_FILL "Source.Mirror.Scaling.Bounds.Fill"
#define P_SCALING_BOUNDS_FILLWIDTH "Source.Mirror.Scaling.Bounds.FillWidth"
#define P_SCALING_BOUNDS_FILLHEIGHT "Source.Mirror.Scaling.Bounds.FillHeight"

// Initializer & Finalizer
INITIALIZER(SourceMirrorInit)
{
	initializerFunctions.push_back([] { source::mirror::mirror_factory::initialize(); });
	finalizerFunctions.push_back([] { source::mirror::mirror_factory::finalize(); });
}

static std::shared_ptr<source::mirror::mirror_factory> factory_instance = nullptr;

void source::mirror::mirror_factory::initialize()
{
	factory_instance = std::make_shared<source::mirror::mirror_factory>();
}

void source::mirror::mirror_factory::finalize()
{
	factory_instance.reset();
}

std::shared_ptr<source::mirror::mirror_factory> source::mirror::mirror_factory::get()
{
	return factory_instance;
}

source::mirror::mirror_factory::mirror_factory()
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

source::mirror::mirror_factory::~mirror_factory() {}

const char* source::mirror::mirror_factory::get_name(void*)
{
	return P_TRANSLATE(S_SOURCE_MIRROR);
}

void source::mirror::mirror_factory::get_defaults(obs_data_t* data)
{
	obs_data_set_default_string(data, P_SOURCE, "");
	obs_data_set_default_bool(data, P_SOURCE_AUDIO, false);
	obs_data_set_default_bool(data, P_SCALING, false);
	obs_data_set_default_string(data, P_SCALING_SIZE, "100x100");
	obs_data_set_default_int(data, P_SCALING_METHOD, (int64_t)obs_scale_type::OBS_SCALE_BILINEAR);
	obs_data_set_default_bool(data, P_SCALING_TRANSFORMKEEPORIGINAL, false);
	obs_data_set_default_int(data, P_SCALING_BOUNDS, (int64_t)obs_bounds_type::OBS_BOUNDS_STRETCH);
}

bool source::mirror::mirror_factory::modified_properties(obs_properties_t* pr, obs_property_t* p, obs_data_t* data)
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
		obs_property_set_visible(obs_properties_get(pr, P_SCALING_BOUNDS), show);
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

obs_properties_t* source::mirror::mirror_factory::get_properties(void*)
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
	obs_property_list_add_int(p, P_TRANSLATE(P_SCALING_METHOD_POINT), (int64_t)obs_scale_type::OBS_SCALE_POINT);
	obs_property_list_add_int(p, P_TRANSLATE(P_SCALING_METHOD_BILINEAR), (int64_t)obs_scale_type::OBS_SCALE_BILINEAR);
	obs_property_list_add_int(p, P_TRANSLATE(P_SCALING_METHOD_BICUBIC), (int64_t)obs_scale_type::OBS_SCALE_BICUBIC);
	obs_property_list_add_int(p, P_TRANSLATE(P_SCALING_METHOD_LANCZOS), (int64_t)obs_scale_type::OBS_SCALE_LANCZOS);

	p = obs_properties_add_text(pr, P_SCALING_SIZE, P_TRANSLATE(P_SCALING_SIZE), OBS_TEXT_DEFAULT);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SCALING_SIZE)));

	p = obs_properties_add_bool(pr, P_SCALING_TRANSFORMKEEPORIGINAL, P_TRANSLATE(P_SCALING_TRANSFORMKEEPORIGINAL));
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SCALING_TRANSFORMKEEPORIGINAL)));

	p = obs_properties_add_list(pr, P_SCALING_BOUNDS, P_TRANSLATE(P_SCALING_BOUNDS), OBS_COMBO_TYPE_LIST,
								OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(P_SCALING_BOUNDS)));
	obs_property_list_add_int(p, P_TRANSLATE(P_SCALING_BOUNDS_STRETCH), (int64_t)obs_bounds_type::OBS_BOUNDS_STRETCH);
	obs_property_list_add_int(p, P_TRANSLATE(P_SCALING_BOUNDS_FIT), (int64_t)obs_bounds_type::OBS_BOUNDS_SCALE_INNER);
	obs_property_list_add_int(p, P_TRANSLATE(P_SCALING_BOUNDS_FILL), (int64_t)obs_bounds_type::OBS_BOUNDS_SCALE_OUTER);
	obs_property_list_add_int(p, P_TRANSLATE(P_SCALING_BOUNDS_FILLWIDTH),
							  (int64_t)obs_bounds_type::OBS_BOUNDS_SCALE_TO_WIDTH);
	obs_property_list_add_int(p, P_TRANSLATE(P_SCALING_BOUNDS_FILLHEIGHT),
							  (int64_t)obs_bounds_type::OBS_BOUNDS_SCALE_TO_HEIGHT);

	return pr;
}

void* source::mirror::mirror_factory::create(obs_data_t* data, obs_source_t* source)
{
	return new source::mirror::mirror_instance(data, source);
}

void source::mirror::mirror_factory::destroy(void* p)
{
	if (p) {
		delete static_cast<source::mirror::mirror_instance*>(p);
	}
}

uint32_t source::mirror::mirror_factory::get_width(void* p)
{
	if (p) {
		return static_cast<source::mirror::mirror_instance*>(p)->get_width();
	}
	return 0;
}

uint32_t source::mirror::mirror_factory::get_height(void* p)
{
	if (p) {
		return static_cast<source::mirror::mirror_instance*>(p)->get_height();
	}
	return 0;
}

void source::mirror::mirror_factory::update(void* p, obs_data_t* data)
{
	if (p) {
		static_cast<source::mirror::mirror_instance*>(p)->update(data);
	}
}

void source::mirror::mirror_factory::activate(void* p)
{
	if (p) {
		static_cast<source::mirror::mirror_instance*>(p)->activate();
	}
}

void source::mirror::mirror_factory::deactivate(void* p)
{
	if (p) {
		static_cast<source::mirror::mirror_instance*>(p)->deactivate();
	}
}

void source::mirror::mirror_factory::video_tick(void* p, float t)
{
	if (p) {
		static_cast<source::mirror::mirror_instance*>(p)->video_tick(t);
	}
}

void source::mirror::mirror_factory::video_render(void* p, gs_effect_t* ef)
{
	if (p) {
		static_cast<source::mirror::mirror_instance*>(p)->video_render(ef);
	}
}

void source::mirror::mirror_factory::enum_active_sources(void* p, obs_source_enum_proc_t enum_callback, void* param)
{
	if (p) {
		static_cast<source::mirror::mirror_instance*>(p)->enum_active_sources(enum_callback, param);
	}
}

void source::mirror::mirror_factory::load(void* p, obs_data_t* d)
{
	if (p) {
		static_cast<source::mirror::mirror_instance*>(p)->load(d);
	}
}

void source::mirror::mirror_factory::save(void* p, obs_data_t* d)
{
	if (p) {
		static_cast<source::mirror::mirror_instance*>(p)->save(d);
	}
}

void source::mirror::mirror_instance::release_input()
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

void source::mirror::mirror_instance::acquire_input(std::string source_name)
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
	this->m_source->events.rename += std::bind(&source::mirror::mirror_instance::on_source_rename, this,
											   std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	try {
		// Audio
		this->m_source_audio = std::make_shared<obs::audio_capture>(this->m_source);
		this->m_source_audio->on.data += std::bind(&source::mirror::mirror_instance::audio_capture_cb, this,
												   std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	} catch (...) {
		P_LOG_ERROR("<Source Mirror:%s> Unexpected error during registering audio callback for '%s'.",
					source_name.c_str());
	}
}

source::mirror::mirror_instance::mirror_instance(obs_data_t* data, obs_source_t* src)
	: m_self(src), m_active(true), m_tick(0), m_scene_rendered(false), m_rescale_enabled(false),
	  m_rescale_keep_orig_size(false), m_rescale_width(1), m_rescale_height(1),
	  m_rescale_type(obs_scale_type::OBS_SCALE_BICUBIC), m_rescale_bounds(obs_bounds_type::OBS_BOUNDS_STRETCH),
	  m_audio_enabled(false), m_audio_kill_thread(false), m_audio_have_output(false), m_source_item(nullptr)
{
	// Initialize Video Rendering
	this->m_scene =
		std::make_shared<obs::source>(obs_scene_get_source(obs_scene_create_private("Source Mirror Internal Scene")));
	this->m_scene_texture_renderer =
		std::make_shared<gfx::source_texture>(this->m_scene, std::make_shared<obs::source>(this->m_self, false, false));

	// Initialize Audio Rendering
	this->m_audio_data.resize(MAX_AUDIO_CHANNELS);
	for (size_t idx = 0; idx < this->m_audio_data.size(); idx++) {
		this->m_audio_data[idx].resize(AUDIO_OUTPUT_FRAMES);
	}
	this->m_audio_thread = std::thread(std::bind(&source::mirror::mirror_instance::audio_output_cb, this));
}

source::mirror::mirror_instance::~mirror_instance()
{
	release_input();

	// Finalize Audio Rendering
	this->m_audio_kill_thread = true;
	this->m_audio_notify.notify_all();
	if (this->m_audio_thread.joinable()) {
		this->m_audio_thread.join();
	}

	// Finalize Video Rendering
	this->m_scene_texture_renderer.reset();
	this->m_scene.reset();
}

uint32_t source::mirror::mirror_instance::get_width()
{
	if (this->m_rescale_enabled && this->m_rescale_width > 0 && !this->m_rescale_keep_orig_size) {
		return this->m_rescale_width;
	}
	obs_source_t* source = obs_sceneitem_get_source(this->m_source_item);
	if (source) {
		return obs_source_get_width(source);
	}
	return 1;
}

uint32_t source::mirror::mirror_instance::get_height()
{
	if (this->m_rescale_enabled && this->m_rescale_height > 0 && !this->m_rescale_keep_orig_size)
		return this->m_rescale_height;
	obs_source_t* source = obs_sceneitem_get_source(this->m_source_item);
	if (source) {
		return obs_source_get_height(source);
	}
	return 1;
}

void source::mirror::mirror_instance::update(obs_data_t* data)
{
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
				this->m_rescale_width   = 1;
			} else {
				this->m_rescale_width = width;
			}

			height = strtoul(xpos + 1, nullptr, 10);
			if (errno == ERANGE || height == 0) {
				this->m_rescale_enabled = false;
				this->m_rescale_height  = 1;
			} else {
				this->m_rescale_height = height;
			}
		} else {
			this->m_rescale_enabled = false;
			this->m_rescale_width   = 1;
			this->m_rescale_height  = 1;
		}

		this->m_rescale_keep_orig_size = obs_data_get_bool(data, P_SCALING_TRANSFORMKEEPORIGINAL);
		this->m_rescale_type           = (obs_scale_type)obs_data_get_int(data, P_SCALING_METHOD);
		this->m_rescale_bounds         = (obs_bounds_type)obs_data_get_int(data, P_SCALING_BOUNDS);
	}
}

void source::mirror::mirror_instance::activate()
{
	this->m_active = true;

	// No source, delayed acquire.
	if (!this->m_source_item) {
		this->acquire_input(this->m_source_name.c_str());
	}
}

void source::mirror::mirror_instance::deactivate()
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

void source::mirror::mirror_instance::video_tick(float time)
{
	this->m_tick += time;
	if (this->m_tick > 0.1f) {
		this->m_tick -= 0.1f;

		// No source, delayed acquire.
		if (!this->m_source_item) {
			this->acquire_input(this->m_source_name.c_str());
		}
	}

	// Update Scene Item Boundaries
	if (this->m_source_item) {
		obs_transform_info info;
		obs_sceneitem_get_info(this->m_source_item, &info);

		info.pos.x       = 0;
		info.pos.y       = 0;
		info.rot         = 0;
		info.scale.x     = 1.f;
		info.scale.y     = 1.f;
		info.bounds.x    = float_t(this->m_source->width());
		info.bounds.y    = float_t(this->m_source->height());
		info.bounds_type = obs_bounds_type::OBS_BOUNDS_STRETCH;

		if (this->m_rescale_enabled) {
			info.bounds.x    = float_t(this->m_rescale_width);
			info.bounds.y    = float_t(this->m_rescale_height);
			info.bounds_type = this->m_rescale_bounds;
		}
		obs_sceneitem_set_info(this->m_source_item, &info);
		obs_sceneitem_force_update_transform(this->m_source_item);

		obs_sceneitem_set_scale_filter(this->m_source_item, this->m_rescale_enabled ? this->m_rescale_type
																					: obs_scale_type::OBS_SCALE_POINT);
	}

	m_scene_rendered = false;
}

void source::mirror::mirror_instance::video_render(gs_effect_t* effect)
{
	if ((this->m_rescale_width == 0) || (this->m_rescale_height == 0) || !this->m_source_item
		|| !this->m_scene_texture_renderer) {
		return;
	}

	// Don't bother rendering sources that aren't video.
	if (obs_source_get_flags(this->m_source->get()) & OBS_SOURCE_VIDEO) {
		return;
	}

	// Only re-render the scene if there was a video_tick, saves GPU cycles.
	if (!m_scene_rendered) {
		// Override render size if rescaling is enabled.
		uint32_t render_width  = this->m_source->width();
		uint32_t render_height = this->m_source->height();
		if (m_rescale_enabled) {
			render_width  = m_rescale_width;
			render_height = m_rescale_height;
		}

		try {
			m_scene_texture  = this->m_scene_texture_renderer->render(render_width, render_height);
			m_scene_rendered = true;
		} catch (...) {
			// If we fail to render the source, just render nothing.
			return;
		}
	}

	// Use default effect unless we are provided a different effect.
	if (!effect) {
		effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);
	}

	// Render the cached scene texture.
	gs_effect_set_texture(gs_effect_get_param_by_name(effect, "image"), m_scene_texture->get_object());
	while (gs_effect_loop(effect, "Draw")) {
		gs_draw_sprite(m_scene_texture->get_object(), 0, this->get_width(), this->get_height());
	}
}

void source::mirror::mirror_instance::audio_capture_cb(std::shared_ptr<obs::source> source,
													   audio_data const* const      audio, bool)
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

void source::mirror::mirror_instance::audio_output_cb()
{
	std::unique_lock<std::mutex> ulock(this->m_audio_lock);

	while (!this->m_audio_kill_thread) {
		if (this->m_audio_have_output) {
			std::unique_lock<std::mutex> ulock(this->m_audio_lock);
			obs_source_output_audio(this->m_self, &this->m_audio_output);
			this->m_audio_have_output = false;
		}
		this->m_audio_notify.wait(ulock, [this]() { return this->m_audio_have_output || this->m_audio_kill_thread; });
	}
}

void source::mirror::mirror_instance::enum_active_sources(obs_source_enum_proc_t enum_callback, void* param)
{
	if (this->m_scene) {
		enum_callback(this->m_self, this->m_scene->get(), param);
	}
}

void source::mirror::mirror_instance::load(obs_data_t* data)
{
	this->update(data);
}

void source::mirror::mirror_instance::save(obs_data_t* data)
{
	if (this->m_source_item) {
		obs_data_set_string(data, P_SOURCE, obs_source_get_name(m_source->get()));
	}
}

void source::mirror::mirror_instance::on_source_rename(obs::source* source, std::string, std::string)
{
	obs_data_t* ref = obs_source_get_settings(this->m_self);
	obs_data_set_string(ref, P_SOURCE, obs_source_get_name(source->get()));
	obs_source_update(this->m_self, ref);
	obs_data_release(ref);
}
