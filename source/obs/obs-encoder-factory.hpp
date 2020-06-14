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
#include "plugin.hpp"

namespace obs {
	template<class _factory, typename _instance>
	class encoder_factory {
		public:
		typedef _factory  factory_t;
		typedef _instance instance_t;

		protected:
		obs_encoder_info _info          = {};
		obs_encoder_info _info_fallback = {};
		std::string      _info_fallback_id;

		std::map<std::string, std::shared_ptr<obs_encoder_info>> _proxies;
		std::set<std::string>                                    _proxy_names;

		public:
		encoder_factory()
		{
			_info.type_data = this;

			_info.get_name        = _get_name;
			_info.create          = _create;
			_info.destroy         = _destroy;
			_info.get_defaults2   = _get_defaults2;
			_info.get_properties2 = _get_properties2;
			_info.update          = _update;
			_info.encode          = _encode;
			_info.get_extra_data  = _get_extra_data;
			_info.get_sei_data    = _get_sei_data;
		}
		virtual ~encoder_factory() {}

		void finish_setup()
		{
			if (_info.type == OBS_ENCODER_AUDIO) {
				_info.get_frame_size = _get_frame_size;
				_info.get_audio_info = _get_audio_info;
			} else if (_info.type == OBS_ENCODER_VIDEO) {
				_info.get_video_info = _get_video_info;
			}
			if (_info.caps & OBS_ENCODER_CAP_PASS_TEXTURE) {
				_info.encode_texture = _encode_texture;

				memcpy(&_info_fallback, &_info, sizeof(obs_encoder_info));
				_info_fallback_id = std::string(_info.id) + "_sw";
				_info_fallback.id = _info_fallback_id.c_str();
				_info_fallback.caps &= ~OBS_ENCODER_CAP_PASS_TEXTURE;
				_info_fallback.caps |= OBS_ENCODER_CAP_DEPRECATED;
				_info_fallback.encode_texture = nullptr;
			}

			obs_register_encoder(&_info);
		}

		void register_proxy(std::string_view name)
		{
			auto iter = _proxy_names.emplace(name);

			// Create proxy.
			std::shared_ptr<obs_encoder_info> proxy = std::make_shared<obs_encoder_info>();
			memcpy(proxy.get(), &_info, sizeof(obs_encoder_info));
			_info.id = iter.first->c_str();
			_info.caps |= OBS_SOURCE_DEPRECATED;
			obs_register_encoder(proxy.get());

			_proxies.emplace(name, proxy);
		}

		private /* Factory */:
		static const char* _get_name(void* type_data) noexcept
		try {
			if (type_data)
				return reinterpret_cast<factory_t*>(type_data)->get_name();
			return nullptr;
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
			return nullptr;
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
			return nullptr;
		}

		static void* _create(obs_data_t* settings, obs_encoder_t* encoder) noexcept
		try {
			try {
				return reinterpret_cast<factory_t*>(obs_encoder_get_type_data(encoder))->create(settings, encoder);
			} catch (const std::exception& ex) {
				if (factory_t* fac = reinterpret_cast<factory_t*>(obs_encoder_get_type_data(encoder));
					fac && (fac->_info.caps & OBS_ENCODER_CAP_PASS_TEXTURE)) {
					return obs_encoder_create_rerouted(encoder, fac->_info_fallback.id);
				} else {
					throw ex;
				}
			}
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
			return nullptr;
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
			return nullptr;
		}

		static void _get_defaults2(obs_data_t* settings, void* type_data) noexcept
		try {
			if (type_data)
				reinterpret_cast<factory_t*>(type_data)->get_defaults2(settings);
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		static obs_properties_t* _get_properties2(void* data, void* type_data) noexcept
		try {
			if (type_data)
				return reinterpret_cast<factory_t*>(type_data)->get_properties2(reinterpret_cast<instance_t*>(data));
			return nullptr;
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
			return nullptr;
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
			return nullptr;
		}

		private /* Instance */:
		static void _destroy(void* data) noexcept
		try {
			if (data)
				delete reinterpret_cast<instance_t*>(data);
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		static bool _update(void* data, obs_data_t* settings) noexcept
		try {
			auto priv = reinterpret_cast<instance_t*>(data);
			if (priv) {
				std::uint64_t version = static_cast<std::uint64_t>(obs_data_get_int(settings, S_VERSION));
				priv->migrate(settings, version);
				obs_data_set_int(settings, S_VERSION, static_cast<std::int64_t>(STREAMFX_VERSION));
				obs_data_set_string(settings, S_COMMIT, STREAMFX_COMMIT);
				return priv->update(settings);
			}
			return false;
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
			return false;
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
			return false;
		}

		static bool _encode(void* data, struct encoder_frame* frame, struct encoder_packet* packet,
							bool* received_packet) noexcept
		try {
			if (data)
				return reinterpret_cast<instance_t*>(data)->encode(frame, packet, received_packet);
			return false;
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
			return false;
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
			return false;
		}

		static bool _encode_texture(void* data, uint32_t handle, int64_t pts, uint64_t lock_key, uint64_t* next_key,
									struct encoder_packet* packet, bool* received_packet) noexcept
		try {
			if (data)
				return reinterpret_cast<instance_t*>(data)->encode_video(handle, pts, lock_key, next_key, packet,
																		 received_packet);
			return false;
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
			return false;
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
			return false;
		}

		static size_t _get_frame_size(void* data) noexcept
		try {
			if (data)
				return reinterpret_cast<instance_t*>(data)->get_frame_size();
			return 0;
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
			return 0;
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
			return 0;
		}

		static bool _get_extra_data(void* data, uint8_t** extra_data, size_t* size) noexcept
		try {
			if (data)
				return reinterpret_cast<instance_t*>(data)->get_extra_data(extra_data, size);
			return false;
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
			return false;
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
			return false;
		}

		static bool _get_sei_data(void* data, uint8_t** sei_data, size_t* size) noexcept
		try {
			if (data)
				return reinterpret_cast<instance_t*>(data)->get_sei_data(sei_data, size);
			return false;
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
			return false;
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
			return false;
		}

		static void _get_audio_info(void* data, struct audio_convert_info* info) noexcept
		try {
			if (data)
				reinterpret_cast<instance_t*>(data)->get_audio_info(info);
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		static void _get_video_info(void* data, struct video_scale_info* info) noexcept
		try {
			if (data)
				reinterpret_cast<instance_t*>(data)->get_video_info(info);
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		public:
		virtual const char* get_name()
		{
			return "Not Yet Implemented";
		}

		virtual void* create(obs_data_t* settings, obs_encoder_t* encoder)
		{
			return reinterpret_cast<void*>(new instance_t(settings, encoder));
		}

		virtual void get_defaults2(obs_data_t* data) {}

		virtual obs_properties_t* get_properties2(instance_t* data)
		{
			return nullptr;
		}
	};

	class encoder_instance {
		protected:
		obs_encoder_t* _self;

		public:
		encoder_instance(obs_data_t* settings, obs_encoder_t* self) : _self(self) {}
		virtual ~encoder_instance(){};

		virtual void migrate(obs_data_t* settings, std::uint64_t version) {}

		virtual bool update(obs_data_t* settings)
		{
			return false;
		}

		virtual bool encode(struct encoder_frame* frame, struct encoder_packet* packet, bool* received_packet)
		{
			auto type = obs_encoder_get_type(_self);
			if (type == OBS_ENCODER_VIDEO) {
				return encode_video(frame, packet, received_packet);
			} else if (type == OBS_ENCODER_AUDIO) {
				return encode_audio(frame, packet, received_packet);
			}
			return false;
		}

		virtual bool encode_audio(struct encoder_frame* frame, struct encoder_packet* packet,
								  bool* received_packet) = 0;

		virtual bool encode_video(struct encoder_frame* frame, struct encoder_packet* packet,
								  bool* received_packet) = 0;

		virtual bool encode_video(uint32_t handle, int64_t pts, uint64_t lock_key, uint64_t* next_key,
								  struct encoder_packet* packet, bool* received_packet) = 0;

		virtual size_t get_frame_size()
		{
			return 0;
		}

		virtual bool get_extra_data(uint8_t** extra_data, size_t* size)
		{
			return false;
		}

		virtual bool get_sei_data(uint8_t** sei_data, size_t* size)
		{
			return false;
		}

		virtual void get_audio_info(struct audio_convert_info* info) {}

		virtual void get_video_info(struct video_scale_info* info) {}
	};

} // namespace obs
