
#pragma once
#include "ui-obs-browser-widget.hpp"

#include "warning-disable.hpp"
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include "warning-enable.hpp"

namespace streamfx::ui {
	class setup : public QDialog {
		Q_OBJECT;

		QLabel* _header_logo;
		QLabel* _header_website;
		QLabel* _header_patreon;
		QLabel* _header_discord;

		QLabel*  _banner_title;
		QWidget* _banner_content;

		QPushButton* _control_cancel;
		QPushButton* _control_prev;
		QPushButton* _control_next;
		QPushButton* _control_finish;

		QLabel* _info_version;

		public:
		setup(QWidget* parent = nullptr);
		virtual ~setup();

		void retranslate();

		Q_SIGNALS:

		protected Q_SLOTS:
	};
} // namespace streamfx::ui
