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
#include "common.hpp"
#include "util/util-event.hpp"

namespace obs {
	class deprecated_source {
		obs_source_t* _self;
		bool          _track_ownership = false;

		static void handle_destroy(void* p, calldata_t* calldata) noexcept;
		static void handle_remove(void* p, calldata_t* calldata) noexcept;
		static void handle_save(void* p, calldata_t* calldata) noexcept;
		static void handle_load(void* p, calldata_t* calldata) noexcept;
		static void handle_activate(void* p, calldata_t* calldata) noexcept;
		static void handle_deactivate(void* p, calldata_t* calldata) noexcept;
		static void handle_show(void* p, calldata_t* calldata) noexcept;
		static void handle_hide(void* p, calldata_t* calldata) noexcept;
		static void handle_enable(void* p, calldata_t* calldata) noexcept;
		static void handle_push_to_mute_changed(void* p, calldata_t* calldata) noexcept;
		static void handle_push_to_mute_delay(void* p, calldata_t* calldata) noexcept;
		static void handle_push_to_talk_changed(void* p, calldata_t* calldata) noexcept;
		static void handle_push_to_talk_delay(void* p, calldata_t* calldata) noexcept;
		static void handle_rename(void* p, calldata_t* calldata) noexcept;
		static void handle_update_properties(void* p, calldata_t* calldata) noexcept;
		static void handle_update_flags(void* p, calldata_t* calldata) noexcept;
		static void handle_mute(void* p, calldata_t* calldata) noexcept;
		static void handle_volume(void* p, calldata_t* calldata) noexcept;
		static void handle_audio_sync(void* p, calldata_t* calldata) noexcept;
		static void handle_audio_mixers(void* p, calldata_t* calldata) noexcept;
		static void handle_audio_data(void* p, obs_source_t* source, const audio_data* audio, bool muted) noexcept;
		static void handle_filter_add(void* p, calldata_t* calldata) noexcept;
		static void handle_filter_remove(void* p, calldata_t* calldata) noexcept;
		static void handle_reorder_filters(void* p, calldata_t* calldata) noexcept;
		static void handle_transition_start(void* p, calldata_t* calldata) noexcept;
		static void handle_transition_video_stop(void* p, calldata_t* calldata) noexcept;
		static void handle_transition_stop(void* p, calldata_t* calldata) noexcept;

		public:
		virtual ~deprecated_source();

		deprecated_source();

		deprecated_source(std::string name, bool track_ownership = true, bool add_reference = true);

		deprecated_source(obs_source_t* source, bool track_ownership = true, bool add_reference = false);

		public /*copy*/:
		deprecated_source(deprecated_source const& other) = delete;
		deprecated_source& operator=(deprecated_source const& other) = delete;

		public /*move*/:
		deprecated_source(deprecated_source&& other);
		deprecated_source& operator=(deprecated_source&& other);

		public:
		obs_source_type type();

		void* type_data();

		uint32_t width();
		uint32_t height();

		bool destroyed();

		public: // Unsafe Methods
		void clear();

		obs_source_t* get();

		public: // Events
		struct {
			// Destroy and Remove
			util::event<obs::deprecated_source*> destroy;
			util::event<obs::deprecated_source*> remove;

			// Saving, Loading and Update
			util::event<obs::deprecated_source*> save;
			util::event<obs::deprecated_source*> load;
			util::event<obs::deprecated_source*> update_properties;

			// Activate, Deactivate
			util::event<obs::deprecated_source*> activate;
			util::event<obs::deprecated_source*> deactivate;

			// Show Hide
			util::event<obs::deprecated_source*> show;
			util::event<obs::deprecated_source*> hide;

			// Other
			util::event<obs::deprecated_source*, bool>                     enable;
			util::event<obs::deprecated_source*, std::string, std::string> rename;
			util::event<obs::deprecated_source*, long long>                update_flags;

			// Hotkeys (PtM, PtT)
			util::event<obs::deprecated_source*, bool>      push_to_mute_changed;
			util::event<obs::deprecated_source*, long long> push_to_mute_delay;
			util::event<obs::deprecated_source*, bool>      push_to_talk_changed;
			util::event<obs::deprecated_source*, long long> push_to_talk_delay;

			// Audio
			util::event<obs::deprecated_source*, bool>                    mute;
			util::event<obs::deprecated_source*, double&>                 volume;
			util::event<obs::deprecated_source*, long long&>              audio_sync;
			util::event<obs::deprecated_source*, long long&>              audio_mixers;
			util::event<obs::deprecated_source*, const audio_data*, bool> audio;

			// Filters
			util::event<obs::deprecated_source*, obs_source_t*> filter_add;
			util::event<obs::deprecated_source*, obs_source_t*> filter_remove;
			util::event<obs::deprecated_source*>                reorder_filters;

			// Transition
			util::event<obs::deprecated_source*> transition_start;
			util::event<obs::deprecated_source*> transition_video_stop;
			util::event<obs::deprecated_source*> transition_stop;
		} events;
	};
} // namespace obs
