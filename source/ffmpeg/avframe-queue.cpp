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

#include "avframe-queue.hpp"
#include "tools.hpp"

std::shared_ptr<AVFrame> ffmpeg::avframe_queue::create_frame()
{
	std::shared_ptr<AVFrame> frame = std::shared_ptr<AVFrame>(av_frame_alloc(), [](AVFrame* frame) {
		av_frame_unref(frame);
		av_frame_free(&frame);
	});
	frame->width                   = this->resolution.first;
	frame->height                  = this->resolution.second;
	frame->format                  = this->format;

	int res = av_frame_get_buffer(frame.get(), 32);
	if (res < 0) {
		throw std::exception(ffmpeg::tools::get_error_description(res));
	}

	return frame;
}

ffmpeg::avframe_queue::avframe_queue() {}

ffmpeg::avframe_queue::~avframe_queue()
{
	clear();
}

void ffmpeg::avframe_queue::set_resolution(uint32_t const width, uint32_t const height)
{
	this->resolution.first  = width;
	this->resolution.second = height;
}

void ffmpeg::avframe_queue::get_resolution(uint32_t& width, uint32_t& height)
{
	width  = this->resolution.first;
	height = this->resolution.second;
}

uint32_t ffmpeg::avframe_queue::get_width()
{
	return this->resolution.first;
}

uint32_t ffmpeg::avframe_queue::get_height()
{
	return this->resolution.second;
}

void ffmpeg::avframe_queue::set_pixel_format(AVPixelFormat const format)
{
	this->format = format;
}

AVPixelFormat ffmpeg::avframe_queue::get_pixel_format()
{
	return this->format;
}

void ffmpeg::avframe_queue::precache(size_t count)
{
	for (size_t n = 0; n < count; n++) {
		push(create_frame());
	}
}

void ffmpeg::avframe_queue::clear()
{
	std::unique_lock<std::mutex> ulock(this->lock);
	frames.clear();
}

void ffmpeg::avframe_queue::push(std::shared_ptr<AVFrame> const frame)
{
	std::unique_lock<std::mutex> ulock(this->lock);
	frames.push_back(frame);
}

std::shared_ptr<AVFrame> ffmpeg::avframe_queue::pop()
{
	std::unique_lock<std::mutex> ulock(this->lock);
	std::shared_ptr<AVFrame>     ret;
	while (ret == nullptr) {
		if (frames.size() == 0) {
			ret = create_frame();
		} else {
			ret = frames.front();
			if (ret == nullptr) {
				ret = create_frame();
			} else {
				frames.pop_front();
				if ((static_cast<uint32_t>(ret->width) != this->resolution.first)
				    || (static_cast<uint32_t>(ret->height) != this->resolution.second)
				    || (ret->format != this->format)) {
					ret = nullptr;
				}
			}
		}
	}
	return ret;
}

std::shared_ptr<AVFrame> ffmpeg::avframe_queue::pop_only()
{
	std::unique_lock<std::mutex> ulock(this->lock);
	if (frames.size() == 0) {
		return nullptr;
	}
	std::shared_ptr<AVFrame> ret = frames.front();
	if (ret == nullptr) {
		return nullptr;
	}
	frames.pop_front();
	return ret;
}

bool ffmpeg::avframe_queue::empty()
{
	return frames.empty();
}

size_t ffmpeg::avframe_queue::size()
{
	return frames.size();
}
