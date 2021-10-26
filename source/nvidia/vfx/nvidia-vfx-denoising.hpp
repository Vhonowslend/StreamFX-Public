// Copyright (c) 2021 Michael Fabian Dirks <info@xaymar.com>
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
#include "nvidia-vfx-effect.hpp"
#include "nvidia-vfx.hpp"
#include "nvidia/cuda/nvidia-cuda-gs-texture.hpp"
#include "nvidia/cuda/nvidia-cuda-obs.hpp"
#include "nvidia/cuda/nvidia-cuda.hpp"
#include "nvidia/cv/nvidia-cv-image.hpp"
#include "nvidia/cv/nvidia-cv-texture.hpp"
#include "obs/gs/gs-texture.hpp"

namespace streamfx::nvidia::vfx {
	class denoising : protected effect {
		bool _dirty;

		std::shared_ptr<::streamfx::nvidia::cv::texture> _input;
		std::shared_ptr<::streamfx::nvidia::cv::image>   _convert_to_fp32;
		std::shared_ptr<::streamfx::nvidia::cv::image>   _source;
		std::shared_ptr<::streamfx::nvidia::cv::image>   _destination;
		std::shared_ptr<::streamfx::nvidia::cv::image>   _convert_to_u8;
		std::shared_ptr<::streamfx::nvidia::cv::texture> _output;
		std::shared_ptr<::streamfx::nvidia::cv::image>   _tmp;

		void*                                  _states[1];
		::streamfx::nvidia::cuda::device_ptr_t _state;
		uint32_t                               _state_size;

		float _strength;

		public:
		~denoising();
		denoising();

		void  set_strength(float strength);
		float strength();

		void size(std::pair<uint32_t, uint32_t>& size);

		std::shared_ptr<::streamfx::obs::gs::texture> process(std::shared_ptr<::streamfx::obs::gs::texture> in);

		private:
		void resize(uint32_t width, uint32_t height);

		void load();
	};
} // namespace streamfx::nvidia::vfx
