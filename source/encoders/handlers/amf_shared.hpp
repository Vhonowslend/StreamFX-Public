// FFMPEG Video Encoder Integration for OBS Studio
// Copyright (c) 2020 Michael Fabian Dirks <info@xaymar.com>
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

// THIS FEATURE IS DEPRECATED. SUBMITTED PATCHES WILL BE REJECTED.

#pragma once
#include "common.hpp"
#include "handler.hpp"

extern "C" {
#include "warning-disable.hpp"
#include <libavcodec/avcodec.h>
#include "warning-enable.hpp"
}

namespace streamfx::encoder::ffmpeg::handler::amf {
	enum class preset : int32_t {
		SPEED,
		BALANCED,
		QUALITY,
		INVALID = -1,
	};

	enum class ratecontrolmode : int64_t {
		CQP,
		CBR,
		VBR_PEAK,
		VBR_LATENCY,
		INVALID = -1,
	};

	extern std::map<preset, std::string> presets;

	extern std::map<preset, std::string> preset_to_opt;

	extern std::map<ratecontrolmode, std::string> ratecontrolmodes;

	extern std::map<ratecontrolmode, std::string> ratecontrolmode_to_opt;

	bool is_available();

	void get_defaults(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context);

	void get_properties_pre(obs_properties_t* props, const AVCodec* codec);

	void get_properties_post(obs_properties_t* props, const AVCodec* codec);

	void get_runtime_properties(obs_properties_t* props, const AVCodec* codec, AVCodecContext* context);

	void migrate(obs_data_t* settings, uint64_t version, const AVCodec* codec, AVCodecContext* context);

	void update(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context);

	void override_update(ffmpeg_instance* instance, obs_data_t* settings);

	void log_options(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context);

} // namespace streamfx::encoder::ffmpeg::handler::amf

/* Parameters by their codec specific name.
 * '#' denotes a parameter specified via the context itself.

H.264			H.265				Options							Done?
usage			usage				transcoding						--
preset			preset				speed,balanced,quality			Defines
profile			profile				<different>						Defines
level			level				<different>						Defines
				tier				main,high						
rc				rc					cqp,cbr,vbr_peak,vbr_latency	Defines
preanalysis		preanalysis			false,true						Defines
vbaq			vbaq				false,true						Defines
enforce_hrd		enforce_hrd			false,true						Defines
filler_data		filler_data			false,true						--
frame_skipping	skip_frame			false,true						Defines
qp_i			qp_i				range(-1 - 51)					Defines
qp_p			qp_p				range(-1 - 51)					Defines
qp_b								range(-1 - 51)					Defines
#max_b_frames														Defines
bf_delta_qp							range(-10 - 10)					--
bf_ref								false,true						Defines
bf_ref_delta_qp						range(-10 - 10)					--
me_half_pel		me_half_pel			false,true						--
me_quarter_pel	me_quarter_pel		false,true						--
aud				aud					false,true						Defines
max_au_size		max_au_size			range(0 - Inf)					--
#refs								range(0 - 16?)					Defines
#color_range						AVCOL_RANGE_JPEG				FFmpeg
#bit_rate															Defines
#rc_max_rate														Defines
#rc_buffer_size														Defines
#rc_initial_buffer_occupancy										--
#flags								AV_CODEC_FLAG_LOOP_FILTER		--
#gop_size															FFmpeg
*/

// AMF H.264
// intra_refresh_mb: 0 - Inf
// header_spacing: -1 - 1000
// coder: auto, cavlc, cabac
// qmin, qmax (HEVC uses its own settings)

// AMF H.265
// header_insertion_mode: none, gop, idr
// gops_per_idr: 0 - Inf
// min_qp_i: -1 - 51
// max_qp_i: -1 - 51
// min_qp_p: -1 - 51
// max_qp_p: -1 - 51
