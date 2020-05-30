/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2020 Michael Fabian Dirks
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

#include "configuration.hpp"
#include "obs/obs-tools.hpp"
#include "plugin.hpp"

constexpr std::string_view version_tag_name = "Version";
constexpr std::string_view path_backup_ext  = ".bk";

streamfx::configuration::~configuration()
{
	// Update version tag.
	obs_data_set_int(_data.get(), version_tag_name.data(), STREAMFX_VERSION);

	try {
		if (_config_path.has_parent_path()) {
			std::filesystem::create_directories(_config_path.parent_path());
		}
		if (!obs_data_save_json_safe(_data.get(), _config_path.string().c_str(), ".tmp", path_backup_ext.data())) {
			throw std::exception();
		}
	} catch (...) {
		LOG_ERROR("Failed to save configuration, next start will be using defaults or backed up configuration.");
	}
}

streamfx::configuration::configuration() : _data(), _config_path()
{
	{ // Retrieve global configuration path.
		const char* path    = obs_module_config_path("config.json");
		_config_path        = path;
	}

	try {
		if (!std::filesystem::exists(_config_path) || !std::filesystem::is_regular_file(_config_path)) {
			throw std::exception();
		} else {
			obs_data_t* data = obs_data_create_from_json_file_safe(_config_path.string().c_str(), path_backup_ext.data());
			if (!data) {
				throw std::exception();
			} else {
				_data = std::shared_ptr<obs_data_t>(data, obs::obs_data_deleter);
			}
		}
	} catch (...) {
		_data = std::shared_ptr<obs_data_t>(obs_data_create(), obs::obs_data_deleter);
	}
}

std::shared_ptr<obs_data_t> streamfx::configuration::get()
{
	obs_data_addref(_data.get());
	return std::shared_ptr<obs_data_t>(_data.get(), obs::obs_data_deleter);
}

uint64_t streamfx::configuration::version()
{
	return static_cast<uint64_t>(obs_data_get_int(_data.get(), version_tag_name.data()));
}

bool streamfx::configuration::is_different_version()
{
	return (version() & STREAMFX_MASK_COMPAT) != (STREAMFX_VERSION & STREAMFX_MASK_COMPAT);
}

static std::shared_ptr<streamfx::configuration> _instance = nullptr;

void streamfx::configuration::initialize()
{
	if (!_instance)
		_instance = std::make_shared<streamfx::configuration>();
}

void streamfx::configuration::finalize()
{
	_instance.reset();
}

std::shared_ptr<streamfx::configuration> streamfx::configuration::instance()
{
	return _instance;
}
