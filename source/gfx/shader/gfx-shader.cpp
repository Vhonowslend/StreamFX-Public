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

#include "gfx-shader.hpp"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include "plugin.hpp"

#define ST "Shader"
#define ST_SHADER ST ".Shader"
#define ST_SHADER_FILE ST_SHADER ".File"
#define ST_SHADER_TECHNIQUE ST_SHADER ".Technique"
#define ST_SHADER_SIZE ST_SHADER ".Size"
#define ST_SHADER_SIZE_WIDTH ST_SHADER_SIZE ".Width"
#define ST_SHADER_SIZE_HEIGHT ST_SHADER_SIZE ".Height"
#define ST_PARAMETERS ST ".Parameters"

gfx::shader::shader::shader(obs_source_t* self, shader_mode mode)
	: _self(self), _mode(mode), _shader(), _shader_file(), _time()
{
	_random.seed(static_cast<unsigned long long>(time(NULL)));
}

gfx::shader::shader::~shader() {}

void gfx::shader::shader::load_shader(std::filesystem::path file)
{
	_shader      = gs::effect(file);
	_shader_file = file;
}

void gfx::shader::shader::load_shader_params()
{
	_shader_params.clear();

	if (gs::effect_technique tech = _shader.get_technique(_shader_tech); tech != nullptr) {
		for (size_t idx = 0; idx < tech.count_passes(); idx++) {
			auto pass = tech.get_pass(idx);

			for (size_t vidx = 0; vidx < pass.count_vertex_parameters(); vidx++) {
				auto el = pass.get_vertex_parameter(vidx);

				auto fnd = _shader_params.find(el.get_name());
				if (fnd != _shader_params.end())
					continue;

				auto param = gfx::shader::parameter::make_parameter(el, ST_PARAMETERS);

				if (param)
					_shader_params.insert_or_assign(el.get_name(), param);
			}

			for (size_t vidx = 0; vidx < pass.count_pixel_parameters(); vidx++) {
				auto el = pass.get_pixel_parameter(vidx);

				auto fnd = _shader_params.find(el.get_name());
				if (fnd != _shader_params.end())
					continue;

				auto param = gfx::shader::parameter::make_parameter(el, ST_PARAMETERS);

				if (param)
					_shader_params.insert_or_assign(el.get_name(), param);
			}
		}
	}
}

void gfx::shader::shader::properties(obs_properties_t* pr)
{
	{
		auto grp = obs_properties_create();
		obs_properties_add_group(pr, ST_SHADER, D_TRANSLATE(ST_SHADER), OBS_GROUP_NORMAL, grp);

		{
			auto p = obs_properties_add_path(grp, ST_SHADER_FILE, D_TRANSLATE(ST_SHADER_FILE), OBS_PATH_FILE, "*.*",
											 nullptr);
			obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SHADER_FILE)));
			obs_property_set_modified_callback2(
				p,
				[](void* priv, obs_properties_t* props, obs_property_t* prop, obs_data_t* data) noexcept {
					return reinterpret_cast<gfx::shader::shader*>(priv)->on_shader_changed(props, prop, data);
				},
				this);
		}
		{
			auto p = obs_properties_add_list(grp, ST_SHADER_TECHNIQUE, D_TRANSLATE(ST_SHADER_TECHNIQUE),
											 OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
			obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SHADER_TECHNIQUE)));
			obs_property_set_modified_callback2(
				p,
				[](void* priv, obs_properties_t* props, obs_property_t* prop, obs_data_t* data) noexcept {
					return reinterpret_cast<gfx::shader::shader*>(priv)->on_technique_changed(props, prop, data);
				},
				this);
		}
		if (_mode != shader_mode::Transition) {
			auto grp2 = obs_properties_create();
			obs_properties_add_group(grp, ST_SHADER_SIZE, D_TRANSLATE(ST_SHADER_SIZE), OBS_GROUP_NORMAL, grp2);

			{
				auto p = obs_properties_add_text(grp2, ST_SHADER_SIZE_WIDTH, D_TRANSLATE(ST_SHADER_SIZE_WIDTH),
												 OBS_TEXT_DEFAULT);
				obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SHADER_SIZE_WIDTH)));
			}
			{
				auto p = obs_properties_add_text(grp2, ST_SHADER_SIZE_HEIGHT, D_TRANSLATE(ST_SHADER_SIZE_HEIGHT),
												 OBS_TEXT_DEFAULT);
				obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SHADER_SIZE_HEIGHT)));
			}
		}
	}
	{
		auto grp = obs_properties_create();
		obs_properties_add_group(pr, ST_PARAMETERS, D_TRANSLATE(ST_PARAMETERS), OBS_GROUP_NORMAL, grp);
	}
}

bool gfx::shader::shader::on_shader_changed(obs_properties_t* props, obs_property_t* prop, obs_data_t* data)
{
	// Load changed shader.
	update_shader(data);
	
	// Clear list of techniques.
	obs_property_t* list = obs_properties_get(props, ST_SHADER_TECHNIQUE);
	obs_property_list_clear(list);

	// Don't go further if there is no shader.
	if (!_shader)
		return true;

	// Rebuild Technique list.
	{
		const char* tech_name_c = obs_data_get_string(data, ST_SHADER_TECHNIQUE);
		std::string tech_name   = tech_name_c ? tech_name_c : "";
		bool        have_tech   = false;
		for (size_t idx = 0, idx_end = _shader.count_techniques(); idx < idx_end; idx++) {
			auto tech = _shader.get_technique(idx);
			obs_property_list_add_string(list, tech.name().c_str(), tech.name().c_str());
			if (tech.name() == tech_name) {
				have_tech = true;
			}
		}
		if (!have_tech && (_shader.count_techniques() > 0)) {
			obs_data_set_string(data, ST_SHADER_TECHNIQUE, _shader.get_technique(0).name().c_str());
			//on_technique_changed(props, prop, data);
		} else {
			obs_data_set_string(data, ST_SHADER_TECHNIQUE, "");
		}
	}

	return true;
}

bool gfx::shader::shader::on_technique_changed(obs_properties_t* props, obs_property_t* prop, obs_data_t* data)
{
	// Clear parameter options.
	auto grp = obs_property_group_content(obs_properties_get(props, ST_PARAMETERS));
	if (auto p = obs_properties_first(grp); p != nullptr) {
		do {
			obs_properties_remove_by_name(grp, obs_property_name(p));
			p = obs_properties_first(grp);
		} while (p != nullptr);
	}

	// Don't go further if there is no shader.
	if (!_shader)
		return true;

	// Load technique.
	update_technique(data);
	
	// Rebuild new parameters.
	for (auto kv : _shader_params) {
		kv.second->properties(grp, data);
	}

	return true;
}

void gfx::shader::shader::update_shader(obs_data_t* data)
{
	{
		const char* file_c = obs_data_get_string(data, ST_SHADER_FILE);
		std::string file   = file_c ? file_c : "";
		if (file != "") {
			try {
				load_shader(file);
			} catch (const std::exception& ex) {
				P_LOG_ERROR("Failed to load shader: %s.", ex.what());
				_shader.reset();
			} catch (...) {
				P_LOG_ERROR("Failed to load shader.");
				_shader.reset();
			}
		} else {
			_shader.reset();
		}
	}
}

void gfx::shader::shader::update_technique(obs_data_t* data)
{
	{
		const char* shader_tech_c = obs_data_get_string(data, ST_SHADER_TECHNIQUE);
		_shader_tech              = shader_tech_c ? shader_tech_c : "";
		load_shader_params();
	}
}

inline std::pair<gfx::shader::size_type, double_t> parse_text_as_size(const char* text)
{
	double_t v = 0;
	if (sscanf(text, "%lf", &v) == 1) {
		const char* prc_chr = strrchr(text, '%');
		if (prc_chr && (*prc_chr == '%')) {
			return {gfx::shader::size_type::Percent, v / 100.0};
		} else {
			return {gfx::shader::size_type::Pixel, v};
		}
	} else {
		return {gfx::shader::size_type::Percent, 1.0};
	}
}

void gfx::shader::shader::update(obs_data_t* data)
{
	update_shader(data);
	update_technique(data);

	{
		auto sz_x    = parse_text_as_size(obs_data_get_string(data, ST_SHADER_SIZE_WIDTH));
		_width_type  = sz_x.first;
		_width_value = std::clamp(sz_x.second, 0.01, 8192.0);

		auto sz_y     = parse_text_as_size(obs_data_get_string(data, ST_SHADER_SIZE_HEIGHT));
		_height_type  = sz_y.first;
		_height_value = std::clamp(sz_y.second, 0.01, 8192.0);
	}

	for (auto kv : _shader_params) {
		kv.second->update(data);
	}
}

uint32_t gfx::shader::shader::width()
{
	switch (_mode) {
	case shader_mode::Transition:
		return _base_width;
	case shader_mode::Source:
		switch (_width_type) {
		case size_type::Pixel:
			return std::clamp(static_cast<uint32_t>(_width_value), 1u, 8192u);
		case size_type::Percent:
			return std::clamp(static_cast<uint32_t>(_width_value * _base_width), 1u, 8192u);
		}
	case shader_mode::Filter:
		switch (_width_type) {
		case size_type::Pixel:
			return std::clamp(static_cast<uint32_t>(_width_value), 1u, 8192u);
		case size_type::Percent:
			if (_input_a) {
				return std::clamp(static_cast<uint32_t>(_width_value * _input_a->get_width()), 1u, 8192u);
			} else {
				return std::clamp(static_cast<uint32_t>(_width_value * _base_width), 1u, 8192u);
			}
		}
	default:
		return 0;
	}
}

uint32_t gfx::shader::shader::height()
{
	switch (_mode) {
	case shader_mode::Transition:
		return _base_height;
	case shader_mode::Source:
		switch (_height_type) {
		case size_type::Pixel:
			return std::clamp(static_cast<uint32_t>(_height_value), 1u, 8192u);
		case size_type::Percent:
			return std::clamp(static_cast<uint32_t>(_height_value * _base_height), 1u, 8192u);
		}
	case shader_mode::Filter:
		switch (_height_type) {
		case size_type::Pixel:
			return std::clamp(static_cast<uint32_t>(_height_value), 1u, 8192u);
		case size_type::Percent:
			if (_input_a) {
				return std::clamp(static_cast<uint32_t>(_height_value * _input_a->get_height()), 1u, 8192u);
			} else {
				return std::clamp(static_cast<uint32_t>(_height_value * _base_height), 1u, 8192u);
			}
		}
	default:
		return 0;
	}
}

bool gfx::shader::shader::tick(float_t time)
{
	// Update State
	_time += time;

	// Update Shader
	if (_shader) {
		if (gs::effect_parameter el = _shader.get_parameter("Time"); el != nullptr) {
			if (el.get_type() == gs::effect_parameter::type::Float4) {
				el.set_float4(
					_time, time, 0,
					static_cast<float_t>(static_cast<double_t>(_random())
										 / static_cast<double_t>(std::numeric_limits<unsigned long long>::max())));
			}
		}

		for (auto kv : _shader_params) {
			kv.second->assign();
		}
	}

	return false;
}

void gfx::shader::shader::render()
{
	if (!_shader)
		return;

	uint32_t szw = width();
	uint32_t szh = height();

	while (gs_effect_loop(_shader.get_object(), _shader_tech.c_str())) {
		gs_draw_sprite(nullptr, 0, szw, szh);
	}
}

void gfx::shader::shader::set_size(uint32_t w, uint32_t h)
{
	_base_width  = w;
	_base_height = h;
}

void gfx::shader::shader::set_input_a(std::shared_ptr<gs::texture> tex)
{
	_input_a = tex;
}

void gfx::shader::shader::set_input_b(std::shared_ptr<gs::texture> tex)
{
	_input_b = tex;
}
