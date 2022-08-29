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
#include "common.hpp"
#include "ui-common.hpp"

#include "warning-disable.hpp"
#include "ui_about.h"
#include "warning-enable.hpp"

namespace streamfx::ui {
	class about : public QDialog, public Ui::About {
		Q_OBJECT

		public:
		enum class role_type : int32_t {
			NONE,
			CONTRIBUTOR,
			TRANSLATOR,
			SUPPORTER,
		};

		struct entry {
			std::string          name;
			ui::about::role_type role;
			std::string          role_custom;
			std::string          link;
		};

		public:
		about();
		~about();

		public slots:
		; // Not having this breaks some linters.
		void on_ok();
	};
} // namespace streamfx::ui
