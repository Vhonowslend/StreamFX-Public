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

#include "ui.hpp"
#include "common.hpp"
#include "strings.hpp"
#include <string_view>
#include "configuration.hpp"
#include "obs/obs-tools.hpp"
#include "plugin.hpp"

#include <obs-frontend-api.h>

// Translation Keys
constexpr std::string_view _i18n_prefix            = "StreamFX::";
constexpr std::string_view _i18n_menu              = "UI.Menu";
constexpr std::string_view _i18n_menu_report_issue = "UI.Menu.ReportIssue";
constexpr std::string_view _i18n_menu_request_help = "UI.Menu.RequestHelp";
constexpr std::string_view _i18n_menu_website      = "UI.Menu.Website";
constexpr std::string_view _i18n_menu_discord      = "UI.Menu.Discord";
constexpr std::string_view _i18n_menu_github       = "UI.Menu.Github";
constexpr std::string_view _i18n_menu_about        = "UI.Menu.About";

// Configuration
constexpr std::string_view _cfg_have_shown_about = "UI.HaveShownAboutStreamFX";

// URLs
constexpr std::string_view _url_report_issue = "https://github.com/Xaymar/obs-StreamFX/issues/new?template=issue.md";
constexpr std::string_view _url_request_help = "https://github.com/Xaymar/obs-StreamFX/issues/new?template=help.md";
constexpr std::string_view _url_website      = "https://streamfx.xaymar.com";
constexpr std::string_view _url_discord      = "https://discordapp.com/invite/DaeJg7M";
constexpr std::string_view _url_github       = "https://github.com/Xaymar/obs-StreamFX";

inline void qt_init_resource()
{
	Q_INIT_RESOURCE(streamfx);
}

inline void qt_cleanup_resource()
{
	Q_CLEANUP_RESOURCE(streamfx);
}

bool streamfx::ui::handler::have_shown_about_streamfx(bool shown)
{
	if (shown) {
		obs_data_set_bool(streamfx::configuration::instance()->get().get(), _cfg_have_shown_about.data(), true);
	}
	if (streamfx::configuration::instance()->is_different_version()) {
		return false;
	} else {
		return obs_data_get_bool(streamfx::configuration::instance()->get().get(), _cfg_have_shown_about.data());
	}
}

streamfx::ui::handler::handler()
	: QObject(), _menu_action(), _menu(),

	  _report_issue(), _request_help(),

	  _link_website(), _link_discord(), _link_github(),

	  _about_action(), _about_dialog()
{
	// Qt Resources and Translators
	qt_init_resource();
	QCoreApplication::installTranslator(new streamfx::ui::translator(this));

	// Handle all frontend events.
	obs_frontend_add_event_callback(frontend_event_handler, this);

	{ // Build StreamFX menu.
		_menu = new QMenu(reinterpret_cast<QWidget*>(obs_frontend_get_main_window()));

		{ // Github Issues
			//_menu->addSeparator();
			_report_issue = _menu->addAction(QString::fromUtf8(D_TRANSLATE(_i18n_menu_report_issue.data())));
			_request_help = _menu->addAction(QString::fromUtf8(D_TRANSLATE(_i18n_menu_request_help.data())));
			connect(_report_issue, &QAction::triggered, this, &streamfx::ui::handler::on_action_report_issue);
			connect(_request_help, &QAction::triggered, this, &streamfx::ui::handler::on_action_request_help);
		}

		{ // Official Links
			_menu->addSeparator();
			_link_website = _menu->addAction(QString::fromUtf8(D_TRANSLATE(_i18n_menu_website.data())));
			_link_discord = _menu->addAction(QString::fromUtf8(D_TRANSLATE(_i18n_menu_discord.data())));
			_link_github  = _menu->addAction(QString::fromUtf8(D_TRANSLATE(_i18n_menu_github.data())));
			connect(_link_website, &QAction::triggered, this, &streamfx::ui::handler::on_action_website);
			connect(_link_discord, &QAction::triggered, this, &streamfx::ui::handler::on_action_discord);
			connect(_link_github, &QAction::triggered, this, &streamfx::ui::handler::on_action_github);
		}

		{ // About StreamFX
			_about_dialog = new streamfx::ui::about();
			_menu->addSeparator();
			_about_action = _menu->addAction(QString::fromUtf8(D_TRANSLATE(_i18n_menu_about.data())));
			connect(_about_action, &QAction::triggered, this, &streamfx::ui::handler::on_action_about);
		}
	}
}

streamfx::ui::handler::~handler()
{
	// Handle all frontend events.
	obs_frontend_remove_event_callback(frontend_event_handler, this);

	// Qt Resources and Translators
	qt_cleanup_resource();
}

void streamfx::ui::handler::frontend_event_handler(obs_frontend_event event, void* private_data)
{
	streamfx::ui::handler* ptr = reinterpret_cast<streamfx::ui::handler*>(private_data);
	switch (event) {
	case OBS_FRONTEND_EVENT_FINISHED_LOADING:
		ptr->on_obs_loaded();
		break;
	default:
		break;
	}
}

void streamfx::ui::handler::on_obs_loaded()
{
	// Add an actual Menu entry.
	QMainWindow* main_widget = reinterpret_cast<QMainWindow*>(obs_frontend_get_main_window());
	_menu_action             = new QAction(main_widget);
	_menu_action->setMenu(_menu);
	_menu_action->setText(QString::fromUtf8(D_TRANSLATE(_i18n_menu.data())));
	main_widget->menuBar()->addAction(_menu_action);

	// Show the 'About StreamFX' dialog if that has not happened yet.
	if (!have_shown_about_streamfx()) {
		// Automatically show it if it has not yet been shown.
		_about_dialog->show();
		have_shown_about_streamfx(true);
	}
}

void streamfx::ui::handler::on_action_report_issue(bool)
{
	QDesktopServices::openUrl(QUrl(QString::fromUtf8(_url_report_issue.data())));
}

void streamfx::ui::handler::on_action_request_help(bool)
{
	QDesktopServices::openUrl(QUrl(QString::fromUtf8(_url_request_help.data())));
}

void streamfx::ui::handler::on_action_website(bool)
{
	QDesktopServices::openUrl(QUrl(QString::fromUtf8(_url_website.data())));
}

void streamfx::ui::handler::on_action_discord(bool)
{
	QDesktopServices::openUrl(QUrl(QString::fromUtf8(_url_discord.data())));
}

void streamfx::ui::handler::on_action_github(bool)
{
	QDesktopServices::openUrl(QUrl(QString::fromUtf8(_url_github.data())));
}

void streamfx::ui::handler::on_action_about(bool checked)
{
	_about_dialog->show();
}

static std::shared_ptr<streamfx::ui::handler> _handler_singleton;

void streamfx::ui::handler::initialize()
{
	_handler_singleton = std::make_shared<streamfx::ui::handler>();
}

void streamfx::ui::handler::finalize()
{
	_handler_singleton.reset();
}

std::shared_ptr<streamfx::ui::handler> streamfx::ui::handler::get()
{
	return _handler_singleton;
}

streamfx::ui::translator::translator(QObject* parent) {}

streamfx::ui::translator::~translator() {}

QString streamfx::ui::translator::translate(const char* context, const char* sourceText, const char* disambiguation,
											int n) const
{
	std::string_view sourceView{sourceText};
	if (sourceView.substr(0, _i18n_prefix.length()) == _i18n_prefix) {
		return QString::fromUtf8(D_TRANSLATE(sourceView.substr(_i18n_prefix.length()).data()));
	}
	return QString();
}
