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

static const std::list<streamfx::ui::about::entry> _entries = {
	// Contributers
	streamfx::ui::about::entry{"Michael \"Xaymar\" Dirks", streamfx::ui::about::role_type::CONTRIBUTOR, "",
							   streamfx::ui::about::link_type::GENERIC, "https://xaymar.com", "Blog & News",
							   streamfx::ui::about::link_type::SOCIAL_TWITCH, "https://twitch.tv/xaymar",
							   text_social_twitch.data()},

	// Translators
	/// https://www.patreon.com/user?u=4473266, https://crowdin.com/profile/kimbech, https://twitch.tv/frozennortherner (Proofreader, Norwegian)
	streamfx::ui::about::entry{"FrozenNortherner", streamfx::ui::about::role_type::TRANSLATOR, "",
							   streamfx::ui::about::link_type::SOCIAL_TWITCH, "https://twitch.tv/frozennortherner",
							   text_social_twitch.data(), streamfx::ui::about::link_type::NONE, "", ""},
	/// https://crowdin.com/profile/hydargos (Proofreader, French)
	streamfx::ui::about::entry{"hydargos", streamfx::ui::about::role_type::TRANSLATOR, "",
							   streamfx::ui::about::link_type::SOCIAL_YOUTUBE, "https://www.youtube.com/hydargos",
							   text_social_youtube.data(), streamfx::ui::about::link_type::NONE, "", ""},
	/// https://crowdin.com/profile/saygo1125 (Proofreader, Japanese)
	streamfx::ui::about::entry{"saygo1125", streamfx::ui::about::role_type::TRANSLATOR, "",
							   streamfx::ui::about::link_type::GENERIC, "https://crowdin.com/profile/saygo1125",
							   "Crowdin", streamfx::ui::about::link_type::NONE, "", ""},
	/// https://crowdin.com/profile/Monsteer (Spanish)
	streamfx::ui::about::entry{"Monsteer", streamfx::ui::about::role_type::TRANSLATOR, "",
							   streamfx::ui::about::link_type::SOCIAL_TWITTER, "https://twitter.com/cooliguay",
							   text_social_twitter.data(), streamfx::ui::about::link_type::NONE, "", ""},
	/// https://crowdin.com/profile/hellnano (Spanish)
	streamfx::ui::about::entry{"Nanito", streamfx::ui::about::role_type::TRANSLATOR, "",
							   streamfx::ui::about::link_type::SOCIAL_TWITCH, "https://twitch.tv/nanito",
							   text_social_twitch.data(), streamfx::ui::about::link_type::SOCIAL_FACEBOOK,
							   "https://facebook.com/nanitotv", text_social_facebook.data()},
	/// https://crowdin.com/profile/wownik98 (Russian)
	streamfx::ui::about::entry{"WoWnik", streamfx::ui::about::role_type::TRANSLATOR, "",
							   streamfx::ui::about::link_type::GENERIC, "https://crowdin.com/profile/wownik98",
							   "Crowdin", streamfx::ui::about::link_type::NONE, "", ""},

	// Separator
	streamfx::ui::about::entry{"", streamfx::ui::about::role_type::THANKYOU, "", streamfx::ui::about::link_type::NONE,
							   "", "", streamfx::ui::about::link_type::NONE, "", ""},

	/*
	streamfx::ui::about::entry{"", streamfx::ui::about::role_type::PATREON_SUPPORTER, "",
							   streamfx::ui::about::link_type::GENERIC, "", "Patreon",
							   streamfx::ui::about::link_type::NONE, "", ""},
	 streamfx::ui::about::entry{"", streamfx::ui::about::role_type::GITHUB_SUPPORTER, "",
							   streamfx::ui::about::link_type::GENERIC, "", "Github",
							   streamfx::ui::about::link_type::NONE, "", ""},
							   */

	// Supporters - Tier 3
	/// https://www.patreon.com/eposvox
	streamfx::ui::about::entry{"EposVox", streamfx::ui::about::role_type::PATREON_SUPPORTER, "",
							   streamfx::ui::about::link_type::SOCIAL_TWITCH, "https://www.twitch.tv/EposVox",
							   text_social_twitch.data(), streamfx::ui::about::link_type::SOCIAL_YOUTUBE,
							   "https://youtube.com/c/EposVox", text_social_youtube.data()},
	/// https://github.com/GranDroidTonight
	streamfx::ui::about::entry{"GranDroidTonight", streamfx::ui::about::role_type::GITHUB_SUPPORTER, "",
							   streamfx::ui::about::link_type::SOCIAL_TWITCH, "https://www.twitch.tv/GranDroidTonight",
							   text_social_twitch.data(), streamfx::ui::about::link_type::SOCIAL_YOUTUBE,
							   "https://youtube.com/channel/UCGoT2XFPpeKaL1QuY_NPDuA", text_social_youtube.data()},
	/// https://github.com/Joefis-x20s
	streamfx::ui::about::entry{"Joefisx20s", streamfx::ui::about::role_type::GITHUB_SUPPORTER, "",
							   streamfx::ui::about::link_type::GENERIC, "https://github.com/Joefis-x20s", "Github",
							   streamfx::ui::about::link_type::NONE, "", ""},

	// Supporters - Tier 2
	/// https://www.patreon.com/user?u=3569213
	streamfx::ui::about::entry{"B B", streamfx::ui::about::role_type::PATREON_SUPPORTER, "",
							   streamfx::ui::about::link_type::GENERIC, "https://www.patreon.com/user?u=3569213",
							   "Patreon", streamfx::ui::about::link_type::NONE, "", ""},
	/// https://github.com/blackmoon1910
	streamfx::ui::about::entry{"blackmoon1910", streamfx::ui::about::role_type::GITHUB_SUPPORTER, "",
							   streamfx::ui::about::link_type::GENERIC, "https://github.com/blackmoon1910", "Github",
							   streamfx::ui::about::link_type::NONE, "", ""},
	/// https://www.patreon.com/user?u=32255073 (chillpanda)
	streamfx::ui::about::entry{"ChillPanda", streamfx::ui::about::role_type::PATREON_SUPPORTER, "",
							   streamfx::ui::about::link_type::GENERIC, "https://www.patreon.com/user?u=32255073",
							   "Patreon", streamfx::ui::about::link_type::SOCIAL_TWITCH,
							   "https://www.twitch.tv/chi11estpanda", text_social_twitch.data()},
	/// https://www.patreon.com/DandiDoesIt
	streamfx::ui::about::entry{"DandiDoesIt", streamfx::ui::about::role_type::PATREON_SUPPORTER, "",
							   streamfx::ui::about::link_type::SOCIAL_TWITCH, "https://www.twitch.tv/DandiDoesIt",
							   text_social_twitch.data(), streamfx::ui::about::link_type::NONE, "", ""},
	/// https://www.patreon.com/user?u=13760765
	streamfx::ui::about::entry{"HoodlumCallum", streamfx::ui::about::role_type::PATREON_SUPPORTER, "",
							   streamfx::ui::about::link_type::SOCIAL_YOUTUBE,
							   "https://www.youtube.com/channel/UC0cTVjYKgAnBrXQKcICyNmA", text_social_youtube.data(),
							   streamfx::ui::about::link_type::SOCIAL_TWITCH, "https://www.twitch.tv/hoodlumcallum",
							   text_social_twitch.data()},
	/// https://www.patreon.com/user?u=5208869
	streamfx::ui::about::entry{
		"KrisCheetah", streamfx::ui::about::role_type::PATREON_SUPPORTER, "", streamfx::ui::about::link_type::GENERIC,
		"https://www.patreon.com/user/creators?u=5208869", "Patreon", streamfx::ui::about::link_type::NONE, "", ""},
	/// https://www.patreon.com/user?u=2382106
	streamfx::ui::about::entry{
		"Sean", streamfx::ui::about::role_type::PATREON_SUPPORTER, "", streamfx::ui::about::link_type::GENERIC,
		"https://www.patreon.com/user/creators?u=2382106", "Patreon", streamfx::ui::about::link_type::NONE, "", ""},
	/// https://github.com/wild-wild-smif
	streamfx::ui::about::entry{"Wild Wild Smif", streamfx::ui::about::role_type::GITHUB_SUPPORTER, "",
							   streamfx::ui::about::link_type::GENERIC, "https://github.com/wild-wild-smif", "Github",
							   streamfx::ui::about::link_type::NONE, "", ""},
	/// https://github.com/xuedi
	streamfx::ui::about::entry{"xuedi", streamfx::ui::about::role_type::GITHUB_SUPPORTER, "",
							   streamfx::ui::about::link_type::GENERIC, "https://github.com/xuedi", "Github",
							   streamfx::ui::about::link_type::NONE, "", ""},
	/// https://www.patreon.com/y0himba
	streamfx::ui::about::entry{"y0himba", streamfx::ui::about::role_type::PATREON_SUPPORTER, "",
							   streamfx::ui::about::link_type::SOCIAL_TWITCH, "https://twitch.tv/y0himba1",
							   text_social_twitch.data(), streamfx::ui::about::link_type::SOCIAL_TWITTER,
							   "https://twitter.com/y0himba", text_social_twitter.data()},

	// Supporters - Tier 1
	/// https://www.patreon.com/benman2785
	streamfx::ui::about::entry{"Benjamin Hoffmeister", streamfx::ui::about::role_type::PATREON_SUPPORTER, "",
							   streamfx::ui::about::link_type::GENERIC, "https://www.patreon.com/benman2785", "Patreon",
							   streamfx::ui::about::link_type::NONE, "", ""},
	/// https://www.patreon.com/user?u=33587406
	streamfx::ui::about::entry{"Hana Pestle", streamfx::ui::about::role_type::PATREON_SUPPORTER, "",
							   streamfx::ui::about::link_type::GENERIC, "https://www.patreon.com/user?u=33587406",
							   "Patreon", streamfx::ui::about::link_type::NONE, "", ""},
	/// https://www.patreon.com/scraplands
	streamfx::ui::about::entry{"iamresist", streamfx::ui::about::role_type::PATREON_SUPPORTER, "",
							   streamfx::ui::about::link_type::SOCIAL_TWITCH, "https://twitch.tv/iamresist",
							   text_social_twitch.data(), streamfx::ui::about::link_type::NONE, "", ""},
	/// https://www.patreon.com/jaid
	streamfx::ui::about::entry{"Jaidchen", streamfx::ui::about::role_type::PATREON_SUPPORTER, "",
							   streamfx::ui::about::link_type::SOCIAL_TWITCH, "https://twitch.tv/Jaidchen",
							   text_social_youtube.data(), streamfx::ui::about::link_type::NONE, "", ""},
	/// https://www.patreon.com/user?u=3214093
	streamfx::ui::about::entry{"JeffCraig", streamfx::ui::about::role_type::PATREON_SUPPORTER, "",
							   streamfx::ui::about::link_type::GENERIC, "https://www.patreon.com/user?u=3214093",
							   "Patreon", streamfx::ui::about::link_type::NONE, "", ""},
	/// https://github.com/LagaV
	streamfx::ui::about::entry{"LagaV", streamfx::ui::about::role_type::GITHUB_SUPPORTER, "",
							   streamfx::ui::about::link_type::GENERIC, "https://github.com/LagaV", "Github",
							   streamfx::ui::about::link_type::NONE, "", ""},
	/// https://www.patreon.com/user?u=283535
	streamfx::ui::about::entry{"MrProducer", streamfx::ui::about::role_type::PATREON_SUPPORTER, "",
							   streamfx::ui::about::link_type::GENERIC, "https://www.patreon.com/user?u=283535",
							   "Patreon", streamfx::ui::about::link_type::NONE, "", ""},
	/// https://www.patreon.com/Nordern
	streamfx::ui::about::entry{"Nordern", streamfx::ui::about::role_type::PATREON_SUPPORTER, "",
							   streamfx::ui::about::link_type::SOCIAL_YOUTUBE, "https://youtube.com/nordern",
							   text_social_youtube.data(), streamfx::ui::about::link_type::SOCIAL_TWITCH,
							   "https://www.twitch.tv/thenordern", text_social_twitch.data()},
	/// https://www.patreon.com/qappz
	streamfx::ui::about::entry{"QappZ", streamfx::ui::about::role_type::PATREON_SUPPORTER, "",
							   streamfx::ui::about::link_type::GENERIC, "https://www.patreon.com/qappz", "Patreon",
							   streamfx::ui::about::link_type::NONE, "", ""},
	/// https://github.com/TheB1gG
	streamfx::ui::about::entry{"TheB1gG", streamfx::ui::about::role_type::GITHUB_SUPPORTER, "",
							   streamfx::ui::about::link_type::GENERIC, "https://github.com/TheB1gG", "Github",
							   streamfx::ui::about::link_type::NONE, "", ""},

	// Supporters - No Tier
	/// https://www.patreon.com/nwgat
	streamfx::ui::about::entry{"nwgat.ninja", streamfx::ui::about::role_type::PATREON_SUPPORTER, "",
							   streamfx::ui::about::link_type::GENERIC, "https://www.patreon.com/nwgat", "Patreon",
							   streamfx::ui::about::link_type::NONE, "", ""},
	/// https://www.patreon.com/user?u=742298
	streamfx::ui::about::entry{"olemars", streamfx::ui::about::role_type::PATREON_SUPPORTER, "",
							   streamfx::ui::about::link_type::GENERIC, "https://www.patreon.com/user?u=742298",
							   "Patreon", streamfx::ui::about::link_type::NONE, "", ""},

	// Separator
	streamfx::ui::about::entry{"", streamfx::ui::about::role_type::THANKYOU, "", streamfx::ui::about::link_type::NONE,
							   "", "", streamfx::ui::about::link_type::NONE, "", ""},

	// Family
	streamfx::ui::about::entry{"Andrea Stenschke", streamfx::ui::about::role_type::FAMILY, "Xaymar",
							   streamfx::ui::about::link_type::NONE, "", "", streamfx::ui::about::link_type::NONE, "",
							   ""},
	streamfx::ui::about::entry{"Carsten Dirks", streamfx::ui::about::role_type::FAMILY, "Xaymar",
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
	streamfx::ui::about::entry{"Christian \"Azekil\" Dirks", streamfx::ui::about::role_type::FAMILY, "Xaymar",
							   streamfx::ui::about::link_type::NONE, "", "", streamfx::ui::about::link_type::NONE, "",
							   ""},

	// Friends
	streamfx::ui::about::entry{"Axelle", streamfx::ui::about::role_type::FRIEND, "Xaymar",
							   streamfx::ui::about::link_type::SOCIAL_TWITCH, "https://www.twitch.tv/axelle123",
							   text_social_twitch.data(), streamfx::ui::about::link_type::SOCIAL_TWITTER,
							   "https://twitter.com/AxellesNobody", text_social_twitter.data()},

	// Separator
	streamfx::ui::about::entry{"", streamfx::ui::about::role_type::THANKYOU, "", streamfx::ui::about::link_type::NONE,
							   "", "", streamfx::ui::about::link_type::NONE, "", ""},

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
	for (auto entry : _entries) {
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
