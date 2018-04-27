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

#pragma once
#include <obs.h>
#include <functional>

namespace obs {
	typedef std::function<void(void* data, struct audio_data const *audio, bool muted)> audio_capture_callback_t;

	class audio_capture {
		obs_source_t* source;
		audio_capture_callback_t cb;
		void* cb_data;

		static void audio_capture_cb(void*, obs_source_t*, struct audio_data const *, bool);

		public:
		audio_capture(obs_source_t* source);
		virtual ~audio_capture();

		void set_callback(audio_capture_callback_t cb, void* data);
		void set_callback(audio_capture_callback_t cb);
	};
}
