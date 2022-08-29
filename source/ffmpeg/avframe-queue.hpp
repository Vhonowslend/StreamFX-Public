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
#include "common.hpp"

#include "warning-disable.hpp"
#include <deque>
#include <mutex>
#include "warning-enable.hpp"

extern "C" {
#include "warning-disable.hpp"
#include <libavutil/frame.h>
#include "warning-enable.hpp"
}

namespace streamfx::ffmpeg {
	class avframe_queue {
		std::deque<std::shared_ptr<AVFrame>> _frames;
		std::mutex                           _lock;

		std::pair<int32_t, int32_t> _resolution;
		AVPixelFormat               _format = AV_PIX_FMT_NONE;

		std::shared_ptr<AVFrame> create_frame();

		public:
		avframe_queue();
		~avframe_queue();

		void    set_resolution(int32_t width, int32_t height);
		void    get_resolution(int32_t& width, int32_t& height);
		int32_t get_width();
		int32_t get_height();

		void          set_pixel_format(AVPixelFormat format);
		AVPixelFormat get_pixel_format();

		void precache(std::size_t count);

		void clear();

		void push(std::shared_ptr<AVFrame> frame);

		std::shared_ptr<AVFrame> pop();

		std::shared_ptr<AVFrame> pop_only();

		bool empty();

		std::size_t size();
	};
} // namespace streamfx::ffmpeg
