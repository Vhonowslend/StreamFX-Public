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

#include "obs-audio-capture.h"

void obs::audio_capture::audio_capture_cb(void* data, obs_source_t*, const struct audio_data* audio, bool muted) {
	auto self = reinterpret_cast<obs::audio_capture*>(data);
	self->cb(self->cb_data, audio, muted);
}

obs::audio_capture::audio_capture(obs_source_t* source) {
	this->source = source;
	obs_source_add_audio_capture_callback(this->source, audio_capture_cb, this);
}

obs::audio_capture::~audio_capture() {
	obs_source_remove_audio_capture_callback(this->source, audio_capture_cb, this);
}

void obs::audio_capture::set_callback(audio_capture_callback_t cb, void* data) {
	this->cb = cb;
	this->cb_data = data;
}

void obs::audio_capture::set_callback(audio_capture_callback_t cb) {
	this->cb = cb;
	this->cb_data = nullptr;
}
