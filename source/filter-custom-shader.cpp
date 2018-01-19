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

/** Shader Definitions
 * - Only one technique, any number of passes.
 * - Technique must be named 'Draw'.
 * - Parameters are split by the last underscore (_) to determine if it is a special parameter or not.
 *
 * Predefined Parameters:
 * - ViewProj: The current view projection matrix (float4x4).
 * - ViewSize: The current rendered size (float2).
 * - ViewSizeI: The current rendered size as an int (int2).
 * - Pass: The current pass (int).
 * - Time: Time passed during total rendering in seconds (float).
 * - TimeActive: Time since last activation (float).
 * - image: The source being filtered (texture2d).
 *
 * Texture Special Parameters:
 * - $(name)_Size: Texture Size (float2).
 * - $(name)_SizeI: Texture Size (int2).
 * - $(name)_Texel: Texture Texel Size (1/Texture Size) (float2).
**/

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
	obs_properties_t *pr = obs_properties_create_param(ptr, nullptr);
	obs_property_t *p;

	p = obs_properties_add_list(pr, S_TYPE, P_TRANSLATE(S_TYPE), obs_combo_type::OBS_COMBO_TYPE_LIST,
		obs_combo_format::OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, P_TRANSLATE(S_TYPE_TEXT), (int64_t)ShaderType::Text);
	obs_property_list_add_int(p, P_TRANSLATE(S_TYPE_FILE), (int64_t)ShaderType::File);
	obs_property_set_modified_callback(p, modified_properties);

	p = obs_properties_add_text(pr, S_CONTENT_TEXT, P_TRANSLATE(S_CONTENT_TEXT),
		obs_text_type::OBS_TEXT_MULTILINE);

	std::string defaultPath = obs_module_file("effects/displace.effect");
	if (ptr)
		defaultPath = reinterpret_cast<Instance*>(ptr)->GetShaderFile();
	p = obs_properties_add_path(pr, S_CONTENT_FILE, P_TRANSLATE(S_CONTENT_FILE),
		obs_path_type::OBS_PATH_FILE, "OBS Studio Effect (*.effect);;Any File (*.*)", defaultPath.c_str());

	if (ptr)
		reinterpret_cast<Instance*>(ptr)->get_properties(pr);

	return pr;
}

bool Filter::CustomShader::modified_properties(obs_properties_t *pr, obs_property_t *, obs_data_t *data) {
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

	Instance* ptr = static_cast<Instance*>(obs_properties_get_param(pr));
	for (Instance::Parameter& prm : ptr->m_effectParameters) {
		switch (prm.type) {
			case GS::EffectParameter::Type::Texture:
				bool isSource = obs_data_get_int(data, prm.uiNames[0].c_str()) == 1;
				obs_property_set_visible(obs_properties_get(pr, prm.uiNames[1].c_str()), isSource);
				obs_property_set_visible(obs_properties_get(pr, prm.uiNames[2].c_str()), !isSource);
				break;
		}
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
	m_source = source;
	m_shaderFile.filePath = obs_module_file("effects/displace.effect");
	m_renderTarget = std::make_unique<GS::RenderTarget>(GS_RGBA, GS_ZS_NONE);
	update(data);
}

Filter::CustomShader::Instance::~Instance() {}

void Filter::CustomShader::Instance::update(obs_data_t *data) {
	ShaderType shaderType = (ShaderType)obs_data_get_int(data, S_TYPE);
	if (shaderType == ShaderType::Text) {
		const char* shaderText = obs_data_get_string(data, S_CONTENT_TEXT);
		try {
			m_effect = GS::Effect(shaderText, "Text Shader");
		} catch (std::runtime_error& ex) {
			const char* filterName = obs_source_get_name(m_source);
			P_LOG_ERROR("[%s] Shader loading failed with error(s): %s", filterName, ex.what());
		}
	} else if (shaderType == ShaderType::File) {
		CheckShaderFile(obs_data_get_string(data, S_CONTENT_FILE), 0.0f);
	}

	m_effectParameters.clear();
	if (m_effect.CountParameters() > 0) {
		for (auto p : m_effect.GetParameters()) {
			std::string p_name = p.GetName();
			std::string p_desc = p_name;

			if (IsSpecialParameter(p_name, p.GetType()))
				continue;

			Parameter prm;
			prm.name = p_name;
			prm.type = p.GetType();

			switch (p.GetType()) {
				case GS::EffectParameter::Type::Boolean:
				{
					prm.uiNames.push_back(p_name);
					prm.uiDescriptions.push_back(p_desc);
					prm.value.b = obs_data_get_bool(data, p_name.c_str());
					break;
				}
				case GS::EffectParameter::Type::Float:
				case GS::EffectParameter::Type::Float2:
				case GS::EffectParameter::Type::Float3:
				case GS::EffectParameter::Type::Float4:
				{
					{
						prm.uiNames.push_back(p_name + "0");
						prm.uiDescriptions.push_back(p_desc + "[0]");
						prm.value.f[0] = obs_data_get_double(data, prm.uiNames.back().c_str());
					}
					if (p.GetType() >= GS::EffectParameter::Type::Float2) {
						prm.uiNames.push_back(p_name + "1");
						prm.uiDescriptions.push_back(p_desc + "[1]");
						prm.value.f[1] = obs_data_get_double(data, prm.uiNames.back().c_str());
					}
					if (p.GetType() >= GS::EffectParameter::Type::Float3) {
						prm.uiNames.push_back(p_name + "2");
						prm.uiDescriptions.push_back(p_desc + "[2]");
						prm.value.f[2] = obs_data_get_double(data, prm.uiNames.back().c_str());
					}
					if (p.GetType() >= GS::EffectParameter::Type::Float4) {
						prm.uiNames.push_back(p_name + "3");
						prm.uiDescriptions.push_back(p_desc + "[3]");
						prm.value.f[3] = obs_data_get_double(data, prm.uiNames.back().c_str());
					}
					break;
				}
				case GS::EffectParameter::Type::Integer:
				case GS::EffectParameter::Type::Integer2:
				case GS::EffectParameter::Type::Integer3:
				case GS::EffectParameter::Type::Integer4:
				{
					{
						prm.uiNames.push_back(p_name + "0");
						prm.uiDescriptions.push_back(p_desc + "[0]");
						prm.value.i[0] = obs_data_get_int(data, prm.uiNames.back().c_str());
					}
					if (p.GetType() >= GS::EffectParameter::Type::Integer2) {
						prm.uiNames.push_back(p_name + "1");
						prm.uiDescriptions.push_back(p_desc + "[1]");
						prm.value.i[1] = obs_data_get_int(data, prm.uiNames.back().c_str());
					}
					if (p.GetType() >= GS::EffectParameter::Type::Integer3) {
						prm.uiNames.push_back(p_name + "2");
						prm.uiDescriptions.push_back(p_desc + "[2]");
						prm.value.i[2] = obs_data_get_int(data, prm.uiNames.back().c_str());
					}
					if (p.GetType() >= GS::EffectParameter::Type::Integer4) {
						prm.uiNames.push_back(p_name + "3");
						prm.uiDescriptions.push_back(p_desc + "[3]");
						prm.value.i[3] = obs_data_get_int(data, prm.uiNames.back().c_str());
					}
					break;
				}
				case GS::EffectParameter::Type::Texture:
				{
					prm.uiNames.push_back(p_name + "_type");
					prm.uiDescriptions.push_back(p_desc + " Type");
					prm.value.textureIsSource = obs_data_get_int(data, prm.uiNames.back().c_str()) == 1;
					prm.uiNames.push_back(p_name + "_source");
					prm.uiDescriptions.push_back(p_desc);
					prm.value.source.name = obs_data_get_string(data, prm.uiNames.back().c_str());
					prm.uiNames.push_back(p_name + "_file");
					prm.uiDescriptions.push_back(p_desc);
					if (obs_data_has_user_value(data, prm.uiNames.back().c_str())) {
						prm.value.file.path = obs_data_get_string(data, prm.uiNames.back().c_str());
					} else {
						prm.value.file.path = obs_module_file("white.png");
					}
					break;
				}
			}
			m_effectParameters.emplace_back(prm);
		}
	}

	for (Parameter& prm : m_effectParameters) {
		switch (prm.type) {
			case GS::EffectParameter::Type::Boolean:
				prm.value.b = obs_data_get_bool(data, prm.uiNames[0].c_str());
				break;
			case GS::EffectParameter::Type::Float4:
				prm.value.f[3] = (float_t)obs_data_get_double(data, prm.uiNames[3].c_str());
			case GS::EffectParameter::Type::Float3:
				prm.value.f[2] = (float_t)obs_data_get_double(data, prm.uiNames[2].c_str());
			case GS::EffectParameter::Type::Float2:
				prm.value.f[1] = (float_t)obs_data_get_double(data, prm.uiNames[1].c_str());
			case GS::EffectParameter::Type::Float:
				prm.value.f[0] = (float_t)obs_data_get_double(data, prm.uiNames[0].c_str());
				break;
			case GS::EffectParameter::Type::Integer4:
				prm.value.i[3] = (int32_t)obs_data_get_int(data, prm.uiNames[3].c_str());
			case GS::EffectParameter::Type::Integer3:
				prm.value.i[2] = (int32_t)obs_data_get_int(data, prm.uiNames[2].c_str());
			case GS::EffectParameter::Type::Integer2:
				prm.value.i[1] = (int32_t)obs_data_get_int(data, prm.uiNames[1].c_str());
			case GS::EffectParameter::Type::Integer:
				prm.value.i[0] = (int32_t)obs_data_get_int(data, prm.uiNames[0].c_str());
				break;
			case GS::EffectParameter::Type::Texture:
				prm.value.textureIsSource = obs_data_get_int(data, prm.uiNames[0].c_str()) == 1;
				std::string nName = obs_data_get_string(data, prm.uiNames[1].c_str());
				if (nName != prm.value.source.name) {
					prm.value.source.name = nName;
					prm.value.source.dirty = true;
				}
				std::string nPath = obs_data_get_string(data, prm.uiNames[2].c_str());
				if (nPath != prm.value.file.path) {
					prm.value.file.path = nPath;
					prm.value.file.dirty = true;
				}
				break;
		}
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
	CheckShaderFile(m_shaderFile.filePath, time);
	CheckTextures(time);
}

void Filter::CustomShader::Instance::video_render(gs_effect_t *effect) {
	if (!m_source || !m_isActive) {
		obs_source_skip_video_filter(m_source);
		return;
	}

	obs_source_t *parent = obs_filter_get_parent(m_source);
	obs_source_t *target = obs_filter_get_target(m_source);
	if (!parent || !target || !m_effect.GetObject()) {
		obs_source_skip_video_filter(m_source);
		return;
	}

	uint32_t baseW = obs_source_get_base_width(target),
		baseH = obs_source_get_base_height(target);
	if (!baseW || !baseH) {
		obs_source_skip_video_filter(m_source);
		return;
	}

	// Render original source to texture.
	{
		auto op = m_renderTarget->Render(baseW, baseH);
		vec4 black; vec4_zero(&black);
		gs_ortho(0, (float_t)baseW, 0, (float_t)baseH, 0, 1);
		gs_clear(GS_CLEAR_COLOR, &black, 0, 0);
		if (obs_source_process_filter_begin(m_source, GS_RGBA, OBS_NO_DIRECT_RENDERING)) {
			obs_source_process_filter_end(m_source,
				effect ? effect : obs_get_base_effect(OBS_EFFECT_DEFAULT), baseW, baseH);
		} else {
			obs_source_skip_video_filter(m_source);
			return;
		}
	}

	// Apply Parameters
	try {
		for (Parameter& prm : m_effectParameters) {
			GS::EffectParameter eprm = m_effect.GetParameter(prm.name);
			switch (prm.type) {
				case GS::EffectParameter::Type::Boolean:
					eprm.SetBoolean(prm.value.b);
					break;
				case GS::EffectParameter::Type::Integer:
					eprm.SetInteger(prm.value.i[0]);
					break;
				case GS::EffectParameter::Type::Integer2:
					eprm.SetInteger2(prm.value.i[0], prm.value.i[1]);
					break;
				case GS::EffectParameter::Type::Integer3:
					eprm.SetInteger3(prm.value.i[0], prm.value.i[1], prm.value.i[2]);
					break;
				case GS::EffectParameter::Type::Integer4:
					eprm.SetInteger4(prm.value.i[0], prm.value.i[1], prm.value.i[2], prm.value.i[3]);
					break;
				case GS::EffectParameter::Type::Float:
					eprm.SetFloat(prm.value.f[0]);
					break;
				case GS::EffectParameter::Type::Float2:
					eprm.SetFloat2(prm.value.f[0], prm.value.f[1]);
					break;
				case GS::EffectParameter::Type::Float3:
					eprm.SetFloat3(prm.value.f[0], prm.value.f[1], prm.value.f[2]);
					break;
				case GS::EffectParameter::Type::Float4:
					eprm.SetFloat4(prm.value.f[0], prm.value.f[1], prm.value.f[2], prm.value.f[3]);
					break;
				case GS::EffectParameter::Type::Texture:
					if (prm.value.textureIsSource) {
						if (prm.value.source.rendertarget) {
							uint32_t w, h;
							w = obs_source_get_width(prm.value.source.source);
							h = obs_source_get_height(prm.value.source.source);
							{
								auto op = prm.value.source.rendertarget->Render(w, h);
								vec4 black; vec4_zero(&black);
								gs_ortho(0, (float_t)w, 0, (float_t)h, 0, 1);
								gs_clear(GS_CLEAR_COLOR, &black, 0, 0);
								obs_source_video_render(prm.value.source.source);
							}
							eprm.SetTexture(prm.value.source.rendertarget->GetTextureObject());
						}
					} else {
						if (prm.value.file.texture) {
							eprm.SetTexture(prm.value.file.texture);
						}
					}
					break;
			}
		}

	} catch (...) {
		obs_source_skip_video_filter(m_source);
		return;
	}

	gs_reset_blend_state();
	gs_enable_depth_test(false);
	while (gs_effect_loop(obs_get_base_effect(OBS_EFFECT_DEFAULT), "Draw")) {
		obs_source_draw(m_renderTarget->GetTextureObject(), 0, 0, baseW, baseH, false);
	}
}

static bool UpdateSourceListCB(void *ptr, obs_source_t* src) {
	obs_property_t* p = (obs_property_t*)ptr;
	obs_property_list_add_string(p, obs_source_get_name(src), obs_source_get_name(src));
	return true;
}

static void UpdateSourceList(obs_property_t* p) {
	obs_property_list_clear(p);
	obs_enum_sources(UpdateSourceListCB, p);
}

void Filter::CustomShader::Instance::get_properties(obs_properties_t *pr) {
	if (m_effect.GetObject() == nullptr)
		return;

	for (Parameter& prm : m_effectParameters) {
		switch (prm.type) {
			case GS::EffectParameter::Type::Boolean:
				obs_properties_add_bool(pr, prm.uiNames[0].c_str(), prm.uiDescriptions[0].c_str());
				break;
			case GS::EffectParameter::Type::Float:
			case GS::EffectParameter::Type::Float2:
			case GS::EffectParameter::Type::Float3:
			case GS::EffectParameter::Type::Float4:
				obs_properties_add_float(pr, prm.uiNames[0].c_str(), prm.uiDescriptions[0].c_str(), FLT_MIN, FLT_MAX, FLT_EPSILON);
				if (prm.type >= GS::EffectParameter::Type::Float2)
					obs_properties_add_float(pr, prm.uiNames[1].c_str(), prm.uiDescriptions[1].c_str(), FLT_MIN, FLT_MAX, FLT_EPSILON);
				if (prm.type >= GS::EffectParameter::Type::Float3)
					obs_properties_add_float(pr, prm.uiNames[2].c_str(), prm.uiDescriptions[2].c_str(), FLT_MIN, FLT_MAX, FLT_EPSILON);
				if (prm.type >= GS::EffectParameter::Type::Float4)
					obs_properties_add_float(pr, prm.uiNames[3].c_str(), prm.uiDescriptions[3].c_str(), FLT_MIN, FLT_MAX, FLT_EPSILON);
				break;
			case GS::EffectParameter::Type::Integer:
			case GS::EffectParameter::Type::Integer2:
			case GS::EffectParameter::Type::Integer3:
			case GS::EffectParameter::Type::Integer4:
				obs_properties_add_int(pr, prm.uiNames[0].c_str(), prm.uiDescriptions[0].c_str(), INT_MIN, INT_MAX, 1);
				if (prm.type >= GS::EffectParameter::Type::Integer2)
					obs_properties_add_int(pr, prm.uiNames[1].c_str(), prm.uiDescriptions[1].c_str(), INT_MIN, INT_MAX, 1);
				if (prm.type >= GS::EffectParameter::Type::Integer3)
					obs_properties_add_int(pr, prm.uiNames[2].c_str(), prm.uiDescriptions[2].c_str(), INT_MIN, INT_MAX, 1);
				if (prm.type >= GS::EffectParameter::Type::Integer4)
					obs_properties_add_int(pr, prm.uiNames[3].c_str(), prm.uiDescriptions[3].c_str(), INT_MIN, INT_MAX, 1);
				break;
			case GS::EffectParameter::Type::Texture:
				obs_property * pt = obs_properties_add_list(pr,
					prm.uiNames[0].c_str(),
					prm.uiDescriptions[0].c_str(),
					OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
				obs_property_list_add_int(pt, "File", 0);
				obs_property_list_add_int(pt, "Source", 1);
				obs_property_set_modified_callback(pt, modified_properties);

				pt = obs_properties_add_list(pr,
					prm.uiNames[1].c_str(),
					prm.uiDescriptions[1].c_str(),
					OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
				UpdateSourceList(pt);

				obs_properties_add_path(pr,
					prm.uiNames[2].c_str(),
					prm.uiDescriptions[2].c_str(),
					OBS_PATH_FILE, "", prm.value.file.path.c_str());
				break;
		}
	}
	return;
}

void Filter::CustomShader::Instance::CheckShaderFile(std::string file, float_t time) {
	bool doRefresh = false;
	if (file != m_shaderFile.filePath) {
		m_shaderFile.filePath = file;
		doRefresh = true;
	}

	m_shaderFile.lastCheck += time;
	if (m_shaderFile.lastCheck < 0.5f && doRefresh == false)
		return;
	m_shaderFile.lastCheck = m_shaderFile.lastCheck - 0.5f;

	struct stat stats;
	if (os_stat(m_shaderFile.filePath.c_str(), &stats) != 0) {
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
			const char* filterName = obs_source_get_name(m_source);
			P_LOG_ERROR("[%s] Shader loading failed with error(s): %s", filterName, ex.what());
		}
	}
}

std::string Filter::CustomShader::Instance::GetShaderFile() {
	return m_shaderFile.filePath;
}

void Filter::CustomShader::Instance::CheckTextures(float_t time) {
	for (Parameter& prm : m_effectParameters) {
		if (prm.type != GS::EffectParameter::Type::Texture)
			continue;

		if (prm.value.textureIsSource) {
			if (!prm.value.source.rendertarget) {
				prm.value.source.rendertarget = std::make_shared<GS::RenderTarget>(GS_RGBA, GS_ZS_NONE);
			}
			if (prm.value.source.dirty || !prm.value.source.source) {
				if (prm.value.source.source)
					obs_source_release(prm.value.source.source);
				prm.value.source.source = obs_get_source_by_name(prm.value.source.name.c_str());
			}
		} else {
			bool doRefresh = false;
			if (prm.value.file.dirty || !prm.value.file.texture) {
				doRefresh = true;
			}

			prm.value.file.lastCheck += time;
			if (prm.value.file.lastCheck < 0.5f && doRefresh == false)
				continue;
			prm.value.file.lastCheck = prm.value.file.lastCheck - 0.5f;

			struct stat stats;
			if (os_stat(m_shaderFile.filePath.c_str(), &stats) != 0) {
				doRefresh = doRefresh ||
					(prm.value.file.createTime != stats.st_ctime) ||
					(prm.value.file.modifiedTime != stats.st_mtime) ||
					(prm.value.file.fileSize != stats.st_size);
				prm.value.file.createTime = stats.st_ctime;
				prm.value.file.modifiedTime = stats.st_mtime;
				prm.value.file.fileSize = stats.st_size;
			}

			if (doRefresh) {
				try {
					prm.value.file.texture = std::make_shared<GS::Texture>(prm.value.file.path);
				} catch (std::runtime_error& ex) {
					const char* filterName = obs_source_get_name(m_source);
					P_LOG_ERROR("[%s] Loading texture file '%s' failed with error(s): %s",
						filterName, prm.value.file.path.c_str(), ex.what());
				}
			}
		}
	}
}

bool Filter::CustomShader::Instance::IsSpecialParameter(std::string name, GS::EffectParameter::Type type) {
	std::pair<std::string, GS::EffectParameter::Type> reservedParameters[] = {
		{ "ViewProj", GS::EffectParameter::Type::Matrix },
		{ "ViewSize", GS::EffectParameter::Type::Matrix },
		{ "ViewSizeI", GS::EffectParameter::Type::Integer2 },
		{ "Pass", GS::EffectParameter::Type::Integer },
		{ "Time", GS::EffectParameter::Type::Float },
		{ "TimeActive", GS::EffectParameter::Type::Float },
		{ "image", GS::EffectParameter::Type::Texture }
	};
	std::pair<std::string, GS::EffectParameter::Type> textureParameters[] = {
		{ "Size", GS::EffectParameter::Type::Float2 },
		{ "SizeI", GS::EffectParameter::Type::Integer2 },
		{ "Texel", GS::EffectParameter::Type::Float2 }
	};

	for (auto& kv : reservedParameters) {
		if ((name == kv.first) && (type == kv.second))
			return true;
	}

	// Split by last underscore(_) (if there is one).
	size_t posUnderscore = name.find_last_of('_');
	if (posUnderscore != std::string::npos) {
		std::string firstPart, secondPart;
		firstPart = name.substr(0, posUnderscore);
		secondPart = name.substr(posUnderscore + 1);

		try {
			GS::EffectParameter prm = m_effect.GetParameter(firstPart);
			if (prm.GetType() == GS::EffectParameter::Type::Texture) {
				for (auto& kv : reservedParameters) {
					if ((secondPart == kv.first) && (type == kv.second))
						return true;
				}
			}
		} catch (...) {
			return false;
		}
	}

	return false;
}
