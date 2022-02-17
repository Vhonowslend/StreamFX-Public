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
#include "handler.hpp"

extern "C" {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4242 4244 4365)
#endif
#include <libavcodec/avcodec.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

/* NVENC has multiple compression modes:
- CBR: Constant Bitrate (rc=cbr)
- VBR: Variable Bitrate (rc=vbr)
- CQP: Constant QP (rc=cqp)
- CQ: Constant Quality (rc=vbr b=0 maxrate=0 qmin=0 qmax=51 cq=qp), this is basically CRF in X264.
*/

namespace streamfx::encoder::ffmpeg::handler::nvenc {
	bool is_available();

	void override_update(ffmpeg_instance* instance, obs_data_t* settings);

	void get_defaults(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context);

	void get_properties_pre(obs_properties_t* props, const AVCodec* codec, const AVCodecContext* context);

	void get_properties_post(obs_properties_t* props, const AVCodec* codec, const AVCodecContext* context);

	void get_runtime_properties(obs_properties_t* props, const AVCodec* codec, AVCodecContext* context);

	void migrate(obs_data_t* settings, uint64_t version, const AVCodec* codec, AVCodecContext* context);

	void update(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context);

	void log_options(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context);
} // namespace streamfx::encoder::ffmpeg::handler::nvenc
