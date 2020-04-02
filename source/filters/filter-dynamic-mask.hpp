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
#include "obs/obs-source-factory.hpp"
#include "obs/obs-source-tracker.hpp"
#include "obs/obs-source.hpp"

namespace filter::dynamic_mask {
	enum class channel : int8_t { Invalid = -1, Red, Green, Blue, Alpha };

	class dynamic_mask_instance : public obs::source_instance {
		std::map<std::tuple<channel, channel, std::string>, std::string> _translation_map;

		gs::effect _effect;

		bool                              _have_filter_texture;
		std::shared_ptr<gs::rendertarget> _filter_rt;
		std::shared_ptr<gs::texture>      _filter_texture;

		bool                                    _have_input_texture;
		std::shared_ptr<obs::deprecated_source> _input;
		std::shared_ptr<gfx::source_texture>    _input_capture;
		std::shared_ptr<gs::texture>            _input_texture;

		bool                              _have_final_texture;
		std::shared_ptr<gs::rendertarget> _final_rt;
		std::shared_ptr<gs::texture>      _final_texture;

		struct channel_data {
			float_t value  = 0.0;
			float_t scale  = 1.0;
			vec4    values = {0};
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

		virtual void update(obs_data_t* settings) override;
		virtual void load(obs_data_t* settings) override;
		virtual void save(obs_data_t* settings) override;

		void input_renamed(obs::deprecated_source* src, std::string old_name, std::string new_name);

		static bool modified(void* self, obs_properties_t* properties, obs_property_t* property,
							 obs_data_t* settings) noexcept;

		void video_tick(float _time);
		void video_render(gs_effect_t* effect);
	};

	class dynamic_mask_factory : public obs::source_factory<filter::dynamic_mask::dynamic_mask_factory,
															filter::dynamic_mask::dynamic_mask_instance> {
		static std::shared_ptr<filter::dynamic_mask::dynamic_mask_factory> factory_instance;

		public: // Singleton
		static void initialize()
		{
			factory_instance = std::make_shared<filter::dynamic_mask::dynamic_mask_factory>();
		}

		static void finalize()
		{
			factory_instance.reset();
		}

		static std::shared_ptr<dynamic_mask_factory> get()
		{
			return factory_instance;
		}

		private:
		std::list<std::string> _translation_cache;

		public:
		dynamic_mask_factory();
		virtual ~dynamic_mask_factory() override;

		virtual const char* get_name() override;

		virtual void get_defaults2(obs_data_t* data) override;

		virtual obs_properties_t* get_properties2(filter::dynamic_mask::dynamic_mask_instance* data) override;

		std::string translate_string(const char* format, ...);
	};
} // namespace filter::dynamic_mask
