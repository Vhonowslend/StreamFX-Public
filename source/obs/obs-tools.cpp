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

#include "obs-tools.hpp"
#include <map>
#include <stdexcept>

struct scs_searchdata {
	obs_source_t*                 source;
	bool                          found = false;
	std::map<obs_source_t*, bool> visited;
};

static bool scs_contains(scs_searchdata& sd, obs_source_t* source);

static void scs_enum_active_cb(obs_source_t*, obs_source_t* child, void* searchdata)
{
	scs_searchdata& sd = reinterpret_cast<scs_searchdata&>(*reinterpret_cast<scs_searchdata*>(searchdata));
	scs_contains(sd, child);
}

static bool scs_enum_items_cb(obs_scene_t*, obs_sceneitem_t* item, void* searchdata)
{
	scs_searchdata& sd     = reinterpret_cast<scs_searchdata&>(*reinterpret_cast<scs_searchdata*>(searchdata));
	obs_source_t*   source = obs_sceneitem_get_source(item);
	return scs_contains(sd, source);
}

static bool scs_contains(scs_searchdata& sd, obs_source_t* source)
{
	if (sd.visited.find(source) != sd.visited.end()) {
		return false;
	} else {
		sd.visited.insert({source, true});
	}

	if (source == sd.source) {
		sd.found = true;
		return true;
	} else {
		if (strcmp(obs_source_get_id(source), "scene")) {
			obs_scene_t* nscene = obs_scene_from_source(source);
			obs_scene_enum_items(nscene, scs_enum_items_cb, &sd);
		} else {
			obs_source_enum_active_sources(source, scs_enum_active_cb, &sd);
		}
	}

	if (sd.found) {
		return false;
	}
	return true;
}

bool obs::tools::scene_contains_source(obs_scene_t* scene, obs_source_t* source)
{
	scs_searchdata sd;
	sd.source = source;
	obs_scene_enum_items(scene, scs_enum_items_cb, &sd);
	return sd.found;
}
