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
	class effect_parameter : protected std::shared_ptr<gs_eparam_t> {
		std::shared_ptr<gs_effect_t>* _effect_parent;
		std::shared_ptr<gs_epass_t>*  _pass_parent;
		std::shared_ptr<gs_eparam_t>* _param_parent;

		public:
		enum class type {
			Unknown,
			Boolean,
			Float,
			Float2,
			Float3,
			Float4,
			Integer,
			Integer2,
			Integer3,
			Integer4,
			Matrix,
			String,
			Texture,

			Invalid = -1,
		};

		public:
		effect_parameter(gs_eparam_t* param);
		effect_parameter(gs_eparam_t* param, std::shared_ptr<gs_effect_t>* parent);
		effect_parameter(gs_eparam_t* param, std::shared_ptr<gs_epass_t>* parent);
		effect_parameter(gs_eparam_t* param, std::shared_ptr<gs_eparam_t>* parent);
		~effect_parameter();

		std::string get_name();

		type get_type();

		size_t                            count_annotations();
		std::shared_ptr<effect_parameter> get_annotation(size_t idx);
		std::shared_ptr<effect_parameter> get_annotation(std::string name);
		bool                              has_annotation(std::string name);
		bool                              has_annotation(std::string name, effect_parameter::type type);
	};
} // namespace gs
