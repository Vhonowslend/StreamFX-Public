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

#include "filter-custom-shader.h"
#include "strings.h"
#include <vector>
#include <tuple>

extern "C" {
#pragma warning (push)
#pragma warning (disable: 4201)
#include "libobs/util/platform.h"
#include "libobs/graphics/graphics.h"
#include "libobs/graphics/matrix4.h"
#pragma warning (pop)
}

#define S			"Filter.CustomShader"
#define S_TYPE			"Filter.CustomShader.Type"
#define S_TYPE_TEXT		"Filter.CustomShader.Type.Text"
#define S_TYPE_FILE		"Filter.CustomShader.Type.File"
#define S_CONTENT_TEXT		"Filter.CustomShader.Content.Text"
#define S_CONTENT_FILE		"Filter.CustomShader.Content.File"

#define DEFAULT_SHADER ""

/* Special Parameters

Default:
- ViewSize - View Resolution, float2
- ViewSizeI - View Resolution, integer2
- Pass - Current Pass Index, integer

Texture2d:
- $(param)_size - Texture Size, float2
- $(param)_isize - Texture Size, integer2
- $(param)_texel - Texture Texel Size, float2

*/

enum class ShaderType : int64_t {
	Text,
	File
};

static Filter::CustomShader* handler;
INITIALIZER(HandlerInit) {
	initializerFunctions.push_back([] {
		handler = new Filter::CustomShader();
	});
	finalizerFunctions.push_back([] {
		delete handler;
	});
}

Filter::CustomShader::CustomShader() {
	memset(&sourceInfo, 0, sizeof(obs_source_info));
	sourceInfo.id = "obs-stream-effects-filter-custom-shader";
	sourceInfo.type = OBS_SOURCE_TYPE_FILTER;
	sourceInfo.output_flags = OBS_SOURCE_VIDEO;
	sourceInfo.get_name = get_name;
	sourceInfo.get_defaults = get_defaults;
	sourceInfo.get_properties = get_properties;

	sourceInfo.create = create;
	sourceInfo.destroy = destroy;
	sourceInfo.update = update;
	sourceInfo.activate = activate;
	sourceInfo.deactivate = deactivate;
	sourceInfo.video_tick = video_tick;
	sourceInfo.video_render = video_render;

	obs_register_source(&sourceInfo);
}

Filter::CustomShader::~CustomShader() {}

const char * Filter::CustomShader::get_name(void *) {
	return P_TRANSLATE(S);
}

void Filter::CustomShader::get_defaults(obs_data_t *data) {
	obs_data_set_default_int(data, S_TYPE, (int64_t)ShaderType::Text);
	obs_data_set_default_string(data, S_CONTENT_TEXT, DEFAULT_SHADER);
}

obs_properties_t * Filter::CustomShader::get_properties(void *ptr) {
	obs_properties_t *pr = obs_properties_create();
	obs_property_t *p;

	p = obs_properties_add_list(pr, S_TYPE, P_TRANSLATE(S_TYPE), obs_combo_type::OBS_COMBO_TYPE_LIST,
		obs_combo_format::OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, P_TRANSLATE(S_TYPE_TEXT), (int64_t)ShaderType::Text);
	obs_property_list_add_int(p, P_TRANSLATE(S_TYPE_FILE), (int64_t)ShaderType::File);
	obs_property_set_modified_callback(p, modified_properties);

	p = obs_properties_add_text(pr, S_CONTENT_TEXT, P_TRANSLATE(S_CONTENT_TEXT),
		obs_text_type::OBS_TEXT_MULTILINE);

	std::string defaultPath = "C:\\";
	if (ptr)
		defaultPath = reinterpret_cast<Instance*>(ptr)->get_shader_file();
	p = obs_properties_add_path(pr, S_CONTENT_FILE, P_TRANSLATE(S_CONTENT_FILE),
		obs_path_type::OBS_PATH_FILE, "OBS Studio Effect (*.effect);;Any File (*.*)", defaultPath.c_str());

	if (ptr)
		reinterpret_cast<Instance*>(ptr)->get_properties(pr);

	return pr;
}

bool Filter::CustomShader::modified_properties(obs_properties_t *pr, obs_property_t *p, obs_data_t *data) {
	ShaderType shaderType = (ShaderType)obs_data_get_int(data, S_TYPE);
	switch (shaderType) {
		case ShaderType::Text:
			obs_property_set_visible(obs_properties_get(pr, S_CONTENT_TEXT), true);
			obs_property_set_visible(obs_properties_get(pr, S_CONTENT_FILE), false);
			break;
		case ShaderType::File:
			obs_property_set_visible(obs_properties_get(pr, S_CONTENT_TEXT), false);
			obs_property_set_visible(obs_properties_get(pr, S_CONTENT_FILE), true);
			break;
	}

	return true;
}

void * Filter::CustomShader::create(obs_data_t *data, obs_source_t *src) {
	return new Instance(data, src);
}

void Filter::CustomShader::destroy(void *ptr) {
	delete reinterpret_cast<Instance*>(ptr);
}

uint32_t Filter::CustomShader::get_width(void *ptr) {
	return reinterpret_cast<Instance*>(ptr)->get_width();
}

uint32_t Filter::CustomShader::get_height(void *ptr) {
	return reinterpret_cast<Instance*>(ptr)->get_height();
}

void Filter::CustomShader::update(void *ptr, obs_data_t *data) {
	reinterpret_cast<Instance*>(ptr)->update(data);
}

void Filter::CustomShader::activate(void *ptr) {
	reinterpret_cast<Instance*>(ptr)->activate();
}

void Filter::CustomShader::deactivate(void *ptr) {
	reinterpret_cast<Instance*>(ptr)->activate();
}

void Filter::CustomShader::video_tick(void *ptr, float time) {
	reinterpret_cast<Instance*>(ptr)->video_tick(time);

}

void Filter::CustomShader::video_render(void *ptr, gs_effect_t *effect) {
	reinterpret_cast<Instance*>(ptr)->video_render(effect);
}

Filter::CustomShader::Instance::Instance(obs_data_t *data, obs_source_t *source) {
	m_sourceContext = source;

	update(data);
}

Filter::CustomShader::Instance::~Instance() {

}

void Filter::CustomShader::Instance::update(obs_data_t *data) {
	ShaderType shaderType = (ShaderType)obs_data_get_int(data, S_TYPE);
	if (shaderType == ShaderType::Text) {
		const char* shaderText = obs_data_get_string(data, S_CONTENT_TEXT);
		try {
			m_effect = GS::Effect(shaderText, "Text Shader");
		} catch (std::runtime_error& ex) {
			const char* filterName = obs_source_get_name(m_sourceContext);
			P_LOG_ERROR("[%s] Shader loading failed with error(s): %s", filterName, ex.what());
		}
	} else if (shaderType == ShaderType::File) {
		update_shader_file(obs_data_get_string(data, S_CONTENT_FILE));
	}
}

uint32_t Filter::CustomShader::Instance::get_width() {
	return 0;
}

uint32_t Filter::CustomShader::Instance::get_height() {
	return 0;
}

void Filter::CustomShader::Instance::activate() {
	m_isActive = true;
}

void Filter::CustomShader::Instance::deactivate() {
	m_isActive = false;
}

void Filter::CustomShader::Instance::video_tick(float time) {
	m_shaderFile.lastCheck += time;
	if (m_shaderFile.lastCheck >= 0.5) {
		update_shader_file(m_shaderFile.filePath);
		m_shaderFile.lastCheck = 0;
	}
}

void Filter::CustomShader::Instance::video_render(gs_effect_t *effect) {
	if (!m_sourceContext || !m_isActive) {
		obs_source_skip_video_filter(m_sourceContext);
		return;
	}

	obs_source_t *parent = obs_filter_get_parent(m_sourceContext);
	obs_source_t *target = obs_filter_get_target(m_sourceContext);
	if (!parent || !target) {
		obs_source_skip_video_filter(m_sourceContext);
		return;
	}

	uint32_t baseW = obs_source_get_base_width(target),
		baseH = obs_source_get_base_height(target);
	if (!baseW || !baseH) {
		obs_source_skip_video_filter(m_sourceContext);
		return;
	}

	// Temporary
	obs_source_skip_video_filter(m_sourceContext);
}

void Filter::CustomShader::Instance::get_properties(obs_properties_t *pr) {	
	if (m_effect.GetObject() == nullptr)
		return;

	m_effectParameters.clear();
	for (auto p : m_effect.GetParameters()) {
		std::string p_name = p.GetName();
		std::string p_desc = p_name;

		if (IsSpecialParameter(p_name, p.GetType()))
			continue;

		Parameter prm;
		prm.origName = p_name;
		prm.origType = p.GetType();

		switch (p.GetType()) {
			case GS::EffectParameter::Type::Boolean:
			{
				prm.names.push_back(p_name);
				prm.descs.push_back(p_desc);
				m_effectParameters.emplace_back(prm);
				obs_properties_add_bool(pr, m_effectParameters.back().names[0].c_str(), m_effectParameters.back().descs[0].c_str());
				break;
			}
			case GS::EffectParameter::Type::Float:
			case GS::EffectParameter::Type::Float2:
			case GS::EffectParameter::Type::Float3:
			case GS::EffectParameter::Type::Float4:
			{				
				prm.names.push_back(p_name + "0");
				prm.descs.push_back(p_desc + "[0]");
				if (p.GetType() >= GS::EffectParameter::Type::Float2) {
					prm.names.push_back(p_name + "1");
					prm.descs.push_back(p_desc + "[1]");
				}
				if (p.GetType() >= GS::EffectParameter::Type::Float3) {
					prm.names.push_back(p_name + "2");
					prm.descs.push_back(p_desc + "[2]");
				}
				if (p.GetType() >= GS::EffectParameter::Type::Float4) {
					prm.names.push_back(p_name + "3");
					prm.descs.push_back(p_desc + "[3]");
				}

				m_effectParameters.emplace_back(prm);

				obs_properties_add_float(pr, m_effectParameters.back().names[0].c_str(), m_effectParameters.back().descs[0].c_str(), FLT_MIN, FLT_MAX, FLT_EPSILON);
				if (p.GetType() >= GS::EffectParameter::Type::Float2)
					obs_properties_add_float(pr, m_effectParameters.back().names[1].c_str(), m_effectParameters.back().descs[1].c_str(), FLT_MIN, FLT_MAX, FLT_EPSILON);
				if (p.GetType() >= GS::EffectParameter::Type::Float3)
					obs_properties_add_float(pr, m_effectParameters.back().names[2].c_str(), m_effectParameters.back().descs[2].c_str(), FLT_MIN, FLT_MAX, FLT_EPSILON);
				if (p.GetType() >= GS::EffectParameter::Type::Float4)
					obs_properties_add_float(pr, m_effectParameters.back().names[3].c_str(), m_effectParameters.back().descs[3].c_str(), FLT_MIN, FLT_MAX, FLT_EPSILON);

				break;
			}
			case GS::EffectParameter::Type::Integer:
			case GS::EffectParameter::Type::Integer2:
			case GS::EffectParameter::Type::Integer3:
			case GS::EffectParameter::Type::Integer4:
			{
				prm.names.push_back(p_name + "0");
				prm.descs.push_back(p_desc + "[0]");
				if (p.GetType() >= GS::EffectParameter::Type::Integer2) {
					prm.names.push_back(p_name + "1");
					prm.descs.push_back(p_desc + "[1]");
				}
				if (p.GetType() >= GS::EffectParameter::Type::Integer3) {
					prm.names.push_back(p_name + "2");
					prm.descs.push_back(p_desc + "[2]");
				}
				if (p.GetType() >= GS::EffectParameter::Type::Integer4) {
					prm.names.push_back(p_name + "3");
					prm.descs.push_back(p_desc + "[3]");
				}

				m_effectParameters.emplace_back(prm);

				obs_properties_add_int(pr, m_effectParameters.back().names[0].c_str(), m_effectParameters.back().descs[0].c_str(), INT_MIN, INT_MAX, FLT_EPSILON);
				if (p.GetType() >= GS::EffectParameter::Type::Integer2)
					obs_properties_add_int(pr, m_effectParameters.back().names[1].c_str(), m_effectParameters.back().descs[1].c_str(), INT_MIN, INT_MAX, FLT_EPSILON);
				if (p.GetType() >= GS::EffectParameter::Type::Integer3)
					obs_properties_add_int(pr, m_effectParameters.back().names[2].c_str(), m_effectParameters.back().descs[2].c_str(), INT_MIN, INT_MAX, FLT_EPSILON);
				if (p.GetType() >= GS::EffectParameter::Type::Integer4)
					obs_properties_add_int(pr, m_effectParameters.back().names[3].c_str(), m_effectParameters.back().descs[3].c_str(), INT_MIN, INT_MAX, FLT_EPSILON);

				break;
			}
		}

	}
	return;
}

void Filter::CustomShader::Instance::update_shader_file(std::string file) {
	bool doRefresh = false;
	struct stat stats;

	if (file != m_shaderFile.filePath) {
		m_shaderFile.filePath = file;
		doRefresh = true;
	} else if (os_stat(m_shaderFile.filePath.c_str(), &stats) != 0) {
		doRefresh = doRefresh ||
			(m_shaderFile.createTime != stats.st_ctime) ||
			(m_shaderFile.modifiedTime != stats.st_mtime);
		m_shaderFile.createTime = stats.st_ctime;
		m_shaderFile.modifiedTime = stats.st_mtime;
	}

	if (doRefresh) {
		try {
			m_effect = GS::Effect(m_shaderFile.filePath);
		} catch (std::runtime_error& ex) {
			const char* filterName = obs_source_get_name(m_sourceContext);
			P_LOG_ERROR("[%s] Shader loading failed with error(s): %s", filterName, ex.what());
		}
	}
}

std::string Filter::CustomShader::Instance::get_shader_file() {
	return m_shaderFile.filePath;
}

bool Filter::CustomShader::Instance::IsSpecialParameter(std::string name, GS::EffectParameter::Type type) {
	if (type == GS::EffectParameter::Type::Matrix) {
		if (name == "ViewProj")
			return true;

	} else if (type == GS::EffectParameter::Type::Float2) {
		if (name == "ViewSize")
			return true;
		
		// Suffix Tests
		size_t posUnderscore = name.find_last_of('_');
		if (posUnderscore != std::string::npos) {
			std::string firstPart, secondPart;
			firstPart = name.substr(0, posUnderscore);
			secondPart = name.substr(posUnderscore + 1);

			// Texture Specials
			if ((secondPart == "size") || (secondPart == "texel")) {
				try {
					GS::EffectParameter texParam = m_effect.GetParameter(firstPart);
					if (texParam.GetType() == GS::EffectParameter::Type::Texture)
						return true;
				} catch (...) {}
			}
		}
	} else if (type == GS::EffectParameter::Type::Float) {
		if (name == "Time" || name == "TimeActive")
			return true;
	}

	return false;
}
