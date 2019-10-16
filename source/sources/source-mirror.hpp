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
#include <queue>
#include <thread>
#include <vector>
#include "gfx/gfx-source-texture.hpp"
#include "obs/gs/gs-rendertarget.hpp"
#include "obs/gs/gs-sampler.hpp"
#include "obs/obs-source-factory.hpp"
#include "obs/obs-source.hpp"
#include "plugin.hpp"

// OBS
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <obs-source.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace source {
	namespace mirror {
		struct mirror_audio_data {
			obs_source_audio                  audio = {};
			std::vector<std::vector<float_t>> data;
		};

		class mirror_instance : public obs::source_instance {
			// Source
			std::shared_ptr<obs::source> _source;
			std::string                  _source_name;

			// Audio
			bool                                           _audio_enabled;
			speaker_layout                                 _audio_layout;
			std::condition_variable                        _audio_notify;
			std::thread                                    _audio_thread;
			bool                                           _audio_kill_thread;
			bool                                           _audio_have_output;
			std::mutex                                     _audio_lock_outputter;
			std::mutex                                     _audio_lock_capturer;
			std::queue<std::shared_ptr<mirror_audio_data>> _audio_data_queue;
			std::queue<std::shared_ptr<mirror_audio_data>> _audio_data_free_queue;

			// Scaling
			bool            _rescale_enabled;
			uint32_t        _rescale_width;
			uint32_t        _rescale_height;
			bool            _rescale_keep_orig_size;
			obs_scale_type  _rescale_type;
			obs_bounds_type _rescale_bounds;
			uint32_t        _rescale_alignment;

			// Caching
			bool                                 _cache_enabled;
			bool                                 _cache_rendered;
			std::shared_ptr<gfx::source_texture> _cache_renderer;
			std::shared_ptr<gs::texture>         _cache_texture;

			// Scene
			std::shared_ptr<obs_source_t>    _scene;
			std::shared_ptr<obs_sceneitem_t> _source_item;

			private:
			void release();
			void acquire(std::string source_name);

			public:
			mirror_instance(obs_data_t* settings, obs_source_t* self);
			virtual ~mirror_instance();

			virtual uint32_t get_width() override;
			virtual uint32_t get_height() override;

			virtual void update(obs_data_t*) override;
			virtual void load(obs_data_t*) override;
			virtual void save(obs_data_t*) override;

			virtual void video_tick(float) override;
			virtual void video_render(gs_effect_t*) override;

			virtual void enum_active_sources(obs_source_enum_proc_t, void*) override;
			virtual void enum_all_sources(obs_source_enum_proc_t, void*) override;

			void audio_output_cb() noexcept;

			void on_source_rename(obs::source* source, std::string new_name, std::string old_name);
			void on_audio_data(obs::source* source, const audio_data* audio, bool muted);
		};

		class mirror_factory
			: public obs::source_factory<source::mirror::mirror_factory, source::mirror::mirror_instance> {
			static std::shared_ptr<source::mirror::mirror_factory> factory_instance;

			public: // Singleton
			static void initialize()
			{
				factory_instance = std::make_shared<source::mirror::mirror_factory>();
			}

			static void finalize()
			{
				factory_instance.reset();
			}

			static std::shared_ptr<mirror_factory> get()
			{
				return factory_instance;
			}

			public:
			mirror_factory();
			virtual ~mirror_factory() override;

			virtual const char* get_name() override;

			virtual void get_defaults2(obs_data_t* data) override;

			virtual obs_properties_t* get_properties2(source::mirror::mirror_instance* data) override;
		};
	} // namespace mirror
};    // namespace source
