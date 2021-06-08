/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2018 Michael Fabian Dirks
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

namespace streamfx::obs {
	namespace tools {
		bool scene_contains_source(obs_scene_t* scene, obs_source_t* source);

		bool obs_properties_remove_by_name(obs_properties_t* props, const char* name);

		class child_source {
			obs_source_t*                 _parent;
			std::shared_ptr<obs_source_t> _child;

			public:
			child_source(obs_source_t* parent, std::shared_ptr<obs_source_t> child);
			virtual ~child_source();

			std::shared_ptr<obs_source_t> get();
		};

		// Class to manage
		class active_source {
			obs_source_t* _child;

			public:
			active_source(obs_source_t* child) : _child(child)
			{
				obs_source_inc_active(_child);
			}
			virtual ~active_source()
			{
				obs_source_dec_active(_child);
			}
		};

		class visible_source {
			obs_source_t* _child;

			public:
			visible_source(obs_source_t* child) : _child(child)
			{
				obs_source_inc_showing(_child);
			}
			virtual ~visible_source()
			{
				obs_source_dec_showing(_child);
			}
		};
	} // namespace tools

	inline void obs_source_deleter(obs_source_t* v)
	{
		obs_source_release(v);
	}

	inline void obs_weak_source_deleter(obs_weak_source_t* v)
	{
		obs_weak_source_release(v);
	}

	inline void obs_scene_deleter(obs_scene_t* v)
	{
		obs_scene_release(v);
	}

	inline void obs_sceneitem_releaser(obs_scene_item* v)
	{
		obs_sceneitem_release(v);
	}

	inline void obs_sceneitem_remover(obs_scene_item* v)
	{
		obs_sceneitem_remove(v);
	}

	inline void obs_data_deleter(obs_data_t* v)
	{
		obs_data_release(v);
	}
} // namespace streamfx::obs
