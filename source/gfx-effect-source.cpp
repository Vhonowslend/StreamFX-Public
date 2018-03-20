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

#include "gfx-effect-source.h"
#include "strings.h"
#include <libobs/util/platform.h>
#include <fstream>

bool gfx::ShaderSource::property_type_modified(void* priv, obs_properties_t* props, obs_property_t* prop, obs_data_t* sett) {
	switch ((InputTypes)obs_data_get_int(sett, D_TYPE)) {
		default:
		case InputTypes::Text:
			obs_property_set_visible(obs_properties_get(props, D_INPUT_TEXT), true);
			obs_property_set_visible(obs_properties_get(props, D_INPUT_FILE), false);
			break;
		case InputTypes::File:
			obs_property_set_visible(obs_properties_get(props, D_INPUT_TEXT), false);
			obs_property_set_visible(obs_properties_get(props, D_INPUT_FILE), true);
			break;
	}
	return true;
}

bool gfx::ShaderSource::property_input_modified(void* priv, obs_properties_t* props, obs_property_t* prop, obs_data_t* sett) {
	return true;
}

gfx::ShaderSource::ShaderSource(obs_data_t* data, obs_source_t* owner) {
	obs_source_addref(owner);
	m_source = owner;
	time_existing = 0;
	time_active = 0;

	update(data);
}

gfx::ShaderSource::~ShaderSource() {
	obs_source_release(m_source);
}

void gfx::ShaderSource::get_properties(obs_properties_t* properties) {
	obs_property_t* p = nullptr;

	p = obs_properties_add_list(properties, D_TYPE, P_TRANSLATE(T_TYPE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(T_TYPE)));
	obs_property_list_add_int(p, P_TRANSLATE(T_TYPE_TEXT), (long long)InputTypes::Text);
	obs_property_list_add_int(p, P_TRANSLATE(T_TYPE_FILE), (long long)InputTypes::File);
	obs_property_set_modified_callback2(p, property_type_modified, this);

	p = obs_properties_add_text(properties, D_INPUT_TEXT, P_TRANSLATE(T_INPUT_TEXT), OBS_TEXT_MULTILINE);
	obs_property_set_long_description(p, P_TRANSLATE(P_DESC(T_INPUT_TEXT)));
	obs_property_set_modified_callback2(p, property_input_modified, this);

	{
		char* tmp_path = obs_module_file("shaders/");
		p = obs_properties_add_path(properties, D_INPUT_FILE, P_TRANSLATE(T_INPUT_FILE), OBS_PATH_FILE,
			"Any (*.effect *.shader *.hlsl);;Effect (*.effect);;Shader (*.shader);;DirectX (*.hlsl)", tmp_path);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(T_INPUT_FILE)));
		obs_property_set_modified_callback2(p, property_input_modified, this);
		bfree(tmp_path);
	}

	// ToDo: Place updated properties here or somewhere else?
}

void gfx::ShaderSource::update(obs_data_t* data) {
	obs_data_addref(data);

	// Update Shader
	InputTypes input_type = (InputTypes)obs_data_get_int(data, D_TYPE);
	if (input_type == InputTypes::Text) {
		const char* text = obs_data_get_string(data, D_INPUT_TEXT);
		test_for_updates(text, nullptr);
	} else if (input_type == InputTypes::File) {
		const char* path = obs_data_get_string(data, D_INPUT_FILE);
		test_for_updates(nullptr, path);
	}

	obs_data_release(data);
}

bool gfx::ShaderSource::test_for_updates(const char* text, const char* path) {
	bool is_shader_different = false;
	if (text != nullptr) {
		if (text != effect.text) {
			effect.text = text;
			is_shader_different = true;
		}

		if (is_shader_different) {
			effect.effect = std::make_unique<GS::Effect>(effect.text, "Text");
		}
	} else if (path != nullptr) {
		if (path != this->effect.path) {
			this->effect.path = path;
			this->effect.file_info.time_updated = 0;
			this->effect.file_info.time_create = 0;
			this->effect.file_info.time_modified = 0;
			this->effect.file_info.file_size = 0;
			is_shader_different = true;
		}

		// If the update timer is 0 or less, grab new file information.
		if (effect.file_info.time_updated <= 0) {
			struct stat stats;
			if (os_stat(effect.path.c_str(), &stats) == 0) {
				effect.file_info.modified = (effect.file_info.time_create != stats.st_ctime)
					| (effect.file_info.time_modified != stats.st_mtime)
					| (effect.file_info.file_size != stats.st_size);

				// Mark shader as different if the file was changed.
				is_shader_different =
					is_shader_different
					| effect.file_info.modified;

				// Update own information
				effect.file_info.time_create = stats.st_ctime;
				effect.file_info.time_modified = stats.st_mtime;
				effect.file_info.file_size = stats.st_size;
			}

			// Increment timer so that the next check is a reasonable timespan away.
			effect.file_info.time_updated += 0.1;
		}

		if (is_shader_different || effect.file_info.modified) {
			// gs_effect_create_from_file caches results, which is bad for us.
			std::vector<char> content;
			std::ifstream fs(effect.path.c_str(), std::ios::binary);

			if (fs.good()) {
				size_t beg = fs.tellg();
				fs.seekg(0, std::ios::end);
				size_t sz = size_t(fs.tellg()) - beg;
				content.resize(sz + 1);
				fs.seekg(0, std::ios::beg);
				fs.read(content.data(), sz);
				fs.close();
				content[sz] = '\0';

				effect.effect = std::make_unique<GS::Effect>(content, effect.path);
			}
		}
	}

	// If the shader is different, rebuild the parameter list.
	if (is_shader_different) {


	}

	return is_shader_different;
}

void gfx::ShaderSource::active() {
	time_active = 0;
}

void gfx::ShaderSource::deactivate() {
	time_active = 0;
}

uint32_t gfx::ShaderSource::get_width() {
	return 0;
}

uint32_t gfx::ShaderSource::get_height() {
	return 0;
}

void gfx::ShaderSource::video_tick(float time) {
	// Shader Timers
	time_existing += time;
	time_active += time;

	// File Timer
	effect.file_info.time_updated -= time;
}

void gfx::ShaderSource::video_render(gs_effect_t* parent_effect) {

}
