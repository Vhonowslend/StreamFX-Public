/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2018 Michael Fabian Dirks
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

#include "utility.hpp"
#include <sstream>
#include "plugin.hpp"

// OBS
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <obs.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

const char* obs_module_recursive_text(const char* to_translate, size_t depth)
{
	static std::unordered_map<std::string, std::string> translate_map;

	if (depth == 0) {
		return obs_module_text(to_translate);
	}

	std::string key   = to_translate;
	auto        value = translate_map.find(std::string(key));
	if (value != translate_map.end()) {
		return value->second.c_str();
	} else {
		std::string       orig = obs_module_text(to_translate);
		std::stringstream out;

		{
			size_t seq_start, seq_end = 0;
			bool   seq_got = false;

			for (size_t pos = 0; pos <= orig.length(); pos++) {
				std::string chr = orig.substr(pos, 2);
				if (chr == "\\@") {
					if (seq_got) {
						out << obs_module_recursive_text(orig.substr(seq_start, pos - seq_start).c_str(), (depth - 1));
						seq_end = pos + 2;
					} else {
						out << orig.substr(seq_end, pos - seq_end);
						seq_start = pos + 2;
					}
					seq_got = !seq_got;
					pos += 1;
				}
			}
			if (seq_end != orig.length()) {
				out << orig.substr(seq_end, orig.length() - seq_end);
			}

			translate_map.insert({key, out.str()});
		}
				
		auto value = translate_map.find(key);
		if (value != translate_map.end()) {
			return value->second.c_str();
		} else {
			throw std::runtime_error("Insert into map failed.");
		}
	}
}
