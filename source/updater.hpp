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

#pragma once
#include "util/util-curl.hpp"
#include "util/util-event.hpp"
#include "util/util-threadpool.hpp"

#include "warning-disable.hpp"
#include <atomic>
#include <chrono>
#include <map>
#include <nlohmann/json.hpp>
#include "warning-enable.hpp"

namespace streamfx {
	enum class version_stage : uint8_t {
		STABLE,    // A.B.C
		CANDIDATE, // A.B.CcD
		BETA,      // A.B.CbD
		ALPHA,     // A.B.CaD
	};
	version_stage    stage_from_string(std::string_view str);
	std::string_view stage_to_string(version_stage t);

	void to_json(nlohmann::json&, const version_stage&);
	void from_json(const nlohmann::json&, version_stage&);

	struct version_info {
		public:
		uint16_t      major;
		uint16_t      minor;
		uint16_t      patch;
		uint16_t      tweak;
		version_stage stage;
		std::string   url;
		std::string   name;

		public:
		version_info();
		version_info(const std::string text);

		bool is_older_than(const version_info other);

		operator std::string();
	};

	void to_json(nlohmann::json&, const version_info&);
	void from_json(const nlohmann::json&, version_info&);

	class updater {
		// Internal
		std::recursive_mutex                              _lock;
		std::weak_ptr<::streamfx::util::threadpool::task> _task;

		// Options
		std::atomic_bool     _data_sharing_allowed;
		std::atomic_bool     _automation;
		version_stage        _channel;
		std::chrono::seconds _lastcheckedat;

		// Update Information
		version_info                          _current_info;
		std::map<version_stage, version_info> _updates;
		bool                                  _dirty;

		private:
		void task(streamfx::util::threadpool::task_data_t);

		bool can_check();

		void load();
		void save();

		public:
		updater();
		~updater();

		// GDPR compliance (must require user interaction!)
		bool is_data_sharing_allowed();
		void set_data_sharing_allowed(bool);

		// Automatic Update checks
		bool is_automated();
		void set_automation(bool);

		// Update Channel
		version_stage get_channel();
		void          set_channel(version_stage channel);

		// Refresh information.
		void refresh();

		// Check current data.
		version_info get_current_info();

		// Check update data.
		bool         is_update_available();
		bool         is_update_available(version_stage channel);
		version_info get_update_info();
		version_info get_update_info(version_stage channel);

		public:
		struct _ {
			streamfx::util::event<updater&, bool>          gdpr_changed;
			streamfx::util::event<updater&, bool>          automation_changed;
			streamfx::util::event<updater&, version_stage> channel_changed;

			streamfx::util::event<updater&, std::string&> error;
			streamfx::util::event<updater&>               refreshed;
		} events;

		public:
		static std::shared_ptr<streamfx::updater> instance();
	};
} // namespace streamfx
