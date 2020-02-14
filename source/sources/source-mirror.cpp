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
#include "obs/gs/gs-helper.hpp"
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
#define ST_SOURCE_AUDIO ST_SOURCE ".Audio"
#define ST_SOURCE_AUDIO_LAYOUT ST_SOURCE_AUDIO ".Layout"
#define ST_SOURCE_AUDIO_LAYOUT_(x) ST_SOURCE_AUDIO_LAYOUT "." D_VSTR(x)

using namespace source;

mirror::mirror_instance::mirror_instance(obs_data_t* settings, obs_source_t* self)
	: obs::source_instance(settings, self), _source(), _audio_enabled(), _audio_layout(), _audio_kill_thread(),
	  _audio_have_output()
{
	// Spawn Audio Thread
	_audio_thread = std::thread(std::bind(&mirror::mirror_instance::audio_output_cb, this));

	update(settings);
}

mirror::mirror_instance::~mirror_instance()
{
	release();

	// Kill Audio Thread
	_audio_kill_thread = true;
	_audio_notify.notify_all();
	if (_audio_thread.joinable()) {
		_audio_thread.join();
	}
}

uint32_t mirror::mirror_instance::get_width()
{
	return obs_source_get_width(_source.get());
}

uint32_t mirror::mirror_instance::get_height()
{
	return obs_source_get_height(_source.get());
}

static void convert_config(obs_data_t* data)
{
	uint64_t version = static_cast<uint64_t>(obs_data_get_int(data, S_VERSION));

	switch (version) {
	case 0:
		obs_data_set_int(data, ST_SOURCE_AUDIO_LAYOUT, obs_data_get_int(data, "Source.Mirror.Audio.Layout"));
		obs_data_unset_user_value(data, "Source.Mirror.Audio.Layout");
	case STREAMFX_VERSION:
		break;
	}

	obs_data_set_int(data, S_VERSION, STREAMFX_VERSION);
	obs_data_set_string(data, S_COMMIT, STREAMFX_COMMIT);
}

void mirror::mirror_instance::update(obs_data_t* data)
{
	convert_config(data);

	// Acquire new source.
	acquire(obs_data_get_string(data, ST_SOURCE));

	// Audio
	_audio_enabled = obs_data_get_bool(data, ST_SOURCE_AUDIO);
	_audio_layout  = static_cast<speaker_layout>(obs_data_get_int(data, ST_SOURCE_AUDIO_LAYOUT));
}

void mirror::mirror_instance::load(obs_data_t* data)
{
	update(data);
}

void mirror::mirror_instance::save(obs_data_t* data)
{
	if (_source) {
		obs_data_set_string(data, ST_SOURCE, obs_source_get_name(_source.get()));
	} else {
		obs_data_unset_user_value(data, ST_SOURCE);
	}
}

void source::mirror::mirror_instance::show()
{
	_visible = obs_source_showing(_self);
}

void source::mirror::mirror_instance::hide()
{
	_visible = obs_source_showing(_self);
}

void source::mirror::mirror_instance::activate()
{
	_active = obs_source_active(_self);
}

void source::mirror::mirror_instance::deactivate()
{
	_active = obs_source_active(_self);
}

void mirror::mirror_instance::video_tick(float time) {}

void mirror::mirror_instance::video_render(gs_effect_t* effect)
{
	if (!_source)
		return;
	if ((obs_source_get_output_flags(_source.get()) & OBS_SOURCE_VIDEO) == 0)
		return;

	obs_source_video_render(_source.get());
}

void mirror::mirror_instance::audio_output_cb() noexcept
try {
	std::unique_lock<std::mutex> ulock(_audio_lock_outputter);

	while (!_audio_kill_thread) {
		_audio_notify.wait(ulock, [this]() { return _audio_have_output || _audio_kill_thread; });

		if (_audio_have_output) { // Get used audio element
			std::shared_ptr<mirror_audio_data> mad;
			{
				std::lock_guard<std::mutex> capture_lock(_audio_lock_capturer);
				if (_audio_data_queue.size() > 0) {
					mad = _audio_data_queue.front();
					_audio_data_queue.pop();
				}
				if (_audio_data_queue.size() == 0) {
					_audio_have_output = false;
				}
			}

			if (mad) {
				ulock.unlock();
				obs_source_output_audio(_self, &mad->audio);
				ulock.lock();

				{
					std::lock_guard<std::mutex> capture_lock(_audio_lock_capturer);
					_audio_data_free_queue.push(mad);
				}
			}
		}
	}
} catch (const std::exception& ex) {
	LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void mirror::mirror_instance::enum_active_sources(obs_source_enum_proc_t cb, void* ptr)
{
	if (!_source || !_active)
		return;
	cb(_self, _source.get(), ptr);
}

void source::mirror::mirror_instance::enum_all_sources(obs_source_enum_proc_t cb, void* ptr)
{
	if (!_source)
		return;

	cb(_self, _source.get(), ptr);
}

void mirror::mirror_instance::acquire(std::string source_name)
try {
	// Find source by name if possible.
	std::shared_ptr<obs_source_t> source =
		std::shared_ptr<obs_source_t>{obs_get_source_by_name(source_name.c_str()), obs::obs_source_deleter};
	if ((!source) || (source.get() == _self)) { // If we failed, just exit early.
		return;
	}

	// Everything went well, store.
	_source_child = std::make_shared<obs::tools::child_source>(_self, source);
	_source       = source;

	// Listen to the rename event to update our own settings.
	_signal_rename = std::make_shared<obs::source_signal_handler>("rename", _source);
	_signal_rename->event.add(
		std::bind(&source::mirror::mirror_instance::on_rename, this, std::placeholders::_1, std::placeholders::_2));

	// Listen to any audio the source spews out.
	_signal_audio = std::make_shared<obs::audio_signal_handler>(_source);
	_signal_audio->event.add(std::bind(&source::mirror::mirror_instance::on_audio, this, std::placeholders::_1,
									   std::placeholders::_2, std::placeholders::_3));
} catch (...) {
	release();
}

void mirror::mirror_instance::release()
{
	_signal_audio.reset();
	_signal_rename.reset();
	_source_child.reset();
	_source.reset();
}

void source::mirror::mirror_instance::on_rename(std::shared_ptr<obs_source_t>, calldata*)
{
	obs_source_save(_self);
}

void source::mirror::mirror_instance::on_audio(std::shared_ptr<obs_source_t>, const audio_data* audio, bool)
{
	if (!_audio_enabled) {
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
		std::lock_guard<std::mutex> capture_lock(_audio_lock_capturer);
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
		if (_audio_layout != SPEAKERS_UNKNOWN) {
			mad->audio.speakers = _audio_layout;
		} else {
			mad->audio.speakers = aoi->speakers;
		}
	}

	{ // Push used audio data element.
		std::lock_guard<std::mutex> capture_lock(_audio_lock_capturer);
		_audio_data_queue.push(mad);
	}

	{ // Signal other side.
		std::lock_guard<std::mutex> output_lock(_audio_lock_outputter);
		_audio_have_output = true;
	}
	_audio_notify.notify_all();
}

std::shared_ptr<mirror::mirror_factory> mirror::mirror_factory::factory_instance;

mirror::mirror_factory::mirror_factory()
{
	_info.id           = "obs-stream-effects-source-mirror";
	_info.type         = OBS_SOURCE_TYPE_INPUT;
	_info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW | OBS_SOURCE_AUDIO;

	set_have_active_child_sources(true);
	set_have_child_sources(true);
	set_visibility_tracking_enabled(true);
	finish_setup();
}

mirror::mirror_factory::~mirror_factory() {}

const char* mirror::mirror_factory::get_name()
{
	return D_TRANSLATE(ST);
}

void mirror::mirror_factory::get_defaults2(obs_data_t* data)
{
	obs_data_set_default_string(data, ST_SOURCE, "");
	obs_data_set_default_bool(data, ST_SOURCE_AUDIO, false);
	obs_data_set_default_int(data, ST_SOURCE_AUDIO_LAYOUT, static_cast<int64_t>(SPEAKERS_UNKNOWN));
}

static bool modified_properties(obs_properties_t* pr, obs_property_t* p, obs_data_t* data) noexcept
try {
	if (obs_properties_get(pr, ST_SOURCE_AUDIO) == p) {
		bool show = obs_data_get_bool(data, ST_SOURCE_AUDIO);
		obs_property_set_visible(obs_properties_get(pr, ST_SOURCE_AUDIO_LAYOUT), show);
		return true;
	}
	return false;
} catch (...) {
	return false;
}

obs_properties_t* mirror::mirror_factory::get_properties2(mirror::mirror_instance* data)
{
	obs_properties_t* pr = obs_properties_create();
	obs_property_t*   p  = nullptr;

	{
		p = obs_properties_add_list(pr, ST_SOURCE, D_TRANSLATE(ST_SOURCE), OBS_COMBO_TYPE_LIST,
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

	{
		p = obs_properties_add_bool(pr, ST_SOURCE_AUDIO, D_TRANSLATE(ST_SOURCE_AUDIO));
		obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SOURCE_AUDIO)));
		obs_property_set_modified_callback(p, modified_properties);
	}

	{
		p = obs_properties_add_list(pr, ST_SOURCE_AUDIO_LAYOUT, D_TRANSLATE(ST_SOURCE_AUDIO_LAYOUT),
									OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
		obs_property_list_add_int(p, D_TRANSLATE(ST_SOURCE_AUDIO_LAYOUT_(Unknown)),
								  static_cast<int64_t>(SPEAKERS_UNKNOWN));
		obs_property_list_add_int(p, D_TRANSLATE(ST_SOURCE_AUDIO_LAYOUT_(Mono)), static_cast<int64_t>(SPEAKERS_MONO));
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

	return pr;
}
