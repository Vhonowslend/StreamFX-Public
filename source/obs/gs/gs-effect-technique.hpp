/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2019 Michael Fabian Dirks
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
#include <cinttypes>
#include <memory>
#include <string>
#include "gs-effect-pass.hpp"

// OBS
extern "C" {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <graphics/graphics.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

namespace gs {
	class effect_technique : public std::shared_ptr<gs_technique_t> {
		std::shared_ptr<gs_effect_t>* _parent;

		public:
		effect_technique(gs_technique_t* technique, std::shared_ptr<gs_effect_t>* parent = nullptr);
		~effect_technique();

		std::string name();

		size_t          count_passes();
		gs::effect_pass get_pass(size_t idx);
		gs::effect_pass get_pass(std::string name);
		bool            has_pass(std::string name);
	};
} // namespace gs
