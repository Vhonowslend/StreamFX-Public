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
#include "nvidia/cv/nvidia-cv.hpp"

#include "warning-disable.hpp"
#include <cinttypes>
#include "warning-enable.hpp"

namespace streamfx::nvidia::cv {
	using ::streamfx::nvidia::cv::component_layout;
	using ::streamfx::nvidia::cv::component_type;
	using ::streamfx::nvidia::cv::memory_location;
	using ::streamfx::nvidia::cv::pixel_format;

	class image {
		protected:
		std::shared_ptr<::streamfx::nvidia::cv::cv> _cv;
		image_t                                     _image;
		uint32_t                                    _alignment;

		public:
		virtual ~image();

		protected:
		image();

		public:
		image(uint32_t width, uint32_t height, pixel_format pix_fmt, component_type cmp_type,
			  component_layout cmp_layout, memory_location location, uint32_t alignment);

		virtual void reallocate(uint32_t width, uint32_t height, pixel_format pix_fmt, component_type cmp_type,
								component_layout cmp_layout, memory_location location, uint32_t alignment);

		virtual void resize(uint32_t width, uint32_t height);

		virtual ::streamfx::nvidia::cv::image_t* get_image();
	};

} // namespace streamfx::nvidia::cv
