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
#include <atomic>
#include <memory>
#include <mutex>
#include "obs/gs/gs-rendertarget.hpp"
#include "obs/gs/gs-texture.hpp"
#include "obs/gs/gs-vertexbuffer.hpp"
#include "obs/obs-source-factory.hpp"
#include "plugin.hpp"
#include "util/util-threadpool.hpp"

#ifdef ENABLE_FILTER_AUTOFRAMING_NVIDIA
#include "nvidia/ar/nvidia-ar-facedetection.hpp"
#endif

namespace streamfx::filter::autoframing {
	enum autoframing_provider {
		INVALID              = -1,
		AUTOMATIC            = 0,
		NVIDIA_FACEDETECTION = 1,
	};

	const char* cstring(autoframing_provider provider);

	std::string string(autoframing_provider provider);

	class autoframing_instance : public obs::source_instance {
		std::pair<uint32_t, uint32_t> _size;

		autoframing_provider                    _provider;
		autoframing_provider                    _provider_ui;
		std::atomic<bool>                       _provider_ready;
		std::mutex                              _provider_lock;
		std::shared_ptr<util::threadpool::task> _provider_task;

		std::shared_ptr<::streamfx::obs::gs::rendertarget>  _input;
		std::shared_ptr<::streamfx::obs::gs::vertex_buffer> _vb;
		bool                                                _dirty;

#ifdef ENABLE_FILTER_AUTOFRAMING_NVIDIA
		std::shared_ptr<::streamfx::nvidia::ar::facedetection> _nvidia_fx;
#endif

		/*

		std::shared_ptr<gs::vertex_buffer>  _vb;
		std::shared_ptr<::gs::rendertarget> _capture;
		bool                                _capture_fresh;

		// Configuration
		bool                    _force_reinit;
		bool                    _track_groups;
		std::pair<float, float> _padding;
		std::pair<float, float> _offset;
		engine                  _engine;

		// Engines
		engine _last_engine;
		struct tnvidia {
			bool available;

			std::shared_ptr<::nvidia::cuda::obs>         _cobs;
			std::shared_ptr<::nvidia::ar::ar>            _ar;
			std::shared_ptr<::nvidia::ar::facedetection> _detector;
		} _nvidia;

		// Tracking, Framing
		std::vector<rect> _regions;
		rect              _frame;
		*/
		public:
		~autoframing_instance();
		autoframing_instance(obs_data_t* settings, obs_source_t* self);

		void load(obs_data_t* data) override;
		void migrate(obs_data_t* data, uint64_t version) override;
		void update(obs_data_t* data) override;
		void properties(obs_properties_t* properties);

		uint32_t get_width() override;
		uint32_t get_height() override;

		virtual void video_tick(float_t seconds) override;
		virtual void video_render(gs_effect_t* effect) override;

		private:
		void switch_provider(autoframing_provider provider);
		void task_switch_provider(util::threadpool_data_t data);

#ifdef ENABLE_FILTER_AUTOFRAMING_NVIDIA
		void nvar_facedetection_load();
		void nvar_facedetection_unload();
		void nvar_facedetection_process();
		void nvar_facedetection_properties(obs_properties_t* props);
		void nvar_facedetection_update(obs_data_t* data);
#endif
	};

	class autoframing_factory : public obs::source_factory<streamfx::filter::autoframing::autoframing_factory,
														   streamfx::filter::autoframing::autoframing_instance> {
#ifdef ENABLE_FILTER_AUTOFRAMING_NVIDIA
		bool                                           _nvidia_available;
		std::shared_ptr<::streamfx::nvidia::cuda::obs> _nvcuda;
		std::shared_ptr<::streamfx::nvidia::cv::cv>    _nvcvi;
		std::shared_ptr<::streamfx::nvidia::ar::ar>    _nvar;
#endif

		public:
		autoframing_factory();
		~autoframing_factory() override;

		const char* get_name() override;

		void              get_defaults2(obs_data_t* data) override;
		obs_properties_t* get_properties2(autoframing_instance* data) override;

#ifdef ENABLE_FRONTEND
		static bool on_manual_open(obs_properties_t* props, obs_property_t* property, void* data);
#endif

		bool                 is_provider_available(autoframing_provider);
		autoframing_provider find_ideal_provider();

		public: // Singleton
		static void                                 initialize();
		static void                                 finalize();
		static std::shared_ptr<autoframing_factory> get();
	};
} // namespace streamfx::filter::autoframing
