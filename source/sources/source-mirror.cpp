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

#include "source-mirror.hpp"
#include <bitset>
#include <cstring>
#include <functional>
#include <memory>
#include <stdexcept>
#include <vector>
#include "obs/obs-source-tracker.hpp"
#include "obs/obs-tools.hpp"
#include "strings.hpp"

// OBS
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
#include <media-io/audio-io.h>
#include <obs-config.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#define ST "Source.Mirror"
#define ST_SOURCE "Source.Mirror.Source"
#define ST_SOURCE_SIZE "Source.Mirror.Source.Size"
#define ST_AUDIO "Source.Mirror.Source.Audio"
#define ST_AUDIO_LAYOUT ST ".Audio.Layout"
#define ST_AUDIO_LAYOUT_(x) ST_AUDIO_LAYOUT "." D_VSTR(x)
#define ST_SCALING "Source.Mirror.Scaling"
#define ST_SCALING_METHOD "Source.Mirror.Scaling.Method"
#define ST_SCALING_METHOD_POINT "Source.Mirror.Scaling.Method.Point"
#define ST_SCALING_METHOD_BILINEAR "Source.Mirror.Scaling.Method.Bilinear"
#define ST_SCALING_METHOD_BICUBIC "Source.Mirror.Scaling.Method.Bicubic"
#define ST_SCALING_METHOD_LANCZOS "Source.Mirror.Scaling.Method.Lanczos"
#define ST_SCALING_SIZE "Source.Mirror.Scaling.Size"
#define ST_SCALING_TRANSFORMKEEPORIGINAL "Source.Mirror.Scaling.TransformKeepOriginal"
#define ST_SCALING_BOUNDS "Source.Mirror.Scaling.Bounds"
#define ST_SCALING_BOUNDS_STRETCH "Source.Mirror.Scaling.Bounds.Stretch"
#define ST_SCALING_BOUNDS_FIT "Source.Mirror.Scaling.Bounds.Fit"
#define ST_SCALING_BOUNDS_FILL "Source.Mirror.Scaling.Bounds.Fill"
#define ST_SCALING_BOUNDS_FILLWIDTH "Source.Mirror.Scaling.Bounds.FillWidth"
#define ST_SCALING_BOUNDS_FILLHEIGHT "Source.Mirror.Scaling.Bounds.FillHeight"
#define ST_SCALING_ALIGNMENT "Source.Mirror.Scaling.Alignment"

static std::shared_ptr<source::mirror::mirror_factory> factory_instance = nullptr;

void source::mirror::mirror_factory::initialize()
{
	factory_instance = std::make_shared<source::mirror::mirror_factory>();
}

void source::mirror::mirror_factory::finalize()
{
	factory_instance.reset();
}

std::shared_ptr<source::mirror::mirror_factory> source::mirror::mirror_factory::get()
{
	return factory_instance;
}

source::mirror::mirror_factory::mirror_factory()
{
	memset(&_source_info, 0, sizeof(obs_source_info));
	_source_info.id           = "obs-stream-effects-source-mirror";
	_source_info.type         = OBS_SOURCE_TYPE_INPUT;
	_source_info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_AUDIO | OBS_SOURCE_CUSTOM_DRAW;

	_source_info.get_name            = get_name;
	_source_info.get_defaults        = get_defaults;
	_source_info.get_properties      = get_properties;
	_source_info.get_width           = get_width;
	_source_info.get_height          = get_height;
	_source_info.create              = create;
	_source_info.destroy             = destroy;
	_source_info.update              = update;
	_source_info.activate            = activate;
	_source_info.deactivate          = deactivate;
	_source_info.video_tick          = video_tick;
	_source_info.video_render        = video_render;
	_source_info.enum_active_sources = enum_active_sources;
	_source_info.load                = load;
	_source_info.save                = save;

	obs_register_source(&_source_info);
}

source::mirror::mirror_factory::~mirror_factory() {}

const char* source::mirror::mirror_factory::get_name(void*) noexcept try {
	return D_TRANSLATE(ST);
} catch (...) {
	P_LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void source::mirror::mirror_factory::get_defaults(obs_data_t* data) noexcept try {
	obs_data_set_default_string(data, ST_SOURCE, "");
	obs_data_set_default_bool(data, ST_AUDIO, false);
	obs_data_set_default_int(data, ST_AUDIO_LAYOUT, static_cast<int64_t>(SPEAKERS_UNKNOWN));
	obs_data_set_default_bool(data, ST_SCALING, false);
	obs_data_set_default_string(data, ST_SCALING_SIZE, "100x100");
	obs_data_set_default_int(data, ST_SCALING_METHOD, (int64_t)obs_scale_type::OBS_SCALE_BILINEAR);
	obs_data_set_default_bool(data, ST_SCALING_TRANSFORMKEEPORIGINAL, false);
	obs_data_set_default_int(data, ST_SCALING_BOUNDS, (int64_t)obs_bounds_type::OBS_BOUNDS_STRETCH);
	obs_data_set_default_int(data, ST_SCALING_ALIGNMENT, OBS_ALIGN_CENTER);
} catch (...) {
	P_LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

bool source::mirror::mirror_factory::modified_properties(obs_properties_t* pr, obs_property_t* p,
														 obs_data_t* data) noexcept try {
	if (obs_properties_get(pr, ST_SOURCE) == p) {
		obs_source_t* target = obs_get_source_by_name(obs_data_get_string(data, ST_SOURCE));
		if (target) {
			std::vector<char> buf(256);
			snprintf(buf.data(), buf.size(), "%" PRIu32 "x%" PRIu32, obs_source_get_width(target),
					 obs_source_get_height(target));
			obs_data_set_string(data, ST_SOURCE_SIZE, buf.data());
		} else {
			obs_data_set_string(data, ST_SOURCE_SIZE, "0x0");
		}
		obs_source_release(target);
	}

	if (obs_properties_get(pr, ST_AUDIO) == p) {
		bool show = obs_data_get_bool(data, ST_AUDIO);
		obs_property_set_visible(obs_properties_get(pr, ST_AUDIO_LAYOUT), show);
		return true;
	}

	if (obs_properties_get(pr, ST_SCALING) == p) {
		bool show = obs_data_get_bool(data, ST_SCALING);
		obs_property_set_visible(obs_properties_get(pr, ST_SCALING_METHOD), show);
		obs_property_set_visible(obs_properties_get(pr, ST_SCALING_SIZE), show);
		obs_property_set_visible(obs_properties_get(pr, ST_SCALING_BOUNDS), show);
		obs_property_set_visible(obs_properties_get(pr, ST_SCALING_ALIGNMENT), show);
		return true;
	}

	if (obs_properties_get(pr, ST_SCALING_BOUNDS) == p) {
		obs_bounds_type scaling_type = static_cast<obs_bounds_type>(obs_data_get_int(data, ST_SCALING_BOUNDS));
		obs_property_t* p2           = obs_properties_get(pr, ST_SCALING_BOUNDS);
		switch (scaling_type) {
		case obs_bounds_type::OBS_BOUNDS_STRETCH:
			obs_property_set_long_description(p2, D_TRANSLATE(D_DESC(ST_SCALING_BOUNDS_STRETCH)));
			break;
		case obs_bounds_type::OBS_BOUNDS_SCALE_INNER:
			obs_property_set_long_description(p2, D_TRANSLATE(D_DESC(ST_SCALING_BOUNDS_FIT)));
			break;
		case obs_bounds_type::OBS_BOUNDS_SCALE_OUTER:
			obs_property_set_long_description(p2, D_TRANSLATE(D_DESC(ST_SCALING_BOUNDS_FILL)));
			break;
		case obs_bounds_type::OBS_BOUNDS_SCALE_TO_WIDTH:
			obs_property_set_long_description(p2, D_TRANSLATE(D_DESC(ST_SCALING_BOUNDS_FILLWIDTH)));
			break;
		case obs_bounds_type::OBS_BOUNDS_SCALE_TO_HEIGHT:
			obs_property_set_long_description(p2, D_TRANSLATE(D_DESC(ST_SCALING_BOUNDS_FILLHEIGHT)));
			break;
		default:
			obs_property_set_long_description(p2, D_TRANSLATE(D_DESC(ST_SCALING_BOUNDS)));
			break;
		}
		return true;
	}

	return false;
} catch (...) {
	P_LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
	return false;
}

obs_properties_t* source::mirror::mirror_factory::get_properties(void*) noexcept try {
	obs_properties_t* pr = obs_properties_create();
	obs_property_t*   p  = nullptr;

	p = obs_properties_add_list(pr, ST_SOURCE, D_TRANSLATE(ST_SOURCE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SOURCE)));
	obs_property_set_modified_callback(p, modified_properties);
	obs_property_list_add_string(p, "", "");
	obs::source_tracker::get()->enumerate(
		[&p](std::string name, obs_source_t*) {
			obs_property_list_add_string(p, std::string(name + " (Source)").c_str(), name.c_str());
			return false;
		},
		obs::source_tracker::filter_sources);
	obs::source_tracker::get()->enumerate(
		[&p](std::string name, obs_source_t*) {
			obs_property_list_add_string(p, std::string(name + " (Scene)").c_str(), name.c_str());
			return false;
		},
		obs::source_tracker::filter_scenes);

	p = obs_properties_add_text(pr, ST_SOURCE_SIZE, D_TRANSLATE(ST_SOURCE_SIZE), OBS_TEXT_DEFAULT);
	obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SOURCE_SIZE)));
	obs_property_set_enabled(p, false);

	p = obs_properties_add_bool(pr, ST_AUDIO, D_TRANSLATE(ST_AUDIO));
	obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_AUDIO)));
	obs_property_set_modified_callback(p, modified_properties);
	p = obs_properties_add_list(pr, ST_AUDIO_LAYOUT, D_TRANSLATE(ST_AUDIO_LAYOUT), OBS_COMBO_TYPE_LIST,
								OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, D_TRANSLATE(ST_AUDIO_LAYOUT_(Unknown)), static_cast<int64_t>(SPEAKERS_UNKNOWN));
	obs_property_list_add_int(p, D_TRANSLATE(ST_AUDIO_LAYOUT_(Mono)), static_cast<int64_t>(SPEAKERS_MONO));
	obs_property_list_add_int(p, D_TRANSLATE(ST_AUDIO_LAYOUT_(Stereo)), static_cast<int64_t>(SPEAKERS_STEREO));
	obs_property_list_add_int(p, D_TRANSLATE(ST_AUDIO_LAYOUT_(StereoLFE)), static_cast<int64_t>(SPEAKERS_2POINT1));
	obs_property_list_add_int(p, D_TRANSLATE(ST_AUDIO_LAYOUT_(Quadraphonic)), static_cast<int64_t>(SPEAKERS_4POINT0));
	obs_property_list_add_int(p, D_TRANSLATE(ST_AUDIO_LAYOUT_(QuadraphonicLFE)),
							  static_cast<int64_t>(SPEAKERS_4POINT1));
	obs_property_list_add_int(p, D_TRANSLATE(ST_AUDIO_LAYOUT_(Surround)), static_cast<int64_t>(SPEAKERS_5POINT1));
	obs_property_list_add_int(p, D_TRANSLATE(ST_AUDIO_LAYOUT_(FullSurround)), static_cast<int64_t>(SPEAKERS_7POINT1));
	obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_AUDIO_LAYOUT)));

	p = obs_properties_add_bool(pr, ST_SCALING, D_TRANSLATE(ST_SCALING));
	obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SCALING)));
	obs_property_set_modified_callback(p, modified_properties);

	p = obs_properties_add_list(pr, ST_SCALING_METHOD, D_TRANSLATE(ST_SCALING_METHOD), OBS_COMBO_TYPE_LIST,
								OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SCALING_METHOD)));
	obs_property_set_modified_callback(p, modified_properties);
	obs_property_list_add_int(p, D_TRANSLATE(ST_SCALING_METHOD_POINT), (int64_t)obs_scale_type::OBS_SCALE_POINT);
	obs_property_list_add_int(p, D_TRANSLATE(ST_SCALING_METHOD_BILINEAR), (int64_t)obs_scale_type::OBS_SCALE_BILINEAR);
	obs_property_list_add_int(p, D_TRANSLATE(ST_SCALING_METHOD_BICUBIC), (int64_t)obs_scale_type::OBS_SCALE_BICUBIC);
	obs_property_list_add_int(p, D_TRANSLATE(ST_SCALING_METHOD_LANCZOS), (int64_t)obs_scale_type::OBS_SCALE_LANCZOS);

	p = obs_properties_add_text(pr, ST_SCALING_SIZE, D_TRANSLATE(ST_SCALING_SIZE), OBS_TEXT_DEFAULT);
	obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SCALING_SIZE)));

	p = obs_properties_add_bool(pr, ST_SCALING_TRANSFORMKEEPORIGINAL, D_TRANSLATE(ST_SCALING_TRANSFORMKEEPORIGINAL));
	obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SCALING_TRANSFORMKEEPORIGINAL)));

	p = obs_properties_add_list(pr, ST_SCALING_BOUNDS, D_TRANSLATE(ST_SCALING_BOUNDS), OBS_COMBO_TYPE_LIST,
								OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SCALING_BOUNDS)));
	obs_property_set_modified_callback(p, modified_properties);
	obs_property_list_add_int(p, D_TRANSLATE(ST_SCALING_BOUNDS_STRETCH), (int64_t)obs_bounds_type::OBS_BOUNDS_STRETCH);
	obs_property_list_add_int(p, D_TRANSLATE(ST_SCALING_BOUNDS_FIT), (int64_t)obs_bounds_type::OBS_BOUNDS_SCALE_INNER);
	obs_property_list_add_int(p, D_TRANSLATE(ST_SCALING_BOUNDS_FILL), (int64_t)obs_bounds_type::OBS_BOUNDS_SCALE_OUTER);
	obs_property_list_add_int(p, D_TRANSLATE(ST_SCALING_BOUNDS_FILLWIDTH),
							  (int64_t)obs_bounds_type::OBS_BOUNDS_SCALE_TO_WIDTH);
	obs_property_list_add_int(p, D_TRANSLATE(ST_SCALING_BOUNDS_FILLHEIGHT),
							  (int64_t)obs_bounds_type::OBS_BOUNDS_SCALE_TO_HEIGHT);

	p = obs_properties_add_list(pr, ST_SCALING_ALIGNMENT, D_TRANSLATE(ST_SCALING_ALIGNMENT), OBS_COMBO_TYPE_LIST,
								OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_SCALING_ALIGNMENT)));
	obs_property_list_add_int(p, obs_module_recursive_text("\\@" S_ALIGNMENT_LEFT "\\@ \\@" S_ALIGNMENT_TOP "\\@"),
							  OBS_ALIGN_LEFT | OBS_ALIGN_TOP);
	obs_property_list_add_int(p, obs_module_recursive_text("\\@" S_ALIGNMENT_TOP "\\@"), OBS_ALIGN_TOP);
	obs_property_list_add_int(p, obs_module_recursive_text("\\@" S_ALIGNMENT_RIGHT "\\@ \\@" S_ALIGNMENT_TOP "\\@"),
							  OBS_ALIGN_RIGHT | OBS_ALIGN_TOP);
	obs_property_list_add_int(p, obs_module_recursive_text("\\@" S_ALIGNMENT_LEFT "\\@"), OBS_ALIGN_LEFT);
	obs_property_list_add_int(p, obs_module_recursive_text("\\@" S_ALIGNMENT_CENTER "\\@"), OBS_ALIGN_CENTER);
	obs_property_list_add_int(p, obs_module_recursive_text("\\@" S_ALIGNMENT_RIGHT "\\@"), OBS_ALIGN_RIGHT);
	obs_property_list_add_int(p, obs_module_recursive_text("\\@" S_ALIGNMENT_LEFT "\\@ \\@" S_ALIGNMENT_BOTTOM "\\@"),
							  OBS_ALIGN_LEFT | OBS_ALIGN_BOTTOM);
	obs_property_list_add_int(p, obs_module_recursive_text("\\@" S_ALIGNMENT_BOTTOM "\\@"), OBS_ALIGN_BOTTOM);
	obs_property_list_add_int(p, obs_module_recursive_text("\\@" S_ALIGNMENT_RIGHT "\\@ \\@" S_ALIGNMENT_BOTTOM "\\@"),
							  OBS_ALIGN_RIGHT | OBS_ALIGN_BOTTOM);

	return pr;
} catch (...) {
	P_LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
	return nullptr;
}

void* source::mirror::mirror_factory::create(obs_data_t* data, obs_source_t* source) noexcept try {
	return new source::mirror::mirror_instance(data, source);
} catch (...) {
	P_LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
	return nullptr;
}

void source::mirror::mirror_factory::destroy(void* p) noexcept try {
	if (p) {
		delete static_cast<source::mirror::mirror_instance*>(p);
	}
} catch (...) {
	P_LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

uint32_t source::mirror::mirror_factory::get_width(void* p) noexcept try {
	if (p) {
		return static_cast<source::mirror::mirror_instance*>(p)->get_width();
	}
	return 0;
} catch (...) {
	P_LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
	return 0;
}

uint32_t source::mirror::mirror_factory::get_height(void* p) noexcept try {
	if (p) {
		return static_cast<source::mirror::mirror_instance*>(p)->get_height();
	}
	return 0;
} catch (...) {
	P_LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
	return 0;
}

void source::mirror::mirror_factory::update(void* p, obs_data_t* data) noexcept try {
	if (p) {
		static_cast<source::mirror::mirror_instance*>(p)->update(data);
	}
} catch (...) {
	P_LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void source::mirror::mirror_factory::activate(void* p) noexcept try {
	if (p) {
		static_cast<source::mirror::mirror_instance*>(p)->activate();
	}
} catch (...) {
	P_LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void source::mirror::mirror_factory::deactivate(void* p) noexcept try {
	if (p) {
		static_cast<source::mirror::mirror_instance*>(p)->deactivate();
	}
} catch (...) {
	P_LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void source::mirror::mirror_factory::video_tick(void* p, float t) noexcept try {
	if (p) {
		static_cast<source::mirror::mirror_instance*>(p)->video_tick(t);
	}
} catch (...) {
	P_LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void source::mirror::mirror_factory::video_render(void* p, gs_effect_t* ef) noexcept try {
	if (p) {
		static_cast<source::mirror::mirror_instance*>(p)->video_render(ef);
	}
} catch (...) {
	P_LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void source::mirror::mirror_factory::enum_active_sources(void* p, obs_source_enum_proc_t enum_callback,
														 void* param) noexcept try {
	if (p) {
		static_cast<source::mirror::mirror_instance*>(p)->enum_active_sources(enum_callback, param);
	}
} catch (...) {
	P_LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void source::mirror::mirror_factory::load(void* p, obs_data_t* d) noexcept try {
	if (p) {
		static_cast<source::mirror::mirror_instance*>(p)->load(d);
	}
} catch (...) {
	P_LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void source::mirror::mirror_factory::save(void* p, obs_data_t* d) noexcept try {
	if (p) {
		static_cast<source::mirror::mirror_instance*>(p)->save(d);
	}
} catch (...) {
	P_LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void source::mirror::mirror_instance::release_input()
{
	// Clear any references to the previous source.
	if (this->_source_item) {
		obs_sceneitem_remove(this->_source_item);
		this->_source_item = nullptr;
	}
	this->_source.reset();
}

void source::mirror::mirror_instance::acquire_input(std::string source_name)
{
	// Acquire new reference to current source.
	obs_source_t* ref_source = obs_get_source_by_name(source_name.c_str());
	if (!ref_source) {
		// Early-Exit: Unable to find a source with this name, likely has been released.
#ifdef _DEBUG
		P_LOG_DEBUG("<Source Mirror:%s> Unable to find target source '%s'.", obs_source_get_name(this->_self),
					source_name.c_str());
#endif
		return;
	} else if (ref_source == this->_self) {
		// Early-Exit: Attempted self-mirror (recursion).
#ifdef _DEBUG
		P_LOG_DEBUG("<Source Mirror:%s> Attempted to mirror _self.", obs_source_get_name(this->_self));
#endif
		obs_source_release(ref_source);
		return;
	}

	std::shared_ptr<obs::source> new_source = std::make_shared<obs::source>(ref_source, true, false);

	// It looks like everything is in order, so continue now.
	this->_source_item = obs_scene_add(obs_scene_from_source(this->_scene->get()), new_source->get());
	if (!this->_source_item) {
		// Late-Exit: OBS detected something bad, so no further action will be taken.
#ifdef _DEBUG
		P_LOG_DEBUG("<Source Mirror:%s> Attempted recursion with scene '%s'.", obs_source_get_name(this->_self),
					source_name.c_str());
#endif
		return;
	}

	// If everything worked fine, we now set everything up.
	this->_source = new_source;
	this->_source->events.rename += std::bind(&source::mirror::mirror_instance::on_source_rename, this,
											  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	if ((obs_source_get_output_flags(this->_source->get()) & OBS_SOURCE_AUDIO) != 0) {
		this->_source->events.audio_data +=
			std::bind(&source::mirror::mirror_instance::on_audio_data, this, std::placeholders::_1,
					  std::placeholders::_2, std::placeholders::_3);
	}
}

source::mirror::mirror_instance::mirror_instance(obs_data_t*, obs_source_t* src)
	: _self(src), _active(true), _tick(0), _scene_rendered(false), _rescale_enabled(false), _rescale_width(1),
	  _rescale_height(1), _rescale_keep_orig_size(false), _rescale_type(obs_scale_type::OBS_SCALE_BICUBIC),
	  _rescale_bounds(obs_bounds_type::OBS_BOUNDS_STRETCH), _audio_enabled(false), _audio_kill_thread(false),
	  _audio_have_output(false), _source_item(nullptr)
{
	// Initialize Video Rendering
	this->_scene =
		std::make_shared<obs::source>(obs_scene_get_source(obs_scene_create_private("Source Mirror Internal Scene")));
	this->_scene_texture_renderer =
		std::make_shared<gfx::source_texture>(this->_scene, std::make_shared<obs::source>(this->_self, false, false));

	// Initialize Audio Rendering
	this->_audio_thread = std::thread(std::bind(&source::mirror::mirror_instance::audio_output_cb, this));
}

source::mirror::mirror_instance::~mirror_instance()
{
	release_input();

	// Finalize Audio Rendering
	this->_audio_kill_thread = true;
	this->_audio_notify.notify_all();
	if (this->_audio_thread.joinable()) {
		this->_audio_thread.join();
	}

	// Finalize Video Rendering
	this->_scene_texture_renderer.reset();
	this->_scene.reset();
}

uint32_t source::mirror::mirror_instance::get_width()
{
	if (_source) {
		if ((obs_source_get_output_flags(this->_source->get()) & OBS_SOURCE_VIDEO) == 0) {
			return 0;
		}
		if (this->_rescale_enabled && this->_rescale_width > 0 && !this->_rescale_keep_orig_size) {
			return this->_rescale_width;
		} else {
			return _source->width();
		}
	}
	return 0;
}

uint32_t source::mirror::mirror_instance::get_height()
{
	if (_source) {
		if ((obs_source_get_output_flags(this->_source->get()) & OBS_SOURCE_VIDEO) == 0) {
			return 0;
		}
		if (this->_rescale_enabled && this->_rescale_height > 0 && !this->_rescale_keep_orig_size) {
			return this->_rescale_height;
		} else {
			return _source->height();
		}
	}
	return 0;
}

void source::mirror::mirror_instance::update(obs_data_t* data)
{
	{ // User changed the source we are tracking.
		release_input();
		this->_source_name = obs_data_get_string(data, ST_SOURCE);
		if (this->_source_name.length() > 0) {
			acquire_input(this->_source_name);
		}
	}

	// Audio
	this->_audio_enabled = obs_data_get_bool(data, ST_AUDIO);
	this->_audio_layout  = static_cast<speaker_layout>(obs_data_get_int(data, ST_AUDIO_LAYOUT));

	// Rescaling
	this->_rescale_enabled = obs_data_get_bool(data, ST_SCALING);
	if (this->_rescale_enabled) { // Parse rescaling settings.
		uint32_t width, height;

		// Read value.
		const char* size = obs_data_get_string(data, ST_SCALING_SIZE);
		const char* xpos = strchr(size, 'x');
		if (xpos != nullptr) {
			// Width
			width = strtoul(size, nullptr, 10);
			if (errno == ERANGE || width == 0) {
				this->_rescale_enabled = false;
				this->_rescale_width   = 1;
			} else {
				this->_rescale_width = width;
			}

			height = strtoul(xpos + 1, nullptr, 10);
			if (errno == ERANGE || height == 0) {
				this->_rescale_enabled = false;
				this->_rescale_height  = 1;
			} else {
				this->_rescale_height = height;
			}
		} else {
			this->_rescale_enabled = false;
			this->_rescale_width   = 1;
			this->_rescale_height  = 1;
		}

		this->_rescale_keep_orig_size = obs_data_get_bool(data, ST_SCALING_TRANSFORMKEEPORIGINAL);
		this->_rescale_type           = static_cast<obs_scale_type>(obs_data_get_int(data, ST_SCALING_METHOD));
		this->_rescale_bounds         = static_cast<obs_bounds_type>(obs_data_get_int(data, ST_SCALING_BOUNDS));
		this->_rescale_alignment      = static_cast<int32_t>(obs_data_get_int(data, ST_SCALING_ALIGNMENT));
	}
}

void source::mirror::mirror_instance::activate()
{
	this->_active = true;

	// No source, delayed acquire.
	if (!this->_source_item && this->_source_name.length() > 0) {
		this->acquire_input(this->_source_name.c_str());
	}
}

void source::mirror::mirror_instance::deactivate()
{
	this->_active = false;
}

void source::mirror::mirror_instance::video_tick(float time)
{
	this->_tick += time;
	if (this->_tick > 0.1f) {
		this->_tick -= 0.1f;

		// No source, delayed acquire.
		if (!this->_source_item && this->_source_name.length() > 0) {
			this->acquire_input(this->_source_name.c_str());
		}
	}

	// Update Scene Item Boundaries
	if ((this->_source_item && this->_source)
		&& ((obs_source_get_output_flags(this->_source->get()) & OBS_SOURCE_VIDEO) != 0)) {
		obs_transform_info info;
		info.pos.x            = 0;
		info.pos.y            = 0;
		info.rot              = 0;
		info.scale.x          = 1.f;
		info.scale.y          = 1.f;
		info.alignment        = OBS_ALIGN_LEFT | OBS_ALIGN_TOP;
		info.bounds.x         = float_t(this->get_width());
		info.bounds.y         = float_t(this->get_height());
		info.bounds_alignment = _rescale_alignment;
		info.bounds_type      = obs_bounds_type::OBS_BOUNDS_STRETCH;
		if (this->_rescale_enabled) {
			info.bounds_type = this->_rescale_bounds;
		}
		obs_sceneitem_set_info(this->_source_item, &info);
		obs_sceneitem_force_update_transform(this->_source_item);
		obs_sceneitem_set_scale_filter(this->_source_item,
									   this->_rescale_enabled ? this->_rescale_type : obs_scale_type::OBS_SCALE_POINT);
	}

	_scene_rendered = false;
}

void source::mirror::mirror_instance::video_render(gs_effect_t* effect)
{
	if ((this->_rescale_width == 0) || (this->_rescale_height == 0) || !this->_source_item
		|| !this->_scene_texture_renderer || !this->_source) {
		return;
	}

	// Don't bother rendering sources that aren't video.
	if ((obs_source_get_output_flags(this->_source->get()) & OBS_SOURCE_VIDEO) == 0) {
		return;
	}

	// Only re-render the scene if there was a video_tick, saves GPU cycles.
	if (!_scene_rendered) {
		// Override render size if rescaling is enabled.
		uint32_t render_width  = this->_source->width();
		uint32_t render_height = this->_source->height();
		if (_rescale_enabled) {
			render_width  = _rescale_width;
			render_height = _rescale_height;
		}

		if (!render_width || !render_height) {
			// Don't render if width or height are 0.
			return;
		}

		try {
			_scene_texture  = this->_scene_texture_renderer->render(render_width, render_height);
			_scene_rendered = true;
		} catch (...) {
			// If we fail to render the source, just render nothing.
			return;
		}
	}

	if (_scene_texture) {
		// Use default effect unless we are provided a different effect.
		if (!effect) {
			effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);
		}

		// Render the cached scene texture.
		gs_effect_set_texture(gs_effect_get_param_by_name(effect, "image"), _scene_texture->get_object());
		while (gs_effect_loop(effect, "Draw")) {
			gs_draw_sprite(_scene_texture->get_object(), 0, this->get_width(), this->get_height());
		}
	}
}

void source::mirror::mirror_instance::audio_output_cb() noexcept try
{
	std::unique_lock<std::mutex> ulock(this->_audio_lock_outputter);

	while (!this->_audio_kill_thread) {
		this->_audio_notify.wait(ulock, [this]() { return this->_audio_have_output || this->_audio_kill_thread; });

		if (this->_audio_have_output) { // Get used audio element
			std::shared_ptr<mirror_audio_data> mad;
			{
				std::lock_guard<std::mutex> capture_lock(this->_audio_lock_capturer);
				if (_audio_data_queue.size() > 0) {
					mad = _audio_data_queue.front();
					_audio_data_queue.pop();
				}
				if (_audio_data_queue.size() == 0) {
					this->_audio_have_output = false;
				}
			}

			if (mad) {
				ulock.unlock();
				obs_source_output_audio(this->_self, &mad->audio);
				ulock.lock();

				{
					std::lock_guard<std::mutex> capture_lock(this->_audio_lock_capturer);
					_audio_data_free_queue.push(mad);
				}
			}
		}
	}
} catch (...) {
	P_LOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
}

void source::mirror::mirror_instance::enum_active_sources(obs_source_enum_proc_t enum_callback, void* param)
{
	if (this->_scene) {
		enum_callback(this->_self, this->_scene->get(), param);
	}
}

void source::mirror::mirror_instance::load(obs_data_t* data)
{
	this->update(data);
}

void source::mirror::mirror_instance::save(obs_data_t* data)
{
	if (!this->_source_item || !this->_source) {
		return;
	}
	obs_data_set_string(data, ST_SOURCE, obs_source_get_name(_source->get()));
}

void source::mirror::mirror_instance::on_source_rename(obs::source* source, std::string, std::string)
{
	obs_data_t* ref = obs_source_get_settings(this->_self);
	obs_data_set_string(ref, ST_SOURCE, obs_source_get_name(source->get()));
	obs_source_update(this->_self, ref);
	obs_data_release(ref);
}

void source::mirror::mirror_instance::on_audio_data(obs::source*, const audio_data* audio, bool)
{
	if (!this->_audio_enabled) {
		return;
	}

	audio_t* aud = obs_get_audio();
	if (!aud) {
		return;
	}
	audio_output_info const* aoi = audio_output_get_info(aud);
	if (!aoi) {
		return;
	}

	std::shared_ptr<mirror_audio_data> mad;
	{ // Get free audio data element.
		std::lock_guard<std::mutex> capture_lock(this->_audio_lock_capturer);
		if (_audio_data_free_queue.size() > 0) {
			mad = _audio_data_free_queue.front();
			_audio_data_free_queue.pop();
		} else {
			mad = std::make_shared<mirror_audio_data>();
			mad->data.resize(MAX_AUDIO_CHANNELS);
			for (size_t idx = 0; idx < mad->data.size(); idx++) {
				mad->data[idx].resize(AUDIO_OUTPUT_FRAMES);
			}
		}
	}

	{ // Copy data
		std::bitset<8> layout;
		for (size_t plane = 0; plane < MAX_AV_PLANES; plane++) {
			float* samples = reinterpret_cast<float_t*>(audio->data[plane]);
			if (!samples) {
				mad->audio.data[plane] = nullptr;
				continue;
			}
			layout.set(plane);

			memcpy(mad->data[plane].data(), audio->data[plane], audio->frames * sizeof(float_t));
			mad->audio.data[plane] = reinterpret_cast<uint8_t*>(mad->data[plane].data());
		}
		mad->audio.format          = aoi->format;
		mad->audio.frames          = audio->frames;
		mad->audio.timestamp       = audio->timestamp;
		mad->audio.samples_per_sec = aoi->samples_per_sec;
		if (this->_audio_layout != SPEAKERS_UNKNOWN) {
			mad->audio.speakers = this->_audio_layout;
		} else {
			mad->audio.speakers = aoi->speakers;
		}
	}

	{ // Push used audio data element.
		std::lock_guard<std::mutex> capture_lock(this->_audio_lock_capturer);
		_audio_data_queue.push(mad);
	}

	{ // Signal other side.
		std::lock_guard<std::mutex> output_lock(this->_audio_lock_outputter);
		this->_audio_have_output = true;
	}
	this->_audio_notify.notify_all();
}
