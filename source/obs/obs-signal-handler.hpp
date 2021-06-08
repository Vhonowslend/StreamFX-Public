/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2020 Michael Fabian Dirks
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
#include "common.hpp"
#include "util/util-event.hpp"

namespace streamfx::obs {
	template<typename T>
	class signal_handler_base {
		protected:
		std::string _signal;

		public:
		streamfx::util::event<T, calldata*> event;
	};

	template<typename T>
	class signal_handler : public signal_handler_base<T> {
		public:
		signal_handler(std::string signal, T keepalive) {}
		virtual ~signal_handler() {}
	};

	template<>
	class signal_handler<std::shared_ptr<obs_source_t>> : public signal_handler_base<std::shared_ptr<obs_source_t>> {
		std::shared_ptr<obs_source_t> _keepalive;

		static void handle_signal(void* ptr, calldata* cd) noexcept
		try {
			auto p = reinterpret_cast<signal_handler<std::shared_ptr<obs_source_t>>*>(ptr);
			p->event(p->_keepalive, cd);
		} catch (...) {
		}

		public:
		signal_handler(std::string signal, std::shared_ptr<obs_source_t> keepalive) : _keepalive(keepalive)
		{
			_signal              = signal;
			signal_handler_t* sh = obs_source_get_signal_handler(_keepalive.get());
			signal_handler_connect(sh, _signal.c_str(), handle_signal, this);
		}
		virtual ~signal_handler()
		{
			event.clear();
			signal_handler_t* sh = obs_source_get_signal_handler(_keepalive.get());
			signal_handler_disconnect(sh, _signal.c_str(), handle_signal, this);
		}
	};

	typedef signal_handler<std::shared_ptr<obs_source_t>> source_signal_handler;

	// Audio Capture is also here, as it could be considered a signal.
	class audio_signal_handler {
		std::shared_ptr<obs_source_t> _keepalive;

		static void handle_audio(void* ptr, obs_source_t*, const struct audio_data* audio_data, bool muted) noexcept
		try {
			auto p = reinterpret_cast<audio_signal_handler*>(ptr);
			p->event(p->_keepalive, audio_data, muted);
		} catch (...) {
		}

		public:
		audio_signal_handler(std::shared_ptr<obs_source_t> keepalive) : _keepalive(keepalive), event()
		{
			obs_source_add_audio_capture_callback(_keepalive.get(), handle_audio, this);
		}
		virtual ~audio_signal_handler()
		{
			event.clear();
			obs_source_remove_audio_capture_callback(_keepalive.get(), handle_audio, this);
		}

		streamfx::util::event<std::shared_ptr<obs_source_t>, const struct audio_data*, bool> event;
	};

} // namespace streamfx::obs
