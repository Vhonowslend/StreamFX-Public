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

#pragma once
#include "ui-about.hpp"
#include "ui-common.hpp"

#ifdef ENABLE_UPDATER
#include "ui-updater.hpp"
#endif

namespace streamfx::ui {
	class handler : public QObject {
		Q_OBJECT

		private:
		QAction* _menu_action;
		QMenu*   _menu;

		// Bug Report, Help Request
		QAction* _report_issue;
		QAction* _request_help;

		// Website, Discord, Source
		QAction* _link_website;
		QAction* _link_discord;
		QAction* _link_github;

		// About Dialog
		QAction*   _about_action;
		ui::about* _about_dialog;

		QTranslator* _translator;

#ifdef ENABLE_UPDATER
		std::shared_ptr<streamfx::ui::updater> _updater;
#endif

		public:
		handler();
		~handler();

		bool have_shown_about_streamfx(bool shown = false);

		private:
		static void frontend_event_handler(obs_frontend_event event, void* private_data);

		void on_obs_loaded();
		void on_obs_exit();

		public slots:
		; // Not having this breaks some linters.

		// Issues & Help
		void on_action_report_issue(bool);
		void on_action_request_help(bool);

		// Official Links
		void on_action_website(bool);
		void on_action_discord(bool);
		void on_action_github(bool);

		// About
		void on_action_about(bool);

		public /* Singleton */:
		static void initialize();

		static void finalize();

		static std::shared_ptr<ui::handler> get();
	};

	class translator : public QTranslator {
		public:
		translator(QObject* parent = nullptr);
		~translator();

		virtual QString translate(const char* context, const char* sourceText, const char* disambiguation = nullptr,
								  int n = -1) const override;
	};

} // namespace streamfx::ui
