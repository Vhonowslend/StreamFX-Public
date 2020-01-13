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

#include <string>
#include "ffmpeg/hwapi/base.hpp"

extern "C" {
#include <obs.h>

#include <obs-data.h>
#include <obs-encoder.h>
#include <obs-properties.h>
#pragma warning(push)
#pragma warning(disable : 4244)
#include <libavcodec/avcodec.h>
#pragma warning(pop)
}

namespace encoder::ffmpeg {
	struct ffmpeg_info;
	class ffmpeg_factory;
	class ffmpeg_instance;

	namespace handler {
		class handler {
			public /*factory*/:
			virtual void adjust_encoder_info(ffmpeg_factory* factory, ffmpeg_info* main, ffmpeg_info* fallback);

			virtual void get_defaults(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context,
									  bool hw_encode);

			public /*settings*/:
			virtual bool has_keyframe_support(ffmpeg_instance* instance);

			virtual void get_properties(obs_properties_t* props, const AVCodec* codec, AVCodecContext* context,
										bool hw_encode);

			virtual void update(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context);

			virtual void override_update(ffmpeg_instance* instance, obs_data_t* settings);

			virtual void log_options(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context);

			public /*instance*/:

			virtual void override_colorformat(AVPixelFormat& target_format, obs_data_t* settings, const AVCodec* codec,
											  AVCodecContext* context);

			virtual void process_avpacket(AVPacket& packet, const AVCodec* codec, AVCodecContext* context);
		};
	} // namespace handler
} // namespace encoder::ffmpeg
