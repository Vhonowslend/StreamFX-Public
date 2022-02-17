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

#include "nvenc_h264_handler.hpp"
#include "strings.hpp"
#include "../codecs/h264.hpp"
#include "../encoder-ffmpeg.hpp"
#include "ffmpeg/tools.hpp"
#include "nvenc_shared.hpp"
#include "plugin.hpp"

extern "C" {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4242 4244 4365)
#endif
#include <obs-module.h>
#include <libavutil/opt.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

#define ST_KEY_PROFILE "H264.Profile"
#define ST_KEY_LEVEL "H264.Level"

using namespace streamfx::encoder::ffmpeg::handler;
using namespace streamfx::encoder::codec::h264;

void nvenc_h264_handler::adjust_info(ffmpeg_factory* fac, const AVCodec*, std::string&, std::string& name, std::string&)
{
	name = "NVIDIA NVENC H.264/AVC (via FFmpeg)";
	if (!nvenc::is_available())
		fac->get_info()->caps |= OBS_ENCODER_CAP_DEPRECATED;
}

void nvenc_h264_handler::get_defaults(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context, bool)
{
	nvenc::get_defaults(settings, codec, context);

	obs_data_set_default_int(settings, ST_KEY_PROFILE, static_cast<int64_t>(profile::HIGH));
	obs_data_set_default_int(settings, ST_KEY_LEVEL, static_cast<int64_t>(level::UNKNOWN));
}

bool nvenc_h264_handler::has_keyframe_support(ffmpeg_factory*)
{
	return true;
}

bool nvenc_h264_handler::is_hardware_encoder(ffmpeg_factory*)
{
	return true;
}

bool nvenc_h264_handler::has_threading_support(ffmpeg_factory*)
{
	return false;
}

bool nvenc_h264_handler::has_pixel_format_support(ffmpeg_factory*)
{
	return true;
}

void nvenc_h264_handler::get_properties(obs_properties_t* props, const AVCodec* codec, AVCodecContext* context, bool)
{
	if (!context) {
		this->get_encoder_properties(props, codec);
	} else {
		this->get_runtime_properties(props, codec, context);
	}
}

void nvenc_h264_handler::update(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context)
{
	nvenc::update(settings, codec, context);

	if (!context->internal) {
		if (auto value = obs_data_get_int(settings, ST_KEY_PROFILE); value > -1) {
			av_opt_set_int(context->priv_data, "profile", value, AV_OPT_SEARCH_CHILDREN);
		}
		if (auto value = obs_data_get_int(settings, ST_KEY_LEVEL); value > -1) {
			av_opt_set_int(context->priv_data, "level", value, AV_OPT_SEARCH_CHILDREN);
		} else {
			av_opt_set(context->priv_data, "level", "auto", AV_OPT_SEARCH_CHILDREN);
		}
	}
}

void nvenc_h264_handler::override_update(ffmpeg_instance* instance, obs_data_t* settings)
{
	nvenc::override_update(instance, settings);
}

void nvenc_h264_handler::log_options(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context)
{
	nvenc::log_options(settings, codec, context);

	DLOG_INFO("[%s]     H.264/AVC:", codec->name);
	::streamfx::ffmpeg::tools::print_av_option_string2(context, context->priv_data, "profile", "      Profile",
													   [](int64_t v, std::string_view o) { return std::string(o); });
	::streamfx::ffmpeg::tools::print_av_option_string2(context, context->priv_data, "level", "      Level",
													   [](int64_t v, std::string_view o) { return std::string(o); });
}

void nvenc_h264_handler::get_encoder_properties(obs_properties_t* props, const AVCodec* codec)
{
	AVCodecContext* context = avcodec_alloc_context3(codec);
	if (!context->priv_data) {
		avcodec_free_context(&context);
		return;
	}

	nvenc::get_properties_pre(props, codec, context);

	{
		obs_properties_t* grp = props;
		if (!streamfx::util::are_property_groups_broken()) {
			grp = obs_properties_create();
			obs_properties_add_group(props, S_CODEC_H264, D_TRANSLATE(S_CODEC_H264), OBS_GROUP_NORMAL, grp);
		}

		{
			auto p = obs_properties_add_list(grp, ST_KEY_PROFILE, D_TRANSLATE(S_CODEC_H264_PROFILE),
											 OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
			obs_property_list_add_int(p, D_TRANSLATE(S_STATE_DEFAULT), -1);
			streamfx::ffmpeg::tools::avoption_list_add_entries(context->priv_data, "profile", p, S_CODEC_H264_PROFILE);
		}
		{
			auto p = obs_properties_add_list(grp, ST_KEY_LEVEL, D_TRANSLATE(S_CODEC_H264_LEVEL), OBS_COMBO_TYPE_LIST,
											 OBS_COMBO_FORMAT_INT);
			obs_property_list_add_int(p, D_TRANSLATE(S_STATE_AUTOMATIC), 0);
			streamfx::ffmpeg::tools::avoption_list_add_entries_unnamed(context->priv_data, "level", p,
																	   [](const AVOption* opt) {
																		   if (opt->default_val.i64 == 0)
																			   return true;
																		   return false;
																	   });
		}
	}

	nvenc::get_properties_post(props, codec, context);

	if (context) {
		avcodec_free_context(&context);
	}
}

void nvenc_h264_handler::get_runtime_properties(obs_properties_t* props, const AVCodec* codec, AVCodecContext* context)
{
	nvenc::get_runtime_properties(props, codec, context);
}

void streamfx::encoder::ffmpeg::handler::nvenc_h264_handler::migrate(obs_data_t* settings, uint64_t version,
																	 const AVCodec* codec, AVCodecContext* context)
{
	nvenc::migrate(settings, version, codec, context);
}

bool nvenc_h264_handler::supports_reconfigure(ffmpeg_factory* instance, bool& threads, bool& gpu, bool& keyframes)
{
	threads   = false;
	gpu       = false;
	keyframes = false;
	return true;
}
