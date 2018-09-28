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
#include <fstream>
#include <util/platform.h>
#include "strings.h"

bool gfx::effect_source::property_type_modified(void*, obs_properties_t* props, obs_property_t*, obs_data_t* sett)
{
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

bool gfx::effect_source::property_input_modified(void* obj, obs_properties_t*, obs_property_t*, obs_data_t* sett)
{
	const char* text = nullptr;
	const char* file = nullptr;

	switch ((InputTypes)obs_data_get_int(sett, D_TYPE)) {
	default:
	case InputTypes::Text:
		text = obs_data_get_string(sett, D_INPUT_TEXT);
		break;
	case InputTypes::File:
		file = obs_data_get_string(sett, D_INPUT_FILE);
		break;
	}

	return reinterpret_cast<gfx::effect_source*>(obj)->test_for_updates(text, file);
}

void gfx::effect_source::fill_source_list(obs_property_t* prop)
{
	obs_enum_sources(
		[](void* ptr, obs_source_t* src) {
			obs_property_t* prop  = (obs_property_t*)ptr;
			const char*     sname = obs_source_get_name(src);
			obs_property_list_add_string(prop, sname, sname);
			return true;
		},
		prop);
}

bool gfx::effect_source::property_texture_type_modified(void* priv, obs_properties_t* props, obs_property_t* prop,
														obs_data_t* sett)
{
	texture_parameter* tpm = reinterpret_cast<texture_parameter*>(priv);

	int64_t v = obs_data_get_int(sett, obs_property_name(prop));
	if (v == 0) { // File
		obs_property_set_visible(obs_properties_get(props, tpm->ui.names[1]), true);
		obs_property_set_visible(obs_properties_get(props, tpm->ui.names[2]), false);
	} else { // Source
		obs_property_set_visible(obs_properties_get(props, tpm->ui.names[1]), false);
		obs_property_set_visible(obs_properties_get(props, tpm->ui.names[2]), true);		
	}

	return true;
}

bool gfx::effect_source::property_texture_input_modified(void* priv, obs_properties_t* props, obs_property_t* prop,
														 obs_data_t* sett)
{
	return false;
}

gfx::effect_source::effect_source(obs_data_t* data, obs_source_t* owner)
{
	m_source       = owner;
	m_timeExisting = 0;
	m_timeActive   = 0;

	m_quadBuffer = std::make_shared<gs::vertex_buffer>(4);
	m_quadBuffer->set_uv_layers(1);
	auto vtx = m_quadBuffer->at(0);
	vec3_set(vtx.position, 0, 0, 0);
	vec4_set(vtx.uv[0], 0, 0, 0, 0);
	vtx = m_quadBuffer->at(2);
	vec3_set(vtx.position, 1, 0, 0);
	vec4_set(vtx.uv[0], 1, 0, 0, 0);
	vtx = m_quadBuffer->at(1);
	vec3_set(vtx.position, 0, 1, 0);
	vec4_set(vtx.uv[0], 0, 1, 0, 0);
	vtx = m_quadBuffer->at(3);
	vec3_set(vtx.position, 1, 1, 0);
	vec4_set(vtx.uv[0], 1, 1, 0, 0);
	m_quadBuffer->update(true);
}

gfx::effect_source::~effect_source()
{
	m_quadBuffer = nullptr;
}

void gfx::effect_source::get_properties(obs_properties_t* properties)
{
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
		char* tmp_path = obs_module_file(m_defaultShaderPath.c_str());
		p              = obs_properties_add_path(
            properties, D_INPUT_FILE, P_TRANSLATE(T_INPUT_FILE), OBS_PATH_FILE,
            "Any (*.effect *.shader *.hlsl);;Effect (*.effect);;Shader (*.shader);;DirectX (*.hlsl)", tmp_path);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(T_INPUT_FILE)));
		obs_property_set_modified_callback2(p, property_input_modified, this);
		bfree(tmp_path);
	}

	// ToDo: Place updated properties here or somewhere else?
	for (auto prm : m_parameters) {
		if (prm.first.second == gs::effect_parameter::type::Boolean) {
			obs_properties_add_bool(properties, prm.second->ui.names[0], prm.second->ui.descs[0]);
		} else if (prm.first.second >= gs::effect_parameter::type::Integer
				   && prm.first.second <= gs::effect_parameter::type::Integer4) {
			size_t cnt = (size_t)prm.first.second - (size_t)gs::effect_parameter::type::Integer;

			for (size_t idx = 0; idx <= cnt; idx++) {
				obs_properties_add_int(properties, prm.second->ui.names[idx], prm.second->ui.descs[idx], INT_MIN,
									   INT_MAX, 1);
			}
		} else if (prm.first.second >= gs::effect_parameter::type::Float
				   && prm.first.second <= gs::effect_parameter::type::Float4) {
			size_t cnt = (size_t)prm.first.second - (size_t)gs::effect_parameter::type::Float;

			for (size_t idx = 0; idx <= cnt; idx++) {
				obs_properties_add_float(properties, prm.second->ui.names[idx], prm.second->ui.descs[idx], -FLT_MAX,
										 FLT_MAX, 0.01);
			}
		} else if (prm.first.second == gs::effect_parameter::type::Texture) {
			// Switch between File and Source Input
			p = obs_properties_add_list(properties, prm.second->ui.names[0], prm.second->ui.descs[0],
										obs_combo_type::OBS_COMBO_TYPE_LIST, obs_combo_format::OBS_COMBO_FORMAT_INT);
			obs_property_set_long_description(p, P_TRANSLATE(P_DESC(T_TEXTURE_TYPE)));
			obs_property_set_modified_callback2(p, property_texture_type_modified, prm.second.get());
			obs_property_list_add_int(p, P_TRANSLATE(T_TEXTURE_TYPE_FILE), 0);
			obs_property_list_add_int(p, P_TRANSLATE(T_TEXTURE_TYPE_SOURCE), 1);

			// Texture Path
			char* defaultPath = obs_module_file("");
			p                 = obs_properties_add_path(properties, prm.second->ui.names[1], prm.second->ui.descs[1],
                                        obs_path_type::OBS_PATH_FILE, "Images (*.bmp *.jpeg *.jpg *.png *.tga *.tiff)",
                                        defaultPath);
			obs_property_set_modified_callback2(p, property_texture_input_modified, prm.second.get());
			bfree(defaultPath);

			// Source Name
			p = obs_properties_add_list(properties, prm.second->ui.names[2], prm.second->ui.descs[2],
										obs_combo_type::OBS_COMBO_TYPE_LIST, obs_combo_format::OBS_COMBO_FORMAT_STRING);
			obs_property_set_modified_callback2(p, property_texture_input_modified, prm.second.get());
			fill_source_list(p);
		}
	}
}

void gfx::effect_source::get_defaults(obs_data_t* data)
{
	obs_data_set_default_int(data, D_TYPE, (long long)InputTypes::Text);
	obs_data_set_default_string(data, D_INPUT_TEXT, "");
	obs_data_set_default_string(data, D_INPUT_FILE, "");
}

void gfx::effect_source::update(obs_data_t* data)
{
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

	update_parameters(data);

	obs_data_release(data);
}

bool gfx::effect_source::test_for_updates(const char* text, const char* path)
{
	bool is_shader_different = false;
	if (text != nullptr) {
		if (text != m_shader.text) {
			m_shader.text       = text;
			is_shader_different = true;
		}

		if (is_shader_different) {
			try {
				m_shader.effect = std::make_unique<gs::effect>(m_shader.text, "Text");
			} catch (...) {
				m_shader.effect = nullptr;
			}
		}
	} else if (path != nullptr) {
		if (path != this->m_shader.path) {
			this->m_shader.path                    = path;
			this->m_shader.file_info.time_updated  = 0;
			this->m_shader.file_info.time_create   = 0;
			this->m_shader.file_info.time_modified = 0;
			this->m_shader.file_info.file_size     = 0;
			is_shader_different                    = true;
		}

		// If the update timer is 0 or less, grab new file information.
		if (m_shader.file_info.time_updated <= 0) {
			struct stat stats;
			if (os_stat(m_shader.path.c_str(), &stats) == 0) {
				m_shader.file_info.modified = (m_shader.file_info.time_create != stats.st_ctime)
											  | (m_shader.file_info.time_modified != stats.st_mtime)
											  | (m_shader.file_info.file_size != stats.st_size);

				// Mark shader as different if the file was changed.
				is_shader_different = is_shader_different | m_shader.file_info.modified;

				// Update own information
				m_shader.file_info.time_create   = stats.st_ctime;
				m_shader.file_info.time_modified = stats.st_mtime;
				m_shader.file_info.file_size     = stats.st_size;
			}

			// Increment timer so that the next check is a reasonable timespan away.
			m_shader.file_info.time_updated += 0.1f;
		}

		if (is_shader_different || m_shader.file_info.modified) {
			// gs_effect_create_from_file caches results, which is bad for us.
			std::vector<char> content;
			std::ifstream     fs(m_shader.path.c_str(), std::ios::binary);

			if (fs.good()) {
				size_t beg = fs.tellg();
				fs.seekg(0, std::ios::end);
				size_t sz = size_t(fs.tellg()) - beg;
				content.resize(sz + 1);
				fs.seekg(0, std::ios::beg);
				fs.read(content.data(), sz);
				fs.close();
				content[sz] = '\0';

				try {
					m_shader.effect = std::make_unique<gs::effect>(std::string(content.data()), m_shader.path);
				} catch (...) {
					m_shader.effect = nullptr;
				}
			}
		}
	}

	// If the shader is different, rebuild the parameter list.
	if (is_shader_different) {
		if (m_shader.effect) {
			// ToDo: Figure out if a recycling approach would work.
			//  Might improve stability in low memory situations.
			std::map<paramident_t, std::shared_ptr<parameter>> new_params;
			auto                                               effect_param_list = m_shader.effect->get_parameters();
			for (auto effect_param : effect_param_list) {
				paramident_t ident;
				ident.first  = effect_param.get_name();
				ident.second = effect_param.get_type();

				if (is_special_parameter(ident.first, ident.second))
					continue;

				auto entry = m_parameters.find(ident);
				if (entry != m_parameters.end()) {
					entry->second->param = std::make_shared<gs::effect_parameter>(effect_param);
					new_params.insert_or_assign(ident, entry->second);
					m_parameters.erase(entry);
				} else {
					std::shared_ptr<parameter> param;

					if (ident.second == gs::effect_parameter::type::Boolean) {
						std::shared_ptr<bool_parameter> nparam = std::make_shared<bool_parameter>();

						std::string ui_name, ui_desc;
						ui_name = ident.first;
						ui_desc = ident.first;

						nparam->ui.buffer.resize(ui_name.size() + 1 + ui_desc.size() + 1);
						memset(nparam->ui.buffer.data(), 0, nparam->ui.buffer.size());
						memcpy(nparam->ui.buffer.data(), ui_name.c_str(), ui_name.size());
						memcpy(nparam->ui.buffer.data() + ui_name.size() + 1, ui_desc.c_str(), ui_desc.size());

						nparam->ui.names.resize(1);
						nparam->ui.names[0] = nparam->ui.buffer.data();

						nparam->ui.descs.resize(1);
						nparam->ui.descs[0] = nparam->ui.buffer.data() + ui_name.size() + 1;

						param = std::dynamic_pointer_cast<parameter>(nparam);
					} else if (ident.second >= gs::effect_parameter::type::Integer
							   && ident.second <= gs::effect_parameter::type::Integer4) {
						std::shared_ptr<int_parameter> nparam = std::make_shared<int_parameter>();

						size_t cnt = (size_t)ident.second - (size_t)gs::effect_parameter::type::Integer;

						std::string ui_name[4], ui_desc[4];
						size_t      bufsize = 0;
						if (cnt > 0) {
							for (size_t idx = 0; idx <= cnt; idx++) {
								ui_name[idx] = ident.first + (char)(48 + idx);
								ui_desc[idx] = ident.first + "[" + (char)(48 + idx) + "]";

								bufsize += ui_name[idx].size() + 1;
								bufsize += ui_desc[idx].size() + 1;
							}
						} else {
							ui_name[0] = ident.first;
							ui_desc[0] = ident.first;
							bufsize += ui_name[0].size() + 1;
							bufsize += ui_desc[0].size() + 1;
						}

						nparam->ui.names.resize(cnt + 1);
						nparam->ui.descs.resize(cnt + 1);

						nparam->ui.buffer.resize(bufsize);
						memset(nparam->ui.buffer.data(), 0, bufsize);
						size_t off = 0;
						for (size_t idx = 0; idx <= cnt; idx++) {
							memcpy(nparam->ui.buffer.data() + off, ui_name[idx].c_str(), ui_name[idx].size());
							nparam->ui.names[idx] = nparam->ui.buffer.data() + off;
							off += ui_name[idx].size() + 1;

							memcpy(nparam->ui.buffer.data() + off, ui_desc[idx].c_str(), ui_desc[idx].size());
							nparam->ui.descs[idx] = nparam->ui.buffer.data() + off;
							off += ui_desc[idx].size() + 1;
						}

						param = std::dynamic_pointer_cast<parameter>(nparam);
					} else if (ident.second >= gs::effect_parameter::type::Float
							   && ident.second <= gs::effect_parameter::type::Float4) {
						std::shared_ptr<float_parameter> nparam = std::make_shared<float_parameter>();

						size_t cnt = (size_t)ident.second - (size_t)gs::effect_parameter::type::Float;

						std::string ui_name[4], ui_desc[4];
						size_t      bufsize = 0;
						if (cnt > 0) {
							for (size_t idx = 0; idx <= cnt; idx++) {
								ui_name[idx] = ident.first + (char)(48 + idx);
								ui_desc[idx] = ident.first + "[" + (char)(48 + idx) + "]";

								bufsize += ui_name[idx].size() + 1;
								bufsize += ui_desc[idx].size() + 1;
							}
						} else {
							ui_name[0] = ident.first;
							ui_desc[0] = ident.first;
							bufsize += ui_name[0].size() + 1;
							bufsize += ui_desc[0].size() + 1;
						}

						nparam->ui.names.resize(cnt + 1);
						nparam->ui.descs.resize(cnt + 1);

						nparam->ui.buffer.resize(bufsize);
						memset(nparam->ui.buffer.data(), 0, bufsize);
						size_t off = 0;
						for (size_t idx = 0; idx <= cnt; idx++) {
							memcpy(nparam->ui.buffer.data() + off, ui_name[idx].c_str(), ui_name[idx].size());
							nparam->ui.names[idx] = nparam->ui.buffer.data() + off;
							off += ui_name[idx].size() + 1;

							memcpy(nparam->ui.buffer.data() + off, ui_desc[idx].c_str(), ui_desc[idx].size());
							nparam->ui.descs[idx] = nparam->ui.buffer.data() + off;
							off += ui_desc[idx].size() + 1;
						}

						param = std::dynamic_pointer_cast<parameter>(nparam);
					} else if (ident.second == gs::effect_parameter::type::Texture) {
						std::shared_ptr<texture_parameter> nparam = std::make_shared<texture_parameter>();

						std::string ui_name[3], ui_desc[3];
						size_t      bufsize = 0;

						ui_name[0] = ident.first + "_type";
						ui_desc[0] = ident.first + " Type";
						ui_name[1] = ident.first + "_file";
						ui_desc[1] = ident.first + " File";
						ui_desc[2] = ident.first + "_source";
						ui_desc[2] = ident.first + " Source";

						for (size_t i = 0; i <= 2; i++) {
							bufsize += ui_name[i].size() + ui_desc[i].size() + 2;
						}
						
						nparam->ui.names.resize(3);
						nparam->ui.descs.resize(3);

						nparam->ui.buffer.resize(bufsize);
						memset(nparam->ui.buffer.data(), 0, bufsize);
						size_t off = 0;
						for (size_t idx = 0; idx <= 2; idx++) {
							memcpy(nparam->ui.buffer.data() + off, ui_name[idx].c_str(), ui_name[idx].size());
							nparam->ui.names[idx] = nparam->ui.buffer.data() + off;
							off += ui_name[idx].size() + 1;

							memcpy(nparam->ui.buffer.data() + off, ui_desc[idx].c_str(), ui_desc[idx].size());
							nparam->ui.descs[idx] = nparam->ui.buffer.data() + off;
							off += ui_desc[idx].size() + 1;
						}

						param = std::dynamic_pointer_cast<parameter>(nparam);
					}

					if (param) {
						param->name  = ident.first;
						param->param = std::make_shared<gs::effect_parameter>(effect_param);
						new_params.insert_or_assign(ident, param);
					}
				}
			}
			m_parameters = std::move(new_params);
		} else {
			m_parameters.clear();
		}
	}

	return is_shader_different;
}

void gfx::effect_source::update_parameters(obs_data_t* data)
{
	for (auto prm : m_parameters) {
		if (prm.first.second == gs::effect_parameter::type::Boolean) {
			auto param   = std::static_pointer_cast<bool_parameter>(prm.second);
			param->value = obs_data_get_bool(data, prm.second->ui.names[0]);
		} else if (prm.first.second >= gs::effect_parameter::type::Integer
				   && prm.first.second <= gs::effect_parameter::type::Integer4) {
			auto param = std::static_pointer_cast<int_parameter>(prm.second);
			for (size_t idx = 0; idx < prm.second->ui.names.size(); idx++) {
				param->value[idx] = obs_data_get_int(data, prm.second->ui.names[idx]);
			}
		} else if (prm.first.second >= gs::effect_parameter::type::Float
				   && prm.first.second <= gs::effect_parameter::type::Float4) {
			auto param = std::static_pointer_cast<float_parameter>(prm.second);
			for (size_t idx = 0; idx < prm.second->ui.names.size(); idx++) {
				param->value[idx] = obs_data_get_double(data, prm.second->ui.names[idx]);
			}
		}
	}
}

void gfx::effect_source::apply_parameters()
{
	for (auto prm : m_parameters) {
		if (prm.first.second == gs::effect_parameter::type::Boolean) {
			auto param = std::static_pointer_cast<bool_parameter>(prm.second);
			param->param->set_bool(param->value);
		} else if (prm.first.second >= gs::effect_parameter::type::Integer
				   && prm.first.second <= gs::effect_parameter::type::Integer4) {
			auto param = std::static_pointer_cast<int_parameter>(prm.second);
			switch (prm.first.second) {
			case gs::effect_parameter::type::Integer:
				param->param->set_int(param->value[0]);
				break;
			case gs::effect_parameter::type::Integer2:
				param->param->set_int2(param->value[0], param->value[1]);
				break;
			case gs::effect_parameter::type::Integer3:
				param->param->set_int3(param->value[0], param->value[1], param->value[2]);
				break;
			case gs::effect_parameter::type::Integer4:
				param->param->set_int4(param->value[0], param->value[1], param->value[2], param->value[3]);
				break;
			}
		} else if (prm.first.second >= gs::effect_parameter::type::Float
				   && prm.first.second <= gs::effect_parameter::type::Float4) {
			auto param = std::static_pointer_cast<float_parameter>(prm.second);
			switch (prm.first.second) {
			case gs::effect_parameter::type::Float:
				param->param->set_float(param->value[0]);
				break;
			case gs::effect_parameter::type::Float2:
				param->param->set_float2(param->value[0], param->value[1]);
				break;
			case gs::effect_parameter::type::Float3:
				param->param->set_float3(param->value[0], param->value[1], param->value[2]);
				break;
			case gs::effect_parameter::type::Float4:
				param->param->set_float4(param->value[0], param->value[1], param->value[2], param->value[3]);
				break;
			}
		}
	}
}

void gfx::effect_source::activate()
{
	m_timeActive = 0;
}

void gfx::effect_source::deactivate()
{
	m_timeActive = 0;
}

std::string gfx::effect_source::get_shader_file()
{
	return m_shader.path;
}

uint32_t gfx::effect_source::get_width()
{
	return 0;
}

uint32_t gfx::effect_source::get_height()
{
	return 0;
}

void gfx::effect_source::video_tick(float time)
{
	// Shader Timers
	m_timeExisting += time;
	m_timeActive += time;

	// File Timer
	m_shader.file_info.time_updated -= time;

	video_tick_impl(time);
}

void gfx::effect_source::video_render(gs_effect_t* parent_effect)
{
	if (!m_source) {
		obs_source_skip_video_filter(m_source);
		return;
	}

	obs_source_t* parent = obs_filter_get_parent(m_source);
	obs_source_t* target = obs_filter_get_target(m_source);
	if (!parent || !target || !m_shader.effect) {
		obs_source_skip_video_filter(m_source);
		return;
	}

	uint32_t viewW = obs_source_get_base_width(target), viewH = obs_source_get_base_height(target);
	if (!viewW || !viewH) {
		obs_source_skip_video_filter(m_source);
		return;
	}

	apply_parameters();
	if (!video_render_impl(parent_effect, viewW, viewH)) {
		obs_source_skip_video_filter(m_source);
		return;
	}
	if (m_shader.effect->has_parameter("ViewSize", gs::effect_parameter::type::Float2)) {
		m_shader.effect->get_parameter("ViewSize").set_float2(float_t(viewW), float_t(viewH));
	}
	if (m_shader.effect->has_parameter("ViewSizeI" /*, gs::effect_parameter::type::Integer2*/)) {
		m_shader.effect->get_parameter("ViewSizeI").set_int2(int32_t(viewW), int32_t(viewH));
	}
	if (m_shader.effect->has_parameter("Time", gs::effect_parameter::type::Float)) {
		m_shader.effect->get_parameter("Time").set_float(m_timeExisting);
	}
	if (m_shader.effect->has_parameter("TimeActive", gs::effect_parameter::type::Float)) {
		m_shader.effect->get_parameter("TimeActive").set_float(m_timeActive);
	}

	gs_load_indexbuffer(nullptr);
	gs_load_vertexbuffer(m_quadBuffer->update());

	gs_reset_blend_state();
	gs_enable_depth_test(false);
	gs_matrix_push();
	gs_matrix_scale3f(viewW, viewH, 1);
	while (gs_effect_loop(m_shader.effect->get_object(), "Draw")) {
		gs_draw(gs_draw_mode::GS_TRISTRIP, 0, 4);
	}
	gs_matrix_pop();

	gs_load_indexbuffer(nullptr);
	gs_load_vertexbuffer(nullptr);
}
