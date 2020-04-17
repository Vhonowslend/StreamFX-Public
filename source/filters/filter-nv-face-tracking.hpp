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
#include <atomic>
#include <vector>
#include "obs/gs/gs-effect.hpp"
#include "obs/gs/gs-rendertarget.hpp"
#include "obs/gs/gs-vertexbuffer.hpp"
#include "obs/obs-source-factory.hpp"

// Nvidia
#include "nvidia/ar/nvidia-ar.hpp"
#include "nvidia/cuda/nvidia-cuda-context.hpp"
#include "nvidia/cuda/nvidia-cuda-gs-texture.hpp"
#include "nvidia/cuda/nvidia-cuda-memory.hpp"
#include "nvidia/cuda/nvidia-cuda-stream.hpp"
#include "nvidia/cuda/nvidia-cuda.hpp"

namespace filter::nvidia {
	class face_tracking_instance : public obs::source_instance {
		// Filter Cache
		bool                                    _rt_is_fresh;
		std::shared_ptr<gs::rendertarget>       _rt;
		std::pair<std::uint32_t, std::uint32_t> _size;

		// Settings
		double_t                      _cfg_roi_zoom;
		std::pair<double_t, double_t> _cfg_roi_offset;
		double_t                      _cfg_roi_stability;

		// Region of Interest
		util::math::kalman1D<double_t>     _roi_filters[4];
		std::mutex                         _roi_lock;
		std::pair<double_t, double_t>      _roi_center;
		std::pair<double_t, double_t>      _roi_size;
		std::shared_ptr<gs::vertex_buffer> _roi_geom;

		// Nvidia CUDA interop
		std::shared_ptr<::nvidia::cuda::cuda>    _cuda;
		std::shared_ptr<::nvidia::cuda::context> _cuda_ctx;
		std::shared_ptr<::nvidia::cuda::stream>  _cuda_stream;

		// Nvidia AR interop
		std::shared_ptr<::nvidia::ar::ar>          _ar_library;
		std::atomic_bool                           _ar_loaded;
		std::shared_ptr<nvAR_Feature>              _ar_feature;
		std::atomic_bool                           _ar_tracked;
		std::vector<float_t>                       _ar_bboxes_confidence;
		std::vector<NvAR_Rect>                     _ar_bboxes_data;
		NvAR_BBoxes                                _ar_bboxes;
		std::shared_ptr<gs::texture>               _ar_texture;
		bool                                       _ar_texture_cuda_fresh;
		std::shared_ptr<::nvidia::cuda::gstexture> _ar_texture_cuda;
		std::shared_ptr<::nvidia::cuda::memory>    _ar_texture_cuda_mem;
		NvCVImage                                  _ar_image;
		NvCVImage                                  _ar_image_bgr;
		NvCVImage                                  _ar_image_temp;

#ifdef _DEBUG
		// Profiling
		std::shared_ptr<util::profiler> _profile_capture;
		std::shared_ptr<util::profiler> _profile_capture_realloc;
		std::shared_ptr<util::profiler> _profile_capture_copy;
		std::shared_ptr<util::profiler> _profile_ar_realloc;
		std::shared_ptr<util::profiler> _profile_ar_copy;
		std::shared_ptr<util::profiler> _profile_ar_transfer;
		std::shared_ptr<util::profiler> _profile_ar_run;
		std::shared_ptr<util::profiler> _profile_ar_calc;
#endif

		public:
		face_tracking_instance(obs_data_t*, obs_source_t*);
		virtual ~face_tracking_instance() override;

		// Tasks
		void async_initialize(std::shared_ptr<void>);

		void refresh_geometry();

		void async_track(std::shared_ptr<void>);

		// Create image buffer.
		//void create_image_buffer(std::size_t width, std::size_t height);

		void roi_refresh();

		void roi_reset();

		virtual void load(obs_data_t* data) override;

		virtual void migrate(obs_data_t* data, std::uint64_t version) override;

		virtual void update(obs_data_t* data) override;

		virtual void video_tick(float_t seconds) override;

		virtual void video_render(gs_effect_t* effect) override;

#ifdef _DEBUG
		bool button_profile(obs_properties_t* props, obs_property_t* property);
#endif
	};

	class face_tracking_factory
		: public obs::source_factory<filter::nvidia::face_tracking_factory, filter::nvidia::face_tracking_instance> {
		static std::shared_ptr<filter::nvidia::face_tracking_factory> factory_instance;

		std::shared_ptr<::nvidia::cuda::cuda>    _cuda;
		std::shared_ptr<::nvidia::cuda::context> _cuda_ctx;
		std::shared_ptr<::nvidia::ar::ar>        _ar;

		public: // Singleton
		static void initialize()
		try {
			factory_instance = std::make_shared<filter::nvidia::face_tracking_factory>();
		} catch (const std::exception& ex) {
			LOG_ERROR("<Nvidia Face Tracking Filter> %s", ex.what());
		}

		static void finalize()
		{
			factory_instance.reset();
		}

		static std::shared_ptr<face_tracking_factory> get()
		{
			return factory_instance;
		}

		public:
		face_tracking_factory();
		virtual ~face_tracking_factory() override;

		virtual const char* get_name() override;

		virtual void get_defaults2(obs_data_t* data) override;

		virtual obs_properties_t* get_properties2(filter::nvidia::face_tracking_instance* data) override;

		std::shared_ptr<::nvidia::cuda::cuda> get_cuda();

		std::shared_ptr<::nvidia::cuda::context> get_cuda_context();

		std::shared_ptr<::nvidia::ar::ar> get_ar();
	};
} // namespace filter::nvidia
