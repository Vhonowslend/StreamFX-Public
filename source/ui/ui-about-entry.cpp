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

#include "ui-about-entry.hpp"

constexpr std::string_view i18n_role_contributor       = "UI.About.Role.Contributor";
constexpr std::string_view i18n_role_translator        = "UI.About.Role.Translator";
constexpr std::string_view i18n_role_family            = "UI.About.Role.Family";
constexpr std::string_view i18n_role_friend            = "UI.About.Role.Friend";
constexpr std::string_view i18n_role_supporter_patreon = "UI.About.Role.Supporter.Patreon";
constexpr std::string_view i18n_role_supporter_github  = "UI.About.Role.Supporter.Github";
constexpr std::string_view i18n_role_supporter_twitch  = "UI.About.Role.Supporter.Twitch";
constexpr std::string_view i18n_role_creator           = "UI.About.Role.Creator";

streamfx::ui::about_entry::about_entry(QWidget* parent, streamfx::ui::about::entry& entry)
	: QWidget(parent), _last_click(std::chrono::high_resolution_clock::now()), _link1_url(), _link2_url()
{
	setupUi(this);

	name->setText(QString::fromStdString(entry.name));
	switch (entry.role) {
	case streamfx::ui::about::role_type::NONE:
		title->setText(QString::fromStdString(entry.role_custom));
		break;
	case streamfx::ui::about::role_type::CONTRIBUTOR:
		title->setText(D_TRANSLATE(i18n_role_contributor.data()));
		break;
	case streamfx::ui::about::role_type::TRANSLATOR:
		title->setText(D_TRANSLATE(i18n_role_translator.data()));
		break;
	case streamfx::ui::about::role_type::FAMILY: {
		const char*       txt = D_TRANSLATE(i18n_role_family.data());
		std::vector<char> buf(2048);
		snprintf(buf.data(), buf.size(), txt, entry.role_custom.c_str());
		title->setText(QString::fromUtf8(buf.data()));
		break;
	}
	case streamfx::ui::about::role_type::FRIEND: {
		const char*       txt = D_TRANSLATE(i18n_role_friend.data());
		std::vector<char> buf(2048);
		snprintf(buf.data(), buf.size(), txt, entry.role_custom.c_str());
		title->setText(QString::fromUtf8(buf.data()));
		break;
	}
	case streamfx::ui::about::role_type::PATREON_SUPPORTER:
		title->setText(D_TRANSLATE(i18n_role_supporter_patreon.data()));
		break;
	case streamfx::ui::about::role_type::GITHUB_SUPPORTER:
		title->setText(D_TRANSLATE(i18n_role_supporter_github.data()));
		break;
	case streamfx::ui::about::role_type::TWITCH_SUPPORTER:
		title->setText(D_TRANSLATE(i18n_role_supporter_twitch.data()));
		break;
	case streamfx::ui::about::role_type::CREATOR:
		title->setText(D_TRANSLATE(i18n_role_creator.data()));
		break;
	default:
		break;
	}

	std::tuple<streamfx::ui::about::link_type, std::string, std::string, QPushButton*, QUrl&> els[]{
		{entry.link1_type, entry.link1_address, entry.link1_text, link1, _link1_url},
		{entry.link2_type, entry.link2_address, entry.link2_text, link2, _link2_url},
	};
	for (auto el : els) {
		switch (std::get<0>(el)) {
		case streamfx::ui::about::link_type::NONE:
			std::get<3>(el)->setHidden(true);
			break;
		case streamfx::ui::about::link_type::GENERIC:
			std::get<3>(el)->setIcon(QIcon(":/linktype/generic"));
			break;
		case streamfx::ui::about::link_type::SOCIAL_TWITCH:
			std::get<3>(el)->setIcon(QIcon(":/linktype/twitch"));
			break;
		case streamfx::ui::about::link_type::SOCIAL_YOUTUBE:
			std::get<3>(el)->setIcon(QIcon(":/linktype/youtube"));
			break;
		case streamfx::ui::about::link_type::SOCIAL_DISCORD:
			std::get<3>(el)->setIcon(QIcon(":/linktype/discord"));
			break;
		case streamfx::ui::about::link_type::SOCIAL_TWITTER:
			std::get<3>(el)->setIcon(QIcon(":/linktype/twitter"));
			break;
		case streamfx::ui::about::link_type::SOCIAL_FACEBOOK:
			std::get<3>(el)->setIcon(QIcon(":/linktype/facebook"));
			break;
		default:
			break;
		}
		std::get<3>(el)->setText(QString::fromUtf8(std::get<2>(el).c_str()));
		std::get<4>(el).setUrl(QString::fromStdString(std::get<1>(el)));
	}
	connect(link1, &QPushButton::pressed, this, &streamfx::ui::about_entry::on_link1_clicked);
	connect(link2, &QPushButton::pressed, this, &streamfx::ui::about_entry::on_link2_clicked);

	// Don't free up space when hidden.
	/*if (!(link1->isVisible() || link2->isVisible())) {
		QSizePolicy qsp = link1->sizePolicy();
		qsp.setRetainSizeWhenHidden(true);
		link1->setSizePolicy(qsp);
	}*/
}

streamfx::ui::about_entry::~about_entry() {}

void streamfx::ui::about_entry::on_link1_clicked()
{
	// FIXME! Button clicks twice?
	for (size_t attempt = 0; (attempt < 10) && (!QDesktopServices::openUrl(_link1_url)); attempt++) {
	}
}

void streamfx::ui::about_entry::on_link2_clicked()
{
	for (size_t attempt = 0; (attempt < 10) && (!QDesktopServices::openUrl(_link2_url)); attempt++) {
	}
}
