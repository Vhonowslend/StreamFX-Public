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
#include "nvidia-ar-feature.hpp"
#include "nvidia/cuda/nvidia-cuda-gs-texture.hpp"
#include "nvidia/cuda/nvidia-cuda-obs.hpp"
#include "nvidia/cuda/nvidia-cuda.hpp"
#include "nvidia/cv/nvidia-cv-image.hpp"
#include "nvidia/cv/nvidia-cv-texture.hpp"
#include "obs/gs/gs-texture.hpp"

namespace streamfx::nvidia::ar {
	class facedetection : public feature {
		std::shared_ptr<::streamfx::nvidia::cv::texture> _input;
		std::shared_ptr<::streamfx::nvidia::cv::image>   _source;
		std::shared_ptr<::streamfx::nvidia::cv::image>   _tmp;

		std::vector<rect_t> _rects;
		std::vector<float>  _rects_confidence;
		bounds_t            _bboxes;

		bool _dirty;

		public:
		~facedetection();

		/** Create a new face detection feature.
		 *
		 * Must be in a graphics and CUDA context when calling.
		 */
		facedetection();

		std::pair<size_t, size_t> tracking_limit_range();

		size_t tracking_limit();

		void set_tracking_limit(size_t v);

		void process(std::shared_ptr<::streamfx::obs::gs::texture> in);

		size_t count();

		rect_t const& at(size_t index);

		rect_t const& at(size_t index, float& confidence);

		private:
		void resize(uint32_t width, uint32_t height);

		void load();
	};
} // namespace streamfx::nvidia::ar
