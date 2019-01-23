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
#include <functional>
#include "obs-source.hpp"

// OBS
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <obs.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace obs {
	class audio_capture {
		std::shared_ptr<obs::source> m_self;

		static void audio_capture_cb(void*, obs_source_t*, struct audio_data const*, bool);

		void on_data_listen();
		void on_data_silence();

		public:
		audio_capture(obs_source_t* source);

		audio_capture(std::shared_ptr<obs::source> source);

		~audio_capture();

		public /*copy*/:
		audio_capture(audio_capture const& other) = delete;
		audio_capture& operator=(audio_capture const& other) = delete;

		public /*move*/:
		audio_capture(audio_capture&& other) = delete;
		audio_capture& operator=(audio_capture&& other) = delete;

		public /*events*/:
		struct {
			//! Called if there is new audio data.
			//
			// @param std::shared_ptr<obs::source> Source
			// @param audio_data const* const Audio Data
			// @param bool Muted
			util::event<std::shared_ptr<obs::source>, audio_data const* const, bool> data;
		} on;
	};
} // namespace obs
