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

#include "ui-about.hpp"
#include <algorithm>
#include <deque>
#include <fstream>
#include <map>
#include <random>

#include <obs-frontend-api.h>
#include <nlohmann/json.hpp>
#include "plugin.hpp"
#include "ui-about-entry.hpp"
#include "util/util-logging.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251 4365 4371 4619 4946)
#endif
#include <QLayout>
#include <QLayoutItem>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifdef _DEBUG
#define ST_PREFIX "<%s> "
#define D_LOG_ERROR(x, ...) P_LOG_ERROR(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_WARNING(x, ...) P_LOG_WARN(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_INFO(x, ...) P_LOG_INFO(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_DEBUG(x, ...) P_LOG_DEBUG(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#else
#define ST_PREFIX "<ui::about> "
#define D_LOG_ERROR(...) P_LOG_ERROR(ST_PREFIX __VA_ARGS__)
#define D_LOG_WARNING(...) P_LOG_WARN(ST_PREFIX __VA_ARGS__)
#define D_LOG_INFO(...) P_LOG_INFO(ST_PREFIX __VA_ARGS__)
#define D_LOG_DEBUG(...) P_LOG_DEBUG(ST_PREFIX __VA_ARGS__)
#endif

static const std::vector<std::string_view> _thankyous = {
	":/thankyou/thankyou_cat",
	":/thankyou/thankyou_otter",
	":/thankyou/thankyou_fox",
};

streamfx::ui::about::about() : QDialog(reinterpret_cast<QWidget*>(obs_frontend_get_main_window()))
{
	std::deque<ui::about::entry> entries;

	// Set-up UI.
	setupUi(this);
	setWindowFlag(Qt::WindowContextHelpButtonHint, false); // Remove Help button.
	content->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Maximum);

	// Create a uniform
	std::random_device                      rd;
	std::mt19937_64                         generator(rd());
	std::uniform_int_distribution<uint64_t> rnd;

	// Load entries from 'thanks.json'
	try {
		auto file = streamfx::data_file_path("thanks.json");
		D_LOG_DEBUG("Attempting to load '%s'...", file.generic_string().c_str());
		if (!std::filesystem::exists(file)) {
			// Crash if this file is missing.
			throw std::runtime_error("File 'thanks.json' is missing.");
		}

		std::ifstream fils{file};
		if (fils.bad() || fils.eof()) {
			// Crash if this file is corrupt.
			throw std::runtime_error("File 'thanks.json' is corrupted.");
		}

		auto data = nlohmann::json::parse(fils);
		if (auto iter = data.find("contributor"); iter != data.end()) {
			D_LOG_DEBUG("  Found %" PRIu64 " contributor entries.", iter->size());
			auto kvs = iter->items();
			for (auto kv : kvs) {
				D_LOG_DEBUG("    '%s' => '%s'", kv.key().c_str(), kv.value().get<std::string>().c_str());
				entries.push_back(
					ui::about::entry{kv.key(), role_type::CONTRIBUTOR, "", kv.value().get<std::string>()});
			}
		}
		if (auto iter = data.find("translator"); iter != data.end()) {
			D_LOG_DEBUG("  Found %" PRIu64 " translator entries.", iter->size());
			auto kvs = iter->items();
			for (auto kv : kvs) {
				D_LOG_DEBUG("    '%s' => '%s'", kv.key().c_str(), kv.value().get<std::string>().c_str());
				entries.push_back(ui::about::entry{kv.key(), role_type::TRANSLATOR, "", kv.value().get<std::string>()});
			}
		}
		if (auto iter = data.find("supporter"); iter != data.end()) {
			auto data2 = *iter;
			if (auto iter2 = data2.find("github"); iter2 != data2.end()) {
				D_LOG_DEBUG("  Found %" PRIu64 " GitHub supporter entries.", iter2->size());
				auto kvs = iter2->items();
				for (auto kv : kvs) {
					D_LOG_DEBUG("    '%s' => '%s'", kv.key().c_str(), kv.value().get<std::string>().c_str());
					entries.push_back(
						ui::about::entry{kv.key(), role_type::SUPPORTER, "GitHub", kv.value().get<std::string>()});
				}
			}
			if (auto iter2 = data2.find("patreon"); iter2 != data2.end()) {
				D_LOG_DEBUG("  Found %" PRIu64 " Patreon supporter entries.", iter2->size());
				auto kvs = iter2->items();
				for (auto kv : kvs) {
					D_LOG_DEBUG("    '%s' => '%s'", kv.key().c_str(), kv.value().get<std::string>().c_str());
					entries.push_back(
						ui::about::entry{kv.key(), role_type::SUPPORTER, "Patreon", kv.value().get<std::string>()});
				}
			}
		}
	} catch (const std::exception& ex) {
		D_LOG_ERROR("Loading '%s' failed with error: %s", "thanks.json", ex.what());
		throw std::runtime_error("File 'thanks.json' is invalid.");
	}

	// Build a grid of random entries.
	{
		QGridLayout* layout = dynamic_cast<QGridLayout*>(content->layout());
		int          row    = 0;
		int          column = 0;
		int          thanks = 0;

		// Fix columns being stretched for no reason.
		layout->setColumnStretch(0, 1);
		layout->setColumnStretch(1, 1);

		D_LOG_DEBUG("Building grid of Thank You entries...", "");

		// Randomize the list.
		std::shuffle(entries.begin(), entries.end(), generator);
		for (auto entry : entries) {
			// Create a new entry.
			streamfx::ui::about_entry* v = new streamfx::ui::about_entry(content, entry);
			layout->addWidget(v, row, column);
			layout->setRowStretch(row, 0);

			D_LOG_DEBUG("  Added '%s' => '%s'.", entry.name.c_str(), entry.link.c_str());

			// Proceed down the grid.
			column += 1;
			if (column >= 2) {
				column = 0;
				row += 1;
				thanks += 1;

				if (thanks % 9 == 8) { // "Thank you" every 8 rows.
					auto image = new QLabel(content);
					auto idx   = rnd(generator) % _thankyous.size();
					image->setPixmap(QPixmap(_thankyous.at(idx).data()));
					image->setScaledContents(true);
					image->setFixedSize(384, 384);
					layout->addWidget(image, row, 0, 1, 2, Qt::AlignTop | Qt::AlignHCenter);
					layout->setRowStretch(row, 0);

					thanks = 0;
					row += 1;
				}
			}
		}

		{ // Fix weird automatic scaling done by Qt at the end of the list.
			if (column != 0) {
				row += 1;
				column = 0;
			}

			auto padder = new QFrame(content);
			auto sp     = padder->sizePolicy();
			sp.setVerticalPolicy(QSizePolicy::Minimum);
			sp.setVerticalStretch(1);
			padder->setSizePolicy(sp);
			padder->setObjectName("PaddleMeDaddy");
			padder->setMaximumHeight(QWIDGETSIZE_MAX);
			padder->setMinimumHeight(1);
			padder->setFrameShape(QFrame::NoFrame);
			layout->addWidget(padder, row, 0, 1, 2);
			layout->setRowStretch(row, std::numeric_limits<int>::max());
		}
	}

	// Update the Version information.
	version->setText(STREAMFX_VERSION_STRING);

	// Make the OK button do things.
	connect(buttonBox, &QDialogButtonBox::accepted, this, &streamfx::ui::about::on_ok);
}

streamfx::ui::about::~about() {}

void streamfx::ui::about::on_ok()
{
	hide();
}
