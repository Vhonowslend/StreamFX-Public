// Copyright (c) 2020 Michael Fabian Dirks <info@xaymar.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "updater.hpp"
#include "version.hpp"
#include <mutex>
#include <string_view>
#include "configuration.hpp"
#include "plugin.hpp"
#include "util/util-logging.hpp"

#ifdef _DEBUG
#define ST_PREFIX "<%s> "
#define D_LOG_ERROR(x, ...) P_LOG_ERROR(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_WARNING(x, ...) P_LOG_WARN(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_INFO(x, ...) P_LOG_INFO(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_DEBUG(x, ...) P_LOG_DEBUG(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#else
#define ST_PREFIX "<updater> "
#define D_LOG_ERROR(...) P_LOG_ERROR(ST_PREFIX __VA_ARGS__)
#define D_LOG_WARNING(...) P_LOG_WARN(ST_PREFIX __VA_ARGS__)
#define D_LOG_INFO(...) P_LOG_INFO(ST_PREFIX __VA_ARGS__)
#define D_LOG_DEBUG(...) P_LOG_DEBUG(ST_PREFIX __VA_ARGS__)
#endif

// TODO:
// - Cache result in the configuration directory (not as a configuration value).
// - Move 'autoupdater.last_checked_at' to out of the configuration.
// - Figure out if nightly updates are viable at all.

#define ST_CFG_GDPR "updater.gdpr"
#define ST_CFG_AUTOMATION "updater.automation"
#define ST_CFG_CHANNEL "updater.channel"
#define ST_CFG_LASTCHECKEDAT "updater.lastcheckedat"

void streamfx::to_json(nlohmann::json& json, const update_info& info)
{
	auto version     = nlohmann::json::object();
	version["major"] = info.version_major;
	version["minor"] = info.version_minor;
	version["patch"] = info.version_patch;
	if (info.version_type)
		version["alpha"] = info.version_type == 'a' ? true : false;
	version["index"] = info.version_index;
	json["version"]  = version;
	json["preview"]  = info.channel == update_channel::TESTING;
	json["url"]      = info.url;
	json["name"]     = info.name;
}

void streamfx::from_json(const nlohmann::json& json, update_info& info)
{
	if (json.find("html_url") != json.end()) {
		// This is a response from GitHub.

		// Retrieve entries from the release object.
		auto entry_tag_name = json.find("tag_name");
		auto entry_name     = json.find("name");
		auto entry_url      = json.find("html_url");
		auto entry_preview  = json.find("prerelease");
		if ((entry_tag_name == json.end()) || (entry_name == json.end()) || (entry_url == json.end())
			|| (entry_preview == json.end())) {
			throw std::runtime_error("JSON is missing one or more required keys.");
		}

		// Initialize update information.
		info.channel = entry_preview->get<bool>() ? update_channel::TESTING : update_channel::RELEASE;
		info.url     = entry_url->get<std::string>();
		info.name    = entry_name->get<std::string>();

		// Parse the tag name as SemVer (A.B.C)
		{
			std::string tag_name = entry_tag_name->get<std::string>();

			size_t dot_1       = tag_name.find_first_of(".", 0) + 1;
			info.version_major = static_cast<uint16_t>(strtoul(&tag_name.at(0), nullptr, 10));
			if (dot_1 < tag_name.size()) {
				info.version_minor = static_cast<uint16_t>(strtoul(&tag_name.at(dot_1), nullptr, 10));
			}

			char*  endptr = nullptr;
			size_t dot_2  = tag_name.find_first_of(".", dot_1) + 1;
			if (dot_2 < tag_name.size()) {
				info.version_patch = static_cast<uint16_t>(strtoul(&tag_name.at(dot_2), &endptr, 10));
			}

			// Check if there's data following the SemVer structure. (A.B.CdE)
			if ((endptr != nullptr) && (endptr < (tag_name.data() + tag_name.size()))) {
				size_t last_num   = static_cast<size_t>(endptr - tag_name.data()) + 1;
				info.version_type = *endptr;
				if (last_num < tag_name.size())
					info.version_index = static_cast<uint16_t>(strtoul(&tag_name.at(last_num), nullptr, 10));
			} else {
				info.version_type  = 0;
				info.version_index = 0;
			}
		}
	} else {
		auto version       = json.at("version");
		info.version_major = version.at("major").get<uint16_t>();
		info.version_minor = version.at("minor").get<uint16_t>();
		info.version_patch = version.at("patch").get<uint16_t>();
		if (version.find("type") != version.end())
			info.version_type = version.at("alpha").get<bool>() ? 'a' : 'b';
		info.version_index = version.at("index").get<uint16_t>();
		info.channel       = json.at("preview").get<bool>() ? update_channel::TESTING : update_channel::RELEASE;
		info.url           = json.at("url").get<std::string>();
		info.name          = json.at("name").get<std::string>();
	}
}

bool streamfx::update_info::is_newer(update_info& other)
{
	// 1. Compare Major version:
	//     A. Ours is greater: Remote is older.
	//     B. Theirs is greater: Remote is newer.
	//     C. Continue the check.
	if (version_major > other.version_major)
		return false;
	if (version_major < other.version_major)
		return true;

	// 2. Compare Minor version:
	//     A. Ours is greater: Remote is older.
	//     B. Theirs is greater: Remote is newer.
	//     C. Continue the check.
	if (version_minor > other.version_minor)
		return false;
	if (version_minor < other.version_minor)
		return true;

	// 3. Compare Patch version:
	//     A. Ours is greater: Remote is older.
	//     B. Theirs is greater: Remote is newer.
	//     C. Continue the check.
	if (version_patch > other.version_patch)
		return false;
	if (version_patch < other.version_patch)
		return true;

	// 4. Compare Type:
	//     A. Ours empty: Remote is older.
	//     B. Theirs empty: Remote is newer.
	//     C. Continue the check.
	// A automatically implies that ours is not empty for B. A&B combined imply that both are not empty for step 5.
	if (version_type == 0)
		return false;
	if (other.version_type == 0)
		return true;

	// 5. Compare Type value:
	//     A. Ours is greater: Remote is older.
	//     B. Theirs is greater: Remote is newer.
	//     C. Continue the check.
	if (version_type > other.version_type)
		return false;
	if (version_type < other.version_type)
		return true;

	// 6. Compare Index:
	//    A. Ours is greater or equal: Remote is older or identical.
	//    B. Remote is newer
	if (version_index >= other.version_index)
		return false;

	return true;
}

void streamfx::updater::task(streamfx::util::threadpool_data_t)
try {
	{
		std::vector<char> buffer;
		task_query(buffer);
		task_parse(buffer);
	}

#ifdef _DEBUG
	{
		std::lock_guard<std::mutex> lock(_lock);
		nlohmann::json              current = _current_info;
		nlohmann::json              stable  = _release_info;
		nlohmann::json              preview = _testing_info;

		D_LOG_DEBUG("Current Version: %s", current.dump().c_str());
		D_LOG_DEBUG("Stable Version: %s", stable.dump().c_str());
		D_LOG_DEBUG("Preview Version: %s", preview.dump().c_str());
	}
#endif

	// Log that we have a potential update.
	if (have_update()) {
		auto info = get_update_info();

		if (info.version_type) {
			D_LOG_INFO("Update to version %d.%d.%d%.1s%d is available at \"%s\".", info.version_major,
					   info.version_minor, info.version_patch, &info.version_type, info.version_index,
					   info.url.c_str());
		} else {
			D_LOG_INFO("Update to version %d.%d.%d is available at \"%s\".", info.version_major, info.version_minor,
					   info.version_patch, info.url.c_str());
		}
	} else {
		D_LOG_DEBUG("No update available.", "");
	}

	// Notify listeners of the update.
	events.refreshed.call(*this);
} catch (const std::exception& ex) {
	// Notify about the error.
	std::string message = ex.what();
	events.error.call(*this, message);
}

void streamfx::updater::task_query(std::vector<char>& buffer)
{
	static constexpr std::string_view ST_API_URL = "https://api.github.com/repos/Xaymar/obs-StreamFX/releases";

	streamfx::util::curl curl;
	size_t               buffer_offset = 0;

	// Set headers (User-Agent is needed so Github can contact us!).
	curl.set_header("User-Agent", "StreamFX Updater v" STREAMFX_VERSION_STRING);
	curl.set_header("Accept", "application/vnd.github.v3+json");

	// Set up request.
	curl.set_option(CURLOPT_HTTPGET, true); // GET
	curl.set_option(CURLOPT_POST, false);   // Not POST
	curl.set_option(CURLOPT_URL, ST_API_URL);
	curl.set_option(CURLOPT_TIMEOUT, 10); // 10s until we fail.

	// Callbacks
	curl.set_write_callback([this, &buffer, &buffer_offset](void* data, size_t s1, size_t s2) {
		size_t size = s1 * s2;
		if (buffer.size() < (size + buffer_offset))
			buffer.resize(buffer_offset + size);

		memcpy(buffer.data() + buffer_offset, data, size);
		buffer_offset += size;

		return s1 * s2;
	});
	//std::bind(&streamfx::updater::task_write_cb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)

	// Clear any unknown data and reserve 64KiB of memory.
	buffer.clear();
	buffer.reserve(0xFFFF);

	// Finally, execute the request.
	D_LOG_DEBUG("Querying for latest releases...", "");
	if (CURLcode res = curl.perform(); res != CURLE_OK) {
		D_LOG_ERROR("Performing query failed with error: %s", curl_easy_strerror(res));
		throw std::runtime_error(curl_easy_strerror(res));
	}

	int32_t status_code = 0;
	if (CURLcode res = curl.get_info(CURLINFO_HTTP_CODE, status_code); res != CURLE_OK) {
		D_LOG_ERROR("Retrieving status code failed with error: %s", curl_easy_strerror(res));
		throw std::runtime_error(curl_easy_strerror(res));
	}
	D_LOG_DEBUG("API returned status code %d.", status_code);

	if (status_code != 200) {
		D_LOG_ERROR("API returned unexpected status code %d.", status_code);
		throw std::runtime_error("Request failed due to one or more reasons.");
	}
}

void streamfx::updater::task_parse(std::vector<char>& buffer)
{
	// Parse the JSON response from the API.
	nlohmann::json json = nlohmann::json::parse(buffer.begin(), buffer.end());

	// Check if it was parsed as an object.
	if (json.type() != nlohmann::json::value_t::array) {
		throw std::runtime_error("Response is not a JSON array.");
	}

	// Iterate over all entries.
	std::lock_guard<std::mutex> lock(_lock);
	for (auto obj : json) {
		try {
			auto info = obj.get<streamfx::update_info>();

			if (info.channel == update_channel::RELEASE) {
				if (_release_info.is_newer(info))
					_release_info = info;
				if (_testing_info.is_newer(info))
					_testing_info = info;
			} else {
				if (_testing_info.is_newer(info))
					_testing_info = info;
			}
		} catch (const std::exception& ex) {
			D_LOG_DEBUG("Failed to parse entry, error: %s", ex.what());
		}
	}
}

bool streamfx::updater::can_check()
{
#ifdef _DEBUG
	return true;
#else
	auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
	auto threshold = (_lastcheckedat + std::chrono::minutes(10));
	return (now > threshold);
#endif
}

void streamfx::updater::load()
{
	std::lock_guard<std::mutex> lock(_lock);
	if (auto config = streamfx::configuration::instance(); config) {
		auto dataptr = config->get();

		if (obs_data_has_user_value(dataptr.get(), ST_CFG_GDPR))
			_gdpr = obs_data_get_bool(dataptr.get(), ST_CFG_GDPR);
		if (obs_data_has_user_value(dataptr.get(), ST_CFG_AUTOMATION))
			_automation = obs_data_get_bool(dataptr.get(), ST_CFG_AUTOMATION);
		if (obs_data_has_user_value(dataptr.get(), ST_CFG_CHANNEL))
			_channel = static_cast<update_channel>(obs_data_get_int(dataptr.get(), ST_CFG_CHANNEL));
		if (obs_data_has_user_value(dataptr.get(), ST_CFG_LASTCHECKEDAT))
			_lastcheckedat = std::chrono::seconds(obs_data_get_int(dataptr.get(), ST_CFG_LASTCHECKEDAT));
	}
}

void streamfx::updater::save()
{
	if (auto config = streamfx::configuration::instance(); config) {
		auto dataptr = config->get();

		obs_data_set_bool(dataptr.get(), ST_CFG_GDPR, _gdpr);
		obs_data_set_bool(dataptr.get(), ST_CFG_AUTOMATION, _automation);
		obs_data_set_int(dataptr.get(), ST_CFG_CHANNEL, static_cast<long long>(_channel));
		obs_data_set_int(dataptr.get(), ST_CFG_LASTCHECKEDAT, static_cast<long long>(_lastcheckedat.count()));
	}
}

streamfx::updater::updater()
	: _lock(), _task(),

	  _gdpr(false), _automation(true), _channel(update_channel::RELEASE), _lastcheckedat(),

	  _current_info(), _release_info(), _testing_info(), _dirty(false)
{
	// Load information from configuration.
	load();

	// Build current version information.
	try {
		_current_info.version_major = STREAMFX_VERSION_MAJOR;
		_current_info.version_minor = STREAMFX_VERSION_MINOR;
		_current_info.version_patch = STREAMFX_VERSION_PATCH;
		_current_info.channel       = _channel;
		std::string suffix          = STREAMFX_VERSION_SUFFIX;
		if (suffix.length()) {
			_current_info.version_type  = suffix.at(0);
			_current_info.version_index = static_cast<uint16_t>(strtoul(&suffix.at(1), nullptr, 10));
		}
	} catch (...) {
		D_LOG_ERROR("Failed to parse current version information, results may be inaccurate.", "");
	}
}

streamfx::updater::~updater()
{
	save();
}

bool streamfx::updater::gdpr()
{
	return _gdpr;
}

void streamfx::updater::set_gdpr(bool value)
{
	_dirty = true;
	_gdpr  = value;
	events.gdpr_changed(*this, _gdpr);

	{
		std::lock_guard<std::mutex> lock(_lock);
		save();
	}

	D_LOG_INFO("User %s the processing of data.", _gdpr ? "allowed" : "disallowed");
}

bool streamfx::updater::automation()
{
	return _automation;
}

void streamfx::updater::set_automation(bool value)
{
	_automation = value;
	events.automation_changed(*this, _automation);

	{
		std::lock_guard<std::mutex> lock(_lock);
		save();
	}

	D_LOG_INFO("Automatic checks at launch are now %s.", value ? "enabled" : "disabled");
}

streamfx::update_channel streamfx::updater::channel()
{
	return _channel;
}

void streamfx::updater::set_channel(update_channel value)
{
	std::lock_guard<std::mutex> lock(_lock);

	_dirty   = true;
	_channel = value;
	events.channel_changed(*this, _channel);

	save();

	D_LOG_INFO("Update channel changed to %s.", value == update_channel::RELEASE ? "Release" : "Testing");
}

void streamfx::updater::refresh()
{
	if (!_task.expired() || !gdpr()) {
		return;
	}

	if (can_check()) {
		std::lock_guard<std::mutex> lock(_lock);

		// Update last checked time.
		_lastcheckedat =
			std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
		save();

		// Spawn a new task.
		_task = streamfx::threadpool()->push(std::bind(&streamfx::updater::task, this, std::placeholders::_1), nullptr);
	} else {
		events.refreshed(*this);
	}
}

bool streamfx::updater::have_update()
{
	auto info = get_update_info();
	return _current_info.is_newer(info);
}

streamfx::update_info streamfx::updater::get_current_info()
{
	return _current_info;
}

streamfx::update_info streamfx::updater::get_update_info()
{
	std::lock_guard<std::mutex> lock(_lock);
	update_info                 info = _release_info;
	if (info.is_newer(_testing_info) && (_channel == update_channel::TESTING))
		info = _testing_info;

	return info;
}

std::shared_ptr<streamfx::updater> streamfx::updater::instance()
{
	static std::weak_ptr<streamfx::updater> _instance;
	static std::mutex                       _lock;

	std::lock_guard<std::mutex> lock(_lock);
	if (_instance.expired()) {
		auto ptr  = std::make_shared<streamfx::updater>();
		_instance = ptr;
		return ptr;
	} else {
		return _instance.lock();
	}
}
