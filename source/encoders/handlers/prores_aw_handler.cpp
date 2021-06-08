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

#include "prores_aw_handler.hpp"
#include <array>
#include "../codecs/prores.hpp"
#include "ffmpeg/tools.hpp"
#include "plugin.hpp"

extern "C" {
#include <obs-module.h>
}

using namespace streamfx::encoder::ffmpeg::handler;
using namespace streamfx::encoder::codec::prores;

void prores_aw_handler::override_colorformat(AVPixelFormat& target_format, obs_data_t* settings, const AVCodec* codec,
											 AVCodecContext*)
{
	static const std::array<std::pair<profile, AVPixelFormat>, static_cast<size_t>(profile::_COUNT)>
		profile_to_format_map{
			std::pair{profile::APCO, AV_PIX_FMT_YUV422P10}, std::pair{profile::APCS, AV_PIX_FMT_YUV422P10},
			std::pair{profile::APCN, AV_PIX_FMT_YUV422P10}, std::pair{profile::APCH, AV_PIX_FMT_YUV422P10},
			std::pair{profile::AP4H, AV_PIX_FMT_YUV444P10}, std::pair{profile::AP4X, AV_PIX_FMT_YUV444P10},
		};

	const int64_t profile_id = obs_data_get_int(settings, S_CODEC_PRORES_PROFILE);
	for (auto kv : profile_to_format_map) {
		if (kv.first == static_cast<profile>(profile_id)) {
			target_format = kv.second;
			break;
		}
	}
}

void prores_aw_handler::get_defaults(obs_data_t* settings, const AVCodec*, AVCodecContext*, bool)
{
	obs_data_set_default_int(settings, S_CODEC_PRORES_PROFILE, 0);
}

bool prores_aw_handler::has_pixel_format_support(ffmpeg_factory* instance)
{
	return false;
}

inline const char* profile_to_name(const AVProfile* ptr)
{
	switch (static_cast<profile>(ptr->profile)) {
	case profile::APCO:
		return D_TRANSLATE(S_CODEC_PRORES_PROFILE_APCO);
	case profile::APCS:
		return D_TRANSLATE(S_CODEC_PRORES_PROFILE_APCS);
	case profile::APCN:
		return D_TRANSLATE(S_CODEC_PRORES_PROFILE_APCN);
	case profile::APCH:
		return D_TRANSLATE(S_CODEC_PRORES_PROFILE_APCH);
	case profile::AP4H:
		return D_TRANSLATE(S_CODEC_PRORES_PROFILE_AP4H);
	case profile::AP4X:
		return D_TRANSLATE(S_CODEC_PRORES_PROFILE_AP4X);
	default:
		return ptr->name;
	}
}

void prores_aw_handler::get_properties(obs_properties_t* props, const AVCodec* codec, AVCodecContext* context, bool)
{
	if (!context) {
		auto p = obs_properties_add_list(props, S_CODEC_PRORES_PROFILE, D_TRANSLATE(S_CODEC_PRORES_PROFILE),
										 OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
		for (auto ptr = codec->profiles; ptr->profile != FF_PROFILE_UNKNOWN; ptr++) {
			obs_property_list_add_int(p, profile_to_name(ptr), static_cast<int64_t>(ptr->profile));
		}
	} else {
		obs_property_set_enabled(obs_properties_get(props, S_CODEC_PRORES_PROFILE), false);
	}
}

void prores_aw_handler::update(obs_data_t* settings, const AVCodec*, AVCodecContext* context)
{
	context->profile = static_cast<int>(obs_data_get_int(settings, S_CODEC_PRORES_PROFILE));
}

void prores_aw_handler::log_options(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context)
{
	DLOG_INFO("[%s]   Apple ProRes:", codec->name);
	::streamfx::ffmpeg::tools::print_av_option_string(context, "profile", "    Profile", [&codec](int64_t v) {
		int val = static_cast<int>(v);
		for (auto ptr = codec->profiles; (ptr->profile != FF_PROFILE_UNKNOWN) && (ptr != nullptr); ptr++) {
			if (ptr->profile == val) {
				return std::string(profile_to_name(ptr));
			}
		}
		return std::string("<Unknown>");
	});
}

void prores_aw_handler::process_avpacket(AVPacket& packet, const AVCodec*, AVCodecContext*)
{
	//FFmpeg Bug:
	// When ProRes content is stored in Matroska, FFmpeg strips the size
	// from the atom. Later when the ProRes content is demuxed from Matroska,
	// FFmpeg creates an atom with the incorrect size, as the ATOM size
	// should be content + atom, but FFmpeg set it to only be content. This
	// difference leads to decoders to be off by 8 bytes.
	//Fix (until FFmpeg stops being broken):
	// Pad the packet with 8 bytes of 0x00.

	av_grow_packet(&packet, 8);
}
