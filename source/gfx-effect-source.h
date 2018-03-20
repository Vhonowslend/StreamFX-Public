// Modern effects for a modern Streamer
// Copyright (C) 2017 Michael Fabian Dirks
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

#pragma once
#include <string>
#include <libobs/obs.h>
#include <memory>
#include "gs-effect.h"
#include "gs-rendertarget.h"
#include "gs-texture.h"
#include "gfx-source-texture.h"
#include <vector>

// Data Defines
#define D_TYPE			"CustomShader.Type"
#define D_INPUT_TEXT		"CustomShader.Input.Text"
#define D_INPUT_FILE		"CustomShader.Input.File"

// Translation Defines
#define T_TYPE			"CustomShader.Type"
#define T_TYPE_TEXT		"CustomShader.Type.Text"
#define T_TYPE_FILE		"CustomShader.Type.File"
#define T_INPUT_TEXT		"CustomShader.Input.Text"
#define T_INPUT_FILE		"CustomShader.Input.File"

namespace gfx {
	class effect_source {
		obs_source_t* m_source;
		std::unique_ptr<gs::rendertarget> m_renderTarget;

		// Effect Information
		struct {
			std::unique_ptr<gs::effect> effect;
			std::string text;
			std::string path;
			struct {
				float_t time_updated;
				time_t time_create;
				time_t time_modified;
				size_t file_size;
				bool modified;
			} file_info;
		} effect;

		// Status
		float_t time_existing;
		float_t time_active;

		protected:
		static bool property_type_modified(void* priv, obs_properties_t* props, obs_property_t* prop, obs_data_t* sett);
		static bool property_input_modified(void* priv, obs_properties_t* props, obs_property_t* prop, obs_data_t* sett);
		
		public:
		effect_source(obs_data_t* data, obs_source_t* owner);
		~effect_source();

		void get_properties(obs_properties_t* properties);
		void get_defaults(obs_data_t* data);
		void update(obs_data_t* data);
		bool test_for_updates(const char* text, const char* path);
		void update_parameters(obs_data_t* data);

		void active();
		void deactivate();

		uint32_t get_width();
		uint32_t get_height();
		void video_tick(float time);
		void video_render(gs_effect_t* parent_effect);

		public:
		enum class InputTypes {
			Text,
			File
		};
	};
}
