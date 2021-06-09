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
#include <vector>
#include "gfx-blur-base.hpp"
#include "obs/gs/gs-effect.hpp"
#include "obs/gs/gs-rendertarget.hpp"
#include "obs/gs/gs-texture.hpp"

namespace streamfx::gfx {
	namespace blur {
		class gaussian_data {
			streamfx::obs::gs::effect            _effect;
			std::map<size_t, std::vector<float>> _kernels;

			public:
			gaussian_data();
			virtual ~gaussian_data();

			streamfx::obs::gs::effect get_effect();

			std::vector<float_t> const& get_kernel(std::size_t width);
		};

		class gaussian_factory : public ::streamfx::gfx::blur::ifactory {
			std::mutex                                          _data_lock;
			std::weak_ptr<::streamfx::gfx::blur::gaussian_data> _data;

			public:
			gaussian_factory();
			virtual ~gaussian_factory() override;

			virtual bool is_type_supported(::streamfx::gfx::blur::type type) override;

			virtual std::shared_ptr<::streamfx::gfx::blur::base> create(::streamfx::gfx::blur::type type) override;

			virtual double_t get_min_size(::streamfx::gfx::blur::type type) override;

			virtual double_t get_step_size(::streamfx::gfx::blur::type type) override;

			virtual double_t get_max_size(::streamfx::gfx::blur::type type) override;

			virtual double_t get_min_angle(::streamfx::gfx::blur::type type) override;

			virtual double_t get_step_angle(::streamfx::gfx::blur::type type) override;

			virtual double_t get_max_angle(::streamfx::gfx::blur::type type) override;

			virtual bool is_step_scale_supported(::streamfx::gfx::blur::type type) override;

			virtual double_t get_min_step_scale_x(::streamfx::gfx::blur::type type) override;

			virtual double_t get_step_step_scale_x(::streamfx::gfx::blur::type type) override;

			virtual double_t get_max_step_scale_x(::streamfx::gfx::blur::type type) override;

			virtual double_t get_min_step_scale_y(::streamfx::gfx::blur::type type) override;

			virtual double_t get_step_step_scale_y(::streamfx::gfx::blur::type type) override;

			virtual double_t get_max_step_scale_y(::streamfx::gfx::blur::type type) override;

			std::shared_ptr<::streamfx::gfx::blur::gaussian_data> data();

			public: // Singleton
			static ::streamfx::gfx::blur::gaussian_factory& get();
		};

		class gaussian : public ::streamfx::gfx::blur::base {
			protected:
			std::shared_ptr<::streamfx::gfx::blur::gaussian_data> _data;

			double_t                                           _size;
			std::pair<double_t, double_t>                      _step_scale;
			std::shared_ptr<::streamfx::obs::gs::texture>      _input_texture;
			std::shared_ptr<::streamfx::obs::gs::rendertarget> _rendertarget;

			private:
			std::shared_ptr<::streamfx::obs::gs::rendertarget> _rendertarget2;

			public:
			gaussian();
			virtual ~gaussian() override;

			virtual void set_input(std::shared_ptr<::streamfx::obs::gs::texture> texture) override;

			virtual ::streamfx::gfx::blur::type get_type() override;

			virtual double_t get_size() override;

			virtual void set_size(double_t width) override;

			virtual void set_step_scale(double_t x, double_t y) override;

			virtual void get_step_scale(double_t& x, double_t& y) override;

			virtual double_t get_step_scale_x() override;

			virtual double_t get_step_scale_y() override;

			virtual std::shared_ptr<::streamfx::obs::gs::texture> render() override;

			virtual std::shared_ptr<::streamfx::obs::gs::texture> get() override;
		};

		class gaussian_directional : public ::streamfx::gfx::blur::gaussian, public ::streamfx::gfx::blur::base_angle {
			double_t m_angle;

			public:
			gaussian_directional();
			virtual ~gaussian_directional() override;

			virtual ::streamfx::gfx::blur::type get_type() override;

			virtual double_t get_angle() override;
			virtual void     set_angle(double_t angle) override;

			virtual std::shared_ptr<::streamfx::obs::gs::texture> render() override;
		};

		class gaussian_rotational : public ::streamfx::gfx::blur::gaussian,
									public ::streamfx::gfx::blur::base_angle,
									public ::streamfx::gfx::blur::base_center {
			std::pair<double_t, double_t> m_center;
			double_t                      m_angle;

			public:
			virtual ::streamfx::gfx::blur::type get_type() override;

			virtual void set_center(double_t x, double_t y) override;
			virtual void get_center(double_t& x, double_t& y) override;

			virtual double_t get_angle() override;
			virtual void     set_angle(double_t angle) override;

			virtual std::shared_ptr<::streamfx::obs::gs::texture> render() override;
		};

		class gaussian_zoom : public ::streamfx::gfx::blur::gaussian, public ::streamfx::gfx::blur::base_center {
			std::pair<double_t, double_t> m_center;

			public:
			virtual ::streamfx::gfx::blur::type get_type() override;

			virtual void set_center(double_t x, double_t y) override;
			virtual void get_center(double_t& x, double_t& y) override;

			virtual std::shared_ptr<::streamfx::obs::gs::texture> render() override;
		};
	} // namespace blur
} // namespace streamfx::gfx
