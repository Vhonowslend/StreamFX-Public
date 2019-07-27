/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2019 Michael Fabian Dirks
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

#include "filter-dynamic-mask.hpp"
#include <sstream>
#include "strings.hpp"

// Filter to allow dynamic masking
// Allow any channel to affect any other channel
//
// Red/Green/Blue/Alpha Mask Input
// - Red Mask Output
// - Blue Mask Output
// - Green Mask Output
// - Alpha Mask Output

#define SOURCE_NAME "Filter.DynamicMask"

#define S_INPUT "Filter.DynamicMask.Input"
#define S_CHANNEL "Filter.DynamicMask.Channel"
#define S_CHANNEL_VALUE "Filter.DynamicMask.Channel.Value"
#define S_CHANNEL_MULTIPLIER "Filter.DynamicMask.Channel.Multiplier"
#define S_CHANNEL_INPUT "Filter.DynamicMask.Channel.Input"

static std::pair<filter::dynamic_mask::channel, const char*> channel_translations[] = {
	{filter::dynamic_mask::channel::Red, S_CHANNEL_RED},
	{filter::dynamic_mask::channel::Green, S_CHANNEL_GREEN},
	{filter::dynamic_mask::channel::Blue, S_CHANNEL_BLUE},
	{filter::dynamic_mask::channel::Alpha, S_CHANNEL_ALPHA},
};

INITIALIZER(FilterDynamicMaskInit)
{
	initializerFunctions.push_back([] { filter::dynamic_mask::dynamic_mask_factory::initialize(); });
	finalizerFunctions.push_back([] { filter::dynamic_mask::dynamic_mask_factory::finalize(); });
}

static std::shared_ptr<filter::dynamic_mask::dynamic_mask_factory> factory_instance = nullptr;

void filter::dynamic_mask::dynamic_mask_factory::initialize()
{
	factory_instance = std::make_shared<filter::dynamic_mask::dynamic_mask_factory>();
}

void filter::dynamic_mask::dynamic_mask_factory::finalize()
{
	factory_instance.reset();
}

std::shared_ptr<filter::dynamic_mask::dynamic_mask_factory> filter::dynamic_mask::dynamic_mask_factory::get()
{
	return factory_instance;
}

filter::dynamic_mask::dynamic_mask_factory::dynamic_mask_factory()
{
	memset(&sourceInfo, 0, sizeof(obs_source_info));
	sourceInfo.id           = "obs-stream-effects-filter-dynamic-mask";
	sourceInfo.type         = OBS_SOURCE_TYPE_FILTER;
	sourceInfo.output_flags = OBS_SOURCE_VIDEO;

	sourceInfo.get_name = [](void*) { return P_TRANSLATE(SOURCE_NAME); };

	sourceInfo.create = [](obs_data_t* settings, obs_source_t* source) {
		return reinterpret_cast<void*>(new filter::dynamic_mask::dynamic_mask_instance(settings, source));
	};
	sourceInfo.destroy = [](void* _ptr) {
		delete reinterpret_cast<filter::dynamic_mask::dynamic_mask_instance*>(_ptr);
	};

	sourceInfo.get_defaults2 = [](void*, obs_data_t* settings) {
		obs_data_set_default_int(settings, S_CHANNEL, static_cast<int64_t>(channel::Red));
		for (auto kv : channel_translations) {
			obs_data_set_default_double(settings, (std::string(S_CHANNEL_VALUE) + "." + kv.second).c_str(), 1.0);
			obs_data_set_default_double(settings, (std::string(S_CHANNEL_MULTIPLIER) + "." + kv.second).c_str(), 1.0);
			for (auto kv2 : channel_translations) {
				obs_data_set_default_double(
					settings, (std::string(S_CHANNEL_INPUT) + "." + kv.second + "." + kv2.second).c_str(), 0.0);
			}
		}
	};
	sourceInfo.get_properties2 = [](void* _ptr, void* _type_data_ptr) {
		obs_properties_t* props = obs_properties_create_param(_type_data_ptr, nullptr);
		reinterpret_cast<filter::dynamic_mask::dynamic_mask_instance*>(_ptr)->get_properties(props);
		return props;
	};
	sourceInfo.update = [](void* _ptr, obs_data_t* settings) {
		reinterpret_cast<filter::dynamic_mask::dynamic_mask_instance*>(_ptr)->update(settings);
	};
	sourceInfo.load = [](void* _ptr, obs_data_t* settings) {
		reinterpret_cast<filter::dynamic_mask::dynamic_mask_instance*>(_ptr)->load(settings);
	};
	sourceInfo.save = [](void* _ptr, obs_data_t* settings) {
		reinterpret_cast<filter::dynamic_mask::dynamic_mask_instance*>(_ptr)->save(settings);
	};

	sourceInfo.video_tick = [](void* _ptr, float_t _seconds) {
		reinterpret_cast<filter::dynamic_mask::dynamic_mask_instance*>(_ptr)->video_tick(_seconds);
	};
	sourceInfo.video_render = [](void* _ptr, gs_effect_t* _effect) {
		reinterpret_cast<filter::dynamic_mask::dynamic_mask_instance*>(_ptr)->video_render(_effect);
	};

	obs_register_source(&sourceInfo);
}

filter::dynamic_mask::dynamic_mask_factory::~dynamic_mask_factory() {}

filter::dynamic_mask::dynamic_mask_instance::dynamic_mask_instance(obs_data_t* data, obs_source_t* self) : self(self)
{
	this->update(data);

	this->filter_rt = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
	this->final_rt  = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);

	{
		char* file = obs_module_file("effects/channel-mask.effect");
		try {
			this->effect = std::make_shared<gs::effect>(file);
		} catch (std::exception& ex) {
			P_LOG_ERROR("Loading channel mask effect failed with error(s):\n%s", ex.what());
		}
		assert(this->effect != nullptr);
		bfree(file);
	}
}

filter::dynamic_mask::dynamic_mask_instance::~dynamic_mask_instance() {}

uint32_t filter::dynamic_mask::dynamic_mask_instance::get_width()
{
	return 0;
}

uint32_t filter::dynamic_mask::dynamic_mask_instance::get_height()
{
	return 0;
}

void filter::dynamic_mask::dynamic_mask_instance::get_properties(obs_properties_t* properties)
{
	obs_property_t* p;

	this->translation_map.clear();

	{
		p = obs_properties_add_list(properties, S_INPUT, P_TRANSLATE(S_INPUT), OBS_COMBO_TYPE_LIST,
									OBS_COMBO_FORMAT_STRING);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(S_INPUT)));
		obs_property_list_add_string(p, "", "");
		obs::source_tracker::get()->enumerate(
			[&p](std::string name, obs_source_t*) {
				std::stringstream sstr;
				sstr << name << " (" << P_TRANSLATE(S_SOURCETYPE_SOURCE) << ")";
				obs_property_list_add_string(p, sstr.str().c_str(), name.c_str());
				return false;
			},
			obs::source_tracker::filter_video_sources);
		obs::source_tracker::get()->enumerate(
			[&p](std::string name, obs_source_t*) {
				std::stringstream sstr;
				sstr << name << " (" << P_TRANSLATE(S_SOURCETYPE_SCENE) << ")";
				obs_property_list_add_string(p, sstr.str().c_str(), name.c_str());
				return false;
			},
			obs::source_tracker::filter_scenes);
	}

	{
		p = obs_properties_add_list(properties, S_CHANNEL, P_TRANSLATE(S_CHANNEL), OBS_COMBO_TYPE_LIST,
									OBS_COMBO_FORMAT_INT);
		obs_property_set_long_description(p, P_TRANSLATE(P_DESC(S_CHANNEL)));
		for (auto kv : channel_translations) {
			obs_property_list_add_int(p, P_TRANSLATE(kv.second), static_cast<int64_t>(kv.first));
		}
		obs_property_set_modified_callback2(p, modified, this);

		for (auto kv : channel_translations) {
			std::string color = P_TRANSLATE(kv.second);

			{
				std::string       _chv = P_TRANSLATE(S_CHANNEL_VALUE);
				std::vector<char> _chv_data(_chv.size() * 2 + color.size() * 2, '\0');
				sprintf_s(_chv_data.data(), _chv_data.size(), _chv.c_str(), color.c_str());
				auto _chv_key = std::tuple{kv.first, channel::Invalid, std::string(S_CHANNEL_VALUE)};
				translation_map.insert({_chv_key, std::string(_chv_data.begin(), _chv_data.end())});
				auto        chv     = translation_map.find(_chv_key);
				std::string chv_key = std::string(S_CHANNEL_VALUE) + "." + kv.second;

				p = obs_properties_add_float_slider(properties, chv_key.c_str(), chv->second.c_str(), -100.0, 100.0,
													0.01);
				obs_property_set_long_description(p, P_TRANSLATE(P_DESC(S_CHANNEL_VALUE)));

				std::string       _chm = P_TRANSLATE(S_CHANNEL_MULTIPLIER);
				std::vector<char> _chm_data(_chm.size() * 2 + color.size() * 2, '\0');
				sprintf_s(_chm_data.data(), _chm_data.size(), _chm.c_str(), color.c_str());
				auto _chm_key = std::tuple{kv.first, channel::Invalid, std::string(S_CHANNEL_MULTIPLIER)};
				translation_map.insert({_chm_key, std::string(_chm_data.begin(), _chm_data.end())});
				auto        chm     = translation_map.find(_chm_key);
				std::string chm_key = std::string(S_CHANNEL_MULTIPLIER) + "." + kv.second;

				p = obs_properties_add_float_slider(properties, chm_key.c_str(), chm->second.c_str(), -100.0, 100.0,
													0.01);
				obs_property_set_long_description(p, P_TRANSLATE(P_DESC(S_CHANNEL_MULTIPLIER)));
			}
		}
	}

	{
		for (auto kv1 : channel_translations) {
			std::string color1 = P_TRANSLATE(kv1.second);
			for (auto kv2 : channel_translations) {
				std::string color2 = P_TRANSLATE(kv2.second);

				std::string       _chm = P_TRANSLATE(S_CHANNEL_INPUT);
				std::vector<char> _chm_data(_chm.size() * 2 + color1.size() * 2 + color2.size() * 2, '\0');
				sprintf_s(_chm_data.data(), _chm_data.size(), _chm.c_str(), color1.c_str(), color2.c_str());
				auto _chm_key = std::tuple{kv1.first, kv2.first, std::string(S_CHANNEL_INPUT)};
				translation_map.insert({_chm_key, std::string(_chm_data.begin(), _chm_data.end())});
				auto        chm     = translation_map.find(_chm_key);
				std::string chm_key = std::string(S_CHANNEL_INPUT) + "." + kv1.second + "." + kv2.second;

				p = obs_properties_add_float_slider(properties, chm_key.c_str(), chm->second.c_str(), -100.0, 100.0,
													0.01);
				obs_property_set_long_description(p, P_TRANSLATE(P_DESC(S_CHANNEL_INPUT)));
			}
		}
	}
}

void filter::dynamic_mask::dynamic_mask_instance::update(obs_data_t* settings)
{
	// Update source.
	try {
		this->input         = std::make_shared<obs::source>(obs_data_get_string(settings, S_INPUT));
		this->input_capture = std::make_shared<gfx::source_texture>(this->input, self);
		this->input->events.rename += std::bind(&filter::dynamic_mask::dynamic_mask_instance::input_renamed, this,
												std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	} catch (...) {
		this->input.reset();
		this->input_capture.reset();
		this->input_texture.reset();
	}

	// Update data store
	for (auto kv1 : channel_translations) {
		auto found = this->channels.find(kv1.first);
		if (found == this->channels.end()) {
			this->channels.insert({kv1.first, channel_data()});
			found = this->channels.find(kv1.first);
			if (found == this->channels.end()) {
				assert(found != this->channels.end());
				throw std::exception("Unable to insert element into data store.");
			}
		}

		std::string chv_key = std::string(S_CHANNEL_VALUE) + "." + kv1.second;
		found->second.value = static_cast<float_t>(obs_data_get_double(settings, chv_key.c_str()));
		this->precalc.base.ptr[static_cast<size_t>(kv1.first)] = found->second.value;

		std::string chm_key = std::string(S_CHANNEL_MULTIPLIER) + "." + kv1.second;
		found->second.scale = static_cast<float_t>(obs_data_get_double(settings, chm_key.c_str()));
		this->precalc.scale.ptr[static_cast<size_t>(kv1.first)] = found->second.scale;

		vec4* ch = nullptr;
		switch (kv1.first) {
		case channel::Red:
			ch = &this->precalc.matrix.x;
			break;
		case channel::Green:
			ch = &this->precalc.matrix.y;
			break;
		case channel::Blue:
			ch = &this->precalc.matrix.z;
			break;
		case channel::Alpha:
			ch = &this->precalc.matrix.t;
			break;
		}

		for (auto kv2 : channel_translations) {
			std::string ab_key = std::string(S_CHANNEL_INPUT) + "." + kv1.second + "." + kv2.second;
			found->second.values.ptr[static_cast<size_t>(kv2.first)] =
				static_cast<float_t>(obs_data_get_double(settings, ab_key.c_str()));
			ch->ptr[static_cast<size_t>(kv2.first)] = found->second.values.ptr[static_cast<size_t>(kv2.first)];
		}
	}
}

void filter::dynamic_mask::dynamic_mask_instance::load(obs_data_t* settings)
{
	update(settings);
}

void filter::dynamic_mask::dynamic_mask_instance::save(obs_data_t* settings)
{
	if (this->input) {
		obs_data_set_string(settings, S_INPUT, obs_source_get_name(this->input->get()));
	}

	for (auto kv1 : channel_translations) {
		auto found = this->channels.find(kv1.first);
		if (found == this->channels.end()) {
			this->channels.insert({kv1.first, channel_data()});
			found = this->channels.find(kv1.first);
			if (found == this->channels.end()) {
				assert(found != this->channels.end());
				throw std::exception("Unable to insert element into data store.");
			}
		}

		std::string chv_key = std::string(S_CHANNEL_VALUE) + "." + kv1.second;
		obs_data_set_double(settings, chv_key.c_str(), static_cast<double_t>(found->second.value));

		std::string chm_key = std::string(S_CHANNEL_MULTIPLIER) + "." + kv1.second;
		obs_data_set_double(settings, chm_key.c_str(), static_cast<double_t>(found->second.scale));

		for (auto kv2 : channel_translations) {
			std::string ab_key = std::string(S_CHANNEL_INPUT) + "." + kv1.second + "." + kv2.second;
			obs_data_set_double(settings, ab_key.c_str(),
								static_cast<double_t>(found->second.values.ptr[static_cast<size_t>(kv2.first)]));
		}
	}
}

void filter::dynamic_mask::dynamic_mask_instance::input_renamed(obs::source*, std::string old_name,
																std::string new_name)
{
	obs_data_t* settings = obs_source_get_settings(self);
	obs_data_set_string(settings, S_INPUT, new_name.c_str());
	obs_source_update(self, settings);
}

bool filter::dynamic_mask::dynamic_mask_instance::modified(void*, obs_properties_t* properties, obs_property_t*,
														   obs_data_t* settings)
{
	channel mask = static_cast<channel>(obs_data_get_int(settings, S_CHANNEL));

	for (auto kv1 : channel_translations) {
		std::string chv_key = std::string(S_CHANNEL_VALUE) + "." + kv1.second;
		obs_property_set_visible(obs_properties_get(properties, chv_key.c_str()), (mask == kv1.first));
		std::string chm_key = std::string(S_CHANNEL_MULTIPLIER) + "." + kv1.second;
		obs_property_set_visible(obs_properties_get(properties, chm_key.c_str()), (mask == kv1.first));

		for (auto kv2 : channel_translations) {
			std::string io_key = std::string(S_CHANNEL_INPUT) + "." + kv1.second + "." + kv2.second;
			obs_property_set_visible(obs_properties_get(properties, io_key.c_str()), (mask == kv1.first));
		}
	}

	return true;
}

void filter::dynamic_mask::dynamic_mask_instance::video_tick(float)
{
	have_input_texture  = false;
	have_filter_texture = false;
	have_final_texture  = false;
}

void filter::dynamic_mask::dynamic_mask_instance::video_render(gs_effect_t* in_effect)
{
	obs_source_t* parent = obs_filter_get_parent(this->self);
	obs_source_t* target = obs_filter_get_target(this->self);
	uint32_t      width  = obs_source_get_base_width(target);
	uint32_t      height = obs_source_get_base_height(target);

	if (!self || !parent || !target || !width || !height || !input || !input_capture || !effect) {
		obs_source_skip_video_filter(self);
		return;
	} else if (!input->width() || !input->height()) {
		obs_source_skip_video_filter(self);
		return;
	}

	gs_effect_t* default_effect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_DEFAULT);

	try { // Capture filter and input
		if (!have_filter_texture) {
			if (obs_source_process_filter_begin(this->self, GS_RGBA, OBS_ALLOW_DIRECT_RENDERING)) {
				auto op = this->filter_rt->render(width, height);

				gs_blend_state_push();
				gs_reset_blend_state();
				gs_enable_blending(false);
				gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);

				gs_set_cull_mode(GS_NEITHER);
				gs_enable_color(true, true, true, true);

				gs_enable_depth_test(false);
				gs_depth_function(GS_ALWAYS);

				gs_enable_stencil_test(false);
				gs_enable_stencil_write(false);
				gs_stencil_function(GS_STENCIL_BOTH, GS_ALWAYS);
				gs_stencil_op(GS_STENCIL_BOTH, GS_KEEP, GS_KEEP, GS_KEEP);
				gs_ortho(0, (float)width, 0, (float)height, -1., 1.);

				obs_source_process_filter_end(this->self, default_effect, width, height);

				gs_blend_state_pop();
			} else {
				throw std::exception("Failed to render filter.");
			}

			this->filter_texture      = this->filter_rt->get_texture();
			this->have_filter_texture = true;
		}

		if (!have_input_texture) {
			this->input_texture      = this->input_capture->render(input->width(), input->height());
			this->have_input_texture = true;
		}

		// Draw source
		if (!this->have_final_texture) {
			{
				auto op = this->final_rt->render(width, height);

				gs_blend_state_push();
				gs_reset_blend_state();
				gs_enable_blending(false);
				gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);

				gs_set_cull_mode(GS_NEITHER);
				gs_enable_color(true, true, true, true);

				gs_enable_depth_test(false);
				gs_depth_function(GS_ALWAYS);

				gs_enable_stencil_test(false);
				gs_enable_stencil_write(false);
				gs_stencil_function(GS_STENCIL_BOTH, GS_ALWAYS);
				gs_stencil_op(GS_STENCIL_BOTH, GS_KEEP, GS_KEEP, GS_KEEP);
				gs_ortho(0, (float)width, 0, (float)height, -1., 1.);

				this->effect->get_parameter("pMaskInputA").set_texture(this->filter_texture);
				this->effect->get_parameter("pMaskInputB").set_texture(this->input_texture);

				this->effect->get_parameter("pMaskBase").set_float4(this->precalc.base);
				this->effect->get_parameter("pMaskMatrix").set_matrix(this->precalc.matrix);
				this->effect->get_parameter("pMaskMultiplier").set_float4(this->precalc.scale);

				while (gs_effect_loop(this->effect->get_object(), "Mask")) {
					gs_draw_sprite(0, 0, width, height);
				}

				gs_blend_state_pop();
			}

			this->final_texture      = this->final_rt->get_texture();
			this->have_final_texture = true;
		}
	} catch (...) {
		obs_source_skip_video_filter(this->self);
		return;
	}

	if (!have_filter_texture || !have_input_texture || !have_final_texture) {
		obs_source_skip_video_filter(this->self);
		return;
	}
	if (!this->filter_texture->get_object() || !this->input_texture->get_object() || !this->final_texture->get_object()) {
		obs_source_skip_video_filter(this->self);
		return;
	}

	// Draw source
	{
		// It is important that we do not modify the blend state here, as it is set correctly by OBS
		gs_set_cull_mode(GS_NEITHER);
		gs_enable_color(true, true, true, true);
		gs_enable_depth_test(false);
		gs_depth_function(GS_ALWAYS);
		gs_enable_stencil_test(false);
		gs_enable_stencil_write(false);
		gs_stencil_function(GS_STENCIL_BOTH, GS_ALWAYS);
		gs_stencil_op(GS_STENCIL_BOTH, GS_ZERO, GS_ZERO, GS_ZERO);

		gs_effect_t* final_effect = in_effect ? in_effect : default_effect;
		gs_eparam_t* param        = gs_effect_get_param_by_name(final_effect, "image");
		if (!param) {
			P_LOG_ERROR("<filter-dynamic-mask:%s> Failed to set image param.", obs_source_get_name(this->self));
			obs_source_skip_video_filter(this->self);
			return;
		} else {
			gs_effect_set_texture(param, this->final_texture->get_object());
		}
		while (gs_effect_loop(final_effect, "Draw")) {
			gs_draw_sprite(0, 0, width, height);
		}
	}
}
