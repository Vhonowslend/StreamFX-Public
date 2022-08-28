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
#include "ui-common.hpp"
#include "updater.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#endif
#include <QAction>
#include <QActionGroup>
#include <QBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QMessageBox>
#include <QWidget>
#include <QWidgetAction>
#include "ui_updater.h"

Q_DECLARE_METATYPE(::streamfx::version_stage);

#ifdef _MSC_VER
#pragma warning(pop)
#else
#pragma GCC diagnostic pop
#endif

namespace streamfx::ui {
	class updater_dialog : public QDialog, public Ui::Updater {
		Q_OBJECT

		private:
		QUrl _update_url;

		public:
		updater_dialog();
		~updater_dialog();

		void show(streamfx::version_info current, streamfx::version_info update);
		void hide();

		public slots:
		; // Needed by some linters.
		void on_ok();
		void on_cancel();
	};

	class updater : public QObject {
		Q_OBJECT

		private:
		std::shared_ptr<streamfx::updater> _updater;

		updater_dialog* _dialog;

		QMessageBox* _gdpr;

		QAction*      _cfu;
		QAction*      _cfu_auto;
		QAction*      _channel;
		QMenu*        _channel_menu;
		QAction*      _channel_stable;
		QAction*      _channel_candidate;
		QAction*      _channel_beta;
		QAction*      _channel_alpha;
		QActionGroup* _channel_group;

		public:
		updater(QMenu* menu);
		~updater();

		void create_gdpr_box();

		void on_updater_automation_changed(streamfx::updater&, bool);
		void on_updater_channel_changed(streamfx::updater&, streamfx::version_stage);
		void on_updater_refreshed(streamfx::updater&);

		void obs_ready();

		signals:
		; // Needed by some linters.

		void autoupdate_changed(bool);
		void channel_changed(streamfx::version_stage);
		void update_detected();
		void check_active(bool);

		private slots:
		; // Needed by some liners.

		// Internal
		void on_autoupdate_changed(bool);
		void on_channel_changed(streamfx::version_stage);
		void on_update_detected();
		void on_check_active(bool);

		// Qt
		void on_gdpr_button(QAbstractButton*);
		void on_cfu_triggered(bool);
		void on_cfu_auto_toggled(bool);
		void on_channel_group_triggered(QAction*);

		public:
		static std::shared_ptr<streamfx::ui::updater> instance(QMenu* menu = nullptr);
	};
} // namespace streamfx::ui
