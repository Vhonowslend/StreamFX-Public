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
#include <stdexcept>
#include <vector>
#include "obs/gs/gs-helper.hpp"

#define MAX_EFFECT_SIZE 32 * 1024 * 1024

static std::string load_file_as_code(std::filesystem::path file)
{
	uintmax_t size = std::filesystem::file_size(file);
	if (size > MAX_EFFECT_SIZE) {
		throw std::runtime_error("File is too large to be loaded.");
	}

	std::ifstream ifs(file, std::ios::binary);
	if (!ifs.is_open() || ifs.bad()) {
		throw std::runtime_error("An unknown error occured trying to open the file.");
	}

	std::vector<char> buf(size_t(size + 1), 0);
	ifs.read(buf.data(), static_cast<std::streamsize>(size));

	return std::string(buf.data(), buf.data() + size);
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

streamfx::obs::gs::effect::effect(std::filesystem::path file) : effect(load_file_as_code(file), file.u8string()) {}

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
