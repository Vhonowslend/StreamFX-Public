// Copyright (c) 2020 Michael Fabian Dirks <info@xaymar.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once
#include <atomic>
#include <memory>
#include <mutex>
#include "obs/gs/gs-effect.hpp"
#include "obs/gs/gs-rendertarget.hpp"
#include "obs/gs/gs-texture.hpp"
#include "obs/obs-source-factory.hpp"
#include "plugin.hpp"
#include "util/util-threadpool.hpp"

#ifdef ENABLE_FILTER_VIRTUAL_GREENSCREEN_NVIDIA
#include "nvidia/vfx/nvidia-vfx-greenscreen.hpp"
#endif

namespace streamfx::filter::virtual_greenscreen {
	enum class virtual_greenscreen_provider {
		INVALID            = -1,
		AUTOMATIC          = 0,
		NVIDIA_GREENSCREEN = 1,
	};

	const char* cstring(virtual_greenscreen_provider provider);

	std::string string(virtual_greenscreen_provider provider);

	class virtual_greenscreen_instance : public ::streamfx::obs::source_instance {
		std::pair<uint32_t, uint32_t> _size;

		std::atomic<virtual_greenscreen_provider> _provider;
		virtual_greenscreen_provider              _provider_ui;
		std::atomic<bool>                         _provider_ready;
		std::mutex                                _provider_lock;
		std::shared_ptr<util::threadpool::task>   _provider_task;

		std::shared_ptr<::streamfx::obs::gs::effect>  _effect;
		std::shared_ptr<::streamfx::obs::gs::sampler> _channel0_sampler;
		std::shared_ptr<::streamfx::obs::gs::sampler> _channel1_sampler;

		std::shared_ptr<::streamfx::obs::gs::rendertarget> _input;
		std::shared_ptr<::streamfx::obs::gs::texture>      _output_color;
		std::shared_ptr<::streamfx::obs::gs::texture>      _output_alpha;
		bool                                               _dirty;

#ifdef ENABLE_FILTER_VIRTUAL_GREENSCREEN_NVIDIA
		std::shared_ptr<::streamfx::nvidia::vfx::greenscreen> _nvidia_fx;
#endif

		public:
		virtual_greenscreen_instance(obs_data_t* data, obs_source_t* self);
		~virtual_greenscreen_instance() override;

		void load(obs_data_t* data) override;
		void migrate(obs_data_t* data, uint64_t version) override;
		void update(obs_data_t* data) override;
		void properties(obs_properties_t* properties);

		uint32_t get_width() override;
		uint32_t get_height() override;

		void video_tick(float_t time) override;
		void video_render(gs_effect_t* effect) override;

		private:
		void switch_provider(virtual_greenscreen_provider provider);
		void task_switch_provider(util::threadpool_data_t data);

#ifdef ENABLE_FILTER_VIRTUAL_GREENSCREEN_NVIDIA
		void nvvfxgs_load();
		void nvvfxgs_unload();
		void nvvfxgs_size();
		void nvvfxgs_process(std::shared_ptr<::streamfx::obs::gs::texture>& color,
							 std::shared_ptr<::streamfx::obs::gs::texture>& alpha);
		void nvvfxgs_properties(obs_properties_t* props);
		void nvvfxgs_update(obs_data_t* data);
#endif
	};

	class virtual_greenscreen_factory : public ::streamfx::obs::source_factory<
											::streamfx::filter::virtual_greenscreen::virtual_greenscreen_factory,
											::streamfx::filter::virtual_greenscreen::virtual_greenscreen_instance> {
#ifdef ENABLE_FILTER_VIRTUAL_GREENSCREEN_NVIDIA
		bool                                           _nvidia_available;
		std::shared_ptr<::streamfx::nvidia::cuda::obs> _nvcuda;
		std::shared_ptr<::streamfx::nvidia::cv::cv>    _nvcvi;
		std::shared_ptr<::streamfx::nvidia::vfx::vfx>  _nvvfx;
#endif

		public:
		virtual ~virtual_greenscreen_factory();
		virtual_greenscreen_factory();

		virtual const char* get_name() override;

		virtual void              get_defaults2(obs_data_t* data) override;
		virtual obs_properties_t* get_properties2(virtual_greenscreen_instance* data) override;

#ifdef ENABLE_FRONTEND
		static bool on_manual_open(obs_properties_t* props, obs_property_t* property, void* data);
#endif

		bool                         is_provider_available(virtual_greenscreen_provider);
		virtual_greenscreen_provider find_ideal_provider();

		public: // Singleton
		static void                                                                                  initialize();
		static void                                                                                  finalize();
		static std::shared_ptr<::streamfx::filter::virtual_greenscreen::virtual_greenscreen_factory> get();
	};

} // namespace streamfx::filter::virtual_greenscreen
