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
#include <sstream>
#include <stdexcept>
#include <vector>
#include "obs/obs-source-tracker.hpp"
#include "obs/obs-tools.hpp"
#include "strings.hpp"

// OBS
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <graphics/matrix4.h>
#include <media-io/audio-io.h>
#include <obs-config.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#define ST "Source.Mirror"
#define ST_SOURCE ST ".Source"
#define ST_SOURCE_SIZE ST_SOURCE ".Size"
#define ST_SOURCE_AUDIO ST_SOURCE ".Audio"
#define ST_SOURCE_AUDIO_LAYOUT ST_SOURCE_AUDIO ".Layout"
#define ST_SOURCE_AUDIO_LAYOUT_(x) ST_SOURCE_AUDIO_LAYOUT "." D_VSTR(x)
#define ST_SCALING ST ".Scaling"
#define ST_SCALING_METHOD ST_SCALING ".Method"
#define ST_SCALING_METHOD_POINT ST_SCALING ".Method.Point"
#define ST_SCALING_METHOD_BILINEAR ST_SCALING ".Method.Bilinear"
#define ST_SCALING_METHOD_BICUBIC ST_SCALING ".Method.Bicubic"
#define ST_SCALING_METHOD_LANCZOS ST_SCALING ".Method.Lanczos"
#define ST_SCALING_SIZE ST_SCALING ".Size"
#define ST_SCALING_TRANSFORMKEEPORIGINAL ST_SCALING ".TransformKeepOriginal"
#define ST_SCALING_BOUNDS ST_SCALING ".Bounds"
#define ST_SCALING_BOUNDS_STRETCH ST_SCALING ".Bounds.Stretch"
#define ST_SCALING_BOUNDS_FIT ST_SCALING ".Bounds.Fit"
#define ST_SCALING_BOUNDS_FILL ST_SCALING ".Bounds.Fill"
#define ST_SCALING_BOUNDS_FILLWIDTH ST_SCALING ".Bounds.FillWidth"
#define ST_SCALING_BOUNDS_FILLHEIGHT ST_SCALING ".Bounds.FillHeight"
#define ST_SCALING_ALIGNMENT ST_SCALING ".Alignment"

void source::mirror::mirror_instance::release()
{
	_source_item.reset();
	if (_source) {
		_source->events.rename.clear();
		_source->events.audio_data.clear();
	}
	_source.reset();
	_source_name.clear();
}

void source::mirror::mirror_instance::acquire(std::string source_name)
{
	using namespace std::placeholders;

	// Try and get source by name.
	std::shared_ptr<obs_source_t> source = std::shared_ptr<obs_source_t>(
		obs_get_source_by_name(source_name.c_str()), [](obs_source_t* ref) { obs_source_release(ref); });
	if (!source) { // If we failed, just exit early.
		return;
	} else if (source.get() == _self) { // Otherwise, if we somehow found self, also early exit.
		return;
	}

	// We seem to have a true link to a source, let's add it to our rendering.
	obs_sceneitem_t* item = obs_scene_add(obs_scene_from_source(_scene.get()), source.get());
	if (!item) { // Can't add this source to our scene, probably due to one or more issues with it.
		return;
	}

	// It seems everything has worked out, so let's update our state.
	_source      = std::make_shared<obs::source>(source.get(), true, true);
	_source_name = obs_source_get_name(source.get());
	_source_item = std::shared_ptr<obs_sceneitem_t>(item, [](obs_sceneitem_t* ref) { obs_sceneitem_remove(ref); });

	// And let's hook up all our events too.
	_source->events.rename.add(std::bind(&source::mirror::mirror_instance::on_source_rename, this, _1, _2, _3));
	if ((obs_source_get_output_flags(this->_source->get()) & OBS_SOURCE_AUDIO) != 0)
		_source->events.audio_data.add(std::bind(&source::mirror::mirror_instance::on_audio_data, this, _1, _2, _3));
}

source::mirror::mirror_instance::mirror_instance(obs_data_t* settings, obs_source_t* self)
	: obs::source_instance(settings, self), _source(), _source_name(), _audio_enabled(), _audio_layout(),
	  _audio_kill_thread(), _audio_have_output(), _rescale_enabled(), _rescale_width(), _rescale_height(),
	  _rescale_keep_orig_size(), _rescale_type(), _rescale_bounds(), _rescale_alignment(), _cache_enabled(),
	  _cache_rendered()
{
	// Create Internal Scene
	_scene = std::shared_ptr<obs_source_t>(obs_scene_get_source(obs_scene_create_private("")),
										   [](obs_source_t* ref) { obs_source_release(ref); });

	// Create Cache Renderer
	_cache_renderer = std::make_shared<gfx::source_texture>(_scene.get(), _self);

	// Spawn Audio Thread
	/// ToDo: Use ThreadPool for this?
	_audio_thread = std::thread(std::bind(&source::mirror::mirror_instance::audio_output_cb, this));
}

source::mirror::mirror_instance::~mirror_instance()
{
	release();

	// Kill Audio Thread
	_audio_kill_thread = true;
	_audio_notify.notify_all();
	if (_audio_thread.joinable()) {
		_audio_thread.join();
	}

	// Delete Cache Renderer
	_cache_renderer.reset();

	// Delete Internal Scene
	_scene.reset();
}

uint32_t source::mirror::mirror_instance::get_width()
{
	if (!_source || !_source_item || !(obs_source_get_output_flags(_source->get()) & OBS_SOURCE_VIDEO))
		return 0;
	if (_rescale_enabled && _rescale_width > 0 && !_rescale_keep_orig_size)
		return _rescale_width;
	return _source->width();
}

uint32_t source::mirror::mirror_instance::get_height()
{
	if (!_source || !_source_item || !(obs_source_get_output_flags(_source->get()) & OBS_SOURCE_VIDEO))
		return 0;
	if (_rescale_enabled && _rescale_height > 0 && !_rescale_keep_orig_size)
		return _rescale_height;
	return _source->height();
}

static void convert_config(obs_data_t* data)
{
	uint64_t version = static_cast<uint64_t>(obs_data_get_int(data, S_VERSION));

	switch (version) {
	case 0:
		obs_data_set_int(data, ST_SOURCE_AUDIO_LAYOUT, obs_data_get_int(data, "Source.Mirror.Audio.Layout"));
		obs_data_unset_user_value(data, "Source.Mirror.Audio.Layout");
	case STREAMEFFECTS_VERSION:
		break;
	}

	obs_data_set_int(data, S_VERSION, STREAMEFFECTS_VERSION);
	obs_data_set_string(data, S_COMMIT, STREAMEFFECTS_COMMIT);
}

void source::mirror::mirror_instance::update(obs_data_t* data)
{
	convert_config(data);

	if (this->_source_name != obs_data_get_string(data, ST_SOURCE)) {
		// Mirrored source was changed, release and reacquire.
		release();

		// Acquire the new source.
		acquire(obs_data_get_string(data, ST_SOURCE));
	}

	// Audio
	this->_audio_enabled = obs_data_get_bool(data, ST_SOURCE_AUDIO);
	this->_audio_layout  = static_cast<speaker_layout>(obs_data_get_int(data, ST_SOURCE_AUDIO_LAYOUT));

	// Rescaling
	this->_rescale_enabled = obs_data_get_bool(data, ST_SCALING);
	if (this->_rescale_enabled) { // Parse rescaling settings.
		uint32_t width, height;

		// Read value.
		const char* size = obs_data_get_string(data, ST_SCALING_SIZE);
		const char* xpos = strchr(size, 'x');
		if (xpos != nullptr) {
			// Width
			width = strtoul(size, nullptr, 10);
			if (errno == ERANGE || width == 0) {
				this->_rescale_enabled = false;
				this->_rescale_width   = 1;
			} else {
				this->_rescale_width = width;
			}

			height = strtoul(xpos + 1, nullptr, 10);
			if (errno == ERANGE || height == 0) {
				this->_rescale_enabled = false;
				this->_rescale_height  = 1;
			} else {
				this->_rescale_height = height;
			}
		} else {
			this->_rescale_enabled = false;
			this->_rescale_width   = 1;
			this->_rescale_height  = 1;
		}

		this->_rescale_keep_orig_size = obs_data_get_bool(data, ST_SCALING_TRANSFORMKEEPORIGINAL);
		this->_rescale_type           = static_cast<obs_scale_type>(obs_data_get_int(data, ST_SCALING_METHOD));
		this->_rescale_bounds         = static_cast<obs_bounds_type>(obs_data_get_int(data, ST_SCALING_BOUNDS));
		this->_rescale_alignment      = static_cast<uint32_t>(obs_data_get_int(data, ST_SCALING_ALIGNMENT));
	}
}

void source::mirror::mirror_instance::load(obs_data_t* data)
{
	this->update(data);
}

void source::mirror::mirror_instance::save(obs_data_t* data)
{
	if (_source) {
		obs_data_set_string(data, ST_SOURCE, obs_source_get_name(_source->get()));
	}
}

void source::mirror::mirror_instance::video_tick(float time)
{
	if (_source_item && ((obs_source_get_output_flags(_source->get()) & OBS_SOURCE_VIDEO) != 0)) {
		obs_transform_info info;

		/// Position, Rotation, Scale, Alignment
		vec2_set(&info.pos, 0, 0);
		info.rot = 0;
		vec2_set(&info.scale, 1., 1.);
		info.alignment = OBS_ALIGN_LEFT | OBS_ALIGN_TOP;

		/// Bounding Box
		if (_rescale_enabled && _rescale_keep_orig_size) {
			vec2_set(&info.bounds, static_cast<float_t>(_rescale_width), static_cast<float_t>(_rescale_height));
		} else {
			vec2_set(&info.bounds, static_cast<float_t>(get_width()), static_cast<float_t>(get_height()));
		}

		info.bounds_alignment = _rescale_alignment;
		info.bounds_type      = OBS_BOUNDS_STRETCH;
		if (_rescale_enabled)
			info.bounds_type = _rescale_bounds;

		obs_sceneitem_set_info(_source_item.get(), &info);
		obs_sceneitem_force_update_transform(_source_item.get());
		obs_sceneitem_set_scale_filter(_source_item.get(), _rescale_enabled ? _rescale_type : OBS_SCALE_DISABLE);
	}

	_cache_rendered = false;
}

void source::mirror::mirror_instance::video_render(gs_effect_t* effect)
{
	if (!_source || !_source_item)
		return;

	if ((obs_source_get_output_flags(_source->get()) & OBS_SOURCE_VIDEO) == 0)
		return;

	GS_DEBUG_MARKER_BEGIN_FORMAT(GS_DEBUG_COLOR_SOURCE, "Source Mirror: %s", obs_source_get_name(_source->get()));

	// Rendering depends on cached or uncached.
	if (_cache_enabled || _rescale_enabled) {
		if (!_cache_rendered) {
			uint32_t width  = get_width();
			uint32_t height = get_height();
			if (_rescale_enabled) {
				width  = _rescale_width;
				height = _rescale_height;
			}

			if (!width || !height)
				return;

			try {
				_cache_texture  = this->_cache_renderer->render(width, height);
				_cache_rendered = true;
			} catch (...) {
			}
		}

		if (!_cache_texture)
			return;

		if (!effect)
			effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);

		GS_DEBUG_MARKER_BEGIN(GS_DEBUG_COLOR_ITEM_TEXTURE, "render_cache");
		gs_effect_set_texture(gs_effect_get_param_by_name(effect, "image"), _cache_texture->get_object());
		while (gs_effect_loop(effect, "Draw")) {
			gs_draw_sprite(nullptr, 0, get_width(), get_height());
		}
		GS_DEBUG_MARKER_END();
	} else {
		obs_source_video_render(_scene.get());
	}

	GS_DEBUG_MARKER_END();
}

void source::mirror::mirror_instance::audio_output_cb() noexcept try {
	std::unique_lock<std::mutex> ulock(this->_audio_lock_outputter);

	while (!this->_audio_kill_thread) {
		this->_audio_notify.wait(ulock, [this]() { return this->_audio_have_output || this->_audio_kill_thread; });

		if (this->_audio_have_output) { // Get used audio element
			std::shared_ptr<mirror_audio_data> mad;
			{
				std::lock_guard<std::mutex> capture_lock(this->_audio_lock_capturer);
				if (_audio_data_queue.size() > 0) {
					mad = _audio_data_queue.front();
					_audio_data_queue.pop();
				}
				if (_audio_data_queue.size() == 0) {
					this->_audio_have_output = false;
				}
			}

			if (mad) {
				ulock.unlock();
				obs_source_output_audio(this->_self, &mad->audio);
				ulock.lock();

				{
					std::lock_guard<std::mutex> capture_lock(this->_audio_lock_capturer);
					_audio_data_free_queue.push(mad);
				}
			}
		}
	}
} catch (const std::exception& ex) {
	P_LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	P_LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void source::mirror::mirror_instance::enum_active_sources(obs_source_enum_proc_t enum_callback, void* param)
{
	if (_scene) {
		enum_callback(_self, _scene.get(), param);
	}
	if (_source) {
		enum_callback(_self, _source->get(), param);
	}
}

void source::mirror::mirror_instance::enum_all_sources(obs_source_enum_proc_t enum_callback, void* param)
{
	if (_scene) {
		enum_callback(_self, _scene.get(), param);
	}
	if (_source) {
		enum_callback(_self, _source->get(), param);
	}
}

void source::mirror::mirror_instance::on_source_rename(obs::source* source, std::string, std::string)
{
	obs_source_save(_self);
}

void source::mirror::mirror_instance::on_audio_data(obs::source*, const audio_data* audio, bool)
{
	if (!this->_audio_enabled) {
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

	std::shared_ptr<mirror_audio_data> mad;
	{ // Get free audio data element.
		std::lock_guard<std::mutex> capture_lock(this->_audio_lock_capturer);
		if (_audio_data_free_queue.size() > 0) {
			mad = _audio_data_free_queue.front();
			_audio_data_free_queue.pop();
		} else {
			mad = std::make_shared<mirror_audio_data>();
			mad->data.resize(MAX_AUDIO_CHANNELS);
			for (size_t idx = 0; idx < mad->data.size(); idx++) {
				mad->data[idx].resize(AUDIO_OUTPUT_FRAMES);
			}
		}
	}

	{ // Copy data
		std::bitset<8> layout;
		for (size_t plane = 0; plane < MAX_AV_PLANES; plane++) {
			float* samples = reinterpret_cast<float_t*>(audio->data[plane]);
			if (!samples) {
				mad->audio.data[plane] = nullptr;
				continue;
			}
			layout.set(plane);

			memcpy(mad->data[plane].data(), audio->data[plane], audio->frames * sizeof(float_t));
			mad->audio.data[plane] = reinterpret_cast<uint8_t*>(mad->data[plane].data());
		}
		mad->audio.format          = aoi->format;
		mad->audio.frames          = audio->frames;
		mad->audio.timestamp       = audio->timestamp;
		mad->audio.samples_per_sec = aoi->samples_per_sec;
		if (this->_audio_layout != SPEAKERS_UNKNOWN) {
			mad->audio.speakers = this->_audio_layout;
		} else {
			mad->audio.speakers = aoi->speakers;
		}
	}

	{ // Push used audio data element.
		std::lock_guard<std::mutex> capture_lock(this->_audio_lock_capturer);
		_audio_data_queue.push(mad);
	}

	{ // Signal other side.
		std::lock_guard<std::mutex> output_lock(this->_audio_lock_outputter);
		this->_audio_have_output = true;
	}
	this->_audio_notify.notify_all();
}

std::shared_ptr<source::mirror::mirror_factory> source::mirror::mirror_factory::factory_instance;

source::mirror::mirror_factory::mirror_factory()
{
	_info.id           = "obs-stream-effects-source-mirror";
	_info.type         = OBS_SOURCE_TYPE_INPUT;
	_info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_AUDIO | OBS_SOURCE_CUSTOM_DRAW;

	obs_register_source(&_info);
}

source::mirror::mirror_factory::~mirror_factory() {}

const char* source::mirror::mirror_factory::get_name()
{
	return D_TRANSLATE(ST);
}

void source::mirror::mirror_factory::get_defaults2(obs_data_t* data)
{
	obs_data_set_default_string(data, ST_SOURCE, "");
	obs_data_set_default_bool(data, ST_SOURCE_AUDIO, false);
	obs_data_set_default_int(data, ST_SOURCE_AUDIO_LAYOUT, static_cast<int64_t>(SPEAKERS_UNKNOWN));
	obs_data_set_default_bool(data, ST_SCALING, false);
	obs_data_set_default_string(data, ST_SCALING_SIZE, "100x100");
	obs_data_set_default_int(data, ST_SCALING_METHOD, (int64_t)obs_scale_type::OBS_SCALE_BILINEAR);
	obs_data_set_default_bool(data, ST_SCALING_TRANSFORMKEEPORIGINAL, false);
	obs_data_set_default_int(data, ST_SCALING_BOUNDS, (int64_t)obs_bounds_type::OBS_BOUNDS_STRETCH);
	obs_data_set_default_int(data, ST_SCALING_ALIGNMENT, OBS_ALIGN_CENTER);
}

static bool modified_properties(obs_properties_t* pr, obs_property_t* p, obs_data_t* data) noexcept try {
	if (obs_properties_get(pr, ST_SOURCE) == p) {
		obs_source_t* target = obs_get_source_by_name(obs_data_get_string(data, ST_SOURCE));
		if (target) {
			std::vector<char> buf(256);
			snprintf(buf.data(), buf.size(), "%" PRIu32 "x%" PRIu32, obs_source_get_width(target),
					 obs_source_get_height(target));
			obs_data_set_string(data, ST_SOURCE_SIZE, buf.data());
		} else {
			obs_data_set_string(data, ST_SOURCE_SIZE, "0x0");
		}
		obs_source_release(target);
	}

	if (obs_properties_get(pr, ST_SOURCE_AUDIO) == p) {
		bool show = obs_data_get_bool(data, ST_SOURCE_AUDIO);
		obs_property_set_visible(obs_properties_get(pr, ST_SOURCE_AUDIO_LAYOUT), show);
		return true;
	}

	if (util::are_property_groups_broken() && (obs_properties_get(pr, ST_SCALING) == p)) {
		bool show = obs_data_get_bool(data, ST_SCALING);
		obs_property_set_visible(obs_properties_get(pr, ST_SCALING_METHOD), show);
		obs_property_set_visible(obs_properties_get(pr, ST_SCALING_SIZE), show);
		obs_property_set_visible(obs_properties_get(pr, ST_SCALING_BOUNDS), show);
		obs_property_set_visible(obs_properties_get(pr, ST_SCALING_ALIGNMENT), show);
		return true;
	}

	if (obs_properties_get(pr, ST_SCALING_BOUNDS) == p) {
		obs_bounds_type scaling_type = static_cast<obs_bounds_type>(obs_data_get_int(data, ST_SCALING_BOUNDS));
		obs_property_t* p2           = obs_properties_get(pr, ST_SCALING_BOUNDS);
		switch (scaling_type) {
		case obs_bounds_type::OBS_BOUNDS_STRETCH:
			obs_property_set_long_description(p2, D_TRANSLATE(D_DESC(ST_SCALING_BOUNDS_STRETCH)));
			break;
		case obs_bounds_type::OBS_BOUNDS_SCALE_INNER:
			obs_property_set_long_description(p2, D_TRANSLATE(D_DESC(ST_SCALING_BOUNDS_FIT)));
			break;
		case obs_bounds_type::OBS_BOUNDS_SCALE_OUTER:
			obs_property_set_long_description(p2, D_TRANSLATE(D_DESC(ST_SCALING_BOUNDS_FILL)));
			break;
		case obs_bounds_type::OBS_BOUNDS_SCALE_TO_WIDTH:
			obs_property_set_long_description(p2, D_TRANSLATE(D_DESC(ST_SCALING_BOUNDS_FILLWIDTH)));
			break;
		case obs_bounds_type::OBS_BOUNDS_SCALE_TO_HEIGHT:
			obs_property_set_long_description(p2, D_TRANSLATE(D_DESC(ST_SCALING_BOUNDS_FILLHEIGHT)));
			break;
		default:
			obs_property_set_long_description(p2, D_TRANSLATE(D_DESC(ST_SCALING_BOUNDS)));
			break;
		}
		return true;
	}

	return false;
} catch (const std::exception& ex) {
	P_LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
	return false;
} catch (...) {
	P_LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
	return false;
}

obs_properties_t* source::mirror::mirror_factory::get_properties2(source::mirror::mirror_instance* data)
{
	obs_properties_t* pr = obs_properties_create();
	obs_property_t*   p  = nullptr;

	{
		obs_properties_t* grp = pr;
		if (!util::are_property_groups_broken()) {
			grp = obs_properties_create();
			p   = obs_properties_add_group(pr, ST, D_TRANSLATE(ST_SOURCE), OBS_GROUP_NORMAL, grp);
			//obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST)));
		}

		{
			p = obs_properties_add_list(grp, ST_SOURCE, D_TRANSLATE(ST_SOURCE), OBS_COMBO_TYPE_LIST,
										OBS_COMBO_FORMAT_STRING);
			obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SOURCE)));
			obs_property_set_modified_callback(p, modified_properties);

			obs_property_list_add_string(p, "", "");
			obs::source_tracker::get()->enumerate(
				[&p](std::string name, obs_source_t*) {
					std::stringstream sstr;
					sstr << name << " (" << D_TRANSLATE(S_SOURCETYPE_SOURCE) << ")";
					obs_property_list_add_string(p, sstr.str().c_str(), name.c_str());
					return false;
				},
				obs::source_tracker::filter_sources);
			obs::source_tracker::get()->enumerate(
				[&p](std::string name, obs_source_t*) {
					std::stringstream sstr;
					sstr << name << " (" << D_TRANSLATE(S_SOURCETYPE_SCENE) << ")";
					obs_property_list_add_string(p, sstr.str().c_str(), name.c_str());
					return false;
				},
				obs::source_tracker::filter_scenes);
		}

		p = obs_properties_add_text(grp, ST_SOURCE_SIZE, D_TRANSLATE(ST_SOURCE_SIZE), OBS_TEXT_DEFAULT);
		obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SOURCE_SIZE)));
		obs_property_set_enabled(p, false);

		p = obs_properties_add_bool(grp, ST_SOURCE_AUDIO, D_TRANSLATE(ST_SOURCE_AUDIO));
		obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SOURCE_AUDIO)));
		obs_property_set_modified_callback(p, modified_properties);

		{
			p = obs_properties_add_list(grp, ST_SOURCE_AUDIO_LAYOUT, D_TRANSLATE(ST_SOURCE_AUDIO_LAYOUT),
										OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
			obs_property_list_add_int(p, D_TRANSLATE(ST_SOURCE_AUDIO_LAYOUT_(Unknown)),
									  static_cast<int64_t>(SPEAKERS_UNKNOWN));
			obs_property_list_add_int(p, D_TRANSLATE(ST_SOURCE_AUDIO_LAYOUT_(Mono)),
									  static_cast<int64_t>(SPEAKERS_MONO));
			obs_property_list_add_int(p, D_TRANSLATE(ST_SOURCE_AUDIO_LAYOUT_(Stereo)),
									  static_cast<int64_t>(SPEAKERS_STEREO));
			obs_property_list_add_int(p, D_TRANSLATE(ST_SOURCE_AUDIO_LAYOUT_(StereoLFE)),
									  static_cast<int64_t>(SPEAKERS_2POINT1));
			obs_property_list_add_int(p, D_TRANSLATE(ST_SOURCE_AUDIO_LAYOUT_(Quadraphonic)),
									  static_cast<int64_t>(SPEAKERS_4POINT0));
			obs_property_list_add_int(p, D_TRANSLATE(ST_SOURCE_AUDIO_LAYOUT_(QuadraphonicLFE)),
									  static_cast<int64_t>(SPEAKERS_4POINT1));
			obs_property_list_add_int(p, D_TRANSLATE(ST_SOURCE_AUDIO_LAYOUT_(Surround)),
									  static_cast<int64_t>(SPEAKERS_5POINT1));
			obs_property_list_add_int(p, D_TRANSLATE(ST_SOURCE_AUDIO_LAYOUT_(FullSurround)),
									  static_cast<int64_t>(SPEAKERS_7POINT1));
			obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SOURCE_AUDIO_LAYOUT)));
		}
	}

	{
		obs_properties_t* grp = pr;
		if (!util::are_property_groups_broken()) {
			grp = obs_properties_create();
			p   = obs_properties_add_group(pr, ST_SCALING, D_TRANSLATE(ST_SCALING), OBS_GROUP_CHECKABLE, grp);
			obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SCALING)));
		} else {
			p = obs_properties_add_bool(pr, ST_SCALING, D_TRANSLATE(ST_SCALING));
			obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SCALING)));
			obs_property_set_modified_callback(p, modified_properties);
		}

		p = obs_properties_add_bool(grp, ST_SCALING_TRANSFORMKEEPORIGINAL,
									D_TRANSLATE(ST_SCALING_TRANSFORMKEEPORIGINAL));
		obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SCALING_TRANSFORMKEEPORIGINAL)));
		obs_property_set_modified_callback(p, modified_properties);

		p = obs_properties_add_text(grp, ST_SCALING_SIZE, D_TRANSLATE(ST_SCALING_SIZE), OBS_TEXT_DEFAULT);
		obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SCALING_SIZE)));

		{
			p = obs_properties_add_list(grp, ST_SCALING_METHOD, D_TRANSLATE(ST_SCALING_METHOD), OBS_COMBO_TYPE_LIST,
										OBS_COMBO_FORMAT_INT);
			obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SCALING_METHOD)));
			obs_property_set_modified_callback(p, modified_properties);
			obs_property_list_add_int(p, D_TRANSLATE(ST_SCALING_METHOD_POINT),
									  (int64_t)obs_scale_type::OBS_SCALE_POINT);
			obs_property_list_add_int(p, D_TRANSLATE(ST_SCALING_METHOD_BILINEAR),
									  (int64_t)obs_scale_type::OBS_SCALE_BILINEAR);
			obs_property_list_add_int(p, D_TRANSLATE(ST_SCALING_METHOD_BICUBIC),
									  (int64_t)obs_scale_type::OBS_SCALE_BICUBIC);
			obs_property_list_add_int(p, D_TRANSLATE(ST_SCALING_METHOD_LANCZOS),
									  (int64_t)obs_scale_type::OBS_SCALE_LANCZOS);
		}

		{
			p = obs_properties_add_list(grp, ST_SCALING_BOUNDS, D_TRANSLATE(ST_SCALING_BOUNDS), OBS_COMBO_TYPE_LIST,
										OBS_COMBO_FORMAT_INT);
			obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SCALING_BOUNDS)));
			obs_property_set_modified_callback(p, modified_properties);
			obs_property_list_add_int(p, D_TRANSLATE(ST_SCALING_BOUNDS_STRETCH),
									  (int64_t)obs_bounds_type::OBS_BOUNDS_STRETCH);
			obs_property_list_add_int(p, D_TRANSLATE(ST_SCALING_BOUNDS_FIT),
									  (int64_t)obs_bounds_type::OBS_BOUNDS_SCALE_INNER);
			obs_property_list_add_int(p, D_TRANSLATE(ST_SCALING_BOUNDS_FILL),
									  (int64_t)obs_bounds_type::OBS_BOUNDS_SCALE_OUTER);
			obs_property_list_add_int(p, D_TRANSLATE(ST_SCALING_BOUNDS_FILLWIDTH),
									  (int64_t)obs_bounds_type::OBS_BOUNDS_SCALE_TO_WIDTH);
			obs_property_list_add_int(p, D_TRANSLATE(ST_SCALING_BOUNDS_FILLHEIGHT),
									  (int64_t)obs_bounds_type::OBS_BOUNDS_SCALE_TO_HEIGHT);
		}

		{
			p = obs_properties_add_list(grp, ST_SCALING_ALIGNMENT, D_TRANSLATE(ST_SCALING_ALIGNMENT),
										OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
			obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SCALING_ALIGNMENT)));
			obs_property_list_add_int(p,
									  obs_module_recursive_text("\\@" S_ALIGNMENT_LEFT "\\@ \\@" S_ALIGNMENT_TOP "\\@"),
									  OBS_ALIGN_LEFT | OBS_ALIGN_TOP);
			obs_property_list_add_int(p, obs_module_recursive_text("\\@" S_ALIGNMENT_TOP "\\@"), OBS_ALIGN_TOP);
			obs_property_list_add_int(
				p, obs_module_recursive_text("\\@" S_ALIGNMENT_RIGHT "\\@ \\@" S_ALIGNMENT_TOP "\\@"),
				OBS_ALIGN_RIGHT | OBS_ALIGN_TOP);
			obs_property_list_add_int(p, obs_module_recursive_text("\\@" S_ALIGNMENT_LEFT "\\@"), OBS_ALIGN_LEFT);
			obs_property_list_add_int(p, obs_module_recursive_text("\\@" S_ALIGNMENT_CENTER "\\@"), OBS_ALIGN_CENTER);
			obs_property_list_add_int(p, obs_module_recursive_text("\\@" S_ALIGNMENT_RIGHT "\\@"), OBS_ALIGN_RIGHT);
			obs_property_list_add_int(
				p, obs_module_recursive_text("\\@" S_ALIGNMENT_LEFT "\\@ \\@" S_ALIGNMENT_BOTTOM "\\@"),
				OBS_ALIGN_LEFT | OBS_ALIGN_BOTTOM);
			obs_property_list_add_int(p, obs_module_recursive_text("\\@" S_ALIGNMENT_BOTTOM "\\@"), OBS_ALIGN_BOTTOM);
			obs_property_list_add_int(
				p, obs_module_recursive_text("\\@" S_ALIGNMENT_RIGHT "\\@ \\@" S_ALIGNMENT_BOTTOM "\\@"),
				OBS_ALIGN_RIGHT | OBS_ALIGN_BOTTOM);
		}
	}

	return pr;
}
