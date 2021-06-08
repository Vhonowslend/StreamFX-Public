/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2018 Michael Fabian Dirks
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

#include "obs-source.hpp"
#include <stdexcept>
#include "plugin.hpp"

void streamfx::obs::deprecated_source::handle_destroy(void* p, calldata_t* calldata) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);

	obs_source_t* source;
	if (!calldata_get_ptr(calldata, "source", &source)) {
		return;
	}

	if (self->_self == source) {
		self->_self = nullptr;
	}

	if (self->events.destroy) {
		return;
	}
	self->events.destroy(self);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_remove(void* p, calldata_t*) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.remove) {
		return;
	}
	self->events.remove(self);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_save(void* p, calldata_t*) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.save) {
		return;
	}
	self->events.save(self);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_load(void* p, calldata_t*) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.load) {
		return;
	}
	self->events.load(self);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_activate(void* p, calldata_t*) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.activate) {
		return;
	}
	self->events.activate(self);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_deactivate(void* p, calldata_t*) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.deactivate) {
		return;
	}
	self->events.deactivate(self);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_show(void* p, calldata_t*) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.show) {
		return;
	}
	self->events.show(self);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_hide(void* p, calldata_t*) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.hide) {
		return;
	}
	self->events.hide(self);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_enable(void* p, calldata_t* calldata) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.enable) {
		return;
	}

	bool enabled = false;
	if (!calldata_get_bool(calldata, "enabled", &enabled)) {
		return;
	}

	self->events.enable(self, enabled);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_push_to_mute_changed(void* p, calldata_t* calldata) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.push_to_mute_changed) {
		return;
	}

	bool enabled = false;
	if (!calldata_get_bool(calldata, "enabled", &enabled)) {
		return;
	}

	self->events.push_to_mute_changed(self, enabled);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_push_to_mute_delay(void* p, calldata_t* calldata) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.push_to_mute_delay) {
		return;
	}

	long long delay;
	if (!calldata_get_int(calldata, "delay", &delay)) {
		return;
	}

	self->events.push_to_mute_delay(self, delay);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_push_to_talk_changed(void* p, calldata_t* calldata) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.push_to_talk_changed) {
		return;
	}

	bool enabled = false;
	if (!calldata_get_bool(calldata, "enabled", &enabled)) {
		return;
	}

	self->events.push_to_talk_changed(self, enabled);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_push_to_talk_delay(void* p, calldata_t* calldata) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.push_to_talk_delay) {
		return;
	}

	long long delay;
	if (!calldata_get_int(calldata, "delay", &delay)) {
		return;
	}

	self->events.push_to_talk_delay(self, delay);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_rename(void* p, calldata_t* calldata) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.enable) {
		return;
	}

	const char* new_name;
	if (!calldata_get_string(calldata, "new_name", &new_name)) {
		return;
	}

	const char* prev_name;
	if (!calldata_get_string(calldata, "prev_name", &prev_name)) {
		return;
	}

	self->events.rename(self, std::string(new_name ? new_name : ""), std::string(prev_name ? prev_name : ""));
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_update_properties(void* p, calldata_t*) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.update_properties) {
		return;
	}
	self->events.update_properties(self);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_update_flags(void* p, calldata_t* calldata) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.update_flags) {
		return;
	}

	long long flags;
	if (!calldata_get_int(calldata, "flags", &flags)) {
		return;
	}

	self->events.update_flags(self, flags);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_mute(void* p, calldata_t* calldata) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.mute) {
		return;
	}

	bool muted;
	if (!calldata_get_bool(calldata, "muted", &muted)) {
		return;
	}

	self->events.mute(self, muted);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_volume(void* p, calldata_t* calldata) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.volume) {
		return;
	}

	double volume;
	if (!calldata_get_float(calldata, "volume", &volume)) {
		return;
	}

	self->events.volume(self, volume);

	calldata_set_float(calldata, "volume", volume);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_audio_sync(void* p, calldata_t* calldata) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.audio_sync) {
		return;
	}

	long long mixers;
	if (!calldata_get_int(calldata, "offset", &mixers)) {
		return;
	}

	self->events.audio_sync(self, mixers);

	calldata_set_int(calldata, "offset", mixers);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_audio_mixers(void* p, calldata_t* calldata) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.audio_mixers) {
		return;
	}

	long long mixers;
	if (!calldata_get_int(calldata, "mixers", &mixers)) {
		return;
	}

	self->events.audio_mixers(self, mixers);

	calldata_set_int(calldata, "mixers", mixers);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_audio_data(void* p, obs_source_t*, const audio_data* audio,
														 bool muted) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.audio) {
		return;
	}

	self->events.audio(self, audio, muted);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_filter_add(void* p, calldata_t* calldata) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.filter_add) {
		return;
	}

	obs_source_t* filter;
	if (!calldata_get_ptr(calldata, "filter", &filter)) {
		return;
	}

	self->events.filter_add(self, filter);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_filter_remove(void* p, calldata_t* calldata) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.filter_remove) {
		return;
	}

	obs_source_t* filter;
	if (!calldata_get_ptr(calldata, "filter", &filter)) {
		return;
	}

	self->events.filter_remove(self, filter);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_reorder_filters(void* p, calldata_t*) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.reorder_filters) {
		return;
	}
	self->events.reorder_filters(self);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_transition_start(void* p, calldata_t*) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.transition_start) {
		return;
	}
	self->events.transition_start(self);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_transition_video_stop(void* p, calldata_t*) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.transition_video_stop) {
		return;
	}
	self->events.transition_video_stop(self);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void streamfx::obs::deprecated_source::handle_transition_stop(void* p, calldata_t*) noexcept
try {
	streamfx::obs::deprecated_source* self = reinterpret_cast<streamfx::obs::deprecated_source*>(p);
	if (!self->events.transition_stop) {
		return;
	}
	self->events.transition_stop(self);
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

streamfx::obs::deprecated_source::~deprecated_source()
{
#ifdef auto_signal_d
#undef auto_signal_d
#endif
#define auto_signal_d(SIGNAL) this->events.SIGNAL.clear()
	auto_signal_d(destroy);
	auto_signal_d(remove);
	auto_signal_d(save);
	auto_signal_d(load);
	auto_signal_d(activate);
	auto_signal_d(deactivate);
	auto_signal_d(show);
	auto_signal_d(hide);
	auto_signal_d(mute);
	auto_signal_d(push_to_mute_changed);
	auto_signal_d(push_to_mute_delay);
	auto_signal_d(push_to_talk_changed);
	auto_signal_d(push_to_talk_delay);
	auto_signal_d(enable);
	auto_signal_d(rename);
	auto_signal_d(volume);
	auto_signal_d(update_properties);
	auto_signal_d(update_flags);
	auto_signal_d(audio_sync);
	auto_signal_d(audio_mixers);
	auto_signal_d(audio);
	auto_signal_d(filter_add);
	auto_signal_d(filter_remove);
	auto_signal_d(reorder_filters);
	auto_signal_d(transition_start);
	auto_signal_d(transition_video_stop);
	auto_signal_d(transition_stop);
#undef auto_signal_d

	if (this->_track_ownership && this->_self) {
		obs_source_release(this->_self);
	}
	this->_self = nullptr;
}

streamfx::obs::deprecated_source::deprecated_source()
{
#ifdef auto_signal_c
#undef auto_signal_c
#endif
#define auto_signal_c(SIGNAL)                                                                             \
	{                                                                                                     \
		this->events.SIGNAL.set_listen_callback([this]() noexcept {                                       \
			if (!this->_self)                                                                             \
				return;                                                                                   \
			auto sh = obs_source_get_signal_handler(this->_self);                                         \
			if (sh) {                                                                                     \
				signal_handler_connect(sh, "" #SIGNAL, obs::deprecated_source::handle_##SIGNAL, this);    \
			}                                                                                             \
		});                                                                                               \
		this->events.SIGNAL.set_silence_callback([this]() noexcept {                                      \
			if (!this->_self)                                                                             \
				return;                                                                                   \
			auto sh = obs_source_get_signal_handler(this->_self);                                         \
			if (sh) {                                                                                     \
				signal_handler_disconnect(sh, "" #SIGNAL, obs::deprecated_source::handle_##SIGNAL, this); \
			}                                                                                             \
		});                                                                                               \
	}
	auto_signal_c(destroy) auto_signal_c(remove) auto_signal_c(save) auto_signal_c(load) auto_signal_c(activate)
		auto_signal_c(deactivate) auto_signal_c(show) auto_signal_c(hide) auto_signal_c(mute)
			auto_signal_c(push_to_mute_changed) auto_signal_c(push_to_mute_delay) auto_signal_c(push_to_talk_changed)
				auto_signal_c(push_to_talk_delay) auto_signal_c(enable) auto_signal_c(rename) auto_signal_c(volume)
					auto_signal_c(update_properties) auto_signal_c(update_flags) auto_signal_c(audio_sync)
						auto_signal_c(audio_mixers) auto_signal_c(filter_add) auto_signal_c(filter_remove)
							auto_signal_c(reorder_filters) auto_signal_c(transition_start)
								auto_signal_c(transition_video_stop) auto_signal_c(transition_stop)
#undef auto_signal_c

	// libOBS unfortunately does not use the event system for audio data callbacks, which is kind of odd as most other
	//  things do. So instead we'll have to manually deal with it for now.
	{
		this->events.audio.set_listen_callback([this]() noexcept {
			if (!this->_self)
				return;
			obs_source_add_audio_capture_callback(this->_self, streamfx::obs::deprecated_source::handle_audio_data,
												  this);
		});
		this->events.audio.set_silence_callback([this]() noexcept {
			if (!this->_self)
				return;
			obs_source_remove_audio_capture_callback(this->_self, streamfx::obs::deprecated_source::handle_audio_data,
													 this);
		});
	}
}

streamfx::obs::deprecated_source::deprecated_source(std::string name, bool ptrack_ownership, bool add_reference)
	: ::streamfx::obs::deprecated_source::deprecated_source()
{
	this->_self = obs_get_source_by_name(name.c_str());
	if (!this->_self) {
		throw std::runtime_error("source with name not found");
	}

	this->_track_ownership = ptrack_ownership;
	if (!add_reference) {
		obs_source_release(this->_self);
	}
}

streamfx::obs::deprecated_source::deprecated_source(obs_source_t* source, bool ptrack_ownership, bool add_reference)
	: ::streamfx::obs::deprecated_source::deprecated_source()
{
	this->_self = source;
	if (!this->_self) {
		throw std::invalid_argument("source must not be null");
	}

	this->_track_ownership = ptrack_ownership;
	if (add_reference) {
		obs_source_addref(this->_self);
	}
}
/*
obs::deprecated_source::deprecated_source(deprecated_source const& other)
{
	this->_self            = other._self;
	this->_track_ownership = other._track_ownership;

	if (this->_track_ownership) {
		obs_source_addref(this->_self);
	}

#ifdef auto_signal_c
#undef auto_signal_c
#endif
#define auto_signal_c(SIGNAL) this->events.SIGNAL = other.events.SIGNAL
	auto_signal_c(destroy);
	auto_signal_c(remove);
	auto_signal_c(save);
	auto_signal_c(load);
	auto_signal_c(activate);
	auto_signal_c(deactivate);
	auto_signal_c(show);
	auto_signal_c(hide);
	auto_signal_c(mute);
	auto_signal_c(push_to_mute_changed);
	auto_signal_c(push_to_mute_delay);
	auto_signal_c(push_to_talk_changed);
	auto_signal_c(push_to_talk_delay);
	auto_signal_c(enable);
	auto_signal_c(rename);
	auto_signal_c(volume);
	auto_signal_c(update_properties);
	auto_signal_c(update_flags);
	auto_signal_c(audio_sync);
	auto_signal_c(audio_mixers);
	auto_signal_c(audio);
	auto_signal_c(filter_add);
	auto_signal_c(filter_remove);
	auto_signal_c(reorder_filters);
	auto_signal_c(transition_start);
	auto_signal_c(transition_video_stop);
	auto_signal_c(transition_stop);
#undef auto_signal_c
}

obs::deprecated_source& obs::deprecated_source::operator=(deprecated_source const& other)
{
	if (this == &other) {
		return *this;
	}

	// Release previous source.
	if (this->_self && this->_track_ownership) {
		obs_source_release(this->_self);
	}

	this->_self            = other._self;
	this->_track_ownership = other._track_ownership;

	if (this->_track_ownership) {
		obs_source_addref(this->_self);
	}

#ifdef auto_signal_c
#undef auto_signal_c
#endif
#define auto_signal_c(SIGNAL) this->events.SIGNAL = other.events.SIGNAL
	auto_signal_c(destroy);
	auto_signal_c(remove);
	auto_signal_c(save);
	auto_signal_c(load);
	auto_signal_c(activate);
	auto_signal_c(deactivate);
	auto_signal_c(show);
	auto_signal_c(hide);
	auto_signal_c(mute);
	auto_signal_c(push_to_mute_changed);
	auto_signal_c(push_to_mute_delay);
	auto_signal_c(push_to_talk_changed);
	auto_signal_c(push_to_talk_delay);
	auto_signal_c(enable);
	auto_signal_c(rename);
	auto_signal_c(volume);
	auto_signal_c(update_properties);
	auto_signal_c(update_flags);
	auto_signal_c(audio_sync);
	auto_signal_c(audio_mixers);
	auto_signal_c(audio);
	auto_signal_c(filter_add);
	auto_signal_c(filter_remove);
	auto_signal_c(reorder_filters);
	auto_signal_c(transition_start);
	auto_signal_c(transition_video_stop);
	auto_signal_c(transition_stop);
#undef auto_signal_c

	return *this;
}
*/

streamfx::obs::deprecated_source::deprecated_source(deprecated_source&& other)
	: _self(std::move(other._self)), _track_ownership(std::move(other._track_ownership))
{
	// Clean out other source
	other._self            = nullptr;
	other._track_ownership = false;

#ifdef auto_signal_c
#undef auto_signal_c
#endif
#define auto_signal_c(SIGNAL) this->events.SIGNAL = std::move(other.events.SIGNAL)
	auto_signal_c(destroy);
	auto_signal_c(remove);
	auto_signal_c(save);
	auto_signal_c(load);
	auto_signal_c(activate);
	auto_signal_c(deactivate);
	auto_signal_c(show);
	auto_signal_c(hide);
	auto_signal_c(mute);
	auto_signal_c(push_to_mute_changed);
	auto_signal_c(push_to_mute_delay);
	auto_signal_c(push_to_talk_changed);
	auto_signal_c(push_to_talk_delay);
	auto_signal_c(enable);
	auto_signal_c(rename);
	auto_signal_c(volume);
	auto_signal_c(update_properties);
	auto_signal_c(update_flags);
	auto_signal_c(audio_sync);
	auto_signal_c(audio_mixers);
	auto_signal_c(audio);
	auto_signal_c(filter_add);
	auto_signal_c(filter_remove);
	auto_signal_c(reorder_filters);
	auto_signal_c(transition_start);
	auto_signal_c(transition_video_stop);
	auto_signal_c(transition_stop);
#undef auto_signal_c
}

streamfx::obs::deprecated_source& streamfx::obs::deprecated_source::operator=(deprecated_source&& other)
{
	if (this != &other) {
		return *this;
	}

	// Release previous source.
	if (this->_self && this->_track_ownership) {
		obs_source_release(this->_self);
	}

	this->_self            = std::move(other._self);
	this->_track_ownership = std::move(other._track_ownership);
	other._self            = nullptr;
	other._track_ownership = false;

#ifdef auto_signal_c
#undef auto_signal_c
#endif
#define auto_signal_c(SIGNAL) this->events.SIGNAL = std::move(other.events.SIGNAL)
	auto_signal_c(destroy);
	auto_signal_c(remove);
	auto_signal_c(save);
	auto_signal_c(load);
	auto_signal_c(activate);
	auto_signal_c(deactivate);
	auto_signal_c(show);
	auto_signal_c(hide);
	auto_signal_c(mute);
	auto_signal_c(push_to_mute_changed);
	auto_signal_c(push_to_mute_delay);
	auto_signal_c(push_to_talk_changed);
	auto_signal_c(push_to_talk_delay);
	auto_signal_c(enable);
	auto_signal_c(rename);
	auto_signal_c(volume);
	auto_signal_c(update_properties);
	auto_signal_c(update_flags);
	auto_signal_c(audio_sync);
	auto_signal_c(audio_mixers);
	auto_signal_c(audio);
	auto_signal_c(filter_add);
	auto_signal_c(filter_remove);
	auto_signal_c(reorder_filters);
	auto_signal_c(transition_start);
	auto_signal_c(transition_video_stop);
	auto_signal_c(transition_stop);
#undef auto_signal_c

	return *this;
}

obs_source_type streamfx::obs::deprecated_source::type()
{
	if (!_self) {
		return static_cast<obs_source_type>(-1);
	}
	return obs_source_get_type(_self);
}

void* streamfx::obs::deprecated_source::type_data()
{
	if (!_self) {
		return nullptr;
	}
	return obs_source_get_type_data(_self);
}

uint32_t streamfx::obs::deprecated_source::width()
{
	if (!_self) {
		return 0;
	}
	return obs_source_get_width(_self);
}

uint32_t streamfx::obs::deprecated_source::height()
{
	if (!_self) {
		return 0;
	}
	return obs_source_get_height(_self);
}

bool streamfx::obs::deprecated_source::destroyed()
{
	return _self == nullptr;
}

void streamfx::obs::deprecated_source::clear()
{
	_self = nullptr;
}

obs_source_t* streamfx::obs::deprecated_source::get()
{
	return _self;
}
