/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2020 Michael Fabian Dirks
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#pragma once
#include "common.hpp"
#include "handler.hpp"

extern "C" {
#pragma warning(push)
#pragma warning(disable : 4244)
#include <libavcodec/avcodec.h>
#pragma warning(pop)
}

namespace streamfx::encoder::ffmpeg::handler {
	class amf_h264_handler : public handler {
		public:
		virtual ~amf_h264_handler(){};

		public /*factory*/:
		void adjust_info(ffmpeg_factory* factory, const AVCodec* codec, std::string& id, std::string& name,
						 std::string& codec_id) override;

		void get_defaults(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context, bool hw_encode) override;

		virtual std::string_view get_help_url(const AVCodec* codec) override
		{
			return "https://github.com/Xaymar/obs-StreamFX/wiki/Encoder-FFmpeg-AMF";
		};

		public /*support tests*/:
		bool has_keyframe_support(ffmpeg_factory* instance) override;

		bool is_hardware_encoder(ffmpeg_factory* instance) override;

		bool has_threading_support(ffmpeg_factory* instance) override;

		bool has_pixel_format_support(ffmpeg_factory* instance) override;

		public /*settings*/:
		void get_properties(obs_properties_t* props, const AVCodec* codec, AVCodecContext* context,
							bool hw_encode) override;

		void migrate(obs_data_t* settings, std::uint64_t version, const AVCodec* codec,
					 AVCodecContext* context) override;

		void update(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context) override;

		void override_update(ffmpeg_instance* instance, obs_data_t* settings) override;

		void log_options(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context) override;

		public /*instance*/:

		void override_colorformat(AVPixelFormat& target_format, obs_data_t* settings, const AVCodec* codec,
								  AVCodecContext* context) override;

		private:
		void get_encoder_properties(obs_properties_t* props, const AVCodec* codec);

		void get_runtime_properties(obs_properties_t* props, const AVCodec* codec, AVCodecContext* context);
	};
} // namespace streamfx::encoder::ffmpeg::handler
