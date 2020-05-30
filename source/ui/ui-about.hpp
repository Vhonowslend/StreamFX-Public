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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251 4365 4371 4619 4946)
#endif
#include "ui_about.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace streamfx::ui {
	class about : public QDialog, public Ui::About {
		Q_OBJECT

		public:
		enum class role_type : std::int32_t {
			NONE,
			SPACER,
			THANKYOU,
			CONTRIBUTOR,
			TRANSLATOR,
			FAMILY,
			FRIEND,
			PATREON_SUPPORTER,
			GITHUB_SUPPORTER,
			TWITCH_SUPPORTER,
			CREATOR,

		};
		enum class link_type : std::int32_t {
			NONE,
			GENERIC,

			// Social Links
			SOCIAL_TWITCH = 2000,
			SOCIAL_YOUTUBE,
			SOCIAL_DISCORD,
			SOCIAL_TWITTER,
			SOCIAL_FACEBOOK,
		};

		struct entry {
			std::string          name;
			ui::about::role_type role;
			std::string          role_custom;

			ui::about::link_type link1_type;
			std::string          link1_address;
			std::string          link1_text;

			ui::about::link_type link2_type;
			std::string          link2_address;
			std::string          link2_text;
		};

		public:
		about();
		~about();

		public slots:
		; // Not having this breaks some linters.
		void on_ok();
	};
} // namespace streamfx::ui
