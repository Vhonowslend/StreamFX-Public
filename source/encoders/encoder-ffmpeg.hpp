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
#include <condition_variable>
#include <map>
#include <mutex>
#include <queue>
#include <stack>
#include <thread>
#include <vector>
#include "ffmpeg/avframe-queue.hpp"
#include "ffmpeg/hwapi/base.hpp"
#include "ffmpeg/swscale.hpp"
#include "handlers/handler.hpp"
#include "obs/obs-encoder-factory.hpp"

extern "C" {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4242 4244 4365)
#endif
#include <obs-properties.h>
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

namespace streamfx::encoder::ffmpeg {
	class ffmpeg_factory;

	class ffmpeg_instance : public obs::encoder_instance {
		ffmpeg_factory* _factory;
		const AVCodec*  _codec;
		AVCodecContext* _context;

		std::shared_ptr<handler::handler> _handler;

		::ffmpeg::swscale _scaler;
		AVPacket          _packet;

		std::shared_ptr<::ffmpeg::hwapi::base>     _hwapi;
		std::shared_ptr<::ffmpeg::hwapi::instance> _hwinst;

		std::size_t _lag_in_frames;
		std::size_t _sent_frames;

		// Extra Data
		bool                      _have_first_frame;
		std::vector<std::uint8_t> _extra_data;
		std::vector<std::uint8_t> _sei_data;

		// Frame Stack and Queue
		std::stack<std::shared_ptr<AVFrame>>           _free_frames;
		std::queue<std::shared_ptr<AVFrame>>           _used_frames;
		std::chrono::high_resolution_clock::time_point _free_frames_last_used;

		public:
		ffmpeg_instance(obs_data_t* settings, obs_encoder_t* self);
		virtual ~ffmpeg_instance();

		public:
		void get_properties(obs_properties_t* props);

		void migrate(obs_data_t* settings, std::uint64_t version) override;

		bool update(obs_data_t* settings) override;

		bool encode_audio(struct encoder_frame* frame, struct encoder_packet* packet, bool* received_packet) override;

		bool encode_video(struct encoder_frame* frame, struct encoder_packet* packet, bool* received_packet) override;

		bool encode_video(uint32_t handle, int64_t pts, uint64_t lock_key, uint64_t* next_key,
						  struct encoder_packet* packet, bool* received_packet) override;

		bool get_extra_data(uint8_t** extra_data, size_t* size) override;

		bool get_sei_data(uint8_t** sei_data, size_t* size) override;

		void get_video_info(struct video_scale_info* info) override;

		public:
		void initialize_sw(obs_data_t* settings);
		void initialize_hw(obs_data_t* settings);

		void                     push_free_frame(std::shared_ptr<AVFrame> frame);
		std::shared_ptr<AVFrame> pop_free_frame();

		void                     push_used_frame(std::shared_ptr<AVFrame> frame);
		std::shared_ptr<AVFrame> pop_used_frame();

		int receive_packet(bool* received_packet, struct encoder_packet* packet);

		int send_frame(std::shared_ptr<AVFrame> frame);

		bool encode_avframe(std::shared_ptr<AVFrame> frame, struct encoder_packet* packet, bool* received_packet);

		public: // Handler API
		bool is_hardware_encode();

		const AVCodec* get_avcodec();

		const AVCodecContext* get_avcodeccontext();

		void parse_ffmpeg_commandline(std::string text);
	};

	class ffmpeg_factory : public obs::encoder_factory<ffmpeg_factory, ffmpeg_instance> {
		std::string _id;
		std::string _codec;
		std::string _name;

		const AVCodec* _avcodec;

		std::shared_ptr<handler::handler> _handler;

		public:
		ffmpeg_factory(const AVCodec* codec);
		virtual ~ffmpeg_factory();

		const char* get_name() override;

		void get_defaults2(obs_data_t* data) override;

		obs_properties_t* get_properties2(instance_t* data) override;

		public:
		const AVCodec* get_avcodec();
	};

	class ffmpeg_manager {
		std::map<const AVCodec*, std::shared_ptr<ffmpeg_factory>> _factories;
		std::map<std::string, std::shared_ptr<handler::handler>>  _handlers;
		std::shared_ptr<handler::handler>                         _debug_handler;

		public:
		ffmpeg_manager();
		~ffmpeg_manager();

		void register_encoders();

		void register_handler(std::string codec, std::shared_ptr<handler::handler> handler);

		std::shared_ptr<handler::handler> get_handler(std::string codec);

		bool has_handler(std::string codec);

		public: // Singleton
		static void initialize();

		static void finalize();

		static std::shared_ptr<ffmpeg_manager> get();
	};
} // namespace streamfx::encoder::ffmpeg
