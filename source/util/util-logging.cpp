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

#include "util-logging.hpp"
#include "common.hpp"
#include <stdarg.h>

void util::logging::log(level lvl, const char* format, ...)
{
	const static std::map<level, int32_t> level_map = {
		{level::LEVEL_DEBUG, LOG_DEBUG},
		{level::LEVEL_INFO, LOG_INFO},
		{level::LEVEL_WARN, LOG_WARNING},
		{level::LEVEL_ERROR, LOG_ERROR},
	};
	thread_local static std::vector<char> buffer;

	va_list vargs;
	va_start(vargs, format);

	int32_t ret = vsnprintf(buffer.data(), buffer.size(), format, vargs);
	buffer.resize(ret + 1);
	ret = vsnprintf(buffer.data(), buffer.size(), format, vargs);

	va_end(vargs);

	blog(level_map.at(lvl), "[StreamFX] %s", buffer.data());
}
