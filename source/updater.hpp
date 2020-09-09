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
#include <atomic>
#include <chrono>
#include <nlohmann/json.hpp>
#include "util/util-curl.hpp"
#include "util/util-event.hpp"
#include "util/util-threadpool.hpp"

namespace streamfx {
	enum class update_channel {
		RELEASE,
		TESTING,
	};

	struct update_info {
		uint16_t       version_major = 0;
		uint16_t       version_minor = 0;
		uint16_t       version_patch = 0;
		char           version_type  = 0;
		uint16_t       version_index = 0;
		update_channel channel       = update_channel::RELEASE;
		std::string    url           = "";
		std::string    name          = "";

		bool is_newer(update_info& other);
	};

	void to_json(nlohmann::json&, const update_info&);
	void from_json(const nlohmann::json&, update_info&);

	class updater {
		// Internal
		std::mutex                              _lock;
		std::weak_ptr<::util::threadpool::task> _task;

		// Options
		std::atomic_bool     _gdpr;
		std::atomic_bool     _automation;
		update_channel       _channel;
		std::chrono::seconds _lastcheckedat;

		// Update Information
		update_info _current_info;
		update_info _release_info;
		update_info _testing_info;
		bool        _dirty;

		private:
		void task(util::threadpool_data_t);
		void task_query(std::vector<char>& buffer);
		void task_parse(std::vector<char>& buffer);

		bool can_check();

		void load();
		void save();

		public:
		updater();
		~updater();

		// GDPR compliance (must require user interaction!)
		bool gdpr();
		void set_gdpr(bool);

		// Automatic Update checks
		bool automation();
		void set_automation(bool);

		// Update Channel
		update_channel channel();
		void           set_channel(update_channel channel);

		// Refresh information.
		void refresh();

		// Check current data.
		bool        have_update();
		update_info get_current_info();
		update_info get_update_info();

		public:
		struct _ {
			util::event<updater&, bool>           gdpr_changed;
			util::event<updater&, bool>           automation_changed;
			util::event<updater&, update_channel> channel_changed;

			util::event<updater&, std::string&> error;
			util::event<updater&>               refreshed;
		} events;

		public:
		static std::shared_ptr<streamfx::updater> instance();
	};
} // namespace streamfx
