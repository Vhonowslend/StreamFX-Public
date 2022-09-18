/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2017 Michael Fabian Dirks
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

namespace streamfx {
	// Threadpool
	std::shared_ptr<streamfx::util::threadpool::threadpool> threadpool();

	void gs_draw_fullscreen_tri();

	std::filesystem::path data_file_path(std::string_view file);
	std::filesystem::path config_file_path(std::string_view file);

#ifdef ENABLE_FRONTEND
	bool open_url(std::string_view url);
#endif
} // namespace streamfx
