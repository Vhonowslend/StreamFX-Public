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
#include "gs-effect.h"
#include "gs-rendertarget.h"
#include <list>
#include <vector>
#include <inttypes.h>
#include "gfx-effect-source.h"

namespace filter {
	class CustomShader {
		public:
		CustomShader();
		~CustomShader();

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
		static void video_tick(void *, float);
		static void video_render(void *, gs_effect_t *);

		private:
		obs_source_info sourceInfo;

		private:
		class Instance : public gfx::effect_source {
			friend class CustomShader;

			std::shared_ptr<gs::rendertarget> m_renderTarget;

			protected:
			bool apply_special_parameters(uint32_t viewW, uint32_t viewH);
			virtual bool is_special_parameter(std::string name, gs::effect_parameter::type type) override;
			virtual bool video_tick_impl(float_t time) override;
			virtual bool video_render_impl(gs_effect_t* parent_effect, uint32_t viewW, uint32_t viewH) override;

			public:
			Instance(obs_data_t*, obs_source_t*);
			~Instance();
			
			uint32_t get_width();
			uint32_t get_height();					
		};
	};
}
