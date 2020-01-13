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
#include "codecs/h264.hpp"
#include "encoder.hpp"
#include "ffmpeg/tools.hpp"
#include "nvenc_shared.hpp"
#include "plugin.hpp"
#include "strings.hpp"
#include "utility.hpp"

extern "C" {
#include <obs-module.h>
#pragma warning(push)
#pragma warning(disable : 4244)
#include <libavutil/opt.h>
#pragma warning(pop)
}

using namespace obsffmpeg::codecs::h264;

std::map<profile, std::string> profiles{
    {profile::BASELINE, "baseline"},
    {profile::MAIN, "main"},
    {profile::HIGH, "high"},
    {profile::HIGH444_PREDICTIVE, "high444p"},
};

std::map<level, std::string> levels{
    {level::L1_0, "1.0"}, {level::L1_0b, "1.0b"}, {level::L1_1, "1.1"}, {level::L1_2, "1.2"}, {level::L1_3, "1.3"},
    {level::L2_0, "2.0"}, {level::L2_1, "2.1"},   {level::L2_2, "2.2"}, {level::L3_0, "3.0"}, {level::L3_1, "3.1"},
    {level::L3_2, "3.2"}, {level::L4_0, "4.0"},   {level::L4_1, "4.1"}, {level::L4_2, "4.2"}, {level::L5_0, "5.0"},
    {level::L5_1, "5.1"}, {level::L5_2, "5.2"},
};

INITIALIZER(nvenc_h264_handler_init)
{
	obsffmpeg::initializers.push_back([]() {
		obsffmpeg::register_codec_handler("h264_nvenc", std::make_shared<obsffmpeg::ui::nvenc_h264_handler>());
	});
};

void obsffmpeg::ui::nvenc_h264_handler::adjust_encoder_info(obsffmpeg::encoder_factory*, obsffmpeg::encoder_info* main,
                                                            obsffmpeg::encoder_info* fallback)
{
	main->readable_name     = "H.264/AVC NVidia NVENC (Hardware)";
	fallback->readable_name = "H.264/AVC NVidia NVENC (Software)";
}

void obsffmpeg::ui::nvenc_h264_handler::get_defaults(obs_data_t* settings, const AVCodec* codec,
                                                     AVCodecContext* context, bool)
{
	nvenc::get_defaults(settings, codec, context);

	obs_data_set_default_int(settings, P_H264_PROFILE, static_cast<int64_t>(codecs::h264::profile::HIGH));
	obs_data_set_default_int(settings, P_H264_LEVEL, static_cast<int64_t>(codecs::h264::level::UNKNOWN));
}

bool obsffmpeg::ui::nvenc_h264_handler::has_keyframe_support(obsffmpeg::encoder*)
{
	return true;
}

void obsffmpeg::ui::nvenc_h264_handler::get_properties(obs_properties_t* props, const AVCodec* codec,
                                                       AVCodecContext* context, bool)
{
	if (!context) {
		this->get_encoder_properties(props, codec);
	} else {
		this->get_runtime_properties(props, codec, context);
	}
}

void obsffmpeg::ui::nvenc_h264_handler::update(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context)
{
	nvenc::update(settings, codec, context);

	{
		auto found =
		    profiles.find(static_cast<codecs::h264::profile>(obs_data_get_int(settings, P_H264_PROFILE)));
		if (found != profiles.end()) {
			av_opt_set(context->priv_data, "profile", found->second.c_str(), 0);
		}
	}

	{
		auto found = levels.find(static_cast<codecs::h264::level>(obs_data_get_int(settings, P_H264_LEVEL)));
		if (found != levels.end()) {
			av_opt_set(context->priv_data, "level", found->second.c_str(), 0);
		} else {
			av_opt_set(context->priv_data, "level", "auto", 0);
		}
	}
}

void obsffmpeg::ui::nvenc_h264_handler::override_update(obsffmpeg::encoder* instance, obs_data_t* settings)
{
	nvenc::override_update(instance, settings);
}

void obsffmpeg::ui::nvenc_h264_handler::log_options(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context)
{
	nvenc::log_options(settings, codec, context);

	PLOG_INFO("[%s]     H.265/HEVC:", codec->name);
	ffmpeg::tools::print_av_option_string(context, "profile", "      Profile", [](int64_t v) {
		profile val   = static_cast<profile>(v);
		auto    index = profiles.find(val);
		if (index != profiles.end())
			return index->second;
		return std::string("<Unknown>");
	});
	ffmpeg::tools::print_av_option_string(context, "level", "      Level", [](int64_t v) {
		level val   = static_cast<level>(v);
		auto  index = levels.find(val);
		if (index != levels.end())
			return index->second;
		return std::string("<Unknown>");
	});
}

void obsffmpeg::ui::nvenc_h264_handler::get_encoder_properties(obs_properties_t* props, const AVCodec* codec)
{
	nvenc::get_properties_pre(props, codec);

	{
		obs_properties_t* grp = props;
		if (!obsffmpeg::are_property_groups_broken()) {
			grp = obs_properties_create();
			obs_properties_add_group(props, P_H264, TRANSLATE(P_H264), OBS_GROUP_NORMAL, grp);
		}

		{
			auto p = obs_properties_add_list(grp, P_H264_PROFILE, TRANSLATE(P_H264_PROFILE),
			                                 OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
			obs_property_set_long_description(p, TRANSLATE(DESC(P_H264_PROFILE)));
			obs_property_list_add_int(p, TRANSLATE(S_STATE_DEFAULT),
			                          static_cast<int64_t>(codecs::h264::profile::UNKNOWN));
			for (auto const kv : profiles) {
				std::string trans = std::string(P_H264_PROFILE) + "." + kv.second;
				obs_property_list_add_int(p, TRANSLATE(trans.c_str()), static_cast<int64_t>(kv.first));
			}
		}
		{
			auto p = obs_properties_add_list(grp, P_H264_LEVEL, TRANSLATE(P_H264_LEVEL),
			                                 OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
			obs_property_set_long_description(p, TRANSLATE(DESC(P_H264_LEVEL)));
			obs_property_list_add_int(p, TRANSLATE(S_STATE_AUTOMATIC),
			                          static_cast<int64_t>(codecs::h264::level::UNKNOWN));
			for (auto const kv : levels) {
				obs_property_list_add_int(p, kv.second.c_str(), static_cast<int64_t>(kv.first));
			}
		}
	}

	nvenc::get_properties_post(props, codec);
}

void obsffmpeg::ui::nvenc_h264_handler::get_runtime_properties(obs_properties_t* props, const AVCodec* codec,
                                                               AVCodecContext* context)
{
	nvenc::get_runtime_properties(props, codec, context);
}
