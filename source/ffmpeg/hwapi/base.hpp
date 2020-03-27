// FFMPEG Video Encoder Integration for OBS Studio
// Copyright (c) 2019 Michael Fabian Dirks <info@xaymar.com>
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
#include <list>
#include <memory>
#include <string>
#include <utility>

extern "C" {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4242 4244 4365)
#endif
#include <libavutil/frame.h>
#include <libavutil/hwcontext.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

namespace ffmpeg::hwapi {
	struct device {
		std::pair<int64_t, int64_t> id;
		std::string                 name;
	};

	class instance {
		public:
		virtual AVBufferRef* create_device_context() = 0;

		virtual std::shared_ptr<AVFrame> allocate_frame(AVBufferRef* frames) = 0;

		virtual void copy_from_obs(AVBufferRef* frames, uint32_t handle, uint64_t lock_key, uint64_t* next_lock_key,
								   std::shared_ptr<AVFrame> frame) = 0;

		virtual std::shared_ptr<AVFrame> avframe_from_obs(AVBufferRef* frames, uint32_t handle, uint64_t lock_key,
														  uint64_t* next_lock_key) = 0;
	};

	class base {
		public:
		virtual std::list<hwapi::device> enumerate_adapters() = 0;

		virtual std::shared_ptr<hwapi::instance> create(hwapi::device target) = 0;

		virtual std::shared_ptr<hwapi::instance> create_from_obs() = 0;
	};
} // namespace ffmpeg::hwapi
