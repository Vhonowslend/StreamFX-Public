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

using namespace ffmpeg;

std::shared_ptr<AVFrame> avframe_queue::create_frame()
{
	std::shared_ptr<AVFrame> frame = std::shared_ptr<AVFrame>(av_frame_alloc(), [](AVFrame* frame) {
		av_frame_unref(frame);
		av_frame_free(&frame);
	});
	frame->width                   = this->_resolution.first;
	frame->height                  = this->_resolution.second;
	frame->format                  = this->_format;

	int res = av_frame_get_buffer(frame.get(), 32);
	if (res < 0) {
		throw std::runtime_error(tools::get_error_description(res));
	}

	return frame;
}

avframe_queue::avframe_queue() {}

avframe_queue::~avframe_queue()
{
	clear();
}

void avframe_queue::set_resolution(std::int32_t const width, std::int32_t const height)
{
	this->_resolution.first  = width;
	this->_resolution.second = height;
}

void avframe_queue::get_resolution(std::int32_t& width, std::int32_t& height)
{
	width  = this->_resolution.first;
	height = this->_resolution.second;
}

std::int32_t avframe_queue::get_width()
{
	return this->_resolution.first;
}

std::int32_t avframe_queue::get_height()
{
	return this->_resolution.second;
}

void avframe_queue::set_pixel_format(AVPixelFormat const format)
{
	this->_format = format;
}

AVPixelFormat avframe_queue::get_pixel_format()
{
	return this->_format;
}

void avframe_queue::precache(std::size_t count)
{
	for (std::size_t n = 0; n < count; n++) {
		push(create_frame());
	}
}

void avframe_queue::clear()
{
	std::unique_lock<std::mutex> ulock(this->_lock);
	_frames.clear();
}

void avframe_queue::push(std::shared_ptr<AVFrame> const frame)
{
	std::unique_lock<std::mutex> ulock(this->_lock);
	_frames.push_back(frame);
}

std::shared_ptr<AVFrame> avframe_queue::pop()
{
	std::unique_lock<std::mutex> ulock(this->_lock);
	std::shared_ptr<AVFrame>     ret;
	while (ret == nullptr) {
		if (_frames.size() == 0) {
			ret = create_frame();
		} else {
			ret = _frames.front();
			if (ret == nullptr) {
				ret = create_frame();
			} else {
				_frames.pop_front();
				if ((static_cast<std::int32_t>(ret->width) != this->_resolution.first)
					|| (static_cast<std::int32_t>(ret->height) != this->_resolution.second)
					|| (ret->format != this->_format)) {
					ret = nullptr;
				}
			}
		}
	}
	return ret;
}

std::shared_ptr<AVFrame> avframe_queue::pop_only()
{
	std::unique_lock<std::mutex> ulock(this->_lock);
	if (_frames.size() == 0) {
		return nullptr;
	}
	std::shared_ptr<AVFrame> ret = _frames.front();
	if (ret == nullptr) {
		return nullptr;
	}
	_frames.pop_front();
	return ret;
}

bool avframe_queue::empty()
{
	return _frames.empty();
}

std::size_t avframe_queue::size()
{
	return _frames.size();
}
