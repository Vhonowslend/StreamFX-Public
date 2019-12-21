// Modern effects for a modern Streamer
// Copyright (C) 2017 Michael Fabian Dirks
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
#include <map>
#include <memory>
#include <string>
#include "obs/gs/gs-rendertarget.hpp"
#include "obs/gs/gs-texture.hpp"
#include "obs/obs-source.hpp"

// OBS
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <obs.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace gfx {
	class source_texture {
		std::shared_ptr<obs::deprecated_source> _parent;
		std::shared_ptr<obs::deprecated_source> _child;

		std::shared_ptr<gs::rendertarget> _rt;

		source_texture(obs_source_t* parent);

		public:
		~source_texture();
		source_texture(obs_source_t* src, obs_source_t* parent);
		source_texture(const char* name, obs_source_t* parent);
		source_texture(std::string name, obs_source_t* parent);

		source_texture(std::shared_ptr<obs::deprecated_source> child, std::shared_ptr<obs::deprecated_source> parent);
		source_texture(std::shared_ptr<obs::deprecated_source> child, obs_source_t* parent);

		public /*copy*/:
		source_texture(source_texture const& other) = delete;
		source_texture& operator=(source_texture const& other) = delete;

		public /*move*/:
		source_texture(source_texture&& other) = delete;
		source_texture& operator=(source_texture&& other) = delete;

		public:
		std::shared_ptr<gs::texture> render(size_t width, size_t height);

		public: // Unsafe Methods
		void clear();

		obs_source_t* get_object();
		obs_source_t* get_parent();
	};

	class source_texture_factory {
		friend class source_texture;

		std::map<std::shared_ptr<obs_weak_source_t>, std::weak_ptr<source_texture>> _cache;

		public:
		source_texture_factory();
		~source_texture_factory();

		std::shared_ptr<source_texture> get_source_texture(std::shared_ptr<obs_source_t> source);

		protected:
		void free_source_texture(std::shared_ptr<obs_source_t> source);

		private: // Singleton
		static std::shared_ptr<source_texture_factory> factory_instance;

		public: // Singleton
		static void initialize()
		{
			factory_instance = std::make_shared<source_texture_factory>();
		}

		static void finalize()
		{
			factory_instance.reset();
		}

		static std::shared_ptr<source_texture_factory> get()
		{
			return factory_instance;
		}
	};
} // namespace gfx
