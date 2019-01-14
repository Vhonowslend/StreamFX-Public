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
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "gfx-source-texture.hpp"
#include "gs-effect.hpp"
#include "gs-mipmapper.hpp"
#include "gs-rendertarget.hpp"
#include "gs-texture.hpp"
#include "gs-vertexbuffer.hpp"

// OBS
#pragma warning(push)
#pragma warning(disable : 4201)
#include <obs.h>
#pragma warning(pop)

// Data Defines
#define D_TYPE "CustomShader.Type"
#define D_INPUT_TEXT "CustomShader.Input.Text"
#define D_INPUT_FILE "CustomShader.Input.File"

// Translation Defines
#define T_TYPE "CustomShader.Type"
#define T_TYPE_TEXT "CustomShader.Type.Text"
#define T_TYPE_FILE "CustomShader.Type.File"
#define T_INPUT_TEXT "CustomShader.Input.Text"
#define T_INPUT_FILE "CustomShader.Input.File"
#define T_TEXTURE_TYPE "CustomShader.Texture.Type"
#define T_TEXTURE_TYPE_FILE "CustomShader.Texture.Type.File"
#define T_TEXTURE_TYPE_SOURCE "CustomShader.Texture.Type.Source"

namespace gfx {
	class effect_source {
		public:
		struct parameter {
			std::string                           name = "";
			std::shared_ptr<gs::effect_parameter> param;

			struct {
				std::vector<char>        buffer;
				std::vector<const char*> names;
				std::vector<const char*> descs;
			} ui;
		};
		struct bool_parameter : parameter {
			bool value = false;
		};
		struct int_parameter : parameter {
			int32_t value[4] = {0, 0, 0, 0};
		};
		struct float_parameter : parameter {
			float_t value[4] = {0, 0, 0, 0};
		};
		struct texture_parameter : parameter {
			bool isSource = false;

			struct {
				std::string                  path = "";
				std::shared_ptr<gs::texture> tex;
				struct {
					float_t time_updated  = 0;
					time_t  time_create   = 0;
					time_t  time_modified = 0;
					size_t  file_size     = 0;
					bool    modified      = true;
				} info;
			} file;
			struct {
				std::string                          name = "";
				std::shared_ptr<gfx::source_texture> tex;
				std::shared_ptr<gs::texture>         final_tex;
			} source;

			struct {
				bool                              doResample = false;
				std::pair<uint32_t, uint32_t>     resolution = {10, 10};
				std::shared_ptr<gs::rendertarget> rt;
			} resample;
		};
		struct matrix_parameter : parameter {};
		typedef std::pair<std::string, gs::effect_parameter::type> paramident_t;

		private:
		protected:
		obs_source_t*                      m_source;
		std::shared_ptr<gs::vertex_buffer> m_quadBuffer;

		// Effect Information
		struct {
			std::shared_ptr<gs::effect> effect;
			std::string                 text;
			std::string                 path;
			struct {
				float_t time_updated;
				time_t  time_create;
				time_t  time_modified;
				size_t  file_size;
				bool    modified;
			} file_info;
		} m_shader;
		std::map<paramident_t, std::shared_ptr<parameter>> m_parameters;

		// Status
		float_t m_timeExisting;
		float_t m_timeActive;

		std::string m_defaultShaderPath = "shaders/";

		static bool property_type_modified(void* priv, obs_properties_t* props, obs_property_t* prop, obs_data_t* sett);
		static bool property_input_modified(void* priv, obs_properties_t* props, obs_property_t* prop,
											obs_data_t* sett);
		static void fill_source_list(obs_property_t* prop);
		static bool property_texture_type_modified(void* priv, obs_properties_t* props, obs_property_t* prop,
												   obs_data_t* sett);
		static bool property_texture_input_modified(void* priv, obs_properties_t* props, obs_property_t* prop,
													obs_data_t* sett);

		virtual bool is_special_parameter(std::string name, gs::effect_parameter::type type) = 0;

		virtual bool video_tick_impl(float_t time)                                                 = 0;
		virtual bool video_render_impl(gs_effect_t* parent_effect, uint32_t viewW, uint32_t viewH) = 0;

		public:
		effect_source(obs_data_t* data, obs_source_t* owner);
		~effect_source();

		void        get_properties(obs_properties_t* properties);
		static void get_defaults(obs_data_t* data);
		void        update(obs_data_t* data);
		bool        test_for_updates(const char* text, const char* path);
		void        update_parameters(obs_data_t* data);
		void        apply_parameters();

		void activate();
		void deactivate();

		std::string get_shader_file();

		uint32_t get_width();
		uint32_t get_height();
		void     video_tick(float time);
		void     video_render(gs_effect_t* parent_effect);

		public:
		enum class InputTypes { Text, File };
	};
} // namespace gfx
