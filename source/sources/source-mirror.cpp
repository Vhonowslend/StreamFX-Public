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
#include "strings.hpp"
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

// OBS
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <media-io/audio-io.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#define ST "Source.Mirror"
#define ST_SOURCE ST ".Source"
#define ST_SOURCE_AUDIO ST_SOURCE ".Audio"
#define ST_SOURCE_AUDIO_LAYOUT ST_SOURCE_AUDIO ".Layout"
#define ST_SOURCE_AUDIO_LAYOUT_(x) ST_SOURCE_AUDIO_LAYOUT "." D_VSTR(x)

using namespace streamfx::source::mirror;

mirror_audio_data::mirror_audio_data(const audio_data* audio, speaker_layout layout)
{
	// Build a clone of a packet.
	audio_t*                 oad = obs_get_audio();
	const audio_output_info* aoi = audio_output_get_info(oad);
	osa.frames                   = audio->frames;
	osa.timestamp                = audio->timestamp;
	osa.speakers                 = layout;
	osa.format                   = aoi->format;
	osa.samples_per_sec          = aoi->samples_per_sec;
	data.resize(MAX_AV_PLANES);
	for (std::size_t idx = 0; idx < MAX_AV_PLANES; idx++) {
		if (!audio->data[idx]) {
			osa.data[idx] = nullptr;
			continue;
		}

		data[idx].resize(audio->frames * get_audio_bytes_per_channel(osa.format));
		memcpy(data[idx].data(), audio->data[idx], data[idx].size());
		osa.data[idx] = data[idx].data();
	}
}

mirror_instance::mirror_instance(obs_data_t* settings, obs_source_t* self)
	: obs::source_instance(settings, self), _source(), _source_child(), _signal_rename(), _audio_enabled(false),
	  _audio_layout(SPEAKERS_UNKNOWN)
{
	update(settings);
}

mirror_instance::~mirror_instance()
{
	release();
}

uint32_t mirror_instance::get_width()
{
	return _source_size.first ? _source_size.first : 1;
}

uint32_t mirror_instance::get_height()
{
	return _source_size.second ? _source_size.second : 1;
}

void mirror_instance::load(obs_data_t* data)
{
	update(data);
}

void mirror_instance::migrate(obs_data_t* data, uint64_t version)
{
	switch (version) {
	case 0:
		obs_data_set_int(data, ST_SOURCE_AUDIO_LAYOUT, obs_data_get_int(data, "Source.Mirror.Audio.Layout"));
		obs_data_unset_user_value(data, "Source.Mirror.Audio.Layout");
	case STREAMFX_VERSION:
		break;
	}
}

void mirror_instance::update(obs_data_t* data)
{
	// Audio
	_audio_enabled = obs_data_get_bool(data, ST_SOURCE_AUDIO);
	_audio_layout  = static_cast<speaker_layout>(obs_data_get_int(data, ST_SOURCE_AUDIO_LAYOUT));

	// Acquire new source.
	acquire(obs_data_get_string(data, ST_SOURCE));
}

void mirror_instance::save(obs_data_t* data)
{
	if (_source) {
		obs_data_set_string(data, ST_SOURCE, obs_source_get_name(_source.get()));
	} else {
		obs_data_unset_user_value(data, ST_SOURCE);
	}
}

void mirror_instance::video_tick(float_t time) {}

void mirror_instance::video_render(gs_effect_t* effect)
{
	if (!_source)
		return;
	if ((obs_source_get_output_flags(_source.get()) & OBS_SOURCE_VIDEO) == 0)
		return;

#ifdef ENABLE_PROFILING
	gs::debug_marker gdmp{gs::debug_color_source, "Source Mirror '%s' for '%s'", obs_source_get_name(_self),
						  obs_source_get_name(_source.get())};
#endif

	_source_size.first  = obs_source_get_width(_source.get());
	_source_size.second = obs_source_get_height(_source.get());

	obs_source_video_render(_source.get());
}

void mirror_instance::enum_active_sources(obs_source_enum_proc_t cb, void* ptr)
{
	if (!_source)
		return;
	cb(_self, _source.get(), ptr);
}

void mirror_instance::enum_all_sources(obs_source_enum_proc_t cb, void* ptr)
{
	if (!_source)
		return;

	cb(_self, _source.get(), ptr);
}

void mirror_instance::acquire(std::string source_name)
try {
	release();

	// Find source by name if possible.
	std::shared_ptr<obs_source_t> source =
		std::shared_ptr<obs_source_t>{obs_get_source_by_name(source_name.c_str()), obs::obs_source_deleter};
	if ((!source) || (source.get() == _self)) { // If we failed, just exit early.
		return;
	}

	// Everything went well, store.
	_source_child       = std::make_shared<obs::tools::child_source>(_self, source);
	_source             = source;
	_source_size.first  = obs_source_get_width(_source.get());
	_source_size.second = obs_source_get_height(_source.get());

	// Listen to the rename event to update our own settings.
	_signal_rename = std::make_shared<obs::source_signal_handler>("rename", _source);
	_signal_rename->event.add(
		std::bind(&mirror_instance::on_rename, this, std::placeholders::_1, std::placeholders::_2));

	// Listen to any audio the source spews out.
	if (_audio_enabled) {
		_signal_audio = std::make_shared<obs::audio_signal_handler>(_source);
		_signal_audio->event.add(std::bind(&mirror_instance::on_audio, this, std::placeholders::_1,
										   std::placeholders::_2, std::placeholders::_3));
	}
} catch (...) {
	release();
}

void mirror_instance::release()
{
	_signal_audio.reset();
	_signal_rename.reset();
	_source_child.reset();
	_source.reset();
}

void mirror_instance::on_rename(std::shared_ptr<obs_source_t>, calldata*)
{
	obs_source_save(_self);
}

void mirror_instance::on_audio(std::shared_ptr<obs_source_t>, const audio_data* audio, bool)
{
	// Immediately quit if there isn't any actual audio to send out.
	if (!_audio_enabled) {
		return;
	}

	// Detect Audio Layout from underlying audio.
	speaker_layout detected_layout;
	if (_audio_layout != SPEAKERS_UNKNOWN) {
		detected_layout = _audio_layout;
	} else {
		std::bitset<MAX_AV_PLANES> layout_detection;
		for (std::size_t idx = 0; idx < MAX_AV_PLANES; idx++) {
			layout_detection.set(idx, audio->data[idx] != nullptr);
		}
		switch (layout_detection.to_ulong()) {
		case 0b00000001:
			detected_layout = SPEAKERS_MONO;
			break;
		case 0b00000011:
			detected_layout = SPEAKERS_STEREO;
			break;
		case 0b00000111:
			detected_layout = SPEAKERS_2POINT1;
			break;
		case 0b00001111:
			detected_layout = SPEAKERS_4POINT0;
			break;
		case 0b00011111:
			detected_layout = SPEAKERS_4POINT1;
			break;
		case 0b00111111:
			detected_layout = SPEAKERS_5POINT1;
			break;
		case 0b11111111:
			detected_layout = SPEAKERS_7POINT1;
			break;
		default:
			detected_layout = SPEAKERS_UNKNOWN;
			break;
		}
	}

	{
		std::unique_lock<std::mutex> ul(_audio_queue_lock);
		_audio_queue.emplace(audio, detected_layout);
	}

	// Create a clone of the audio data and push it to the thread pool.
	streamfx::threadpool()->push(std::bind(&mirror_instance::audio_output, this, std::placeholders::_1), nullptr);
}

void mirror_instance::audio_output(std::shared_ptr<void> data)
{
	std::unique_lock<std::mutex> ul(_audio_queue_lock);
	while (_audio_queue.size() > 0) {
		obs_source_output_audio(_self, &((_audio_queue.front()).osa));
		_audio_queue.pop();
	}
}

mirror_factory::mirror_factory()
{
	_info.id           = PREFIX "source-mirror";
	_info.type         = OBS_SOURCE_TYPE_INPUT;
	_info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW | OBS_SOURCE_AUDIO;

	set_have_active_child_sources(true);
	set_have_child_sources(true);
	finish_setup();
	register_proxy("obs-stream-effects-source-mirror");
}

mirror_factory::~mirror_factory() {}

const char* mirror_factory::get_name()
{
	return D_TRANSLATE(ST);
}

void mirror_factory::get_defaults2(obs_data_t* data)
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

obs_properties_t* mirror_factory::get_properties2(mirror_instance* data)
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

std::shared_ptr<mirror_factory> _source_mirror_factory_instance;

void streamfx::source::mirror::mirror_factory::initialize()
{
	if (!_source_mirror_factory_instance)
		_source_mirror_factory_instance = std::make_shared<mirror_factory>();
}

void streamfx::source::mirror::mirror_factory::finalize() {}

std::shared_ptr<mirror_factory> streamfx::source::mirror::mirror_factory::get()
{
	return std::shared_ptr<mirror_factory>();
}
