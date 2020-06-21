/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2017-2018 Michael Fabian Dirks
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
#include "handler.hpp"

extern "C" {
#pragma warning(push)
#pragma warning(disable : 4244)
#include <libavcodec/avcodec.h>
#pragma warning(pop)
}

namespace streamfx::encoder::ffmpeg::handler {
	class amf_hevc_handler : public handler {
		public:
		virtual ~amf_hevc_handler(){};

		public /*factory*/:
		virtual void adjust_info(ffmpeg_factory* factory, const AVCodec* codec, std::string& id, std::string& name,
								 std::string& codec_id);

		virtual void get_defaults(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context, bool hw_encode);

		public /*support tests*/:
		virtual bool has_keyframe_support(ffmpeg_factory* instance);

		virtual bool is_hardware_encoder(ffmpeg_factory* instance);

		virtual bool has_threading_support(ffmpeg_factory* instance);

		virtual bool has_pixel_format_support(ffmpeg_factory* instance);

		public /*settings*/:
		virtual void get_properties(obs_properties_t* props, const AVCodec* codec, AVCodecContext* context,
									bool hw_encode);

		virtual void migrate(obs_data_t* settings, std::uint64_t version, const AVCodec* codec,
							 AVCodecContext* context);

		virtual void update(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context);

		virtual void override_update(ffmpeg_instance* instance, obs_data_t* settings);

		virtual void log_options(obs_data_t* settings, const AVCodec* codec, AVCodecContext* context);

		private:
		void get_encoder_properties(obs_properties_t* props, const AVCodec* codec);

		void get_runtime_properties(obs_properties_t* props, const AVCodec* codec, AVCodecContext* context);
	};
} // namespace streamfx::encoder::ffmpeg::handler
