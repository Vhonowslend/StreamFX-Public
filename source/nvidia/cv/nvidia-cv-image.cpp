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

// NVIDIA CVImage is part of:
// - NVIDIA Video Effects SDK
// - NVIDIA Augmented Reality SDK

#include "nvidia-cv-image.hpp"
#include "nvidia/cuda/nvidia-cuda-obs.hpp"
#include "obs/gs/gs-helper.hpp"

#ifdef _DEBUG
#define ST_PREFIX "<%s> "
#define D_LOG_ERROR(x, ...) P_LOG_ERROR(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_WARNING(x, ...) P_LOG_WARN(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_INFO(x, ...) P_LOG_INFO(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_DEBUG(x, ...) P_LOG_DEBUG(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#else
#define ST_PREFIX "<nvidia::cv::image> "
#define D_LOG_ERROR(...) P_LOG_ERROR(ST_PREFIX __VA_ARGS__)
#define D_LOG_WARNING(...) P_LOG_WARN(ST_PREFIX __VA_ARGS__)
#define D_LOG_INFO(...) P_LOG_INFO(ST_PREFIX __VA_ARGS__)
#define D_LOG_DEBUG(...) P_LOG_DEBUG(ST_PREFIX __VA_ARGS__)
#endif

using ::streamfx::nvidia::cv::image;
using ::streamfx::nvidia::cv::result;

image::~image()
{
	auto gctx = ::streamfx::obs::gs::context();
	auto cctx = ::streamfx::nvidia::cuda::obs::get()->get_context()->enter();

	_cv->NvCVImage_Dealloc(&_image);
}

image::image() : _cv(::streamfx::nvidia::cv::cv::get()), _image(), _alignment(1)
{
	// Forcefully clear the image storage.
	memset(&_image, sizeof(_image), 0);
}

image::image(uint32_t width, uint32_t height, pixel_format pix_fmt, component_type cmp_type,
			 component_layout cmp_layout, memory_location location, uint32_t alignment)
	: image()
{
	auto gctx = ::streamfx::obs::gs::context();
	auto cctx = ::streamfx::nvidia::cuda::obs::get()->get_context()->enter();

	_alignment = alignment;
	if (auto res = _cv->NvCVImage_Alloc(&_image, width, height, pix_fmt, cmp_type, static_cast<uint32_t>(cmp_layout),
										static_cast<uint32_t>(location), _alignment);
		res != result::SUCCESS) {
		throw std::runtime_error(_cv->NvCV_GetErrorStringFromCode(res));
	}
}

void streamfx::nvidia::cv::image::reallocate(uint32_t width, uint32_t height, pixel_format pix_fmt,
											 component_type cmp_type, component_layout cmp_layout,
											 memory_location location, uint32_t alignment)
{
	auto gctx = ::streamfx::obs::gs::context();
	auto cctx = ::streamfx::nvidia::cuda::obs::get()->get_context()->enter();

	if (auto res = _cv->NvCVImage_Realloc(&_image, width, height, pix_fmt, cmp_type, static_cast<uint32_t>(cmp_layout),
										  static_cast<uint32_t>(location), alignment);
		res != result::SUCCESS) {
		throw std::runtime_error(_cv->NvCV_GetErrorStringFromCode(res));
	}
	_alignment = alignment;
}

void streamfx::nvidia::cv::image::resize(uint32_t width, uint32_t height)
{
	reallocate(width, height, _image.pxl_format, _image.comp_type, static_cast<component_layout>(_image.comp_layout),
			   static_cast<memory_location>(_image.mem_location), _alignment);
}

streamfx::nvidia::cv::image_t* streamfx::nvidia::cv::image::get_image()
{
	return &_image;
}
