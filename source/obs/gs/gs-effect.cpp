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

// OBS
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <graphics/effect.h>
#include <obs.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

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
	ifs.read(buf.data(), size);

	return std::string(buf.data(), buf.data() + size);
}

gs::effect::effect(std::string code, std::string name)
{
	auto gctx = gs::context();

	char*        error_buffer = nullptr;
	gs_effect_t* effect       = gs_effect_create(code.c_str(), name.c_str(), &error_buffer);

	if (!effect) {
		throw error_buffer ? std::runtime_error(error_buffer)
						   : std::runtime_error("Unknown error during effect compile.");
	}

	reset(effect);
}

gs::effect::effect(std::filesystem::path file) : effect(load_file_as_code(file), file.string()) {}

gs::effect::~effect()
{
	auto gctx = gs::context();
	gs_effect_destroy(get());
}

size_t gs::effect::count_techniques()
{
	return static_cast<size_t>(get()->techniques.num);
}

gs::effect_technique gs::effect::get_technique(size_t idx)
{
	if (idx >= count_techniques()) {
		return nullptr;
	}

	return gs::effect_technique(get()->techniques.array + idx, this);
}

gs::effect_technique gs::effect::get_technique(std::string name)
{
	for (size_t idx = 0; idx < count_techniques(); idx++) {
		auto ptr = get()->techniques.array + idx;
		if (strcmp(ptr->name, name.c_str()) == 0) {
			return gs::effect_technique(ptr, this);
		}
	}

	return nullptr;
}

bool gs::effect::has_technique(std::string name)
{
	if (get_technique(name))
		return true;
	return false;
}

size_t gs::effect::count_parameters()
{
	return get()->params.num;
}

gs::effect_parameter gs::effect::get_parameter(size_t idx)
{
	if (idx >= count_parameters()) {
		throw std::out_of_range("Index is out of range.");
	}

	return gs::effect_parameter(get()->params.array + idx, this);
}

gs::effect_parameter gs::effect::get_parameter(std::string name)
{
	for (size_t idx = 0; idx < count_techniques(); idx++) {
		auto ptr = get()->params.array + idx;
		if (strcmp(ptr->name, name.c_str()) == 0) {
			return gs::effect_parameter(ptr, this);
		}
	}

	return nullptr;
}

bool gs::effect::has_parameter(std::string name)
{
	if (get_parameter(name))
		return true;
	return false;
}

bool gs::effect::has_parameter(std::string name, effect_parameter::type type)
{
	auto eprm = get_parameter(name);
	if (eprm)
		return eprm.get_type() == type;
	return false;
}
