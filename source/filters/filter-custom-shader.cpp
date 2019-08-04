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

#include "filter-custom-shader.hpp"
#include <fstream>
#include <tuple>
#include <vector>
#include "strings.hpp"

// OBS
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <graphics/graphics.h>
#include <graphics/matrix4.h>
#include <util/platform.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#define S "Filter.CustomShader"

/** Shader Definitions
 * - Only one technique, any number of passes.
 * - Technique must be named 'Draw'.
 * - Parameters are split by the last underscore (_) to determine if it is a special parameter or not.
 *
 * Predefined Parameters:
 * - ViewProj: The current view projection matrix (float4x4).
 * - ViewSize: The current rendered size (float2).
 * - ViewSizeI: The current rendered size as an int (int2).
 * - Time: Time passed during total rendering in seconds (float).
 * - TimeActive: Time since last activation (float).
 * - image: The source being filtered (texture2d).
 *
 * Texture Special Parameters:
 * - $(name)_Size: Texture Size (float2).
 * - $(name)_SizeI: Texture Size (int2).
 * - $(name)_Texel: Texture Texel Size (1/Texture Size) (float2).
**/

enum class ShaderType : int64_t { Text, File };

static filter::CustomShader* handler;
P_INITIALIZER(HandlerInit)
{
	initializer_functions.push_back([] { handler = new filter::CustomShader(); });
	finalizer_functions.push_back([] { delete handler; });
}

filter::CustomShader::CustomShader()
{
	return; // TEMP
	memset(&sourceInfo, 0, sizeof(obs_source_info));
	sourceInfo.id             = "obs-stream-effects-filter-custom-shader";
	sourceInfo.type           = OBS_SOURCE_TYPE_FILTER;
	sourceInfo.output_flags   = OBS_SOURCE_VIDEO;
	sourceInfo.get_name       = get_name;
	sourceInfo.get_defaults   = get_defaults;
	sourceInfo.get_properties = get_properties;

	sourceInfo.create       = create;
	sourceInfo.destroy      = destroy;
	sourceInfo.update       = update;
	sourceInfo.activate     = activate;
	sourceInfo.deactivate   = deactivate;
	sourceInfo.video_tick   = video_tick;
	sourceInfo.video_render = video_render;

	obs_register_source(&sourceInfo);
}

filter::CustomShader::~CustomShader() {}

const char* filter::CustomShader::get_name(void*)
{
	return D_TRANSLATE(S);
}

void filter::CustomShader::get_defaults(obs_data_t* data)
{
	gfx::effect_source::get_defaults(data);
}

obs_properties_t* filter::CustomShader::get_properties(void* ptr)
{
	obs_properties_t* pr = obs_properties_create_param(ptr, nullptr);
	reinterpret_cast<Instance*>(ptr)->get_properties(pr);
	return pr;
}

void* filter::CustomShader::create(obs_data_t* data, obs_source_t* src)
{
	return new Instance(data, src);
}

void filter::CustomShader::destroy(void* ptr)
{
	delete reinterpret_cast<Instance*>(ptr);
}

uint32_t filter::CustomShader::get_width(void* ptr)
{
	return reinterpret_cast<Instance*>(ptr)->get_width();
}

uint32_t filter::CustomShader::get_height(void* ptr)
{
	return reinterpret_cast<Instance*>(ptr)->get_height();
}

void filter::CustomShader::update(void* ptr, obs_data_t* data)
{
	reinterpret_cast<Instance*>(ptr)->update(data);
}

void filter::CustomShader::activate(void* ptr)
{
	reinterpret_cast<Instance*>(ptr)->activate();
}

void filter::CustomShader::deactivate(void* ptr)
{
	reinterpret_cast<Instance*>(ptr)->deactivate();
}

void filter::CustomShader::video_tick(void* ptr, float time)
{
	reinterpret_cast<Instance*>(ptr)->video_tick(time);
}

void filter::CustomShader::video_render(void* ptr, gs_effect_t* effect)
{
	reinterpret_cast<Instance*>(ptr)->video_render(effect);
}

filter::CustomShader::Instance::Instance(obs_data_t* data, obs_source_t* source) : gfx::effect_source(data, source)
{
	m_defaultShaderPath = "shaders/filter/example._effect";
	m_renderTarget      = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
	update(data);
}

filter::CustomShader::Instance::~Instance() {}

//void filter::CustomShader::Instance::update(obs_data_t *data) {
//	ShaderType shaderType = (ShaderType)obs_data_get_int(data, S_TYPE);
//	if (shaderType == ShaderType::Text) {
//		const char* shaderText = obs_data_get_string(data, S_CONTENT_TEXT);
//		try {
//			m_effect.effect = std::make_unique<gs::effect>(shaderText, "Text Shader");
//		} catch (std::runtime_error& ex) {
//			const char* filterName = obs_source_get_name(source);
//			P_LOG_ERROR("[%s] Shader loading failed with error(s): %s", filterName, ex.what());
//		}
//	} else if (shaderType == ShaderType::File) {
//		CheckShaderFile(obs_data_get_string(data, S_CONTENT_FILE), 0.0f);
//	}
//
//	m_effectParameters.clear();
//	if (m_effect.effect && m_effect.effect->count_parameters() > 0) {
//		for (auto p : m_effect.effect->get_parameters()) {
//			std::string p_name = p.get_name();
//			std::string p_desc = p_name;
//
//			if (IsSpecialParameter(p_name, p.get_type()))
//				continue;
//
//			Parameter prm;
//			prm.name = p_name;
//			prm.type = p.get_type();
//
//			switch (p.get_type()) {
//				case gs::effect_parameter::type::Boolean:
//				{
//					prm.uiNames.push_back(p_name);
//					prm.uiDescriptions.push_back(p_desc);
//					prm.value.b = obs_data_get_bool(data, p_name.c_str());
//					break;
//				}
//				case gs::effect_parameter::type::Float:
//				case gs::effect_parameter::type::Float2:
//				case gs::effect_parameter::type::Float3:
//				case gs::effect_parameter::type::Float4:
//				{
//					{
//						prm.uiNames.push_back(p_name + "0");
//						prm.uiDescriptions.push_back(p_desc + "[0]");
//						prm.value.f[0] = (float_t)obs_data_get_double(data, prm.uiNames.back().c_str());
//					}
//					if (p.get_type() >= gs::effect_parameter::type::Float2) {
//						prm.uiNames.push_back(p_name + "1");
//						prm.uiDescriptions.push_back(p_desc + "[1]");
//						prm.value.f[1] = (float_t)obs_data_get_double(data, prm.uiNames.back().c_str());
//					}
//					if (p.get_type() >= gs::effect_parameter::type::Float3) {
//						prm.uiNames.push_back(p_name + "2");
//						prm.uiDescriptions.push_back(p_desc + "[2]");
//						prm.value.f[2] = (float_t)obs_data_get_double(data, prm.uiNames.back().c_str());
//					}
//					if (p.get_type() >= gs::effect_parameter::type::Float4) {
//						prm.uiNames.push_back(p_name + "3");
//						prm.uiDescriptions.push_back(p_desc + "[3]");
//						prm.value.f[3] = (float_t)obs_data_get_double(data, prm.uiNames.back().c_str());
//					}
//					break;
//				}
//				case gs::effect_parameter::type::Integer:
//				case gs::effect_parameter::type::Integer2:
//				case gs::effect_parameter::type::Integer3:
//				case gs::effect_parameter::type::Integer4:
//				{
//					{
//						prm.uiNames.push_back(p_name + "0");
//						prm.uiDescriptions.push_back(p_desc + "[0]");
//						prm.value.i[0] = (int32_t)obs_data_get_int(data, prm.uiNames.back().c_str());
//					}
//					if (p.get_type() >= gs::effect_parameter::type::Integer2) {
//						prm.uiNames.push_back(p_name + "1");
//						prm.uiDescriptions.push_back(p_desc + "[1]");
//						prm.value.i[1] = (int32_t)obs_data_get_int(data, prm.uiNames.back().c_str());
//					}
//					if (p.get_type() >= gs::effect_parameter::type::Integer3) {
//						prm.uiNames.push_back(p_name + "2");
//						prm.uiDescriptions.push_back(p_desc + "[2]");
//						prm.value.i[2] = (int32_t)obs_data_get_int(data, prm.uiNames.back().c_str());
//					}
//					if (p.get_type() >= gs::effect_parameter::type::Integer4) {
//						prm.uiNames.push_back(p_name + "3");
//						prm.uiDescriptions.push_back(p_desc + "[3]");
//						prm.value.i[3] = (int32_t)obs_data_get_int(data, prm.uiNames.back().c_str());
//					}
//					break;
//				}
//				case gs::effect_parameter::type::Texture:
//				{
//					prm.uiNames.push_back(p_name + "_type");
//					prm.uiDescriptions.push_back(p_desc + " Type");
//					prm.value.textureIsSource = obs_data_get_int(data, prm.uiNames.back().c_str()) == 1;
//					prm.uiNames.push_back(p_name + "_source");
//					prm.uiDescriptions.push_back(p_desc);
//					prm.value.source.name = obs_data_get_string(data, prm.uiNames.back().c_str());
//					prm.uiNames.push_back(p_name + "_file");
//					prm.uiDescriptions.push_back(p_desc);
//					if (obs_data_has_user_value(data, prm.uiNames.back().c_str())) {
//						prm.value.file.path = obs_data_get_string(data, prm.uiNames.back().c_str());
//					} else {
//						prm.value.file.path = obs_module_file("white.png");
//					}
//					break;
//				}
//			}
//			m_effectParameters.emplace_back(prm);
//		}
//	}
//
//	for (Parameter& prm : m_effectParameters) {
//		switch (prm.type) {
//			case gs::effect_parameter::type::Boolean:
//				prm.value.b = obs_data_get_bool(data, prm.uiNames[0].c_str());
//				break;
//			case gs::effect_parameter::type::Float4:
//				prm.value.f[3] = (float_t)obs_data_get_double(data, prm.uiNames[3].c_str());
//			case gs::effect_parameter::type::Float3:
//				prm.value.f[2] = (float_t)obs_data_get_double(data, prm.uiNames[2].c_str());
//			case gs::effect_parameter::type::Float2:
//				prm.value.f[1] = (float_t)obs_data_get_double(data, prm.uiNames[1].c_str());
//			case gs::effect_parameter::type::Float:
//				prm.value.f[0] = (float_t)obs_data_get_double(data, prm.uiNames[0].c_str());
//				break;
//			case gs::effect_parameter::type::Integer4:
//				prm.value.i[3] = (int32_t)obs_data_get_int(data, prm.uiNames[3].c_str());
//			case gs::effect_parameter::type::Integer3:
//				prm.value.i[2] = (int32_t)obs_data_get_int(data, prm.uiNames[2].c_str());
//			case gs::effect_parameter::type::Integer2:
//				prm.value.i[1] = (int32_t)obs_data_get_int(data, prm.uiNames[1].c_str());
//			case gs::effect_parameter::type::Integer:
//				prm.value.i[0] = (int32_t)obs_data_get_int(data, prm.uiNames[0].c_str());
//				break;
//			case gs::effect_parameter::type::Texture:
//				prm.value.textureIsSource = obs_data_get_int(data, prm.uiNames[0].c_str()) == 1;
//				std::string nName = obs_data_get_string(data, prm.uiNames[1].c_str());
//				if (nName != prm.value.source.name) {
//					prm.value.source.name = nName;
//					prm.value.source.dirty = true;
//				}
//				std::string nPath = obs_data_get_string(data, prm.uiNames[2].c_str());
//				if (nPath != prm.value.file.path) {
//					prm.value.file.path = nPath;
//					prm.value.file.dirty = true;
//				}
//				break;
//		}
//	}
//}

uint32_t filter::CustomShader::Instance::get_width()
{
	return 0;
}

uint32_t filter::CustomShader::Instance::get_height()
{
	return 0;
}

bool filter::CustomShader::Instance::is_special_parameter(std::string name, gs::effect_parameter::type type)
{
	std::pair<std::string, gs::effect_parameter::type> reservedParameters[] = {
		{"ViewProj", gs::effect_parameter::type::Matrix},    {"ViewSize", gs::effect_parameter::type::Float2},
		{"ViewSizeI", gs::effect_parameter::type::Integer2}, {"Time", gs::effect_parameter::type::Float},
		{"TimeActive", gs::effect_parameter::type::Float},   {"Image", gs::effect_parameter::type::Texture},
	};
	std::pair<std::string, gs::effect_parameter::type> textureParameters[] = {
		{"Size", gs::effect_parameter::type::Float2},
		{"SizeI", gs::effect_parameter::type::Integer2},
		{"Texel", gs::effect_parameter::type::Float2}};

	for (auto& kv : reservedParameters) {
		if ((name == kv.first) && (type == kv.second))
			return true;
	}

	// Split by last underscore(_) (if there is one).
	size_t posUnderscore = name.find_last_of('_');
	if (posUnderscore != std::string::npos) {
		std::string firstPart, secondPart;
		firstPart  = name.substr(0, posUnderscore);
		secondPart = name.substr(posUnderscore + 1);

		try {
			gs::effect_parameter prm = m_shader.effect->get_parameter(firstPart);
			if (prm.get_type() == gs::effect_parameter::type::Texture) {
				for (auto& kv : textureParameters) {
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

bool filter::CustomShader::Instance::apply_special_parameters(uint32_t, uint32_t)
{
	std::unique_ptr<gs::texture> imageTexture;
	m_renderTarget->get_texture(imageTexture);

	if (m_shader.effect->has_parameter("Image", gs::effect_parameter::type::Texture)) {
		m_shader.effect->get_parameter("Image").set_texture(imageTexture->get_object());
	} else {
		return false;
	}
	if (m_shader.effect->has_parameter("Image_Size", gs::effect_parameter::type::Float2)) {
		m_shader.effect->get_parameter("Image_Size")
			.set_float2(float_t(imageTexture->get_width()), float_t(imageTexture->get_height()));
	}
	if (m_shader.effect->has_parameter("Image_SizeI" /*, gs::effect_parameter::type::Integer2*/)) {
		m_shader.effect->get_parameter("Image_SizeI")
			.set_int2(static_cast<int32_t>(imageTexture->get_width()),
					  static_cast<int32_t>(imageTexture->get_height()));
	}
	if (m_shader.effect->has_parameter("Image_Texel", gs::effect_parameter::type::Float2)) {
		m_shader.effect->get_parameter("Image_Texel")
			.set_float2(float_t(1.0 / imageTexture->get_width()), float_t(1.0 / imageTexture->get_height()));
	}

	return true;
}

bool filter::CustomShader::Instance::video_tick_impl(float_t)
{
	return true;
}

bool filter::CustomShader::Instance::video_render_impl(gs_effect_t* parent_effect, uint32_t viewW, uint32_t viewH)
{
	// Render original source to render target.
	{
		auto op = m_renderTarget->render(viewW, viewH);
		vec4 black;
		vec4_zero(&black);
		gs_ortho(0, (float_t)viewW, 0, (float_t)viewH, 0, 1);
		gs_clear(GS_CLEAR_COLOR, &black, 0, 0);
		if (obs_source_process_filter_begin(m_source, GS_RGBA, OBS_NO_DIRECT_RENDERING)) {
			obs_source_process_filter_end(
				m_source, parent_effect ? parent_effect : obs_get_base_effect(OBS_EFFECT_DEFAULT), viewW, viewH);
		}
	}
	gs_texture_t* sourceTexture = m_renderTarget->get_object();
	if (!sourceTexture) {
		return false;
	}

	if (!apply_special_parameters(viewW, viewH)) {
		return false;
	}

	return true;
}

//void filter::CustomShader::Instance::video_render(gs_effect_t *effect) {
//		for (Parameter& prm : m_effectParameters) {
//			gs::effect_parameter eprm = m_effect.effect->get_parameter(prm.name);
//			switch (prm.type) {
//				case gs::effect_parameter::type::Texture:
//					if (prm.value.textureIsSource) {
//						if (prm.value.source.rendertarget && prm.value.source.source) {
//							uint32_t w, h;
//							w = obs_source_get_width(prm.value.source.source);
//							h = obs_source_get_height(prm.value.source.source);
//							{
//								auto op = prm.value.source.rendertarget->render(w, h);
//								vec4 black; vec4_zero(&black);
//								gs_ortho(0, (float_t)w, 0, (float_t)h, 0, 1);
//								gs_clear(GS_CLEAR_COLOR, &black, 0, 0);
//								obs_source_video_render(prm.value.source.source);
//							}
//							eprm.set_texture(prm.value.source.rendertarget->get_object());
//						}
//					} else {
//						if (prm.value.file.texture) {
//							eprm.set_texture(prm.value.file.texture);
//						}
//					}
//					break;
//			}
//		}
//
//}

//void filter::CustomShader::Instance::CheckTextures(float_t time) {
//
//	for (Parameter& prm : m_effectParameters) {
//		if (prm.type != gs::effect_parameter::type::Texture)
//			continue;
//
//		if (prm.value.textureIsSource) {
//			// If the source field is empty, simply clear the source reference.
//			if (prm.value.source.name.empty()) {
//				if (prm.value.source.source)
//					obs_source_release(source);
//				prm.value.source.source = nullptr;
//				continue;
//			}
//
//			// Ensure that a render target exists.
//			if (!prm.value.source.rendertarget)
//				prm.value.source.rendertarget = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
//
//			// Finally check if the source property was modified or is empty.
//			if (prm.value.source.dirty || !prm.value.source.source) {
//				prm.value.source.dirty = false;
//				if (prm.value.source.source)
//					obs_source_release(prm.value.source.source);
//				prm.value.source.source = obs_get_source_by_name(prm.value.source.name.c_str());
//			}
//		} else {
//			bool doRefresh = false;
//
//			// If the path is empty, don't attempt to update any files and simply null the texture.
//			if (prm.value.file.path.empty()) {
//				prm.value.file.texture = nullptr;
//				continue;
//			}
//
//			// If the property was modified or the texture is empty, force a refresh.
//			if (prm.value.file.dirty || !prm.value.file.texture) {
//				doRefresh = true;
//			}
//
//			// Skip testing if the last test was less than 1/2 of a second away.
//			prm.value.file.lastCheck += time;
//			if (prm.value.file.lastCheck < 0.5f) {
//				if (!doRefresh)
//					continue;
//			} else {
//				prm.value.file.lastCheck = prm.value.file.lastCheck - 0.5f;
//			}
//
//			// Test if the file was modified.
//			struct stat stats;
//			if (os_stat(prm.value.file.path.c_str(), &stats) == 0) {
//				doRefresh = doRefresh ||
//					(prm.value.file.createTime != stats.st_ctime) ||
//					(prm.value.file.modifiedTime != stats.st_mtime) ||
//					(prm.value.file.fileSize != stats.st_size);
//				prm.value.file.createTime = stats.st_ctime;
//				prm.value.file.modifiedTime = stats.st_mtime;
//				prm.value.file.fileSize = stats.st_size;
//			}
//
//			if (doRefresh) {
//				prm.value.file.dirty = false;
//				try {
//					prm.value.file.texture = std::make_shared<gs::texture>(prm.value.file.path);
//				} catch (std::runtime_error& ex) {
//					const char* filterName = obs_source_get_name(source);
//					P_LOG_ERROR("[%s] Loading texture file '%s' failed with error(s): %s",
//						filterName, prm.value.file.path.c_str(), ex.what());
//				}
//			}
//		}
//	}
//}
