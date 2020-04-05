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
		std::uint32_t                     _width;
		std::uint32_t                     _height;
		bool                              _up_to_date;
		std::shared_ptr<gs::rendertarget> _rt;

		// Settings
		double_t                      _cfg_roi_zoom;
		std::pair<double_t, double_t> _cfg_roi_offset;
		double_t                      _cfg_roi_stability;

		// Region of Interest
		util::math::kalman1D<double_t> _roi_filters[4];
		std::pair<double_t, double_t>  _roi_center;
		std::pair<double_t, double_t>  _roi_size;
		gs::vertex_buffer              _roi_geom;

		// Nvidia CUDA interop
		std::shared_ptr<::nvidia::cuda::cuda>      _cuda;
		std::shared_ptr<::nvidia::cuda::context>   _cuda_ctx;
		std::shared_ptr<::nvidia::cuda::stream>    _cuda_stream;
		std::shared_ptr<::nvidia::cuda::memory>    _cuda_mem;
		bool                                       _cuda_flush_cache;
		std::shared_ptr<::nvidia::cuda::gstexture> _cuda_rt_cache;

		// Nvidia AR interop
		std::shared_ptr<::nvidia::ar::ar> _ar;
		std::string                       _ar_models_path;
		std::shared_ptr<nvAR_Feature>     _ar_tracker;
		std::atomic_bool                  _ar_ready;
		std::atomic_bool                  _ar_fail;
		std::vector<NvAR_Rect>            _ar_bboxes_data;
		NvAR_BBoxes                       _ar_bboxes;
		std::vector<float_t>              _ar_bboxes_confidence;
		NvCVImage                         _ar_image;
		NvCVImage                         _ar_image_bgr;
		NvCVImage                         _ar_image_temp;

#ifdef _DEBUG
		// Profiling
		std::shared_ptr<util::profiler> _profile_capture;
		std::shared_ptr<util::profiler> _profile_cuda_register;
		std::shared_ptr<util::profiler> _profile_cuda_copy;
		std::shared_ptr<util::profiler> _profile_ar_transfer;
		std::shared_ptr<util::profiler> _profile_ar_run;
#endif

		public:
		face_tracking_instance(obs_data_t*, obs_source_t*);
		virtual ~face_tracking_instance() override;

		// Initialize face detection.
		void face_detection_initialize();

		void face_detection_initialize_thread(std::shared_ptr<void> param);

		// Create image buffer.
		void create_image_buffer(std::size_t width, std::size_t height);

		void roi_refresh();

		void roi_reset();

		virtual void load(obs_data_t* data) override;

		virtual void migrate(obs_data_t* data, std::uint64_t version) override;

		virtual void update(obs_data_t* data) override;

		virtual void video_tick(float seconds) override;

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
