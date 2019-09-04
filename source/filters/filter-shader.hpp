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

#include "gfx/gfx-effect-source.hpp"
#include "obs/gs/gs-rendertarget.hpp"
#include "plugin.hpp"

extern "C" {
#include <obs.h>
}

namespace filter {
	namespace shader {
		class shader_factory {
			obs_source_info _source_info;

			public: // Singleton
			static void                            initialize();
			static void                            finalize();
			static std::shared_ptr<shader_factory> get();

			public:
			shader_factory();
			~shader_factory();
		};

		class shader_instance {
			obs_source_t* _self;
			bool          _active;

			uint32_t _width, _height;

			bool     _scale_locked;
			float_t  _wscale, _hscale;
			uint32_t _swidth, _sheight;

			std::shared_ptr<gs::rendertarget> _rt;
			bool                              _rt_updated;
			std::shared_ptr<gs::texture>      _rt_tex;

			std::shared_ptr<gs::rendertarget> _rt2;
			bool                              _rt2_updated;
			std::shared_ptr<gs::texture>      _rt2_tex;

			std::shared_ptr<gfx::effect_source::effect_source> _fx;

			public:
			shader_instance(obs_data_t* data, obs_source_t* self);
			~shader_instance();

			uint32_t width();
			uint32_t height();

			void properties(obs_properties_t* props);

			void load(obs_data_t* data);
			void update(obs_data_t* data);

			void activate();
			void deactivate();

			bool valid_param(std::shared_ptr<gs::effect_parameter> param);
			void override_param(std::shared_ptr<gs::effect> effect);

			void enum_active_sources(obs_source_enum_proc_t, void*);

			void video_tick(float_t sec_since_last);
			void video_render(gs_effect_t* effect);
		};
	} // namespace shader
} // namespace filter
