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
#include <deque>
#include <mutex>

extern "C" {
#pragma warning(push)
#pragma warning(disable : 4244)
#include <libavutil/frame.h>
#pragma warning(pop)
}

namespace ffmpeg {
	class avframe_queue {
		std::deque<std::shared_ptr<AVFrame>> frames;
		std::mutex                           lock;

		std::pair<uint32_t, uint32_t> resolution;
		AVPixelFormat                 format = AV_PIX_FMT_NONE;

		std::shared_ptr<AVFrame> create_frame();

		public:
		avframe_queue();
		~avframe_queue();

		void     set_resolution(uint32_t width, uint32_t height);
		void     get_resolution(uint32_t& width, uint32_t& height);
		uint32_t get_width();
		uint32_t get_height();

		void          set_pixel_format(AVPixelFormat format);
		AVPixelFormat get_pixel_format();

		void precache(size_t count);

		void clear();

		void push(std::shared_ptr<AVFrame> frame);

		std::shared_ptr<AVFrame> pop();

		std::shared_ptr<AVFrame> pop_only();

		bool empty();

		size_t size();
	};
} // namespace ffmpeg
