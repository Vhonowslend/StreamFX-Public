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
#include "handler.hpp"

extern "C" {
#include "warning-disable.hpp"
#include <libavcodec/avcodec.h>
#include "warning-enable.hpp"
}

namespace streamfx::encoder::ffmpeg::handler {
	class prores_aw_handler : public handler {
		public:
		virtual ~prores_aw_handler(){};

		public /*factory*/:
		void get_defaults(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context, bool hw_encode) override;

		virtual std::string_view get_help_url(const AVCodec* codec) override
		{
			return "https://github.com/Xaymar/obs-StreamFX/wiki/Encoder-FFmpeg-Apple-ProRes";
		};

		public /*support tests*/:
		bool has_pixel_format_support(ffmpeg_factory* instance) override;

		public /*settings*/:
		void get_properties(obs_properties_t* props, const AVCodec* codec, AVCodecContext* context,
							bool hw_encode) override;

		void update(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context) override;

		void log_options(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context) override;

		public /*instance*/:
		void override_colorformat(AVPixelFormat& target_format, obs_data_t* settings, const AVCodec* codec,
								  AVCodecContext* context) override;
	};
} // namespace streamfx::encoder::ffmpeg::handler
