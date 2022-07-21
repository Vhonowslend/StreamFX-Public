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

extern "C" {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4242 4244 4365)
#endif
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/pixfmt.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

namespace streamfx::ffmpeg::tools {
	const char* get_pixel_format_name(AVPixelFormat v);

	const char* get_color_space_name(AVColorSpace v);

	const char* get_error_description(int error);

	AVPixelFormat obs_videoformat_to_avpixelformat(video_format v);
	video_format  avpixelformat_to_obs_videoformat(AVPixelFormat v);

	AVPixelFormat get_least_lossy_format(const AVPixelFormat* haystack, AVPixelFormat needle);

	AVColorRange                  obs_to_av_color_range(video_range_type v);
	AVColorSpace                  obs_to_av_color_space(video_colorspace v);
	AVColorPrimaries              obs_to_av_color_primary(video_colorspace v);
	AVColorTransferCharacteristic obs_to_av_color_transfer_characteristics(video_colorspace v);

	bool can_hardware_encode(const AVCodec* codec);

	std::vector<AVPixelFormat> get_software_formats(const AVPixelFormat* list);

	void context_setup_from_obs(const video_output_info* voi, AVCodecContext* context);

	const char* get_std_compliance_name(int compliance);

	const char* get_thread_type_name(int thread_type);

	void print_av_option_bool(AVCodecContext* context, const char* option, std::string_view text, bool inverse = false);
	void print_av_option_bool(AVCodecContext* ctx_codec, void* ctx_option, const char* option, std::string_view text,
							  bool inverse = false);

	void print_av_option_int(AVCodecContext* context, const char* option, std::string_view text,
							 std::string_view suffix);
	void print_av_option_int(AVCodecContext* ctx_codec, void* ctx_option, const char* option, std::string_view text,
							 std::string_view suffix);

	void print_av_option_string(AVCodecContext* context, const char* option, std::string_view text,
								std::function<std::string(int64_t)> decoder);
	void print_av_option_string(AVCodecContext* ctx_codec, void* ctx_option, const char* option, std::string_view text,
								std::function<std::string(int64_t)> decoder);

	void print_av_option_string2(AVCodecContext* context, std::string_view option, std::string_view text,
								 std::function<std::string(int64_t, std::string_view)> decoder);
	void print_av_option_string2(AVCodecContext* ctx_codec, void* ctx_option, std::string_view option,
								 std::string_view text, std::function<std::string(int64_t, std::string_view)> decoder);

	bool avoption_exists(const void* obj, std::string_view name);

	const char* avoption_name_from_unit_value(const void* obj, std::string_view unit, int64_t value);

	void avoption_list_add_entries(const void* obj, std::string_view unit,
								   std::function<void(const AVOption*)> inserter = nullptr);

} // namespace streamfx::ffmpeg::tools
