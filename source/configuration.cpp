// Copyright (C) 2020-2022 Michael Fabian Dirks
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

#include "configuration.hpp"
#include "obs/obs-tools.hpp"
#include "plugin.hpp"
#include "util/util-logging.hpp"

#ifdef _DEBUG
#define ST_PREFIX "<%s> "
#define D_LOG_ERROR(x, ...) P_LOG_ERROR(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_WARNING(x, ...) P_LOG_WARN(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_INFO(x, ...) P_LOG_INFO(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_DEBUG(x, ...) P_LOG_DEBUG(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#else
#define ST_PREFIX "<configuration> "
#define D_LOG_ERROR(...) P_LOG_ERROR(ST_PREFIX __VA_ARGS__)
#define D_LOG_WARNING(...) P_LOG_WARN(ST_PREFIX __VA_ARGS__)
#define D_LOG_INFO(...) P_LOG_INFO(ST_PREFIX __VA_ARGS__)
#define D_LOG_DEBUG(...) P_LOG_DEBUG(ST_PREFIX __VA_ARGS__)
#endif

constexpr std::string_view version_tag_name = "Version";
constexpr std::string_view path_backup_ext  = ".bk";

streamfx::configuration::~configuration()
{
	try {
		save();
		_save_task->wait();
	} catch (std::exception const& ex) {
		DLOG_ERROR("Failed to save configuration: %s", ex.what());
	}
}

streamfx::configuration::configuration() : _config_path(), _data(), _task_lock(), _save_task()
{
	// Retrieve global configuration path.
	_config_path = streamfx::config_file_path("config.json");

	try {
		if (!std::filesystem::exists(_config_path) || !std::filesystem::is_regular_file(_config_path)) {
			throw std::runtime_error("Configuration does not exist.");
		} else {
			obs_data_t* data =
				obs_data_create_from_json_file_safe(_config_path.u8string().c_str(), path_backup_ext.data());
			if (!data) {
				throw std::runtime_error("Failed to load configuration from disk.");
			} else {
				_data = std::shared_ptr<obs_data_t>(data, obs::obs_data_deleter);
			}
		}
	} catch (...) {
		_data = std::shared_ptr<obs_data_t>(obs_data_create(), obs::obs_data_deleter);
	}
}

void streamfx::configuration::save()
{
	std::lock_guard<std::mutex> lg(_task_lock);
	if (!_save_task || _save_task->is_completed()) {
		_save_task = streamfx::threadpool()->push([this](streamfx::util::threadpool::task_data_t) {
			// ToDo: Implement delayed tasks in ::threadpool.
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			// Update version tag.
			obs_data_set_int(_data.get(), version_tag_name.data(), STREAMFX_VERSION);

			if (_config_path.has_parent_path()) {
				std::filesystem::create_directories(_config_path.parent_path());
			}
			if (!obs_data_save_json_safe(_data.get(), _config_path.u8string().c_str(), ".tmp",
										 path_backup_ext.data())) {
				D_LOG_ERROR("Failed to save configuration file.", nullptr);
			}
		});
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
