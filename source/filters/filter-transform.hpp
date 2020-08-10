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
#include <vector>
#include "obs/gs/gs-mipmapper.hpp"
#include "obs/gs/gs-rendertarget.hpp"
#include "obs/gs/gs-texture.hpp"
#include "obs/gs/gs-vertexbuffer.hpp"
#include "obs/obs-source-factory.hpp"

namespace streamfx::filter::transform {
	class transform_instance : public obs::source_instance {
		// Cache
		bool                              _cache_rendered;
		std::shared_ptr<gs::rendertarget> _cache_rt;
		std::shared_ptr<gs::texture>      _cache_texture;

		// Mip-mapping
		bool                         _mipmap_enabled;
		bool                         _mipmap_rendered;
		gs::mipmapper                _mipmapper;
		std::shared_ptr<gs::texture> _mipmap_texture;

		// Input
		bool                              _source_rendered;
		std::pair<uint32_t, uint32_t>     _source_size;
		std::shared_ptr<gs::rendertarget> _source_rt;
		std::shared_ptr<gs::texture>      _source_texture;

		// Mesh
		bool                               _update_mesh;
		std::shared_ptr<gs::vertex_buffer> _vertex_buffer;
		uint32_t                           _rotation_order;
		std::unique_ptr<util::vec3a>       _position;
		std::unique_ptr<util::vec3a>       _rotation;
		std::unique_ptr<util::vec3a>       _scale;
		std::unique_ptr<util::vec3a>       _shear;

		// Camera
		bool    _camera_orthographic;
		float_t _camera_fov;

		public:
		transform_instance(obs_data_t*, obs_source_t*);
		virtual ~transform_instance() override;

		virtual void load(obs_data_t* settings) override;
		virtual void migrate(obs_data_t* data, uint64_t version) override;
		virtual void update(obs_data_t*) override;

		virtual void video_tick(float_t) override;
		virtual void video_render(gs_effect_t*) override;
	};

	class transform_factory
		: public obs::source_factory<filter::transform::transform_factory, filter::transform::transform_instance> {
		public:
		transform_factory();
		virtual ~transform_factory() override;

		virtual const char* get_name() override;

		virtual void get_defaults2(obs_data_t* data) override;

		virtual obs_properties_t* get_properties2(filter::transform::transform_instance* data) override;

		public: // Singleton
		static void initialize();

		static void finalize();

		static std::shared_ptr<transform_factory> get();
	};
} // namespace streamfx::filter::transform
