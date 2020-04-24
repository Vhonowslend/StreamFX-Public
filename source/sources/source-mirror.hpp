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
#include "common.hpp"
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include "gfx/gfx-source-texture.hpp"
#include "obs/gs/gs-rendertarget.hpp"
#include "obs/gs/gs-sampler.hpp"
#include "obs/obs-signal-handler.hpp"
#include "obs/obs-source-factory.hpp"
#include "obs/obs-source.hpp"
#include "obs/obs-tools.hpp"

namespace streamfx::source::mirror {
	struct mirror_audio_data {
		mirror_audio_data(const audio_data*, speaker_layout);

		obs_source_audio                       osa;
		std::vector<std::vector<std::uint8_t>> data;
	};

	class mirror_instance : public obs::source_instance {
		// Source
		std::shared_ptr<obs_source_t>               _source;
		std::shared_ptr<obs::tools::child_source>   _source_child;
		std::shared_ptr<obs::source_signal_handler> _signal_rename;
		std::shared_ptr<obs::audio_signal_handler>  _signal_audio;
		std::pair<uint32_t, uint32_t>               _source_size;

		// Audio
		bool                          _audio_enabled;
		speaker_layout                _audio_layout;
		std::mutex                    _audio_queue_lock;
		std::queue<mirror_audio_data> _audio_queue;

		public:
		mirror_instance(obs_data_t* settings, obs_source_t* self);
		virtual ~mirror_instance();

		virtual std::uint32_t get_width() override;
		virtual std::uint32_t get_height() override;

		virtual void load(obs_data_t*) override;
		virtual void migrate(obs_data_t*, std::uint64_t) override;
		virtual void update(obs_data_t*) override;
		virtual void save(obs_data_t*) override;

		virtual void video_tick(float_t) override;
		virtual void video_render(gs_effect_t*) override;

		virtual void enum_active_sources(obs_source_enum_proc_t, void*) override;
		virtual void enum_all_sources(obs_source_enum_proc_t, void*) override;

		private:
		void acquire(std::string source_name);
		void release();

		void on_rename(std::shared_ptr<obs_source_t>, calldata*);
		void on_audio(std::shared_ptr<obs_source_t>, const struct audio_data*, bool);

		void audio_output(std::shared_ptr<void> data);
	};

	class mirror_factory : public obs::source_factory<source::mirror::mirror_factory, source::mirror::mirror_instance> {
		public:
		mirror_factory();
		virtual ~mirror_factory() override;

		virtual const char* get_name() override;

		virtual void get_defaults2(obs_data_t* data) override;

		virtual obs_properties_t* get_properties2(source::mirror::mirror_instance* data) override;

		public: // Singleton
		static void initialize();

		static void finalize();

		static std::shared_ptr<mirror_factory> get();
	};
} // namespace streamfx::source::mirror
