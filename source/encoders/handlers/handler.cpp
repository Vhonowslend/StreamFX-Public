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

#include "handler.hpp"
#include "../ffmpeg-encoder.hpp"

void encoder::ffmpeg::handler::handler::adjust_encoder_info(encoder::ffmpeg::ffmpeg_factory*, ffmpeg_info*,
															ffmpeg_info*)
{}

void encoder::ffmpeg::handler::handler::get_defaults(obs_data_t*, const AVCodec*, AVCodecContext*, bool) {}

bool encoder::ffmpeg::handler::handler::has_keyframe_support(ffmpeg_instance* instance)
{
	return (instance->get_avcodec()->capabilities & AV_CODEC_CAP_INTRA_ONLY) == 0;
}

void encoder::ffmpeg::handler::handler::get_properties(obs_properties_t*, const AVCodec*, AVCodecContext*, bool) {}

void encoder::ffmpeg::handler::handler::update(obs_data_t*, const AVCodec*, AVCodecContext*) {}

void encoder::ffmpeg::handler::handler::override_update(ffmpeg_instance*, obs_data_t*) {}

void encoder::ffmpeg::handler::handler::log_options(obs_data_t*, const AVCodec*, AVCodecContext*) {}

void encoder::ffmpeg::handler::handler::override_colorformat(AVPixelFormat&, obs_data_t*, const AVCodec*,
															 AVCodecContext*)
{}

void encoder::ffmpeg::handler::handler::process_avpacket(AVPacket&, const AVCodec*, AVCodecContext*) {}
