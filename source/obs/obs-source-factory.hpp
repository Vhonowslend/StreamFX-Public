/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2018 Michael Fabian Dirks
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
	class source_factory {
		protected:
		obs_source_info                                         _info = {};
		std::map<std::string, std::shared_ptr<obs_source_info>> _proxies;
		std::set<std::string>                                   _proxy_names;

		public:
		source_factory()
		{
			_info.type_data = this;

			_info.get_name        = _get_name;
			_info.create          = _create;
			_info.destroy         = _destroy;
			_info.get_defaults2   = _get_defaults2;
			_info.get_properties2 = _get_properties2;
			_info.load            = _load;
			_info.update          = _update;
			_info.save            = _save;
			_info.filter_remove   = _filter_remove;

			set_resolution_enabled(true);
			set_activity_tracking_enabled(false);
			set_visibility_tracking_enabled(false);
			set_input_enabled(false);
			set_have_child_sources(false);
		}
		virtual ~source_factory() {}

		protected:
		void set_resolution_enabled(bool v)
		{
			if (v) {
				_info.get_width  = _get_width;
				_info.get_height = _get_height;
			} else {
				_info.get_width  = nullptr;
				_info.get_height = nullptr;
			}
		}

		void set_activity_tracking_enabled(bool v)
		{
			if (v) {
				_info.activate   = _activate;
				_info.deactivate = _deactivate;
			} else {
				_info.activate   = nullptr;
				_info.deactivate = nullptr;
			}
		}

		void set_visibility_tracking_enabled(bool v)
		{
			if (v) {
				_info.show = _show;
				_info.hide = _hide;
			} else {
				_info.show = nullptr;
				_info.hide = nullptr;
			}
		}

		void set_input_enabled(bool v)
		{
			if (v) {
				_info.mouse_click = _mouse_click;
				_info.mouse_move  = _mouse_move;
				_info.mouse_wheel = _mouse_wheel;
				_info.focus       = _focus;
				_info.key_click   = _key_click;
			} else {
				_info.mouse_click = nullptr;
				_info.mouse_move  = nullptr;
				_info.mouse_wheel = nullptr;
				_info.focus       = nullptr;
				_info.key_click   = nullptr;
			}
		}

		void set_have_child_sources(bool v)
		{
			if (v) {
				_info.enum_all_sources = _enum_all_sources;
			} else {
				_info.enum_all_sources = nullptr;
			}
		}

		void set_have_active_child_sources(bool v)
		{
			if (v) {
				_info.enum_active_sources = _enum_active_sources;
			} else {
				_info.enum_active_sources = nullptr;
			}
		}

		void finish_setup()
		{
			if (_info.output_flags & OBS_SOURCE_INTERACTION) {
				set_input_enabled(true);
			} else {
				set_input_enabled(false);
			}

			if (_info.type == OBS_SOURCE_TYPE_TRANSITION) {
				set_resolution_enabled(false);
				_info.transition_start = _transition_start;
				_info.transition_stop  = _transition_stop;
				_info.audio_render     = _audio_render;
				_info.video_tick       = _video_tick;
				_info.video_render     = _video_render;
			} else if (_info.type == OBS_SOURCE_TYPE_FILTER) {
				switch (_info.output_flags & OBS_SOURCE_ASYNC_VIDEO) {
				case OBS_SOURCE_ASYNC_VIDEO:
					_info.filter_video = _filter_video;
					break;
				case OBS_SOURCE_VIDEO:
					_info.video_tick   = _video_tick;
					_info.video_render = _video_render;
					break;
				}
				if ((_info.output_flags & OBS_SOURCE_AUDIO) != 0) {
					_info.filter_audio = _filter_audio;
					if ((_info.output_flags & OBS_SOURCE_COMPOSITE) != 0) {
						_info.audio_render = _audio_render;
					}
				}
			} else {
				if ((_info.output_flags & OBS_SOURCE_ASYNC_VIDEO) != 0) {
					if ((_info.output_flags & OBS_SOURCE_ASYNC) == 0) {
						set_resolution_enabled(true);
					}
					_info.video_tick   = _video_tick;
					_info.video_render = _video_render;
				}
				if ((_info.output_flags & OBS_SOURCE_COMPOSITE) != 0) {
					_info.audio_render = _audio_render;
				}
			}

			obs_register_source(&_info);
		}

		void register_proxy(std::string_view name)
		{
			auto iter = _proxy_names.emplace(name);

			// Create proxy.
			std::shared_ptr<obs_source_info> proxy = std::make_shared<obs_source_info>();
			memcpy(proxy.get(), &_info, sizeof(obs_source_info));
			_info.id = iter.first->c_str();
			_info.output_flags |= OBS_SOURCE_DEPRECATED;
			obs_register_source(proxy.get());

			_proxies.emplace(name, proxy);
		}

		private /* Factory */:
		static const char* _get_name(void* type_data) noexcept
		try {
			if (type_data)
				return reinterpret_cast<_factory*>(type_data)->get_name();
			return nullptr;
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
			return nullptr;
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
			return nullptr;
		}

		static void* _create(obs_data_t* settings, obs_source_t* source) noexcept
		try {
			return reinterpret_cast<_factory*>(obs_source_get_type_data(source))->create(settings, source);
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
			return nullptr;
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
			return nullptr;
		}

		static void _get_defaults2(void* type_data, obs_data_t* settings) noexcept
		try {
			if (type_data)
				reinterpret_cast<_factory*>(type_data)->get_defaults2(settings);
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		static obs_properties_t* _get_properties2(void* data, void* type_data) noexcept
		try {
			if (type_data)
				return reinterpret_cast<_factory*>(type_data)->get_properties2(reinterpret_cast<_instance*>(data));
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
				delete reinterpret_cast<_instance*>(data);
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		static std::uint32_t _get_width(void* data) noexcept
		try {
			if (data)
				return reinterpret_cast<_instance*>(data)->get_width();
			return 0;
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
			return 0;
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
			return 0;
		}

		static std::uint32_t _get_height(void* data) noexcept
		try {
			if (data)
				return reinterpret_cast<_instance*>(data)->get_height();
			return 0;
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
			return 0;
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
			return 0;
		}

		static void _activate(void* data) noexcept
		try {
			if (data)
				reinterpret_cast<_instance*>(data)->activate();
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		static void _deactivate(void* data) noexcept
		try {
			if (data)
				reinterpret_cast<_instance*>(data)->deactivate();
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		static void _show(void* data) noexcept
		try {
			if (data)
				reinterpret_cast<_instance*>(data)->show();
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		static void _hide(void* data) noexcept
		try {
			if (data)
				reinterpret_cast<_instance*>(data)->hide();
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		static void _video_tick(void* data, float seconds) noexcept
		try {
			if (data)
				reinterpret_cast<_instance*>(data)->video_tick(seconds);
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		static void _video_render(void* data, gs_effect_t* effect) noexcept
		try {
			if (data)
				reinterpret_cast<_instance*>(data)->video_render(effect);
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		static struct obs_source_frame* _filter_video(void* data, struct obs_source_frame* frame) noexcept
		try {
			if (data)
				return reinterpret_cast<_instance*>(data)->filter_video(frame);
			return frame;
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
			return frame;
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
			return frame;
		}

		static struct obs_audio_data* _filter_audio(void* data, struct obs_audio_data* frame) noexcept
		try {
			if (data)
				return reinterpret_cast<_instance*>(data)->filter_audio(frame);
			return frame;
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
			return frame;
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
			return frame;
		}

		static void _enum_active_sources(void* data, obs_source_enum_proc_t enum_callback, void* param) noexcept
		try {
			if (data)
				reinterpret_cast<_instance*>(data)->enum_active_sources(enum_callback, param);
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		static void _load(void* data, obs_data_t* settings) noexcept
		try {
			auto priv = reinterpret_cast<_instance*>(data);
			if (priv) {
				std::uint64_t version = static_cast<std::uint64_t>(obs_data_get_int(settings, S_VERSION));
				priv->migrate(settings, version);
				obs_data_set_int(settings, S_VERSION, static_cast<std::int64_t>(STREAMFX_VERSION));
				obs_data_set_string(settings, S_COMMIT, STREAMFX_COMMIT);
				priv->load(settings);
			}
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		static void _update(void* data, obs_data_t* settings) noexcept
		try {
			if (data)
				reinterpret_cast<_instance*>(data)->update(settings);
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		static void _save(void* data, obs_data_t* settings) noexcept
		try {
			if (data) {
				reinterpret_cast<_instance*>(data)->save(settings);
				obs_data_set_int(settings, S_VERSION, static_cast<std::int64_t>(STREAMFX_VERSION));
				obs_data_set_string(settings, S_COMMIT, STREAMFX_COMMIT);
			}
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		static void _mouse_click(void* data, const struct obs_mouse_event* event, int32_t type, bool mouse_up,
								 std::uint32_t click_count) noexcept
		try {
			if (data)
				reinterpret_cast<_instance*>(data)->mouse_click(event, type, mouse_up, click_count);
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		static void _mouse_move(void* data, const struct obs_mouse_event* event, bool mouse_leave) noexcept
		try {
			if (data)
				reinterpret_cast<_instance*>(data)->mouse_move(event, mouse_leave);
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		static void _mouse_wheel(void* data, const struct obs_mouse_event* event, int x_delta, int y_delta) noexcept
		try {
			if (data)
				reinterpret_cast<_instance*>(data)->mouse_wheel(event, x_delta, y_delta);
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		static void _focus(void* data, bool focus) noexcept
		try {
			if (data)
				reinterpret_cast<_instance*>(data)->focus(focus);
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		static void _key_click(void* data, const struct obs_key_event* event, bool key_up) noexcept
		try {
			if (data)
				reinterpret_cast<_instance*>(data)->key_click(event, key_up);
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		static void _filter_remove(void* data, obs_source_t* source) noexcept
		try {
			if (data)
				reinterpret_cast<_instance*>(data)->filter_remove(source);
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		static bool _audio_render(void* data, uint64_t* ts_out, struct obs_source_audio_mix* audio_output,
								  std::uint32_t mixers, std::size_t channels, std::size_t sample_rate) noexcept
		try {
			if (data)
				return reinterpret_cast<_instance*>(data)->audio_render(ts_out, audio_output, mixers, channels,
																		sample_rate);
			return false;
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
			return false;
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
			return false;
		}

		static void _enum_all_sources(void* data, obs_source_enum_proc_t enum_callback, void* param) noexcept
		try {
			if (data)
				reinterpret_cast<_instance*>(data)->enum_all_sources(enum_callback, param);
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		static void _transition_start(void* data) noexcept
		try {
			if (data)
				reinterpret_cast<_instance*>(data)->transition_start();
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		static void _transition_stop(void* data) noexcept
		try {
			if (data)
				reinterpret_cast<_instance*>(data)->transition_stop();
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
		}

		static bool _audio_mix(void* data, uint64_t* ts_out, struct audio_output_data* audio_output,
							   std::size_t channels, std::size_t sample_rate) noexcept
		try {
			if (data)
				return reinterpret_cast<_instance*>(data)->audio_mix(ts_out, audio_output, channels, sample_rate);
			return false;
		} catch (const std::exception& ex) {
			LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
			return false;
		} catch (...) {
			LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
			return false;
		}

		public:
		virtual const char* get_name()
		{
			return "Not Yet Implemented";
		}

		virtual void* create(obs_data_t* settings, obs_source_t* source)
		{
			return reinterpret_cast<void*>(new _instance(settings, source));
		}

		virtual void get_defaults2(obs_data_t* data) {}

		virtual obs_properties_t* get_properties2(_instance* data)
		{
			return nullptr;
		}
	};

	class source_instance {
		protected:
		obs_source_t* _self;

		public:
		source_instance(obs_data_t* settings, obs_source_t* source) : _self(source) {}
		virtual ~source_instance(){};

		virtual std::uint32_t get_width()
		{
			return 0;
		}

		virtual std::uint32_t get_height()
		{
			return 0;
		}

		virtual void activate() {}

		virtual void deactivate() {}

		virtual void show() {}

		virtual void hide() {}

		virtual void video_tick(float_t seconds) {}

		virtual void video_render(gs_effect_t* effect) {}

		virtual struct obs_source_frame* filter_video(struct obs_source_frame* frame)
		{
			return frame;
		}

		virtual struct obs_audio_data* filter_audio(struct obs_audio_data* audio)
		{
			return audio;
		}

		virtual void enum_active_sources(obs_source_enum_proc_t enum_callback, void* param) {}

		virtual void load(obs_data_t* settings) {}

		virtual void migrate(obs_data_t* settings, std::uint64_t version) {}

		virtual void update(obs_data_t* settings) {}

		virtual void save(obs_data_t* settings) {}

		virtual void mouse_click(const struct obs_mouse_event* event, std::int32_t type, bool mouse_up,
								 std::uint32_t click_count)
		{}

		virtual void mouse_move(const struct obs_mouse_event* event, bool mouse_leave) {}

		virtual void mouse_wheel(const struct obs_mouse_event* event, std::int32_t x_delta, std::int32_t y_delta) {}

		virtual void focus(bool focus) {}

		virtual void key_click(const struct obs_key_event* event, bool key_up) {}

		virtual void filter_remove(obs_source_t* source) {}

		virtual bool audio_render(std::uint64_t* ts_out, struct obs_source_audio_mix* audio_output,
								  std::uint32_t mixers, std::size_t channels, std::size_t sample_rate)
		{
			return false;
		}

		virtual void enum_all_sources(obs_source_enum_proc_t enum_callback, void* param) {}

		virtual void transition_start() {}

		virtual void transition_stop() {}

		virtual bool audio_mix(std::uint64_t* ts_out, struct audio_output_data* audio_output, std::size_t channels,
							   std::size_t sample_rate)
		{
			return false;
		}
	};

} // namespace obs
