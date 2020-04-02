// Modern effects for a modern Streamer
// Copyright (C) 2019 Michael Fabian Dirks
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

#pragma once
#include "common.hpp"
#include "obs/gs/gs-texture.hpp"

namespace gfx {
	namespace blur {
		enum class type : int64_t {
			Invalid = -1,
			Area,
			Directional,
			Rotational,
			Zoom,
		};

		class base {
			public:
			virtual ~base() {}

			virtual void set_input(std::shared_ptr<::gs::texture> texture) = 0;

			virtual ::gfx::blur::type get_type() = 0;

			virtual double_t get_size() = 0;

			virtual void set_size(double_t width) = 0;

			virtual void set_step_scale(double_t x, double_t y) = 0;

			virtual void set_step_scale_x(double_t v);

			virtual void set_step_scale_y(double_t v);

			virtual void get_step_scale(double_t& x, double_t& y) = 0;

			virtual double_t get_step_scale_x();

			virtual double_t get_step_scale_y();

			virtual std::shared_ptr<::gs::texture> render() = 0;

			virtual std::shared_ptr<::gs::texture> get() = 0;
		};

		class base_angle {
			public:
			virtual ~base_angle() {}

			virtual double_t get_angle() = 0;

			virtual void set_angle(double_t angle) = 0;
		};

		class base_center {
			public:
			virtual ~base_center() {}

			virtual void set_center(double_t x, double_t y) = 0;

			virtual void set_center_x(double_t v);

			virtual void set_center_y(double_t v);

			virtual void get_center(double_t& x, double_t& y) = 0;

			virtual double_t get_center_x();

			virtual double_t get_center_y();
		};

		class ifactory {
			public:
			virtual ~ifactory() {}

			virtual bool is_type_supported(::gfx::blur::type type) = 0;

			virtual std::shared_ptr<::gfx::blur::base> create(::gfx::blur::type type) = 0;

			virtual double_t get_min_size(::gfx::blur::type type) = 0;

			virtual double_t get_step_size(::gfx::blur::type type) = 0;

			virtual double_t get_max_size(::gfx::blur::type type) = 0;

			virtual double_t get_min_angle(::gfx::blur::type type) = 0;

			virtual double_t get_step_angle(::gfx::blur::type type) = 0;

			virtual double_t get_max_angle(::gfx::blur::type type) = 0;

			virtual bool is_step_scale_supported(::gfx::blur::type type) = 0;

			virtual double_t get_min_step_scale_x(::gfx::blur::type type) = 0;

			virtual double_t get_step_step_scale_x(::gfx::blur::type type) = 0;

			virtual double_t get_max_step_scale_x(::gfx::blur::type type) = 0;

			virtual double_t get_min_step_scale_y(::gfx::blur::type type) = 0;

			virtual double_t get_step_step_scale_y(::gfx::blur::type type) = 0;

			virtual double_t get_max_step_scale_y(::gfx::blur::type type) = 0;
		};
	} // namespace blur
} // namespace gfx
