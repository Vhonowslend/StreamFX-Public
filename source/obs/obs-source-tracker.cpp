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
#include "obs/obs-tools.hpp"
#include "util/util-logging.hpp"

#include "warning-disable.hpp"
#include <mutex>
#include <stdexcept>
#include "warning-enable.hpp"

#ifdef _DEBUG
#define ST_PREFIX "<%s> "
#define D_LOG_ERROR(x, ...) P_LOG_ERROR(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_WARNING(x, ...) P_LOG_WARN(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_INFO(x, ...) P_LOG_INFO(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_DEBUG(x, ...) P_LOG_DEBUG(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#else
#define ST_PREFIX "<obs::source_tracker> "
#define D_LOG_ERROR(...) P_LOG_ERROR(ST_PREFIX __VA_ARGS__)
#define D_LOG_WARNING(...) P_LOG_WARN(ST_PREFIX __VA_ARGS__)
#define D_LOG_INFO(...) P_LOG_INFO(ST_PREFIX __VA_ARGS__)
#define D_LOG_DEBUG(...) P_LOG_DEBUG(ST_PREFIX __VA_ARGS__)
#endif

void streamfx::obs::source_tracker::source_create_handler(void* ptr, calldata_t* data) noexcept
{
	auto* self = reinterpret_cast<streamfx::obs::source_tracker*>(ptr);
	try {
		obs_source_t* source = nullptr;
		if (calldata_get_ptr(data, "source", &source); !source) {
			throw std::runtime_error("Missing 'source' parameter.");
		}

		self->insert_source(source);
	} catch (const std::exception& ex) {
		DLOG_ERROR("Event 'source_create' caused exception: %s", ex.what());
	} catch (...) {
		DLOG_ERROR("Event 'source_create' caused unknown exception.", nullptr);
	}
}

void streamfx::obs::source_tracker::source_destroy_handler(void* ptr, calldata_t* data) noexcept
{
	auto* self = reinterpret_cast<streamfx::obs::source_tracker*>(ptr);
	try {
		obs_source_t* source = nullptr;
		if (calldata_get_ptr(data, "source", &source); !source) {
			throw std::runtime_error("Missing 'source' parameter.");
		}

	} catch (const std::exception& ex) {
		DLOG_ERROR("Event 'source_destroy' caused exception: %s", ex.what());
	} catch (...) {
		DLOG_ERROR("Event 'source_destroy' caused unknown exception.", nullptr);
	}
}

void streamfx::obs::source_tracker::source_rename_handler(void* ptr, calldata_t* data) noexcept
{
	auto* self = reinterpret_cast<streamfx::obs::source_tracker*>(ptr);
	try {
		obs_source_t* source = nullptr;
		if (calldata_get_ptr(data, "source", &source); !source) {
			throw std::runtime_error("Missing 'source' parameter.");
		}

		const char* old_name = nullptr;
		if (calldata_get_string(data, "prev_name", &old_name); !old_name) {
			throw std::runtime_error("Missing 'prev_name' parameter.");
		}

		const char* new_name = nullptr;
		if (calldata_get_string(data, "new_name", &new_name); !new_name) {
			throw std::runtime_error("Missing 'new_name' parameter.");
		}

		self->rename_source(old_name, new_name, source);
	} catch (...) {
		DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
	}
}

void streamfx::obs::source_tracker::insert_source(obs_source_t* source)
{
	const auto* name = obs_source_get_name(source);
	if (!name) { // Do not track unnamed sources.
		return;
	}

	std::shared_ptr<obs_weak_source_t> weak{obs_source_get_weak_source(source), streamfx::obs::obs_weak_source_deleter};
	if (!weak) { // This source has already been deleted, do not track.
		return;
	}

	std::unique_lock<std::mutex> lock(_mutex);
	_sources.insert({
		std::string(name),
		weak,
	});
}

void streamfx::obs::source_tracker::remove_source(obs_source_t* source)
{
	const char* name = obs_source_get_name(source);

	// Lock read & write access to the map.
	std::unique_lock<std::mutex> ul(_mutex);

	// Try and remove the source by name.
	if (name != nullptr) {
		auto found = _sources.find(std::string(name));
		if (found != _sources.end()) {
			_sources.erase(found);
			return;
		}
	}

	// If that didn't work, try and remove it by handle.
	for (auto iter = _sources.begin(); iter != _sources.end(); iter++) {
		if (obs_weak_source_get_source(iter->second.get()) == source) {
			_sources.erase(iter);
			return;
		}
	}

	// If that all failed, and the source is named, throw and report an error.
	if (name) {
		D_LOG_WARNING("Source '%s' was not tracked.", name);
		throw std::runtime_error("Failed to find given source.");
	}
}

void streamfx::obs::source_tracker::rename_source(std::string_view old_name, std::string_view new_name,
												  obs_source_t* source)
{
	if (old_name == new_name) {
		throw std::runtime_error("New and old name are identical.");
	}

	std::unique_lock<std::mutex> ul(_mutex);
	auto                         found = _sources.find(std::string(old_name));
	if (found == _sources.end()) {
		insert_source(source);
		return;
	}

	// Insert at new key, remove old pair.
	_sources.insert({new_name.data(), found->second});
	_sources.erase(found);
}

streamfx::obs::source_tracker::source_tracker() : _sources(), _mutex()
{
	auto osi = obs_get_signal_handler();
	signal_handler_connect(osi, "source_create", &source_create_handler, this);
	signal_handler_connect(osi, "source_destroy", &source_destroy_handler, this);
	signal_handler_connect(osi, "source_rename", &source_rename_handler, this);

	// Enumerate all current sources and filters.
	obs_enum_all_sources(
		[](void* param, obs_source_t* source) {
			auto* self = reinterpret_cast<::streamfx::obs::source_tracker*>(param);
			self->insert_source(source);
			return true;
		},
		this);
}

streamfx::obs::source_tracker::~source_tracker()
{
	auto osi = obs_get_signal_handler();
	if (osi) {
		signal_handler_disconnect(osi, "source_create", &source_create_handler, this);
		signal_handler_disconnect(osi, "source_destroy", &source_destroy_handler, this);
		signal_handler_disconnect(osi, "source_rename", &source_rename_handler, this);
	}

	this->_sources.clear();
}

void streamfx::obs::source_tracker::enumerate(enumerate_cb_t ecb, filter_cb_t fcb)
{
	// Need func-local copy, otherwise we risk corruption if a new source is created or destroyed.
	decltype(_sources) _clone;
	{
		std::unique_lock<std::mutex> ul(_mutex);
		_clone = _sources;
	}

	for (auto kv : _clone) {
		auto source = std::shared_ptr<obs_source_t>(obs_weak_source_get_source(kv.second.get()),
													streamfx::obs::obs_source_deleter);
		if (!source) {
			continue;
		}

		if (fcb) {
			if (fcb(kv.first, source.get())) {
				continue;
			}
		}

		if (ecb) {
			if (ecb(kv.first, source.get())) {
				break;
			}
		}
	}
}

bool streamfx::obs::source_tracker::filter_sources(std::string, obs_source_t* source)
{
	return (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT);
}

bool streamfx::obs::source_tracker::filter_audio_sources(std::string, obs_source_t* source)
{
	uint32_t flags = obs_source_get_output_flags(source);
	return !(flags & OBS_SOURCE_AUDIO) || (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT);
}

bool streamfx::obs::source_tracker::filter_video_sources(std::string, obs_source_t* source)
{
	uint32_t flags = obs_source_get_output_flags(source);
	return !(flags & OBS_SOURCE_VIDEO) || (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT);
}

bool streamfx::obs::source_tracker::filter_transitions(std::string, obs_source_t* source)
{
	return (obs_source_get_type(source) != OBS_SOURCE_TYPE_TRANSITION);
}

bool streamfx::obs::source_tracker::filter_scenes(std::string, obs_source_t* source)
{
	return (obs_source_get_type(source) != OBS_SOURCE_TYPE_SCENE);
}

std::shared_ptr<streamfx::obs::source_tracker> streamfx::obs::source_tracker::get()
{
	static std::mutex                                   inst_mtx;
	static std::weak_ptr<streamfx::obs::source_tracker> inst_weak;

	std::unique_lock<std::mutex> lock(inst_mtx);
	if (inst_weak.expired()) {
		auto instance = std::shared_ptr<streamfx::obs::source_tracker>(new streamfx::obs::source_tracker());
		inst_weak     = instance;
		return instance;
	} else {
		return inst_weak.lock();
	}
}
