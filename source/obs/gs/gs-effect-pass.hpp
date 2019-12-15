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

#include <graphics/graphics.h>

namespace gs {
	class effect_pass : public std::shared_ptr<gs_epass_t> {
		std::shared_ptr<gs_technique_t>* _parent;

		public:
		effect_pass(gs_epass_t* pass, std::shared_ptr<gs_technique_t>* parent = nullptr);
		~effect_pass();

		std::string name();

		//gs::shader get_pixel_shader();
		//gs::shader get_vertex_shader();

		size_t count_vertex_parameters();
		//gs::parameter get_vertex_parameter(size_t idx);
		//gs::parameter get_vertex_parameter(std::string name);
		//bool has_vertex_parameter(std::string name);
		//bool has_vertex_parameter(std::string name, ... type);

		size_t count_pixel_parameters();
		//gs::parameter get_vertex_parameter(size_t idx);
		//gs::parameter get_vertex_parameter(std::string name);
		//bool has_vertex_parameter(std::string name);
		//bool has_vertex_parameter(std::string name, ... type);
	};
} // namespace gs
