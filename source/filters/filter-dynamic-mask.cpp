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
#include <stdexcept>
#include "strings.hpp"

// Filter to allow dynamic masking
// Allow any channel to affect any other channel
//
// Red/Green/Blue/Alpha Mask Input
// - Red Mask Output
// - Blue Mask Output
// - Green Mask Output
// - Alpha Mask Output

#define ST "Filter.DynamicMask"

#define ST_INPUT "Filter.DynamicMask.Input"
#define ST_CHANNEL "Filter.DynamicMask.Channel"
#define ST_CHANNEL_VALUE "Filter.DynamicMask.Channel.Value"
#define ST_CHANNEL_MULTIPLIER "Filter.DynamicMask.Channel.Multiplier"
#define ST_CHANNEL_INPUT "Filter.DynamicMask.Channel.Input"

std::shared_ptr<filter::dynamic_mask::dynamic_mask_factory>
	filter::dynamic_mask::dynamic_mask_factory::factory_instance = nullptr;

static std::pair<filter::dynamic_mask::channel, const char*> channel_translations[] = {
	{filter::dynamic_mask::channel::Red, S_CHANNEL_RED},
	{filter::dynamic_mask::channel::Green, S_CHANNEL_GREEN},
	{filter::dynamic_mask::channel::Blue, S_CHANNEL_BLUE},
	{filter::dynamic_mask::channel::Alpha, S_CHANNEL_ALPHA},
};

filter::dynamic_mask::dynamic_mask_instance::dynamic_mask_instance(obs_data_t* settings, obs_source_t* self)
	: obs::source_instance(settings, self), _have_filter_texture(false), _have_input_texture(false),
	  _have_final_texture(false), _precalc(), _effect()
{
	this->_filter_rt = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
	this->_final_rt  = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);

	{
		char* file = obs_module_file("effects/channel-mask.effect");
		try {
			this->_effect = gs::effect::create(file);
		} catch (const std::exception& ex) {
			P_LOG_ERROR("Loading channel mask effect failed with error(s):\n%s", ex.what());
		}
		assert(this->_effect != nullptr);
		bfree(file);
	}

	this->update(settings);
}

filter::dynamic_mask::dynamic_mask_instance::~dynamic_mask_instance() {}

void filter::dynamic_mask::dynamic_mask_instance::update(obs_data_t* settings)
{
	// Update source.
	try {
		this->_input         = std::make_shared<obs::source>(obs_data_get_string(settings, ST_INPUT));
		this->_input_capture = std::make_shared<gfx::source_texture>(this->_input, _self);
		this->_input->events.rename += std::bind(&filter::dynamic_mask::dynamic_mask_instance::input_renamed, this,
												 std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	} catch (...) {
		this->_input.reset();
		this->_input_capture.reset();
		this->_input_texture.reset();
	}

	// Update data store
	for (auto kv1 : channel_translations) {
		auto found = this->_channels.find(kv1.first);
		if (found == this->_channels.end()) {
			this->_channels.insert({kv1.first, channel_data()});
			found = this->_channels.find(kv1.first);
			if (found == this->_channels.end()) {
				assert(found != this->_channels.end());
				throw std::runtime_error("Unable to insert element into data _store.");
			}
		}

		std::string chv_key = std::string(ST_CHANNEL_VALUE) + "." + kv1.second;
		found->second.value = static_cast<float_t>(obs_data_get_double(settings, chv_key.c_str()));
		this->_precalc.base.ptr[static_cast<size_t>(kv1.first)] = found->second.value;

		std::string chm_key = std::string(ST_CHANNEL_MULTIPLIER) + "." + kv1.second;
		found->second.scale = static_cast<float_t>(obs_data_get_double(settings, chm_key.c_str()));
		this->_precalc.scale.ptr[static_cast<size_t>(kv1.first)] = found->second.scale;

		vec4* ch = &_precalc.matrix.x;
		switch (kv1.first) {
		case channel::Red:
			ch = &this->_precalc.matrix.x;
			break;
		case channel::Green:
			ch = &this->_precalc.matrix.y;
			break;
		case channel::Blue:
			ch = &this->_precalc.matrix.z;
			break;
		case channel::Alpha:
			ch = &this->_precalc.matrix.t;
			break;
		default:
			break;
		}

		for (auto kv2 : channel_translations) {
			std::string ab_key = std::string(ST_CHANNEL_INPUT) + "." + kv1.second + "." + kv2.second;
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
	if (this->_input) {
		obs_data_set_string(settings, ST_INPUT, obs_source_get_name(this->_input->get()));
	}

	for (auto kv1 : channel_translations) {
		auto found = this->_channels.find(kv1.first);
		if (found == this->_channels.end()) {
			this->_channels.insert({kv1.first, channel_data()});
			found = this->_channels.find(kv1.first);
			if (found == this->_channels.end()) {
				assert(found != this->_channels.end());
				throw std::runtime_error("Unable to insert element into data _store.");
			}
		}

		std::string chv_key = std::string(ST_CHANNEL_VALUE) + "." + kv1.second;
		obs_data_set_double(settings, chv_key.c_str(), static_cast<double_t>(found->second.value));

		std::string chm_key = std::string(ST_CHANNEL_MULTIPLIER) + "." + kv1.second;
		obs_data_set_double(settings, chm_key.c_str(), static_cast<double_t>(found->second.scale));

		for (auto kv2 : channel_translations) {
			std::string ab_key = std::string(ST_CHANNEL_INPUT) + "." + kv1.second + "." + kv2.second;
			obs_data_set_double(settings, ab_key.c_str(),
								static_cast<double_t>(found->second.values.ptr[static_cast<size_t>(kv2.first)]));
		}
	}
}

void filter::dynamic_mask::dynamic_mask_instance::input_renamed(obs::source*, std::string old_name,
																std::string new_name)
{
	obs_data_t* settings = obs_source_get_settings(_self);
	obs_data_set_string(settings, ST_INPUT, new_name.c_str());
	obs_source_update(_self, settings);
}

bool filter::dynamic_mask::dynamic_mask_instance::modified(void*, obs_properties_t* properties, obs_property_t*,
														   obs_data_t* settings) noexcept
try {
	channel mask = static_cast<channel>(obs_data_get_int(settings, ST_CHANNEL));

	for (auto kv1 : channel_translations) {
		std::string chv_key = std::string(ST_CHANNEL_VALUE) + "." + kv1.second;
		obs_property_set_visible(obs_properties_get(properties, chv_key.c_str()), (mask == kv1.first));
		std::string chm_key = std::string(ST_CHANNEL_MULTIPLIER) + "." + kv1.second;
		obs_property_set_visible(obs_properties_get(properties, chm_key.c_str()), (mask == kv1.first));

		for (auto kv2 : channel_translations) {
			std::string io_key = std::string(ST_CHANNEL_INPUT) + "." + kv1.second + "." + kv2.second;
			obs_property_set_visible(obs_properties_get(properties, io_key.c_str()), (mask == kv1.first));
		}
	}

	return true;
} catch (const std::exception& ex) {
	P_LOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
	return false;
} catch (...) {
	P_LOG_ERROR("Unexpected exception in modified_properties callback.");
	return false;
}

void filter::dynamic_mask::dynamic_mask_instance::video_tick(float)
{
	_have_input_texture  = false;
	_have_filter_texture = false;
	_have_final_texture  = false;
}

void filter::dynamic_mask::dynamic_mask_instance::video_render(gs_effect_t* in_effect)
{
	obs_source_t* parent = obs_filter_get_parent(this->_self);
	obs_source_t* target = obs_filter_get_target(this->_self);
	uint32_t      width  = obs_source_get_base_width(target);
	uint32_t      height = obs_source_get_base_height(target);

	if (!_self || !parent || !target || !width || !height || !_input || !_input_capture || !_effect) {
		obs_source_skip_video_filter(_self);
		return;
	} else if (!_input->width() || !_input->height()) {
		obs_source_skip_video_filter(_self);
		return;
	}

	gs_effect_t* default_effect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_DEFAULT);

	try { // Capture filter and input
		if (!_have_filter_texture) {
			if (obs_source_process_filter_begin(this->_self, GS_RGBA, OBS_ALLOW_DIRECT_RENDERING)) {
				auto op = this->_filter_rt->render(width, height);

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

				obs_source_process_filter_end(this->_self, default_effect, width, height);

				gs_blend_state_pop();
			} else {
				throw std::runtime_error("Failed to render filter.");
			}

			this->_filter_texture      = this->_filter_rt->get_texture();
			this->_have_filter_texture = true;
		}

		if (!_have_input_texture) {
			this->_input_texture      = this->_input_capture->render(_input->width(), _input->height());
			this->_have_input_texture = true;
		}

		// Draw source
		if (!this->_have_final_texture) {
			{
				auto op = this->_final_rt->render(width, height);

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

				_effect.get_parameter("pMaskInputA").set_texture(this->_filter_texture);
				_effect.get_parameter("pMaskInputB").set_texture(this->_input_texture);

				_effect.get_parameter("pMaskBase").set_float4(this->_precalc.base);
				_effect.get_parameter("pMaskMatrix").set_matrix(this->_precalc.matrix);
				_effect.get_parameter("pMaskMultiplier").set_float4(this->_precalc.scale);

				while (gs_effect_loop(_effect.get(), "Mask")) {
					gs_draw_sprite(0, 0, width, height);
				}

				gs_blend_state_pop();
			}

			this->_final_texture      = this->_final_rt->get_texture();
			this->_have_final_texture = true;
		}
	} catch (...) {
		obs_source_skip_video_filter(this->_self);
		return;
	}

	if (!_have_filter_texture || !_have_input_texture || !_have_final_texture) {
		obs_source_skip_video_filter(this->_self);
		return;
	}
	if (!this->_filter_texture->get_object() || !this->_input_texture->get_object()
		|| !this->_final_texture->get_object()) {
		obs_source_skip_video_filter(this->_self);
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
			P_LOG_ERROR("<filter-dynamic-mask:%s> Failed to set image param.", obs_source_get_name(this->_self));
			obs_source_skip_video_filter(this->_self);
			return;
		} else {
			gs_effect_set_texture(param, this->_final_texture->get_object());
		}
		while (gs_effect_loop(final_effect, "Draw")) {
			gs_draw_sprite(0, 0, width, height);
		}
	}
}

filter::dynamic_mask::dynamic_mask_factory::dynamic_mask_factory()
{
	_info.id           = "obs-stream-effects-filter-dynamic-mask";
	_info.type         = OBS_SOURCE_TYPE_FILTER;
	_info.output_flags = OBS_SOURCE_VIDEO;

	finish_setup();
}

filter::dynamic_mask::dynamic_mask_factory::~dynamic_mask_factory() {}

const char* filter::dynamic_mask::dynamic_mask_factory::get_name()
{
	return D_TRANSLATE(ST);
}

void filter::dynamic_mask::dynamic_mask_factory::get_defaults2(obs_data_t* data)
{
	obs_data_set_default_int(data, ST_CHANNEL, static_cast<int64_t>(filter::dynamic_mask::channel::Red));
	for (auto kv : channel_translations) {
		obs_data_set_default_double(data, (std::string(ST_CHANNEL_VALUE) + "." + kv.second).c_str(), 1.0);
		obs_data_set_default_double(data, (std::string(ST_CHANNEL_MULTIPLIER) + "." + kv.second).c_str(), 1.0);
		for (auto kv2 : channel_translations) {
			obs_data_set_default_double(
				data, (std::string(ST_CHANNEL_INPUT) + "." + kv.second + "." + kv2.second).c_str(), 0.0);
		}
	}
}

obs_properties_t*
	filter::dynamic_mask::dynamic_mask_factory::get_properties2(filter::dynamic_mask::dynamic_mask_instance* data)
{
	obs_properties_t* props = obs_properties_create();
	obs_property_t*   p;

	_translation_cache.clear();

	{ // Input
		p = obs_properties_add_list(props, ST_INPUT, D_TRANSLATE(ST_INPUT), OBS_COMBO_TYPE_LIST,
									OBS_COMBO_FORMAT_STRING);
		obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_INPUT)));
		obs_property_list_add_string(p, "", "");
		obs::source_tracker::get()->enumerate(
			[&p](std::string name, obs_source_t*) {
				std::stringstream sstr;
				sstr << name << " (" << D_TRANSLATE(S_SOURCETYPE_SOURCE) << ")";
				obs_property_list_add_string(p, sstr.str().c_str(), name.c_str());
				return false;
			},
			obs::source_tracker::filter_video_sources);
		obs::source_tracker::get()->enumerate(
			[&p](std::string name, obs_source_t*) {
				std::stringstream sstr;
				sstr << name << " (" << D_TRANSLATE(S_SOURCETYPE_SCENE) << ")";
				obs_property_list_add_string(p, sstr.str().c_str(), name.c_str());
				return false;
			},
			obs::source_tracker::filter_scenes);
	}

	const char* pri_chs[] = {S_CHANNEL_RED, S_CHANNEL_GREEN, S_CHANNEL_BLUE, S_CHANNEL_ALPHA};
	for (auto pri_ch : pri_chs) {
		auto grp = obs_properties_create();

		{
			_translation_cache.push_back(translate_string(D_TRANSLATE(ST_CHANNEL_VALUE), D_TRANSLATE(pri_ch)));
			std::string buf = std::string(ST_CHANNEL_VALUE) + "." + pri_ch;
			p = obs_properties_add_float_slider(grp, buf.c_str(), _translation_cache.back().c_str(), -100.0, 100.0,
												0.01);
			obs_property_set_long_description(p, D_TRANSLATE(ST_CHANNEL_VALUE));
		}

		const char* sec_chs[] = {S_CHANNEL_RED, S_CHANNEL_GREEN, S_CHANNEL_BLUE, S_CHANNEL_ALPHA};
		for (auto sec_ch : sec_chs) {
			_translation_cache.push_back(translate_string(D_TRANSLATE(ST_CHANNEL_INPUT), D_TRANSLATE(sec_ch)));
			std::string buf = std::string(ST_CHANNEL_INPUT) + "." + pri_ch + "." + sec_ch;
			p = obs_properties_add_float_slider(grp, buf.c_str(), _translation_cache.back().c_str(), -100.0, 100.0,
												0.01);
			obs_property_set_long_description(p, D_TRANSLATE(ST_CHANNEL_INPUT));
		}

		{
			_translation_cache.push_back(translate_string(D_TRANSLATE(ST_CHANNEL_MULTIPLIER), D_TRANSLATE(pri_ch)));
			std::string buf = std::string(ST_CHANNEL_MULTIPLIER) + "." + pri_ch;
			p = obs_properties_add_float_slider(grp, buf.c_str(), _translation_cache.back().c_str(), -100.0, 100.0,
												0.01);
			obs_property_set_long_description(p, D_TRANSLATE(ST_CHANNEL_MULTIPLIER));
		}

		{
			_translation_cache.push_back(translate_string(D_TRANSLATE(ST_CHANNEL), D_TRANSLATE(pri_ch)));
			std::string buf = std::string(ST_CHANNEL) + "." + pri_ch;
			obs_properties_add_group(props, buf.c_str(), _translation_cache.back().c_str(),
									 obs_group_type::OBS_GROUP_NORMAL, grp);
		}
	}

	return props;
}

std::string filter::dynamic_mask::dynamic_mask_factory::translate_string(const char* format, ...)
{
	va_list vargs;
	va_start(vargs, format);
	std::vector<char> buffer(2048);
	size_t            len = static_cast<size_t>(vsnprintf(buffer.data(), buffer.size(), format, vargs));
	va_end(vargs);
	return std::string(buffer.data(), buffer.data() + len);
}
