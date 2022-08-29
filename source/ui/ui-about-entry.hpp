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
#include "ui-common.hpp"
#include "ui-about.hpp"

#include "warning-disable.hpp"
#include <chrono>
#include "warning-enable.hpp"

#include "warning-disable.hpp"
#include <QMouseEvent>
#include "ui_about-entry.h"
#include "warning-enable.hpp"

namespace streamfx::ui {
	class about_entry : public QWidget, public Ui::AboutEntry {
		Q_OBJECT

		private:
		QUrl _link;

		public:
		about_entry(QWidget* parent, const ui::about::entry& entry);
		~about_entry();

		protected:
		virtual void mousePressEvent(QMouseEvent* event) override;
	};
} // namespace streamfx::ui
