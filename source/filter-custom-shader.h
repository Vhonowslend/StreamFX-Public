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

namespace Filter {
	class CustomShader {
		public:
		CustomShader();
		~CustomShader();

		static const char *get_name(void *);
		static void get_defaults(obs_data_t *);
		static obs_properties_t *get_properties(void *);
		static bool modified_properties(obs_properties_t *,
			obs_property_t *, obs_data_t *);

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
		class Instance {
			public:
			Instance(obs_data_t*, obs_source_t*);
			~Instance();
			
			void update(obs_data_t*);
			uint32_t get_width();
			uint32_t get_height();
			void activate();
			void deactivate();
			void video_tick(float);
			void video_render(gs_effect_t*);
			void get_properties(obs_properties_t *);

			private:
			void CheckShaderFile(std::string file, float_t time);
			std::string GetShaderFile();
			void CheckTextures(float_t time);

			bool IsSpecialParameter(std::string name, GS::EffectParameter::Type type);
			
			private:
			obs_source_t * m_source;
			bool m_isActive = true;
			std::unique_ptr<GS::RenderTarget> m_renderTarget;

			float_t m_activeTime, m_renderTime;

			// Shader
			struct Effect {
				std::string path;
				time_t createTime, modifiedTime;
				size_t size;
				float_t lastCheck;
				std::unique_ptr<GS::Effect> effect;
			} m_effect;

			struct Parameter {
				std::string name;
				GS::EffectParameter::Type type;

				std::vector<std::string> uiNames;
				std::vector<std::string> uiDescriptions;

				struct {
					union {
						int32_t i[4];
						float_t f[4];
						bool b;
					};
					bool textureIsSource = false;
					struct Source {
						bool dirty = false;
						std::string name;
						obs_source_t* source = nullptr;
						std::shared_ptr<GS::RenderTarget> rendertarget;
					} source;
					struct File {
						bool dirty = false;
						std::string path;
						time_t createTime, modifiedTime;
						size_t fileSize;
						float_t lastCheck;
						std::shared_ptr<GS::Texture> texture;
					} file;
				} value;
			};
			std::list<Parameter> m_effectParameters;

			friend class CustomShader;
		};
	};
}
