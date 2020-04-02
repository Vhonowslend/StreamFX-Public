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
#include "gfx/shader/gfx-shader.hpp"
#include "obs/gs/gs-rendertarget.hpp"
#include "obs/obs-source-factory.hpp"

namespace filter::shader {
	class shader_instance : public obs::source_instance {
		std::shared_ptr<gfx::shader::shader> _fx;
		std::shared_ptr<gs::rendertarget>    _rt;

		public:
		shader_instance(obs_data_t* data, obs_source_t* self);
		virtual ~shader_instance();

		virtual uint32_t get_width() override;
		virtual uint32_t get_height() override;

		void properties(obs_properties_t* props);

		virtual void load(obs_data_t* data) override;
		virtual void update(obs_data_t* data) override;

		virtual void video_tick(float_t sec_since_last) override;
		virtual void video_render(gs_effect_t* effect) override;
	};

	class shader_factory : public obs::source_factory<filter::shader::shader_factory, filter::shader::shader_instance> {
		static std::shared_ptr<filter::shader::shader_factory> factory_instance;

		public: // Singleton
		static void initialize()
		{
			factory_instance = std::make_shared<filter::shader::shader_factory>();
		}

		static void finalize()
		{
			factory_instance.reset();
		}

		static std::shared_ptr<shader_factory> get()
		{
			return factory_instance;
		}

		public:
		shader_factory();
		virtual ~shader_factory();

		virtual const char* get_name() override;

		virtual void get_defaults2(obs_data_t* data) override;

		virtual obs_properties_t* get_properties2(filter::shader::shader_instance* data) override;
	};
} // namespace filter::shader
