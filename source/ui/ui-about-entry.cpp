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

constexpr std::string_view i18n_role_contributor = "UI.About.Role.Contributor";
constexpr std::string_view i18n_role_translator  = "UI.About.Role.Translator";
constexpr std::string_view i18n_role_family      = "UI.About.Role.Family";
constexpr std::string_view i18n_role_friend      = "UI.About.Role.Friend";
constexpr std::string_view i18n_role_supporter   = "UI.About.Role.Supporter";
constexpr std::string_view i18n_role_creator     = "UI.About.Role.Creator";

streamfx::ui::about_entry::about_entry(QWidget* parent, const streamfx::ui::about::entry& entry)
	: QWidget(parent), _link()
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
	case streamfx::ui::about::role_type::SUPPORTER:
		title->setText(D_TRANSLATE(i18n_role_supporter.data()));
		break;
	case streamfx::ui::about::role_type::CREATOR:
		title->setText(D_TRANSLATE(i18n_role_creator.data()));
		break;
	default:
		break;
	}

	if (!entry.link.empty()) {
		this->setCursor(Qt::PointingHandCursor);
		_link = QUrl(QString::fromUtf8(entry.link.c_str()));
	}
}

streamfx::ui::about_entry::~about_entry() {}

void streamfx::ui::about_entry::mousePressEvent(QMouseEvent* event)
{
	if (_link.isEmpty())
		return;

	if (event->button() == Qt::LeftButton) {
		QDesktopServices::openUrl(_link);
	}
}
