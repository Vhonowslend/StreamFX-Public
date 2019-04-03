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

void obs::source::handle_destroy(void* p, calldata_t* calldata)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);

	obs_source_t* source;
	if (!calldata_get_ptr(calldata, "source", &source)) {
		return;
	}

	if (self->self == source) {
		self->self = nullptr;
	}

	if (self->events.destroy) {
		return;
	}
	self->events.destroy(self);
}

void obs::source::handle_remove(void* p, calldata_t*)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.remove) {
		return;
	}
	self->events.remove(self);
}

void obs::source::handle_save(void* p, calldata_t*)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.save) {
		return;
	}
	self->events.save(self);
}

void obs::source::handle_load(void* p, calldata_t*)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.load) {
		return;
	}
	self->events.load(self);
}

void obs::source::handle_activate(void* p, calldata_t*)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.activate) {
		return;
	}
	self->events.activate(self);
}

void obs::source::handle_deactivate(void* p, calldata_t*)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.deactivate) {
		return;
	}
	self->events.deactivate(self);
}

void obs::source::handle_show(void* p, calldata_t*)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.show) {
		return;
	}
	self->events.show(self);
}

void obs::source::handle_hide(void* p, calldata_t*)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.hide) {
		return;
	}
	self->events.hide(self);
}

void obs::source::handle_enable(void* p, calldata_t* calldata)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.enable) {
		return;
	}

	bool enabled = false;
	if (!calldata_get_bool(calldata, "enabled", &enabled)) {
		return;
	}

	self->events.enable(self, enabled);
}

void obs::source::handle_push_to_mute_changed(void* p, calldata_t* calldata)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.push_to_mute_changed) {
		return;
	}

	bool enabled = false;
	if (!calldata_get_bool(calldata, "enabled", &enabled)) {
		return;
	}

	self->events.push_to_mute_changed(self, enabled);
}

void obs::source::handle_push_to_mute_delay(void* p, calldata_t* calldata)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.push_to_mute_delay) {
		return;
	}

	long long delay;
	if (!calldata_get_int(calldata, "delay", &delay)) {
		return;
	}

	self->events.push_to_mute_delay(self, delay);
}

void obs::source::handle_push_to_talk_changed(void* p, calldata_t* calldata)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.push_to_talk_changed) {
		return;
	}

	bool enabled = false;
	if (!calldata_get_bool(calldata, "enabled", &enabled)) {
		return;
	}

	self->events.push_to_talk_changed(self, enabled);
}

void obs::source::handle_push_to_talk_delay(void* p, calldata_t* calldata)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.push_to_talk_delay) {
		return;
	}

	long long delay;
	if (!calldata_get_int(calldata, "delay", &delay)) {
		return;
	}

	self->events.push_to_talk_delay(self, delay);
}

void obs::source::handle_rename(void* p, calldata_t* calldata)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
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
}

void obs::source::handle_update_properties(void* p, calldata_t*)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.update_properties) {
		return;
	}
	self->events.update_properties(self);
}

void obs::source::handle_update_flags(void* p, calldata_t* calldata)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.update_flags) {
		return;
	}

	long long flags;
	if (!calldata_get_int(calldata, "flags", &flags)) {
		return;
	}

	self->events.update_flags(self, flags);
}

void obs::source::handle_mute(void* p, calldata_t* calldata)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.mute) {
		return;
	}

	bool muted;
	if (!calldata_get_bool(calldata, "muted", &muted)) {
		return;
	}

	self->events.mute(self, muted);
}

void obs::source::handle_volume(void* p, calldata_t* calldata)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.volume) {
		return;
	}

	double volume;
	if (!calldata_get_float(calldata, "volume", &volume)) {
		return;
	}

	self->events.volume(self, volume);

	calldata_set_float(calldata, "volume", volume);
}

void obs::source::handle_audio_sync(void* p, calldata_t* calldata)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.audio_sync) {
		return;
	}

	long long mixers;
	if (!calldata_get_int(calldata, "offset", &mixers)) {
		return;
	}

	self->events.audio_sync(self, mixers);

	calldata_set_int(calldata, "offset", mixers);
}

void obs::source::handle_audio_mixers(void* p, calldata_t* calldata)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.audio_mixers) {
		return;
	}

	long long mixers;
	if (!calldata_get_int(calldata, "mixers", &mixers)) {
		return;
	}

	self->events.audio_mixers(self, mixers);

	calldata_set_int(calldata, "mixers", mixers);
}

void obs::source::handle_audio_data(void* p, obs_source_t*, const audio_data* audio, bool muted)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.audio_data) {
		return;
	}

	self->events.audio_data(self, audio, muted);
}

void obs::source::handle_filter_add(void* p, calldata_t* calldata)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.filter_add) {
		return;
	}

	obs_source_t* filter;
	if (!calldata_get_ptr(calldata, "filter", &filter)) {
		return;
	}

	self->events.filter_add(self, filter);
}

void obs::source::handle_filter_remove(void* p, calldata_t* calldata)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.filter_remove) {
		return;
	}

	obs_source_t* filter;
	if (!calldata_get_ptr(calldata, "filter", &filter)) {
		return;
	}

	self->events.filter_remove(self, filter);
}

void obs::source::handle_reorder_filters(void* p, calldata_t*)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.reorder_filters) {
		return;
	}
	self->events.reorder_filters(self);
}

void obs::source::handle_transition_start(void* p, calldata_t*)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.transition_start) {
		return;
	}
	self->events.transition_start(self);
}

void obs::source::handle_transition_video_stop(void* p, calldata_t*)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.transition_video_stop) {
		return;
	}
	self->events.transition_video_stop(self);
}

void obs::source::handle_transition_stop(void* p, calldata_t*)
{
	obs::source* self = reinterpret_cast<obs::source*>(p);
	if (!self->events.transition_stop) {
		return;
	}
	self->events.transition_stop(self);
}

obs::source::~source()
{
#ifdef auto_signal_d
#undef auto_signal_d
#endif
#define auto_signal_d(SIGNAL) this->events.##SIGNAL.clear();
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
	auto_signal_d(audio_data);
	auto_signal_d(filter_add);
	auto_signal_d(filter_remove);
	auto_signal_d(reorder_filters);
	auto_signal_d(transition_start);
	auto_signal_d(transition_video_stop);
	auto_signal_d(transition_stop);
#undef auto_signal_d

	if (this->track_ownership && this->self) {
		obs_source_release(this->self);
	}
	this->self = nullptr;
}

obs::source::source()
{
#ifdef auto_signal_c
#undef auto_signal_c
#endif
#define auto_signal_c(SIGNAL)                                                                  \
	{                                                                                          \
		this->events.##SIGNAL.set_listen_callback([this] {                                     \
			if (!this->self)                                                                   \
				return;                                                                        \
			auto sh = obs_source_get_signal_handler(this->self);                               \
			if (sh) {                                                                          \
				signal_handler_connect(sh, "" #SIGNAL, obs::source::handle_##SIGNAL, this);    \
			}                                                                                  \
		});                                                                                    \
		this->events.##SIGNAL.set_silence_callback([this] {                                    \
			if (!this->self)                                                                   \
				return;                                                                        \
			auto sh = obs_source_get_signal_handler(this->self);                               \
			if (sh) {                                                                          \
				signal_handler_disconnect(sh, "" #SIGNAL, obs::source::handle_##SIGNAL, this); \
			}                                                                                  \
		});                                                                                    \
	}
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
	auto_signal_c(filter_add);
	auto_signal_c(filter_remove);
	auto_signal_c(reorder_filters);
	auto_signal_c(transition_start);
	auto_signal_c(transition_video_stop);
	auto_signal_c(transition_stop);
#undef auto_signal_c

	// libOBS unfortunately does not use the event system for audio data callbacks, which is kind of odd as most other
	//  things do. So instead we'll have to manually deal with it for now.
	{
		this->events.audio_data.set_listen_callback(
			[this] { obs_source_add_audio_capture_callback(this->self, obs::source::handle_audio_data, this); });
		this->events.audio_data.set_silence_callback(
			[this] { obs_source_remove_audio_capture_callback(this->self, obs::source::handle_audio_data, this); });
	}
}

obs::source::source(std::string name, bool track_ownership, bool add_reference) : source()
{
	this->self = obs_get_source_by_name(name.c_str());
	if (!this->self) {
		throw std::runtime_error("source with name not found");
	}

	this->track_ownership = track_ownership;
	if (!add_reference) {
		obs_source_release(this->self);
	}
}

obs::source::source(obs_source_t* source, bool track_ownership, bool add_reference) : source()
{
	this->self = source;
	if (!this->self) {
		throw std::invalid_argument("source must not be null");
	}

	this->track_ownership = track_ownership;
	if (add_reference) {
		obs_source_addref(this->self);
	}
}

obs::source::source(source const& other)
{
	this->self            = other.self;
	this->track_ownership = other.track_ownership;

	if (this->track_ownership) {
		obs_source_addref(this->self);
	}

#ifdef auto_signal_c
#undef auto_signal_c
#endif
#define auto_signal_c(SIGNAL) this->events.##SIGNAL = other.events.##SIGNAL;
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
	auto_signal_c(audio_data);
	auto_signal_c(filter_add);
	auto_signal_c(filter_remove);
	auto_signal_c(reorder_filters);
	auto_signal_c(transition_start);
	auto_signal_c(transition_video_stop);
	auto_signal_c(transition_stop);
#undef auto_signal_c
}

obs::source& obs::source::operator=(source const& other)
{
	if (this == &other) {
		return *this;
	}

	// Release previous source.
	if (this->self && this->track_ownership) {
		obs_source_release(this->self);
	}

	this->self            = other.self;
	this->track_ownership = other.track_ownership;

	if (this->track_ownership) {
		obs_source_addref(this->self);
	}

#ifdef auto_signal_c
#undef auto_signal_c
#endif
#define auto_signal_c(SIGNAL) this->events.##SIGNAL = other.events.##SIGNAL;
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
	auto_signal_c(audio_data);
	auto_signal_c(filter_add);
	auto_signal_c(filter_remove);
	auto_signal_c(reorder_filters);
	auto_signal_c(transition_start);
	auto_signal_c(transition_video_stop);
	auto_signal_c(transition_stop);
#undef auto_signal_c

	return *this;
}

obs::source::source(source&& other) : self(std::move(other.self)), track_ownership(std::move(other.track_ownership))
{
	// Clean out other source
	other.self            = nullptr;
	other.track_ownership = false;

#ifdef auto_signal_c
#undef auto_signal_c
#endif
#define auto_signal_c(SIGNAL) this->events.##SIGNAL = std::move(other.events.##SIGNAL);
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
	auto_signal_c(audio_data);
	auto_signal_c(filter_add);
	auto_signal_c(filter_remove);
	auto_signal_c(reorder_filters);
	auto_signal_c(transition_start);
	auto_signal_c(transition_video_stop);
	auto_signal_c(transition_stop);
#undef auto_signal_c
}

obs::source& obs::source::operator=(source&& other)
{
	if (this != &other) {
		return *this;
	}

	// Release previous source.
	if (this->self && this->track_ownership) {
		obs_source_release(this->self);
	}

	this->self            = std::move(other.self);
	this->track_ownership = std::move(other.track_ownership);
	other.self            = nullptr;
	other.track_ownership = false;

#ifdef auto_signal_c
#undef auto_signal_c
#endif
#define auto_signal_c(SIGNAL) this->events.##SIGNAL = std::move(other.events.##SIGNAL);
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
	auto_signal_c(audio_data);
	auto_signal_c(filter_add);
	auto_signal_c(filter_remove);
	auto_signal_c(reorder_filters);
	auto_signal_c(transition_start);
	auto_signal_c(transition_video_stop);
	auto_signal_c(transition_stop);
#undef auto_signal_c

	return *this;
}

obs_source_type obs::source::type()
{
	if (!self) {
		return (obs_source_type)-1;
	}
	return obs_source_get_type(self);
}

void* obs::source::type_data()
{
	if (!self) {
		return nullptr;
	}
	return obs_source_get_type_data(self);
}

uint32_t obs::source::width()
{
	if (!self) {
		return 0;
	}
	return obs_source_get_width(self);
}

uint32_t obs::source::height()
{
	if (!self) {
		return 0;
	}
	return obs_source_get_height(self);
}

bool obs::source::destroyed()
{
	return self == nullptr;
}

void obs::source::clear()
{
	self = nullptr;
}

obs_source_t* obs::source::get()
{
	return self;
}
