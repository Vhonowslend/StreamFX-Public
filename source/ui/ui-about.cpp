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
#include <obs-frontend-api.h>
#include <map>
#include <random>
#include "ui-about-entry.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251 4365 4371 4619 4946)
#endif
#include <QLayout>
#include <QLayoutItem>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

constexpr std::string_view text_social_facebook = "Facebook";
constexpr std::string_view text_social_twitch   = "Twitch";
constexpr std::string_view text_social_twitter  = "Twitter";
constexpr std::string_view text_social_youtube  = "YouTube";

static const std::vector<std::string_view> _thankyous = {
	":/thankyou/thankyou_cat",
	":/thankyou/thankyou_otter",
	":/thankyou/thankyou_fox",
};

static const streamfx::ui::about::entry _entries[] = {
	// Contributers
	// - 2021
	streamfx::ui::about::entry{"Michael \"Xaymar\" Dirks", streamfx::ui::about::role_type::CONTRIBUTOR, "",
							   "https://xaymar.com"},
	streamfx::ui::about::entry{"cpyarger", streamfx::ui::about::role_type::CONTRIBUTOR, "",
							   "https://github.com/cpyarger"},
	streamfx::ui::about::entry{"tyten652", streamfx::ui::about::role_type::CONTRIBUTOR, "",
							   "https://github.com/tytan652"},
	streamfx::ui::about::entry{"kilinbox", streamfx::ui::about::role_type::CONTRIBUTOR, "",
							   "https://github.com/kilinbox"},
	// - 2020
	streamfx::ui::about::entry{"Oncorporation", streamfx::ui::about::role_type::CONTRIBUTOR, "",
							   "https://github.com/Oncorporation"},
	streamfx::ui::about::entry{"dghodgson", streamfx::ui::about::role_type::CONTRIBUTOR, "",
							   "https://github.com/dghodgson"},
	streamfx::ui::about::entry{"danimo", streamfx::ui::about::role_type::CONTRIBUTOR, "", "https://github.com/danimo"},
	streamfx::ui::about::entry{"brandonedens", streamfx::ui::about::role_type::CONTRIBUTOR, "",
							   "https://github.com/brandonedens"},
	streamfx::ui::about::entry{"rjmoggach", streamfx::ui::about::role_type::CONTRIBUTOR, "",
							   "https://github.com/rjmoggach"},
	// - 2019
	streamfx::ui::about::entry{"catb0t", streamfx::ui::about::role_type::CONTRIBUTOR, "", "https://github.com/catb0t"},
	streamfx::ui::about::entry{"Vainock", streamfx::ui::about::role_type::CONTRIBUTOR, "",
							   "https://github.com/Vainock"},
	streamfx::ui::about::entry{"wwj402", streamfx::ui::about::role_type::CONTRIBUTOR, "", "https://github.com/wwj402"},
	// - 2018

	// Translators
	// - TODO

	// Separator
	streamfx::ui::about::entry{"", streamfx::ui::about::role_type::THANKYOU, "", ""},

	// Supporters
	// - TODO

	// Separator
	streamfx::ui::about::entry{"", streamfx::ui::about::role_type::THANKYOU, "", ""},

	// Family & Friends
	streamfx::ui::about::entry{"Andrea Stenschke", streamfx::ui::about::role_type::FAMILY, "Xaymar", ""},
	streamfx::ui::about::entry{"Carsten Dirks", streamfx::ui::about::role_type::FAMILY, "Xaymar", ""},
	streamfx::ui::about::entry{"Gabriele Rantfl", streamfx::ui::about::role_type::FAMILY, "Xaymar", ""},
	streamfx::ui::about::entry{"Reiner Rantfl", streamfx::ui::about::role_type::FAMILY, "Xaymar", ""},
	streamfx::ui::about::entry{"René \"Dex\" Dirks", streamfx::ui::about::role_type::FAMILY, "Xaymar",
							   "https://worldofdex.de"},
	streamfx::ui::about::entry{"Christian \"Azekil\" Dirks", streamfx::ui::about::role_type::FAMILY, "Xaymar", ""},
	streamfx::ui::about::entry{"Axelle", streamfx::ui::about::role_type::FRIEND, "Xaymar",
							   "https://www.twitch.tv/axelle123"},
};

streamfx::ui::about::about() : QDialog(reinterpret_cast<QWidget*>(obs_frontend_get_main_window()))
{
	setupUi(this);

	// Remove some extra styling.
	setWindowFlag(Qt::WindowContextHelpButtonHint, false); // Remove the unimplemented help button.

	// Random thing
	auto            rnd = std::uniform_int_distribution(size_t(0), _thankyous.size() - 1);
	std::mt19937_64 rnde;

	// Thank every single helper.
	bool         column_selector = false;
	size_t       row_selector    = 0;
	QGridLayout* content_layout  = dynamic_cast<QGridLayout*>(content->layout());
	for (const auto& entry : _entries) {
		if (entry.role == role_type::SPACER) {
			row_selector += 2;
			column_selector = 0;

			// Add a separator line.
			auto separator = new QFrame(content);
			{
				auto sp = separator->sizePolicy();
				sp.setVerticalPolicy(QSizePolicy::Fixed);
				separator->setSizePolicy(sp);
			}
			separator->setFrameShape(QFrame::HLine);
			separator->setFrameShadow(QFrame::Sunken);
			separator->setMaximumHeight(1);
			separator->setMinimumHeight(1);
			separator->setFixedHeight(1);
			separator->setLineWidth(1);
			content_layout->addWidget(separator, static_cast<int>(row_selector - 1), 0, 1, 2);
			content_layout->setRowStretch(static_cast<int>(row_selector - 1), 0);
		} else if (entry.role == role_type::THANKYOU) {
			row_selector += 2;
			column_selector = 0;

			auto element = new QLabel(content);

			auto elrnd = rnd(rnde);
			element->setPixmap(QPixmap(_thankyous.at(elrnd).data()));

			element->setScaledContents(true);
			element->setFixedSize(384, 384);

			content_layout->addWidget(element, static_cast<int>(row_selector - 1), 0, 1, 2,
									  Qt::AlignTop | Qt::AlignHCenter);
			content_layout->setRowStretch(static_cast<int>(row_selector - 1), 0);
		} else {
			streamfx::ui::about_entry* v = new streamfx::ui::about_entry(content, entry);

			content_layout->addWidget(v, static_cast<int>(row_selector), column_selector ? 1 : 0);
			content_layout->setRowStretch(static_cast<int>(row_selector), 0);

			if (column_selector) {
				row_selector++;
			}
			column_selector = !column_selector;
		}
	}
	{
		row_selector++;
		auto padder = new QFrame(content);
		{
			auto sp = padder->sizePolicy();
			sp.setVerticalPolicy(QSizePolicy::Minimum);
			sp.setVerticalStretch(1);
			padder->setSizePolicy(sp);
		}
		padder->setObjectName("PaddleMeDaddy");
		padder->setMaximumHeight(QWIDGETSIZE_MAX);
		padder->setMinimumHeight(1);
		padder->setFrameShape(QFrame::NoFrame);
		content_layout->addWidget(padder, static_cast<int>(row_selector), 0, 1, 2);
		content_layout->setRowStretch(static_cast<int>(row_selector), 9999);
	}

	content_layout->setColumnStretch(0, 1);
	content_layout->setColumnStretch(1, 1);
	content->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Maximum);

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
