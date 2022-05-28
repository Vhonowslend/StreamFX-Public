/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2017 Michael Fabian Dirks
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
#include <list>
#include <map>
#include "gfx/gfx-source-texture.hpp"
#include "obs/gs/gs-effect.hpp"
#include "obs/obs-source-active-reference.hpp"
#include "obs/obs-source-factory.hpp"
#include "obs/obs-source-showing-reference.hpp"
#include "obs/obs-source-tracker.hpp"
#include "obs/obs-source.hpp"
#include "obs/obs-tools.hpp"

namespace streamfx::filter::dynamic_mask {
	enum class channel : int8_t { Invalid = -1, Red, Green, Blue, Alpha };

	class dynamic_mask_instance : public obs::source_instance {
		std::map<std::tuple<channel, channel, std::string>, std::string> _translation_map;

		streamfx::obs::gs::effect _effect;

		bool                                             _have_filter_texture;
		std::shared_ptr<streamfx::obs::gs::rendertarget> _filter_rt;
		std::shared_ptr<streamfx::obs::gs::texture>      _filter_texture;

		bool                                                       _have_input_texture;
		::streamfx::obs::weak_source                               _input;
		std::shared_ptr<streamfx::gfx::source_texture>             _input_capture;
		std::shared_ptr<streamfx::obs::gs::texture>                _input_texture;
		std::shared_ptr<::streamfx::obs::source_showing_reference> _input_vs;
		std::shared_ptr<::streamfx::obs::source_active_reference>  _input_ac;

		bool                                             _have_final_texture;
		std::shared_ptr<streamfx::obs::gs::rendertarget> _final_rt;
		std::shared_ptr<streamfx::obs::gs::texture>      _final_texture;

		struct channel_data {
			float_t value  = 0.0;
			float_t scale  = 1.0;
			vec4    values = {0, 0, 0, 0};
		};
		std::map<channel, channel_data> _channels;

		struct _precalc {
			vec4    base;
			vec4    scale;
			matrix4 matrix;
		} _precalc;

		public:
		dynamic_mask_instance(obs_data_t* data, obs_source_t* self);
		virtual ~dynamic_mask_instance();

		virtual void load(obs_data_t* settings) override;
		virtual void migrate(obs_data_t* data, uint64_t version) override;
		virtual void update(obs_data_t* settings) override;
		virtual void save(obs_data_t* settings) override;

		virtual void video_tick(float_t _time) override;
		virtual void video_render(gs_effect_t* effect) override;

		void enum_active_sources(obs_source_enum_proc_t enum_callback, void* param) override;
		void enum_all_sources(obs_source_enum_proc_t enum_callback, void* param) override;

		void show() override;
		void hide() override;
		void activate() override;
		void deactivate() override;

		bool acquire(std::string_view name);
		void release();
	};

	class dynamic_mask_factory : public obs::source_factory<filter::dynamic_mask::dynamic_mask_factory,
															filter::dynamic_mask::dynamic_mask_instance> {
		std::list<std::string> _translation_cache;

		public:
		dynamic_mask_factory();
		virtual ~dynamic_mask_factory() override;

		virtual const char* get_name() override;

		virtual void get_defaults2(obs_data_t* data) override;

		virtual obs_properties_t* get_properties2(filter::dynamic_mask::dynamic_mask_instance* data) override;

		std::string translate_string(const char* format, ...);

#ifdef ENABLE_FRONTEND
		static bool on_manual_open(obs_properties_t* props, obs_property_t* property, void* data);
#endif

		public: // Singleton
		static void initialize();

		static void finalize();

		static std::shared_ptr<dynamic_mask_factory> get();
	};
} // namespace streamfx::filter::dynamic_mask
