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

#pragma once
#include <functional>
#include <map>
#include <memory>

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
	class source_tracker {
		std::map<std::string, obs_weak_source_t*> _source_map;

		static void source_create_handler(void* ptr, calldata_t* data) noexcept;
		static void source_destroy_handler(void* ptr, calldata_t* data) noexcept;
		static void source_rename_handler(void* ptr, calldata_t* data) noexcept;

		public: // Singleton
		static void                                 initialize();
		static void                                 finalize();
		static std::shared_ptr<obs::source_tracker> get();

		public:
		source_tracker();
		~source_tracker();

		public:
		// Callback function for enumerating sources.
		//
		// @param std::string Name of the Source
		// @param obs_source_t* Source
		// @return true to abort enumeration, false to keep going.
		typedef std::function<bool(std::string, obs_source_t*)> enumerate_cb_t;

		// Filter function for enumerating sources.
		//
		// @param std::string Name of the Source
		// @param obs_source_t* Source
		// @return true to skip, false to pass along.
		typedef std::function<bool(std::string, obs_source_t*)> filter_cb_t;

		//! Enumerate all tracked sources
		//
		// @param enumerate_cb The function called for each tracked source.
		// @param filter_cb Filter function to narrow down results.
		void enumerate(enumerate_cb_t enumerate_cb, filter_cb_t filter_cb = nullptr);

		public:
		static bool filter_sources(std::string name, obs_source_t* source);
		static bool filter_audio_sources(std::string name, obs_source_t* source);
		static bool filter_video_sources(std::string name, obs_source_t* source);
		static bool filter_transitions(std::string name, obs_source_t* source);
		static bool filter_scenes(std::string name, obs_source_t* source);
	};
} // namespace obs
