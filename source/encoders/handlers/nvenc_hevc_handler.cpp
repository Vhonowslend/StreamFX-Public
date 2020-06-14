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

#include "nvenc_hevc_handler.hpp"
#include "strings.hpp"
#include "../codecs/hevc.hpp"
#include "../encoder-ffmpeg.hpp"
#include "ffmpeg/tools.hpp"
#include "nvenc_shared.hpp"
#include "plugin.hpp"
#include "utility.hpp"

extern "C" {
#include <obs-module.h>
#pragma warning(push)
#pragma warning(disable : 4242 4244 4365)
#include <libavutil/opt.h>
#pragma warning(pop)
}

#define KEY_PROFILE "H265.Profile"
#define KEY_TIER "H265.Tier"
#define KEY_LEVEL "H265.Level"

using namespace streamfx::encoder::ffmpeg::handler;
using namespace streamfx::encoder::codec::hevc;

std::map<profile, std::string> profiles{
	{profile::MAIN, "main"},
	{profile::MAIN10, "main10"},
	{profile::RANGE_EXTENDED, "rext"},
};

std::map<tier, std::string> tiers{
	{tier::MAIN, "main"},
	{tier::HIGH, "high"},
};

std::map<level, std::string> levels{
	{level::L1_0, "1.0"}, {level::L2_0, "2.0"}, {level::L2_1, "2.1"}, {level::L3_0, "3.0"}, {level::L3_1, "3.1"},
	{level::L4_0, "4.0"}, {level::L4_1, "4.1"}, {level::L5_0, "5.0"}, {level::L5_1, "5.1"}, {level::L5_2, "5.2"},
	{level::L6_0, "6.0"}, {level::L6_1, "6.1"}, {level::L6_2, "6.2"},
};

void nvenc_hevc_handler::adjust_info(ffmpeg_factory*, const AVCodec*, std::string&, std::string& name, std::string&)
{
	name = "NVIDIA NVENC H.265/HEVC Encoder";
}

void nvenc_hevc_handler::get_defaults(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context, bool)
{
	nvenc::get_defaults(settings, codec, context);

	obs_data_set_default_int(settings, KEY_PROFILE, static_cast<int64_t>(profile::MAIN));
	obs_data_set_default_int(settings, KEY_TIER, static_cast<int64_t>(profile::MAIN));
	obs_data_set_default_int(settings, KEY_LEVEL, static_cast<int64_t>(level::UNKNOWN));
}

bool nvenc_hevc_handler::has_keyframe_support(ffmpeg_factory*)
{
	return true;
}

bool nvenc_hevc_handler::is_hardware_encoder(ffmpeg_factory* instance)
{
	return true;
}

bool nvenc_hevc_handler::has_threading_support(ffmpeg_factory* instance)
{
	return false;
}

bool nvenc_hevc_handler::has_pixel_format_support(ffmpeg_factory* instance)
{
	return true;
}

void nvenc_hevc_handler::get_properties(obs_properties_t* props, const AVCodec* codec, AVCodecContext* context, bool)
{
	if (!context) {
		this->get_encoder_properties(props, codec);
	} else {
		this->get_runtime_properties(props, codec, context);
	}
}

void nvenc_hevc_handler::update(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context)
{
	nvenc::update(settings, codec, context);

	{ // HEVC Options
		auto found = profiles.find(static_cast<profile>(obs_data_get_int(settings, KEY_PROFILE)));
		if (found != profiles.end()) {
			av_opt_set(context->priv_data, "profile", found->second.c_str(), 0);
		}
	}
	{
		auto found = tiers.find(static_cast<tier>(obs_data_get_int(settings, KEY_TIER)));
		if (found != tiers.end()) {
			av_opt_set(context->priv_data, "tier", found->second.c_str(), 0);
		}
	}
	{
		auto found = levels.find(static_cast<level>(obs_data_get_int(settings, KEY_LEVEL)));
		if (found != levels.end()) {
			av_opt_set(context->priv_data, "level", found->second.c_str(), 0);
		} else {
			av_opt_set(context->priv_data, "level", "auto", 0);
		}
	}
}

void nvenc_hevc_handler::override_update(ffmpeg_instance* instance, obs_data_t* settings)
{
	nvenc::override_update(instance, settings);
}

void nvenc_hevc_handler::log_options(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context)
{
	nvenc::log_options(settings, codec, context);

	LOG_INFO("[%s]     H.265/HEVC:", codec->name);
	::ffmpeg::tools::print_av_option_string(context, "profile", "      Profile", [](int64_t v) {
		profile val   = static_cast<profile>(v);
		auto    index = profiles.find(val);
		if (index != profiles.end())
			return index->second;
		return std::string("<Unknown>");
	});
	::ffmpeg::tools::print_av_option_string(context, "level", "      Level", [](int64_t v) {
		level val   = static_cast<level>(v);
		auto  index = levels.find(val);
		if (index != levels.end())
			return index->second;
		return std::string("<Unknown>");
	});
	::ffmpeg::tools::print_av_option_string(context, "tier", "      Tier", [](int64_t v) {
		tier val   = static_cast<tier>(v);
		auto index = tiers.find(val);
		if (index != tiers.end())
			return index->second;
		return std::string("<Unknown>");
	});
}

void nvenc_hevc_handler::get_encoder_properties(obs_properties_t* props, const AVCodec* codec)
{
	nvenc::get_properties_pre(props, codec);

	{
		obs_properties_t* grp = props;
		if (!util::are_property_groups_broken()) {
			grp = obs_properties_create();
			obs_properties_add_group(props, P_HEVC, D_TRANSLATE(P_HEVC), OBS_GROUP_NORMAL, grp);
		}

		{
			auto p = obs_properties_add_list(grp, KEY_PROFILE, D_TRANSLATE(P_HEVC_PROFILE), OBS_COMBO_TYPE_LIST,
											 OBS_COMBO_FORMAT_INT);
			obs_property_set_long_description(p, D_TRANSLATE(D_DESC(P_HEVC_PROFILE)));
			obs_property_list_add_int(p, D_TRANSLATE(S_STATE_DEFAULT), static_cast<int64_t>(profile::UNKNOWN));
			for (auto const kv : profiles) {
				std::string trans = std::string(P_HEVC_PROFILE) + "." + kv.second;
				obs_property_list_add_int(p, D_TRANSLATE(trans.c_str()), static_cast<int64_t>(kv.first));
			}
		}
		{
			auto p = obs_properties_add_list(grp, KEY_TIER, D_TRANSLATE(P_HEVC_TIER), OBS_COMBO_TYPE_LIST,
											 OBS_COMBO_FORMAT_INT);
			obs_property_set_long_description(p, D_TRANSLATE(D_DESC(P_HEVC_TIER)));
			obs_property_list_add_int(p, D_TRANSLATE(S_STATE_DEFAULT), static_cast<int64_t>(tier::UNKNOWN));
			for (auto const kv : tiers) {
				std::string trans = std::string(P_HEVC_TIER) + "." + kv.second;
				obs_property_list_add_int(p, D_TRANSLATE(trans.c_str()), static_cast<int64_t>(kv.first));
			}
		}
		{
			auto p = obs_properties_add_list(grp, KEY_LEVEL, D_TRANSLATE(P_HEVC_LEVEL), OBS_COMBO_TYPE_LIST,
											 OBS_COMBO_FORMAT_INT);
			obs_property_set_long_description(p, D_TRANSLATE(D_DESC(P_HEVC_LEVEL)));
			obs_property_list_add_int(p, D_TRANSLATE(S_STATE_AUTOMATIC), static_cast<int64_t>(level::UNKNOWN));
			for (auto const kv : levels) {
				obs_property_list_add_int(p, kv.second.c_str(), static_cast<int64_t>(kv.first));
			}
		}
	}

	nvenc::get_properties_post(props, codec);
}

void nvenc_hevc_handler::get_runtime_properties(obs_properties_t* props, const AVCodec* codec, AVCodecContext* context)
{
	nvenc::get_runtime_properties(props, codec, context);
}
