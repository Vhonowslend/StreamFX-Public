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
#include <chrono>
#include "ui-about.hpp"
#include "ui-common.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251 4365 4371 4619 4946)
#endif
#include <QMouseEvent>
#include "ui_about-entry.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

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
