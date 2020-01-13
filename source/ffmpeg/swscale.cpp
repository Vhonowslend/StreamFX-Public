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

#include "swscale.hpp"
#include <stdexcept>

ffmpeg::swscale::swscale() {}

ffmpeg::swscale::~swscale()
{
	finalize();
}

void ffmpeg::swscale::set_source_size(uint32_t width, uint32_t height)
{
	source_size.first  = width;
	source_size.second = height;
}

void ffmpeg::swscale::get_source_size(uint32_t& width, uint32_t& height)
{
	width  = this->source_size.first;
	height = this->source_size.second;
}

std::pair<uint32_t, uint32_t> ffmpeg::swscale::get_source_size()
{
	return this->source_size;
}

uint32_t ffmpeg::swscale::get_source_width()
{
	return this->source_size.first;
}

uint32_t ffmpeg::swscale::get_source_height()
{
	return this->source_size.second;
}

void ffmpeg::swscale::set_source_format(AVPixelFormat format)
{
	source_format = format;
}

AVPixelFormat ffmpeg::swscale::get_source_format()
{
	return this->source_format;
}

void ffmpeg::swscale::set_source_color(bool full_range, AVColorSpace space)
{
	source_full_range = full_range;
	source_colorspace = space;
}

void ffmpeg::swscale::set_source_colorspace(AVColorSpace space)
{
	this->source_colorspace = space;
}

AVColorSpace ffmpeg::swscale::get_source_colorspace()
{
	return this->source_colorspace;
}

void ffmpeg::swscale::set_source_full_range(bool full_range)
{
	this->source_full_range = full_range;
}

bool ffmpeg::swscale::is_source_full_range()
{
	return this->source_full_range;
}

void ffmpeg::swscale::set_target_size(uint32_t width, uint32_t height)
{
	target_size.first  = width;
	target_size.second = height;
}

void ffmpeg::swscale::get_target_size(uint32_t& width, uint32_t& height)
{
	width  = target_size.first;
	height = target_size.second;
}

std::pair<uint32_t, uint32_t> ffmpeg::swscale::get_target_size()
{
	return this->target_size;
}

uint32_t ffmpeg::swscale::get_target_width()
{
	return this->target_size.first;
}

uint32_t ffmpeg::swscale::get_target_height()
{
	return this->target_size.second;
}

void ffmpeg::swscale::set_target_format(AVPixelFormat format)
{
	target_format = format;
}

AVPixelFormat ffmpeg::swscale::get_target_format()
{
	return this->target_format;
}

void ffmpeg::swscale::set_target_color(bool full_range, AVColorSpace space)
{
	target_full_range = full_range;
	target_colorspace = space;
}

void ffmpeg::swscale::set_target_colorspace(AVColorSpace space)
{
	this->target_colorspace = space;
}

AVColorSpace ffmpeg::swscale::get_target_colorspace()
{
	return this->target_colorspace;
}

void ffmpeg::swscale::set_target_full_range(bool full_range)
{
	this->target_full_range = full_range;
}

bool ffmpeg::swscale::is_target_full_range()
{
	return this->target_full_range;
}

bool ffmpeg::swscale::initialize(int flags)
{
	if (this->context) {
		return false;
	}
	if (source_size.first == 0 || source_size.second == 0 || source_format == AV_PIX_FMT_NONE
	    || source_colorspace == AVCOL_SPC_UNSPECIFIED) {
		throw std::invalid_argument("not all source parameters were set");
	}
	if (target_size.first == 0 || target_size.second == 0 || target_format == AV_PIX_FMT_NONE
	    || target_colorspace == AVCOL_SPC_UNSPECIFIED) {
		throw std::invalid_argument("not all target parameters were set");
	}

	this->context = sws_getContext(source_size.first, source_size.second, source_format, target_size.first,
	                               target_size.second, target_format, flags, nullptr, nullptr, nullptr);
	if (!this->context) {
		return false;
	}

	sws_setColorspaceDetails(this->context, sws_getCoefficients(source_colorspace), source_full_range ? 1 : 0,
	                         sws_getCoefficients(target_colorspace), target_full_range ? 1 : 0, 1L << 16 | 0L,
	                         1L << 16 | 0L, 1L << 16 | 0L);

	return true;
}

bool ffmpeg::swscale::finalize()
{
	if (this->context) {
		sws_freeContext(this->context);
		this->context = nullptr;
		return true;
	}
	return false;
}

int32_t ffmpeg::swscale::convert(const uint8_t* const source_data[], const int source_stride[], int32_t source_row,
                                 int32_t source_rows, uint8_t* const target_data[], const int target_stride[])
{
	if (!this->context) {
		return 0;
	}
	int height =
	    sws_scale(this->context, source_data, source_stride, source_row, source_rows, target_data, target_stride);
	return height;
}
