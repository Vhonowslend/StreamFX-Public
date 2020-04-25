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
#include <functional>
#include <string>
#include <vector>

extern "C" {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4242 4244 4365)
#endif
#include <libavcodec/avcodec.h>
#include <libavutil/pixfmt.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

namespace ffmpeg::tools {
	std::string translate_encoder_capabilities(int capabilities);

	const char* get_pixel_format_name(AVPixelFormat v);

	const char* get_color_space_name(AVColorSpace v);

	const char* get_error_description(int error);

	AVPixelFormat obs_videoformat_to_avpixelformat(video_format v);

	video_format avpixelformat_to_obs_videoformat(AVPixelFormat v);

	AVPixelFormat get_least_lossy_format(const AVPixelFormat* haystack, AVPixelFormat needle);

	AVColorSpace obs_videocolorspace_to_avcolorspace(video_colorspace v);

	AVColorRange obs_videorangetype_to_avcolorrange(video_range_type v);

	bool can_hardware_encode(const AVCodec* codec);

	std::vector<AVPixelFormat> get_software_formats(const AVPixelFormat* list);

	void setup_obs_color(video_colorspace colorspace, video_range_type range, AVCodecContext* context);

	const char* get_std_compliance_name(int compliance);

	const char* get_thread_type_name(int thread_type);

	void print_av_option_bool(AVCodecContext* context, const char* option, std::string text, bool inverse = false);
	void print_av_option_bool(AVCodecContext* ctx_codec, void* ctx_option, const char* option, std::string text,
							  bool inverse = false);

	void print_av_option_int(AVCodecContext* context, const char* option, std::string text, std::string suffix);
	void print_av_option_int(AVCodecContext* ctx_codec, void* ctx_option, const char* option, std::string text,
							 std::string suffix);

	void print_av_option_string(AVCodecContext* context, const char* option, std::string text,
								std::function<std::string(int64_t)> decoder);
	void print_av_option_string(AVCodecContext* ctx_codec, void* ctx_option, const char* option, std::string text,
								std::function<std::string(int64_t)> decoder);

} // namespace ffmpeg::tools
