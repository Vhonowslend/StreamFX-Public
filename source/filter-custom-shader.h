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
#include <list>
#include <vector>

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

			protected:
			void update_shader_file(std::string file);
			std::string get_shader_file();

			bool IsSpecialParameter(std::string name, GS::EffectParameter::Type type);
			void ApplySpecialParameter();

			private:
			obs_source_t *m_sourceContext;
			bool m_isActive = true;

			// Shader
			struct {
				std::string filePath;
				time_t createTime, modifiedTime;
				size_t size;
				float_t lastCheck;
			} m_shaderFile;
			GS::Effect m_effect;
			struct Parameter {
				std::vector<std::string> names;
				std::vector<std::string> descs;
				std::string origName;
				GS::EffectParameter::Type origType;

				// Texture Input
				bool texInputSource = false;
				struct {
					std::string path;
					time_t timeCreated, timeModified;
					size_t fileSize;
					float_t lastChecked;
				} textureFile;
				struct {
					obs_source_t* source;
					std::pair<uint32_t, uint32_t> resolution;
				} textureSource;

			};
			std::list<Parameter> m_effectParameters;


			friend class CustomShader;
		};
	};
}
