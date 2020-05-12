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

static const std::list<streamfx::ui::about::entry> _entries = {
	// Contributers
	streamfx::ui::about::entry{"Michael \"Xaymar\" Dirks", streamfx::ui::about::role_type::CONTRIBUTOR, "",
							   streamfx::ui::about::link_type::GENERIC, "https://xaymar.com", "Blog & Stuff",
							   streamfx::ui::about::link_type::SOCIAL_TWITCH, "https://twitch.tv/xaymar",
							   text_social_twitch.data()},

	// Translators
	streamfx::ui::about::entry{"FrozenNortherner", streamfx::ui::about::role_type::TRANSLATOR, "",
							   streamfx::ui::about::link_type::SOCIAL_TWITCH, "https://twitch.tv/frozennortherner",
							   text_social_twitch.data(), streamfx::ui::about::link_type::NONE, "", ""},
	streamfx::ui::about::entry{"HANAWINS", streamfx::ui::about::role_type::TRANSLATOR, "",
							   streamfx::ui::about::link_type::SOCIAL_TWITCH, "https://www.twitch.tv/hanawins",
							   text_social_twitch.data(), streamfx::ui::about::link_type::NONE, "", ""},
	streamfx::ui::about::entry{"hydargos", streamfx::ui::about::role_type::TRANSLATOR, "",
							   streamfx::ui::about::link_type::SOCIAL_TWITCH, "https://www.youtube.com/hydargos",
							   text_social_youtube.data(), streamfx::ui::about::link_type::NONE, "", ""},
	streamfx::ui::about::entry{"Monsteer", streamfx::ui::about::role_type::TRANSLATOR, "",
							   streamfx::ui::about::link_type::SOCIAL_TWITTER, "https://twitter.com/cooliguay",
							   text_social_twitter.data(), streamfx::ui::about::link_type::NONE, "", ""},
	streamfx::ui::about::entry{"Nanito", streamfx::ui::about::role_type::TRANSLATOR, "",
							   streamfx::ui::about::link_type::SOCIAL_TWITCH, "https://twitch.tv/nanito",
							   text_social_twitch.data(), streamfx::ui::about::link_type::SOCIAL_FACEBOOK,
							   "https://facebook.com/nanitotv", text_social_facebook.data()},

	// Separator
	streamfx::ui::about::entry{"", streamfx::ui::about::role_type::NONE, "", streamfx::ui::about::link_type::NONE, "",
							   "", streamfx::ui::about::link_type::NONE, "", ""},

	// Supporters
	streamfx::ui::about::entry{"GranDroidTonight", streamfx::ui::about::role_type::PATREON_SUPPORTER, "",
							   streamfx::ui::about::link_type::SOCIAL_TWITCH, "https://www.twitch.tv/GranDroidTonight",
							   text_social_twitch.data(), streamfx::ui::about::link_type::SOCIAL_YOUTUBE,
							   "https://youtube.com/channel/UCGoT2XFPpeKaL1QuY_NPDuA", text_social_youtube.data()},
	streamfx::ui::about::entry{"chi11estpanda", streamfx::ui::about::role_type::PATREON_SUPPORTER, "",
							   streamfx::ui::about::link_type::SOCIAL_TWITCH, "https://www.twitch.tv/chi11estpanda",
							   "Gaming, Life and Laughs", streamfx::ui::about::link_type::NONE, "", ""},
	streamfx::ui::about::entry{"DandiDoesIt", streamfx::ui::about::role_type::PATREON_SUPPORTER, "",
							   streamfx::ui::about::link_type::SOCIAL_TWITCH, "https://www.twitch.tv/DandiDoesIt",
							   "Twitch Channel", streamfx::ui::about::link_type::NONE, "", ""},
	streamfx::ui::about::entry{"iamresist", streamfx::ui::about::role_type::PATREON_SUPPORTER, "",
							   streamfx::ui::about::link_type::SOCIAL_TWITCH, "https://twitch.tv/iamresist",
							   text_social_twitch.data(), streamfx::ui::about::link_type::NONE, "", ""},
	streamfx::ui::about::entry{"Nordern", streamfx::ui::about::role_type::PATREON_SUPPORTER, "",
							   streamfx::ui::about::link_type::SOCIAL_YOUTUBE, "https://youtube.com/nordern",
							   text_social_youtube.data(), streamfx::ui::about::link_type::SOCIAL_TWITCH,
							   "https://www.twitch.tv/thenordern", text_social_twitch.data()},

	// Separator
	streamfx::ui::about::entry{"", streamfx::ui::about::role_type::NONE, "", streamfx::ui::about::link_type::NONE, "",
							   "", streamfx::ui::about::link_type::NONE, "", ""},

	// Family
	streamfx::ui::about::entry{"Andrea Stenschke", streamfx::ui::about::role_type::FAMILY, "Xaymar",
							   streamfx::ui::about::link_type::NONE, "", "", streamfx::ui::about::link_type::NONE, "",
							   ""},
	streamfx::ui::about::entry{"Carsten Dirks", streamfx::ui::about::role_type::FAMILY, "Xaymar",
							   streamfx::ui::about::link_type::NONE, "", "", streamfx::ui::about::link_type::NONE, "",
							   ""},
	streamfx::ui::about::entry{"Christian \"Azekil\" Dirks", streamfx::ui::about::role_type::FAMILY, "Xaymar",
							   streamfx::ui::about::link_type::NONE, "", "", streamfx::ui::about::link_type::NONE, "",
							   ""},
	streamfx::ui::about::entry{"Gabriele Rantfl", streamfx::ui::about::role_type::FAMILY, "Xaymar",
							   streamfx::ui::about::link_type::NONE, "", "", streamfx::ui::about::link_type::NONE, "",
							   ""},
	streamfx::ui::about::entry{"Reiner Rantfl", streamfx::ui::about::role_type::FAMILY, "Xaymar",
							   streamfx::ui::about::link_type::NONE, "", "", streamfx::ui::about::link_type::NONE, "",
							   ""},
	streamfx::ui::about::entry{"René \"Dex\" Dirks", streamfx::ui::about::role_type::FAMILY, "Xaymar",
							   streamfx::ui::about::link_type::SOCIAL_TWITCH, "https://twitch.tv/vektordex",
							   text_social_twitch.data(), streamfx::ui::about::link_type::GENERIC,
							   "https://worldofdex.de", "Website"},

	// Friends
	streamfx::ui::about::entry{"Axelle", streamfx::ui::about::role_type::FRIEND, "Xaymar",
							   streamfx::ui::about::link_type::SOCIAL_TWITCH, "https://www.twitch.tv/axelle123",
							   text_social_twitch.data(), streamfx::ui::about::link_type::SOCIAL_TWITTER,
							   "https://twitter.com/AxellesNobody", text_social_twitter.data()},

	// Separator
	streamfx::ui::about::entry{"", streamfx::ui::about::role_type::NONE, "", streamfx::ui::about::link_type::NONE, "",
							   "", streamfx::ui::about::link_type::NONE, "", ""},

	// Creators
	streamfx::ui::about::entry{"EposVox", streamfx::ui::about::role_type::CREATOR, "",
							   streamfx::ui::about::link_type::SOCIAL_TWITCH, "https://www.twitch.tv/EposVox",
							   text_social_twitch.data(), streamfx::ui::about::link_type::SOCIAL_YOUTUBE,
							   "https://youtube.com/c/EposVox", text_social_youtube.data()},

};

streamfx::ui::about::about() : QDialog(reinterpret_cast<QWidget*>(obs_frontend_get_main_window()))
{
	setupUi(this);

	// Remove some extra styling.
	setWindowFlag(Qt::WindowContextHelpButtonHint, false); // Remove the unimplemented help button.

	// Thank every single helper.
	bool         column_selector = false;
	size_t       row_selector    = 0;
	QGridLayout* content_layout  = dynamic_cast<QGridLayout*>(content->layout());
	for (auto entry : _entries) {
		if (entry.name.size() == 0) {
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
