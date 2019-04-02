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
#include <cinttypes>
#include <memory>
#include <mutex>
#include <vector>
#include "gfx-blur-base.hpp"
#include "obs/gs/gs-effect.hpp"
#include "obs/gs/gs-rendertarget.hpp"
#include "obs/gs/gs-texture.hpp"

namespace gfx {
	namespace blur {
		class dual_filtering_data {
			std::shared_ptr<::gs::effect> m_effect;

			public:
			dual_filtering_data();
			~dual_filtering_data();

			std::shared_ptr<::gs::effect> get_effect();
		};

		class dual_filtering_factory : public ::gfx::blur::ifactory {
			std::mutex                                   m_data_lock;
			std::weak_ptr<::gfx::blur::dual_filtering_data> m_data;

			public:
			dual_filtering_factory();
			virtual ~dual_filtering_factory();

			virtual bool is_type_supported(::gfx::blur::type type) override;

			virtual std::shared_ptr<::gfx::blur::ibase> create(::gfx::blur::type type) override;

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

			std::shared_ptr<::gfx::blur::dual_filtering_data> data();

			public: // Singleton
			static ::gfx::blur::dual_filtering_factory& get();
		};

		class dual_filtering : public ::gfx::blur::ibase {
			std::shared_ptr<::gfx::blur::dual_filtering_data> m_data;

			double_t m_size;
			size_t   m_size_iterations;

			std::shared_ptr<gs::texture> m_input_texture;

			std::vector<std::shared_ptr<gs::rendertarget>> m_rendertargets;

			public:
			dual_filtering();
			virtual ~dual_filtering();
		
			virtual void set_input(std::shared_ptr<::gs::texture> texture) override;

			virtual ::gfx::blur::type get_type() override;

			virtual double_t          get_size() override;

			virtual void              set_size(double_t width) override;

			virtual void              set_step_scale(double_t x, double_t y) override;

			virtual void              get_step_scale(double_t& x, double_t& y) override;

			virtual std::shared_ptr<::gs::texture> render() override;

			virtual std::shared_ptr<::gs::texture> get() override;
		};
	}; // namespace blur

}; // namespace gfx
