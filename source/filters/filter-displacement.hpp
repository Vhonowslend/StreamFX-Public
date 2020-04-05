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
#include "obs/gs/gs-effect.hpp"
#include "obs/obs-source-factory.hpp"

namespace streamfx::filter::displacement {
	class displacement_instance : public obs::source_instance {
		gs::effect _effect;

		// Displacement Map
		std::shared_ptr<gs::texture> _texture;
		std::string                  _texture_file;
		float_t                      _scale[2];
		float_t                      _scale_type;

		// Cache
		std::uint32_t _width;
		std::uint32_t _height;

		public:
		displacement_instance(obs_data_t*, obs_source_t*);
		virtual ~displacement_instance();

		virtual void load(obs_data_t* settings) override;
		virtual void migrate(obs_data_t* data, std::uint64_t version) override;
		virtual void update(obs_data_t* settings) override;

		virtual void video_tick(float_t) override;
		virtual void video_render(gs_effect_t*) override;

		std::string get_file();
	};

	class displacement_factory : public obs::source_factory<filter::displacement::displacement_factory,
															filter::displacement::displacement_instance> {
		public:
		displacement_factory();
		virtual ~displacement_factory();

		virtual const char* get_name() override;

		virtual void get_defaults2(obs_data_t* data) override;

		virtual obs_properties_t* get_properties2(filter::displacement::displacement_instance* data) override;

		public: // Singleton
		static void initialize();

		static void finalize();

		static std::shared_ptr<displacement_factory> get();
	};
} // namespace streamfx::filter::displacement
