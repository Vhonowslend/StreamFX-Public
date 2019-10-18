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
#include <memory>
#include <vector>
#include "obs/gs/gs-mipmapper.hpp"
#include "obs/gs/gs-rendertarget.hpp"
#include "obs/gs/gs-texture.hpp"
#include "obs/gs/gs-vertexbuffer.hpp"
#include "obs/obs-source-factory.hpp"
#include "plugin.hpp"

namespace filter {
	namespace transform {
		class transform_instance : public obs::source_instance {
			// Input
			std::shared_ptr<gs::rendertarget> _source_rendertarget;
			std::shared_ptr<gs::texture>      _source_texture;
			bool                              _source_rendered;
			std::pair<uint32_t, uint32_t>     _source_size;

			// Mipmapping
			bool                     _mipmap_enabled;
			double_t                 _mipmap_strength;
			gs::mipmapper::generator _mipmap_generator;
			gs::mipmapper            _mipmapper;

			// Rendering
			std::shared_ptr<gs::rendertarget> _shape_rendertarget;
			std::shared_ptr<gs::texture>      _shape_texture;

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

			virtual void update(obs_data_t*) override;
			virtual void video_tick(float) override;
			virtual void video_render(gs_effect_t*) override;
		};

		class transform_factory
			: public obs::source_factory<filter::transform::transform_factory, filter::transform::transform_instance> {
			static std::shared_ptr<filter::transform::transform_factory> factory_instance;

			public: // Singleton
			static void initialize()
			{
				factory_instance = std::make_shared<filter::transform::transform_factory>();
			}

			static void finalize()
			{
				factory_instance.reset();
			}

			static std::shared_ptr<transform_factory> get()
			{
				return factory_instance;
			}

			public:
			transform_factory();
			virtual ~transform_factory() override;

			virtual const char* get_name() override;

			virtual void get_defaults2(obs_data_t* data) override;

			virtual obs_properties_t* get_properties2(filter::transform::transform_instance* data) override;
		};
	} // namespace transform
} // namespace filter
