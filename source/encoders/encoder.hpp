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

#include <condition_variable>
#include <mutex>
#include <queue>
#include <stack>
#include <thread>
#include <vector>
#include "ffmpeg/avframe-queue.hpp"
#include "ffmpeg/swscale.hpp"
#include "hwapi/base.hpp"
#include "ui/handler.hpp"

extern "C" {
#include <obs-properties.h>
#include <obs.h>
#pragma warning(push)
#pragma warning(disable : 4244)
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#pragma warning(pop)
}

namespace obsffmpeg {
	class unsupported_gpu_exception : public std::runtime_error {
		public:
		unsupported_gpu_exception(const std::string& reason) : runtime_error(reason) {}
	};

	struct encoder_info {
		std::string      uid;
		std::string      codec;
		std::string      readable_name;
		obs_encoder_info oei = {0};
	};

	class encoder_factory {
		encoder_info   info;
		encoder_info   info_fallback;
		const AVCodec* avcodec_ptr;

		std::shared_ptr<obsffmpeg::ui::handler> _handler;

		public:
		encoder_factory(const AVCodec* codec);
		virtual ~encoder_factory();

		void register_encoder();

		void get_defaults(obs_data_t* settings, bool hw_encoder = false);

		void get_properties(obs_properties_t* props, bool hw_encoder = false);

		const AVCodec* get_avcodec();

		const encoder_info& get_info();

		const encoder_info& get_fallback();
	};

	class encoder {
		obs_encoder_t*   _self;
		encoder_factory* _factory;

		const AVCodec*  _codec;
		AVCodecContext* _context;

		std::shared_ptr<obsffmpeg::ui::handler> _handler;

		std::shared_ptr<obsffmpeg::hwapi::base>     _hwapi;
		std::shared_ptr<obsffmpeg::hwapi::instance> _hwinst;

		ffmpeg::swscale _swscale;
		AVPacket        _current_packet;

		size_t _lag_in_frames;
		size_t _count_send_frames;

		// Extra Data
		bool                 _have_first_frame;
		std::vector<uint8_t> _extra_data;
		std::vector<uint8_t> _sei_data;

		// Frame Stack and Queue
		std::stack<std::shared_ptr<AVFrame>>           _free_frames;
		std::queue<std::shared_ptr<AVFrame>>           _used_frames;
		std::chrono::high_resolution_clock::time_point _free_frames_last_used;

		void initialize_sw(obs_data_t* settings);
		void initialize_hw(obs_data_t* settings);

		void                     push_free_frame(std::shared_ptr<AVFrame> frame);
		std::shared_ptr<AVFrame> pop_free_frame();

		void                     push_used_frame(std::shared_ptr<AVFrame> frame);
		std::shared_ptr<AVFrame> pop_used_frame();

		public:
		encoder(obs_data_t* settings, obs_encoder_t* encoder, bool is_texture_encode = false);
		virtual ~encoder();

		public: // OBS API
		// Shared
		void get_properties(obs_properties_t* props, bool hw_encode = false);

		bool update(obs_data_t* settings);

		// Audio only
		void get_audio_info(struct audio_convert_info* info);

		size_t get_frame_size();

		bool audio_encode(struct encoder_frame* frame, struct encoder_packet* packet, bool* received_packet);

		// Video only
		void get_video_info(struct video_scale_info* info);

		bool get_sei_data(uint8_t** sei_data, size_t* size);

		bool get_extra_data(uint8_t** extra_data, size_t* size);

		bool video_encode(struct encoder_frame* frame, struct encoder_packet* packet, bool* received_packet);

		bool video_encode_texture(uint32_t handle, int64_t pts, uint64_t lock_key, uint64_t* next_key,
		                          struct encoder_packet* packet, bool* received_packet);

		int receive_packet(bool* received_packet, struct encoder_packet* packet);

		int send_frame(std::shared_ptr<AVFrame> frame);

		bool encode_avframe(std::shared_ptr<AVFrame> frame, struct encoder_packet* packet,
		                    bool* received_packet);

		public: // Handler API
		bool is_hardware_encode();

		const AVCodec* get_avcodec();

		const AVCodecContext* get_avcodeccontext();

		void parse_ffmpeg_commandline(std::string text);
	};
} // namespace obsffmpeg
