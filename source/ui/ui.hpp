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

		// Menu Actions
		QAction* _action_support;
		QAction* _action_wiki;
		QAction* _action_website;
		QAction* _action_discord;
		QAction* _action_twitter;
		QAction* _action_youtube;

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

		// Menu Actions
		void on_action_support(bool);
		void on_action_wiki(bool);
		void on_action_website(bool);
		void on_action_discord(bool);
		void on_action_twitter(bool);
		void on_action_youtube(bool);

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
