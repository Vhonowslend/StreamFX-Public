/*
* Modern effects for a modern Streamer
* Copyright (C) 2017 Michael Fabian Dirks
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
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include "gfx-source-texture.h"
#include "gs-rendertarget.h"
#include "gs-sampler.h"
#include "obs-audio-capture.h"
#include "obs-source.hpp"
#include "plugin.h"

extern "C" {
#include <obs-source.h>
}

namespace Source {
	class MirrorAddon {
		obs_source_info osi;

		public:
		MirrorAddon();
		~MirrorAddon();

		static const char *get_name(void *);
		static void get_defaults(obs_data_t *);
		static bool modified_properties(obs_properties_t *, obs_property_t *, obs_data_t *);
		static obs_properties_t *get_properties(void *);

		static void *create(obs_data_t *, obs_source_t *);
		static void destroy(void *);

		static uint32_t get_width(void *);
		static uint32_t get_height(void *);

		static void update(void *, obs_data_t *);
		static void activate(void *);
		static void deactivate(void *);
		static void video_tick(void *, float);
		static void video_render(void *, gs_effect_t *);
		static void enum_active_sources(void *, obs_source_enum_proc_t, void *);
	};

	class Mirror {
		bool m_active = false;
		obs_source_t* m_source = nullptr;
		float_t m_tick = 0;

		// Input
		obs_scene_t* m_scene = nullptr;
		std::shared_ptr<obs::source> m_scene_source;
		std::unique_ptr<gfx::source_texture> m_source_texture;

		// Input Source
		obs_sceneitem_t* m_sceneitem = nullptr;
		std::string m_mirrorName;

		// Scaling
		bool m_rescale = false;
		uint32_t m_width, m_height;
		gs_effect_t* m_scalingEffect = nullptr;
		bool m_keepOriginalSize = false;
		std::unique_ptr<gs::rendertarget> m_renderTargetScale;
		std::shared_ptr<gs::sampler> m_sampler;

		// Audio
		bool m_enableAudio = false;
		std::unique_ptr<obs::audio_capture> m_audioCapture;
		std::mutex m_audioLock;
		std::condition_variable m_audioNotify;
		obs_source_audio m_audioOutput;
		std::vector<std::vector<float_t>> m_audioData;
		std::thread m_audioThread;
		bool m_killAudioThread = false;
		bool m_haveAudioOutput = false;

		public:
		Mirror(obs_data_t*, obs_source_t*);
		~Mirror();

		uint32_t get_width();
		uint32_t get_height();

		void update(obs_data_t*);
		void activate();
		void deactivate();
		void video_tick(float);
		void video_render(gs_effect_t*);
		void audio_capture_cb(void* data, const audio_data* audio, bool muted);
		void audio_output_cb();
		void enum_active_sources(obs_source_enum_proc_t, void *);
	};
};
