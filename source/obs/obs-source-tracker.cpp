/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2017-2018 Michael Fabian Dirks
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

#include "obs-source-tracker.hpp"
#include <stdexcept>

static std::shared_ptr<obs::source_tracker> source_tracker_instance;

void obs::source_tracker::source_create_handler(void* ptr, calldata_t* data)
{
	obs::source_tracker* self = reinterpret_cast<obs::source_tracker*>(ptr);

	obs_source_t* target = nullptr;
	calldata_get_ptr(data, "source", &target);

	if (!target) {
		return;
	}

	const char* name = obs_source_get_name(target);
	if (!name) {
		// Do not track unnamed sources.
		return;
	}

	obs_weak_source_t* weak = obs_source_get_weak_source(target);
	if (!weak) {
		return;
	}

	self->_source_map.insert({std::string(name), weak});
}

void obs::source_tracker::source_destroy_handler(void* ptr, calldata_t* data)
{
	obs::source_tracker* self = reinterpret_cast<obs::source_tracker*>(ptr);

	obs_source_t* target = nullptr;
	calldata_get_ptr(data, "source", &target);

	if (!target) {
		return;
	}

	const char* name = obs_source_get_name(target);
	if (!name) {
		// Not tracking unnamed sources.
		return;
	}

	auto found = self->_source_map.find(std::string(name));
	if (found == self->_source_map.end()) {
		return;
	}

	obs_weak_source_release(found->second);
	self->_source_map.erase(found);
}

void obs::source_tracker::source_rename_handler(void* ptr, calldata_t* data)
{
	obs::source_tracker* self = reinterpret_cast<obs::source_tracker*>(ptr);

	obs_source_t* target    = nullptr;
	const char*   prev_name = nullptr;
	const char*   new_name  = nullptr;
	calldata_get_ptr(data, "source", &target);
	calldata_get_string(data, "prev_name", &prev_name);
	calldata_get_string(data, "new_name", &new_name);

	if (strcmp(prev_name, new_name) == 0) {
		// They weren't renamed at all, invalid event.
		return;
	}

	auto found = self->_source_map.find(std::string(prev_name));
	if (found == self->_source_map.end()) {
		// Untracked source, insert.
		obs_weak_source_t* weak = obs_source_get_weak_source(target);
		if (!weak) {
			return;
		}
		self->_source_map.insert({new_name, weak});
		return;
	}

	// Insert at new key, remove old pair.
	self->_source_map.insert({new_name, found->second});
	self->_source_map.erase(found);
}

void obs::source_tracker::initialize()
{
	source_tracker_instance = std::make_shared<obs::source_tracker>();
}

void obs::source_tracker::finalize()
{
	source_tracker_instance.reset();
}

std::shared_ptr<obs::source_tracker> obs::source_tracker::get()
{
	return source_tracker_instance;
}

obs::source_tracker::source_tracker()
{
	auto osi = obs_get_signal_handler();
	signal_handler_connect(osi, "source_create", &source_create_handler, this);
	signal_handler_connect(osi, "source_destroy", &source_destroy_handler, this);
	signal_handler_connect(osi, "source_rename", &source_rename_handler, this);
}

obs::source_tracker::~source_tracker()
{
	auto osi = obs_get_signal_handler();
	if (osi) {
		signal_handler_disconnect(osi, "source_create", &source_create_handler, this);
		signal_handler_disconnect(osi, "source_destroy", &source_destroy_handler, this);
		signal_handler_disconnect(osi, "source_rename", &source_rename_handler, this);
	}

	for (auto kv : this->_source_map) {
		obs_weak_source_release(kv.second);
	}
	this->_source_map.clear();
}

void obs::source_tracker::enumerate(enumerate_cb_t ecb, filter_cb_t fcb)
{
	// Need func-local copy, otherwise we risk corruption if a new source is created or destroyed.
	auto source_map_copy = this->_source_map;
	for (auto kv : this->_source_map) {
		obs_source_t* source = obs_weak_source_get_source(kv.second);
		if (!source) {
			continue;
		}

		if (fcb) {
			if (fcb(kv.first, source)) {
				obs_source_release(source);
				continue;
			}
		}

		if (ecb) {
			if (ecb(kv.first, source)) {
				obs_source_release(source);
				break;
			}
		}

		obs_source_release(source);
	}
}

bool obs::source_tracker::filter_sources(std::string, obs_source_t* source)
{
	return (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT);
}

bool obs::source_tracker::filter_audio_sources(std::string, obs_source_t* source)
{
	uint32_t flags = obs_source_get_output_flags(source);
	return !(flags & OBS_SOURCE_AUDIO) || (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT);
}

bool obs::source_tracker::filter_video_sources(std::string, obs_source_t* source)
{
	uint32_t flags = obs_source_get_output_flags(source);
	return !(flags & OBS_SOURCE_VIDEO) || (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT);
}

bool obs::source_tracker::filter_transitions(std::string, obs_source_t* source)
{
	return (obs_source_get_type(source) != OBS_SOURCE_TYPE_TRANSITION);
}

bool obs::source_tracker::filter_scenes(std::string, obs_source_t* source)
{
	return (obs_source_get_type(source) != OBS_SOURCE_TYPE_SCENE);
}
