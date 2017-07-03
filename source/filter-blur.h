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
#include "plugin.h"
#include "gs-helper.h"

#define P_FILTER_BLUR					"Filter.Blur"
#define P_FILTER_BLUR_TYPE				"Filter.Blur.Type"
#define P_FILTER_BLUR_TYPE_BOX				"Filter.Blur.Type.Box"
#define P_FILTER_BLUR_TYPE_GAUSSIAN			"Filter.Blur.Type.Gaussian"
#define P_FILTER_BLUR_SIZE				"Filter.Blur.Size"

namespace Filter {
	class Blur {
		public:
		Blur();
		~Blur();

		static const char *get_name(void *);
		static void get_defaults(obs_data_t *);
		static obs_properties_t *get_properties(void *);

		static void *create(obs_data_t *, obs_source_t *);
		static void destroy(void *);
		static uint32_t get_width(void *);
		static uint32_t get_height(void *);
		static void update(void *, obs_data_t *);
		static void activate(void *);
		static void deactivate(void *);
		static void show(void *);
		static void hide(void *);
		static void video_tick(void *, float);
		static void video_render(void *, gs_effect_t *);

		enum Type : int64_t {
			Box,
			Gaussian,
		};

		private:
		obs_source_info sourceInfo;

		private:
		class Instance {
			public:
			Instance(obs_data_t*, obs_source_t*);
			~Instance();

			void update(obs_data_t*);
			uint32_t get_width();
			uint32_t get_height();
			void activate();
			void deactivate();
			void show();
			void hide();
			void video_tick(float);
			void video_render(gs_effect_t*);

			private:
			obs_source_t
				*m_source;
			gs_effect_t
				*m_effect;
			gs_technique_t
				*m_technique;
			gs_texrender_t 
				*m_primaryRT,
				*m_secondaryRT;
			int m_filterWidth;
		};
	};
}
