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

#include "gs-effect.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include "obs/gs/gs-helper.hpp"
#include "util/util-platform.hpp"

#define MAX_EFFECT_SIZE 32 * 1024 * 1024 // 32 MiB, big enough for everything.

static std::string load_file_as_code(std::filesystem::path shader_file, bool is_top_level = true)
{
	std::stringstream     shader_stream;
	std::filesystem::path shader_path = std::filesystem::absolute(shader_file);
	std::filesystem::path shader_root = std::filesystem::path(shader_path).remove_filename();

	// Ensure it meets size limits.
	uintmax_t size = std::filesystem::file_size(shader_path);
	if (size > MAX_EFFECT_SIZE) {
		throw std::runtime_error("File is too large to be loaded.");
	}

	// Try to open as-is.
	std::ifstream ifs(shader_path, std::ios::in);
	if (!ifs.is_open() || ifs.bad()) {
		throw std::runtime_error("Failed to open file.");
	}

	// Push Graphics API to shader.
	if (is_top_level) {
		auto gctx = streamfx::obs::gs::context();
		switch (gs_get_device_type()) {
		case GS_DEVICE_DIRECT3D_11:
			shader_stream << "#define GS_DEVICE_DIRECT3D_11" << std::endl;
			shader_stream << "#define GS_DEVICE_DIRECT3D" << std::endl;
			break;
		case GS_DEVICE_OPENGL:
			shader_stream << "#define GS_DEVICE_OPENGL" << std::endl;
			break;
		}
	}

	// Pre-process the shader.
	std::string line;
	while (std::getline(ifs, line)) {
		std::string line_trimmed = line;

		{ // Figure out the length of the trim.
			size_t trim_length = 0;
			for (size_t idx = 0, edx = line_trimmed.length(); idx < edx; idx++) {
				char ch = line_trimmed.at(idx);
				if ((ch != ' ') && (ch != '\t')) {
					trim_length = idx;
					break;
				}
			}
			if (trim_length > 0) {
				line_trimmed.erase(0, trim_length);
			}
		}

		// Handle '#include'
		if (line_trimmed.substr(0, 8) == "#include") {
			std::string           include_str  = line_trimmed.substr(10, line_trimmed.size() - 11); // '#include "'
			std::filesystem::path include_path = include_str;

			if (!include_path.is_absolute()) {
				include_path = shader_root / include_str;
			}

			line = load_file_as_code(include_path, false);
		}

		shader_stream << line << std::endl;
	}

	return shader_stream.str();
}

streamfx::obs::gs::effect::effect(const std::string& code, const std::string& name)
{
	auto gctx = streamfx::obs::gs::context();

	char*        error_buffer = nullptr;
	gs_effect_t* effect       = gs_effect_create(code.c_str(), name.c_str(), &error_buffer);

	if (!effect) {
		throw error_buffer ? std::runtime_error(error_buffer)
						   : std::runtime_error("Unknown error during effect compile.");
	}

	reset(effect, [](gs_effect_t* ptr) { gs_effect_destroy(ptr); });
}

streamfx::obs::gs::effect::effect(std::filesystem::path file)
	: effect(load_file_as_code(file),
			 streamfx::util::platform::utf8_to_native(std::filesystem::absolute(file)).generic_u8string())
{}

streamfx::obs::gs::effect::~effect()
{
	auto gctx = streamfx::obs::gs::context();
	reset();
}

std::size_t streamfx::obs::gs::effect::count_techniques()
{
	return static_cast<size_t>(get()->techniques.num);
}

streamfx::obs::gs::effect_technique streamfx::obs::gs::effect::get_technique(std::size_t idx)
{
	if (idx >= count_techniques()) {
		return nullptr;
	}

	return streamfx::obs::gs::effect_technique(get()->techniques.array + idx, *this);
}

streamfx::obs::gs::effect_technique streamfx::obs::gs::effect::get_technique(const std::string& name)
{
	for (std::size_t idx = 0; idx < count_techniques(); idx++) {
		auto ptr = get()->techniques.array + idx;
		if (strcmp(ptr->name, name.c_str()) == 0) {
			return streamfx::obs::gs::effect_technique(ptr, *this);
		}
	}

	return nullptr;
}

bool streamfx::obs::gs::effect::has_technique(const std::string& name)
{
	if (get_technique(name))
		return true;
	return false;
}

std::size_t streamfx::obs::gs::effect::count_parameters()
{
	return get()->params.num;
}

streamfx::obs::gs::effect_parameter streamfx::obs::gs::effect::get_parameter(std::size_t idx)
{
	if (idx >= count_parameters()) {
		throw std::out_of_range("Index is out of range.");
	}

	return streamfx::obs::gs::effect_parameter(get()->params.array + idx, *this);
}

streamfx::obs::gs::effect_parameter streamfx::obs::gs::effect::get_parameter(const std::string& name)
{
	for (std::size_t idx = 0; idx < count_parameters(); idx++) {
		auto ptr = get()->params.array + idx;
		if (strcmp(ptr->name, name.c_str()) == 0) {
			return streamfx::obs::gs::effect_parameter(ptr, *this);
		}
	}

	return nullptr;
}

bool streamfx::obs::gs::effect::has_parameter(const std::string& name)
{
	if (get_parameter(name))
		return true;
	return false;
}

bool streamfx::obs::gs::effect::has_parameter(const std::string& name, effect_parameter::type type)
{
	auto eprm = get_parameter(name);
	if (eprm)
		return eprm.get_type() == type;
	return false;
}
