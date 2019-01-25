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
#include "gs-effect.hpp"
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

#define S_FILTER_DISPLACEMENT "Filter.Displacement"
#define S_FILTER_DISPLACEMENT_FILE "Filter.Displacement.File"
#define S_FILTER_DISPLACEMENT_FILE_TYPES "Filter.Displacement.File.Types"
#define S_FILTER_DISPLACEMENT_RATIO "Filter.Displacement.Ratio"
#define S_FILTER_DISPLACEMENT_SCALE "Filter.Displacement.Scale"

namespace filter {
	namespace displacement {
		class factory {
			friend class std::_Ptr_base<filter::displacement::factory>;

			obs_source_info sourceInfo;

			public: // Singleton
			static void                     initialize();
			static void                     finalize();
			static std::shared_ptr<factory> get();

			public:
			factory();
			~factory();

			static const char* get_name(void*);

			static void*             create(obs_data_t*, obs_source_t*);
			static void              destroy(void*);
			static uint32_t          get_width(void*);
			static uint32_t          get_height(void*);
			static void              get_defaults(obs_data_t*);
			static obs_properties_t* get_properties(void*);
			static void              update(void*, obs_data_t*);
			static void              activate(void*);
			static void              deactivate(void*);
			static void              show(void*);
			static void              hide(void*);
			static void              video_tick(void*, float);
			static void              video_render(void*, gs_effect_t*);
		};

		class instance {
			obs_source_t* m_self;
			bool          m_active;
			float_t       m_timer;

			// Rendering
			std::shared_ptr<gs::effect> m_effect;
			float_t                     m_distance;
			vec2                        m_displacement_scale;

			// Displacement Map
			std::string                  m_file_name;
			std::shared_ptr<gs::texture> m_file_texture;
			time_t                       m_file_create_time;
			time_t                       m_file_modified_time;
			size_t                       m_file_size;

			void validate_file_texture(std::string file);

			public:
			instance(obs_data_t*, obs_source_t*);
			~instance();

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
