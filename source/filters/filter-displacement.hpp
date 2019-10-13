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
#include <memory>
#include <string>
#include "obs/gs/gs-effect.hpp"
#include "plugin.hpp"

// OBS
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <obs-source.h>
#include <util/platform.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace filter {
	namespace displacement {
		class displacement_factory {
			obs_source_info _source_info;

			public: // Singleton
			static void                                  initialize();
			static void                                  finalize();
			static std::shared_ptr<displacement_factory> get();

			public:
			displacement_factory();
			~displacement_factory();
		};

		class displacement_instance {
			obs_source_t* _self;
			float_t       _timer;

			// Rendering
			std::shared_ptr<gs::effect> _effect;
			float_t                     _distance;
			vec2                        _displacement_scale;

			// Displacement Map
			std::string                  _file_name;
			std::shared_ptr<gs::texture> _file_texture;
			time_t                       _file_create_time;
			time_t                       _file_modified_time;
			size_t                       _file_size;

			void validate_file_texture(std::string file);

			public:
			displacement_instance(obs_data_t*, obs_source_t*);
			~displacement_instance();

			void     update(obs_data_t*);
			uint32_t get_width();
			uint32_t get_height();
			void     activate();
			void     deactivate();
			void     show();
			void     hide();
			void     video_tick(float);
			void     video_render(gs_effect_t*);

			std::string get_file();
		};
	} // namespace displacement
} // namespace filter
