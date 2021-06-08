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
#include <mutex>
#include "gfx-blur-base.hpp"
#include "obs/gs/gs-effect.hpp"
#include "obs/gs/gs-rendertarget.hpp"
#include "obs/gs/gs-texture.hpp"

namespace gfx {
	namespace blur {
		class box_data {
			streamfx::obs::gs::effect _effect;

			public:
			box_data();
			virtual ~box_data();

			streamfx::obs::gs::effect get_effect();
		};

		class box_factory : public ::gfx::blur::ifactory {
			std::mutex                           _data_lock;
			std::weak_ptr<::gfx::blur::box_data> _data;

			public:
			box_factory();
			virtual ~box_factory() override;

			virtual bool is_type_supported(::gfx::blur::type type) override;

			virtual std::shared_ptr<::gfx::blur::base> create(::gfx::blur::type type) override;

			virtual double_t get_min_size(::gfx::blur::type type) override;

			virtual double_t get_step_size(::gfx::blur::type type) override;

			virtual double_t get_max_size(::gfx::blur::type type) override;

			virtual double_t get_min_angle(::gfx::blur::type type) override;

			virtual double_t get_step_angle(::gfx::blur::type type) override;

			virtual double_t get_max_angle(::gfx::blur::type type) override;

			virtual bool is_step_scale_supported(::gfx::blur::type type) override;

			virtual double_t get_min_step_scale_x(::gfx::blur::type type) override;

			virtual double_t get_step_step_scale_x(::gfx::blur::type type) override;

			virtual double_t get_max_step_scale_x(::gfx::blur::type type) override;

			virtual double_t get_min_step_scale_y(::gfx::blur::type type) override;

			virtual double_t get_step_step_scale_y(::gfx::blur::type type) override;

			virtual double_t get_max_step_scale_y(::gfx::blur::type type) override;

			std::shared_ptr<::gfx::blur::box_data> data();

			public: // Singleton
			static ::gfx::blur::box_factory& get();
		};

		class box : public ::gfx::blur::base {
			protected:
			std::shared_ptr<::gfx::blur::box_data> _data;

			double_t                                           _size;
			std::pair<double_t, double_t>                      _step_scale;
			std::shared_ptr<::streamfx::obs::gs::texture>      _input_texture;
			std::shared_ptr<::streamfx::obs::gs::rendertarget> _rendertarget;

			private:
			std::shared_ptr<::streamfx::obs::gs::rendertarget> _rendertarget2;

			public:
			box();
			virtual ~box() override;

			virtual void set_input(std::shared_ptr<::streamfx::obs::gs::texture> texture) override;

			virtual ::gfx::blur::type get_type() override;

			virtual double_t get_size() override;
			virtual void     set_size(double_t width) override;

			virtual void     set_step_scale(double_t x, double_t y) override;
			virtual void     get_step_scale(double_t& x, double_t& y) override;
			virtual double_t get_step_scale_x() override;
			virtual double_t get_step_scale_y() override;

			virtual std::shared_ptr<::streamfx::obs::gs::texture> render() override;
			virtual std::shared_ptr<::streamfx::obs::gs::texture> get() override;
		};

		class box_directional : public ::gfx::blur::box, public ::gfx::blur::base_angle {
			double_t _angle;

			public:
			box_directional();

			virtual ::gfx::blur::type get_type() override;

			virtual double_t get_angle() override;
			virtual void     set_angle(double_t angle) override;

			virtual std::shared_ptr<::streamfx::obs::gs::texture> render() override;
		};

		class box_rotational : public ::gfx::blur::box,
							   public ::gfx::blur::base_angle,
							   public ::gfx::blur::base_center {
			std::pair<double_t, double_t> _center;
			double_t                      _angle;

			public:
			virtual ::gfx::blur::type get_type() override;

			virtual void set_center(double_t x, double_t y) override;
			virtual void get_center(double_t& x, double_t& y) override;

			virtual double_t get_angle() override;
			virtual void     set_angle(double_t angle) override;

			virtual std::shared_ptr<::streamfx::obs::gs::texture> render() override;
		};

		class box_zoom : public ::gfx::blur::box, public ::gfx::blur::base_center {
			std::pair<double_t, double_t> _center;

			public:
			virtual ::gfx::blur::type get_type() override;

			virtual void set_center(double_t x, double_t y) override;
			virtual void get_center(double_t& x, double_t& y) override;

			virtual std::shared_ptr<::streamfx::obs::gs::texture> render() override;
		};
	} // namespace blur
} // namespace gfx
