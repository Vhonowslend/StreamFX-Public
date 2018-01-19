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
#include "gs-rendertarget.h"
#include <memory>

namespace Source {
	class MirrorAddon {
		public:
		MirrorAddon();
		~MirrorAddon();

		static const char *get_name(void *);
		static void get_defaults(obs_data_t *);
		static bool modified_properties(obs_properties_t *, obs_property_t *, obs_data_t *);
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
		obs_source_info osi;
	};

	class Mirror {
		public:
		Mirror(obs_data_t*, obs_source_t*);
		~Mirror();

		uint32_t get_width();
		uint32_t get_height();

		void update(obs_data_t*);
		void activate();
		void deactivate();
		void video_tick(float);
		void video_render(gs_effect_t*);

		private:
		bool m_active;
		obs_source_t* m_source;
		obs_source_t* m_target;
		std::string m_targetName;

		bool m_rescale = false;
		bool m_keepOriginalSize = false;
		uint32_t m_width, m_height;
		std::unique_ptr<GS::RenderTarget> m_renderTarget;
		std::unique_ptr<GS::RenderTarget> m_renderTargetScale;
		gs_effect_t* m_scalingEffect;
	};
};
