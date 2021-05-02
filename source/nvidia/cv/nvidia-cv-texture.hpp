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
#include <cinttypes>
#include "nvidia/cv/nvidia-cv-image.hpp"
#include "obs/gs/gs-texture.hpp"

namespace streamfx::nvidia::cv {
	using ::streamfx::nvidia::cv::component_layout;
	using ::streamfx::nvidia::cv::component_type;
	using ::streamfx::nvidia::cv::image;
	using ::streamfx::nvidia::cv::memory_location;
	using ::streamfx::nvidia::cv::pixel_format;

	class texture : public image {
		std::shared_ptr<::streamfx::obs::gs::texture> _texture;

		public:
		~texture() override;
		texture(uint32_t width, uint32_t height, gs_color_format pix_fmt);

		void resize(uint32_t width, uint32_t height) override;

		std::shared_ptr<::streamfx::obs::gs::texture> get_texture();

		private:
		void alloc();
		void free();
	};

} // namespace streamfx::nvidia::cv
