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

#include "obs-audio-capture.hpp"

void obs::audio_capture::audio_capture_cb(void* data, obs_source_t*, const struct audio_data* audio, bool muted)
{
	auto self = reinterpret_cast<obs::audio_capture*>(data);
	if (self->on.data) {
		self->on.data(self->m_self, audio, muted);
	}
}

void obs::audio_capture::on_data_listen()
{
	if (this->m_self) {
		obs_source_add_audio_capture_callback(this->m_self->get(), audio_capture_cb, this);
	}
}

void obs::audio_capture::on_data_silence()
{
	if (this->m_self) {
		obs_source_remove_audio_capture_callback(this->m_self->get(), audio_capture_cb, this);
	}
}

obs::audio_capture::audio_capture(obs_source_t* source)
{
	this->m_self = std::make_shared<obs::source>(source, true, true);
	this->on.data.set_listen_callback(std::bind(&obs::audio_capture::on_data_listen, this));
	this->on.data.set_silence_callback(std::bind(&obs::audio_capture::on_data_silence, this));
}

obs::audio_capture::audio_capture(std::shared_ptr<obs::source> source) : audio_capture(source->get()) {}

obs::audio_capture::~audio_capture()
{
	on.data.clear();
	this->m_self.reset();
}
