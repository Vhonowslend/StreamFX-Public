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
#include "plugin.hpp"

// OBS
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <obs-properties.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

struct scs_searchdata {
	obs_source_t*                 source;
	bool                          found = false;
	std::map<obs_source_t*, bool> visited;
};

static bool scs_contains(scs_searchdata& sd, obs_source_t* source);

static void scs_enum_active_cb(obs_source_t*, obs_source_t* child, void* searchdata) noexcept
try {
	scs_searchdata& sd = reinterpret_cast<scs_searchdata&>(*reinterpret_cast<scs_searchdata*>(searchdata));
	scs_contains(sd, child);
} catch (...) {
	LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

static bool scs_enum_items_cb(obs_scene_t*, obs_sceneitem_t* item, void* searchdata) noexcept
try {
	scs_searchdata& sd     = reinterpret_cast<scs_searchdata&>(*reinterpret_cast<scs_searchdata*>(searchdata));
	obs_source_t*   source = obs_sceneitem_get_source(item);
	return scs_contains(sd, source);
} catch (...) {
	LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
	return false;
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

extern "C" {
struct _hack_obs_properties;

struct _hack_obs_property {
	char*                  name;
	char*                  desc;
	char*                  long_desc;
	void*                  priv;
	enum obs_property_type type;
	bool                   visible;
	bool                   enabled;

	struct _hack_obs_properties* parent;

	obs_property_modified_t  modified;
	obs_property_modified2_t modified2;

	struct _hack_obs_property* next;
};

struct _hack_obs_properties {
	void* param;
	void (*destroy)(void* param);
	uint32_t flags;

	struct _hack_obs_property*  first_property;
	struct _hack_obs_property** last;
	struct _hack_obs_property*  parent;
};
}

bool obs::tools::obs_properties_remove_by_name(obs_properties_t* props, const char* name)
{
	// Due to a bug in obs_properties_remove_by_name, calling it on the first or last element of a group corrupts the
	// obs_properties_t's first and last pointers, which now point at nonsense.
	//
	// There are two ways to work around this issue for now:
	// 1. Add some invisible properties to the beginning and end of the list, ensuring that you never hit the first or
	//    last element with a obs_properties_remove_by_name.
	// 2. Manually adjust the pointers using a dirty hack like in gs::mipmapper.
	// I've opted for the 2nd way, at it is way simpler to implement.

	// Assume that this is fixed in libobs 24.0.7 or newer.
	if (obs_get_version() >= MAKE_SEMANTIC_VERSION(24, 0, 7)) {
		::obs_properties_remove_by_name(props, name);
		return true;
	}

	auto rprops = reinterpret_cast<_hack_obs_properties*>(props);
	auto rprop  = reinterpret_cast<_hack_obs_property*>(obs_properties_get(props, name));

	for (_hack_obs_property *el_prev = rprops->first_property, *el_cur = el_prev; el_cur != nullptr;
		 el_prev = el_cur, el_cur = el_cur->next) {
		if (strcmp(el_cur->name, name) == 0) {
			// Store some information.
			_hack_obs_property* next     = el_cur->next;
			bool                is_first = (rprops->first_property == el_cur);
			bool                is_last  = (rprops->last == &el_cur->next);
			bool                is_solo  = (el_cur == el_prev);

			// Call the real one which fixes the element pointer and deallocates the element.
			::obs_properties_remove_by_name(props, name);

			// Fix up the memory pointers after the element was deleted.
			if (is_last) {
				if (is_solo) {
					rprops->last = &rprops->first_property;
				} else {
					rprops->last = &el_prev->next;
				}
			}
			if (is_first) {
				rprops->first_property = next;
			}

			// Finally break out as we no longer have to process the properties list.
			return true;
		}

		if (el_cur->type == OBS_PROPERTY_GROUP) {
			if (obs::tools::obs_properties_remove_by_name(
					obs_property_group_content(reinterpret_cast<obs_property_t*>(el_cur)), name))
				return true;
		}
	}

	return false;
}
