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
#include "strings.hpp"
#include <sstream>
#include <stdexcept>
#include <vector>
#include "obs/gs/gs-helper.hpp"
#include "util/util-logging.hpp"

#ifdef _DEBUG
#define ST_PREFIX "<%s> "
#define D_LOG_ERROR(x, ...) P_LOG_ERROR(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_WARNING(x, ...) P_LOG_WARN(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_INFO(x, ...) P_LOG_INFO(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_DEBUG(x, ...) P_LOG_DEBUG(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#else
#define ST_PREFIX "<filter::dynamic_mask> "
#define D_LOG_ERROR(...) P_LOG_ERROR(ST_PREFIX __VA_ARGS__)
#define D_LOG_WARNING(...) P_LOG_WARN(ST_PREFIX __VA_ARGS__)
#define D_LOG_INFO(...) P_LOG_INFO(ST_PREFIX __VA_ARGS__)
#define D_LOG_DEBUG(...) P_LOG_DEBUG(ST_PREFIX __VA_ARGS__)
#endif

// Filter to allow dynamic masking
// Allow any channel to affect any other channel
//
// Red/Green/Blue/Alpha Mask Input
// - Red Mask Output
// - Blue Mask Output
// - Green Mask Output
// - Alpha Mask Output

#define ST_I18N "Filter.DynamicMask"

#define ST_I18N_INPUT "Filter.DynamicMask.Input"
#define ST_KEY_INPUT "Filter.DynamicMask.Input"
#define ST_I18N_CHANNEL "Filter.DynamicMask.Channel"
#define ST_KEY_CHANNEL "Filter.DynamicMask.Channel"
#define ST_I18N_CHANNEL_VALUE "Filter.DynamicMask.Channel.Value"
#define ST_KEY_CHANNEL_VALUE "Filter.DynamicMask.Channel.Value"
#define ST_I18N_CHANNEL_MULTIPLIER "Filter.DynamicMask.Channel.Multiplier"
#define ST_KEY_CHANNEL_MULTIPLIER "Filter.DynamicMask.Channel.Multiplier"
#define ST_I18N_CHANNEL_INPUT "Filter.DynamicMask.Channel.Input"
#define ST_KEY_CHANNEL_INPUT "Filter.DynamicMask.Channel.Input"

using namespace streamfx::filter::dynamic_mask;

static constexpr std::string_view HELP_URL = "https://github.com/Xaymar/obs-StreamFX/wiki/Filter-Dynamic-Mask";

static std::pair<channel, const char*> channel_translations[] = {
	{channel::Red, S_CHANNEL_RED},
	{channel::Green, S_CHANNEL_GREEN},
	{channel::Blue, S_CHANNEL_BLUE},
	{channel::Alpha, S_CHANNEL_ALPHA},
};

dynamic_mask_instance::dynamic_mask_instance(obs_data_t* settings, obs_source_t* self)
	: obs::source_instance(settings, self), _translation_map(), _effect(), _have_filter_texture(false), _filter_rt(),
	  _filter_texture(), _have_input_texture(false), _input(), _input_capture(), _input_texture(),
	  _have_final_texture(false), _final_rt(), _final_texture(), _channels(), _precalc()
{
	{
		auto gctx = streamfx::obs::gs::context();

		_filter_rt = std::make_shared<streamfx::obs::gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
		_final_rt  = std::make_shared<streamfx::obs::gs::rendertarget>(GS_RGBA, GS_ZS_NONE);

		{
			auto file = streamfx::data_file_path("effects/channel-mask.effect");
			try {
				_effect = streamfx::obs::gs::effect::create(file);
			} catch (std::exception& ex) {
				D_LOG_ERROR("Error loading '%s': %s", file.u8string().c_str(), ex.what());
				throw;
			}
		}
	}

	update(settings);
}

dynamic_mask_instance::~dynamic_mask_instance()
{
	release();
}

void dynamic_mask_instance::load(obs_data_t* settings)
{
	update(settings);
}

void dynamic_mask_instance::migrate(obs_data_t* data, uint64_t version) {}

void dynamic_mask_instance::update(obs_data_t* settings)
{
	// Update source.
	if (const char* v = obs_data_get_string(settings, ST_KEY_INPUT); (v != nullptr) && (v[0] != '\0')) {
		if (!acquire(v))
			DLOG_ERROR("Failed to acquire Input source '%s'.", v);
	} else {
		release();
	}

	// Update data store
	for (auto kv1 : channel_translations) {
		auto found = _channels.find(kv1.first);
		if (found == _channels.end()) {
			_channels.insert({kv1.first, channel_data()});
			found = _channels.find(kv1.first);
			if (found == _channels.end()) {
				assert(found != _channels.end());
				throw std::runtime_error("Unable to insert element into data _store.");
			}
		}

		std::string chv_key = std::string(ST_KEY_CHANNEL_VALUE) + "." + kv1.second;
		found->second.value = static_cast<float_t>(obs_data_get_double(settings, chv_key.c_str()));
		_precalc.base.ptr[static_cast<size_t>(kv1.first)] = found->second.value;

		std::string chm_key = std::string(ST_KEY_CHANNEL_MULTIPLIER) + "." + kv1.second;
		found->second.scale = static_cast<float_t>(obs_data_get_double(settings, chm_key.c_str()));
		_precalc.scale.ptr[static_cast<size_t>(kv1.first)] = found->second.scale;

		vec4* ch = &_precalc.matrix.x;
		switch (kv1.first) {
		case channel::Red:
			ch = &_precalc.matrix.x;
			break;
		case channel::Green:
			ch = &_precalc.matrix.y;
			break;
		case channel::Blue:
			ch = &_precalc.matrix.z;
			break;
		case channel::Alpha:
			ch = &_precalc.matrix.t;
			break;
		default:
			break;
		}

		for (auto kv2 : channel_translations) {
			std::string ab_key = std::string(ST_KEY_CHANNEL_INPUT) + "." + kv1.second + "." + kv2.second;
			found->second.values.ptr[static_cast<size_t>(kv2.first)] =
				static_cast<float_t>(obs_data_get_double(settings, ab_key.c_str()));
			ch->ptr[static_cast<size_t>(kv2.first)] = found->second.values.ptr[static_cast<size_t>(kv2.first)];
		}
	}
}

void dynamic_mask_instance::save(obs_data_t* settings)
{
	if (_input) {
		obs_data_set_string(settings, ST_KEY_INPUT, _input.lock().name().data());
	}

	for (auto kv1 : channel_translations) {
		auto found = _channels.find(kv1.first);
		if (found == _channels.end()) {
			_channels.insert({kv1.first, channel_data()});
			found = _channels.find(kv1.first);
			if (found == _channels.end()) {
				assert(found != _channels.end());
				throw std::runtime_error("Unable to insert element into data _store.");
			}
		}

		std::string chv_key = std::string(ST_KEY_CHANNEL_VALUE) + "." + kv1.second;
		obs_data_set_double(settings, chv_key.c_str(), static_cast<double_t>(found->second.value));

		std::string chm_key = std::string(ST_KEY_CHANNEL_MULTIPLIER) + "." + kv1.second;
		obs_data_set_double(settings, chm_key.c_str(), static_cast<double_t>(found->second.scale));

		for (auto kv2 : channel_translations) {
			std::string ab_key = std::string(ST_KEY_CHANNEL_INPUT) + "." + kv1.second + "." + kv2.second;
			obs_data_set_double(settings, ab_key.c_str(),
								static_cast<double_t>(found->second.values.ptr[static_cast<size_t>(kv2.first)]));
		}
	}
}

void dynamic_mask_instance::video_tick(float)
{
	_have_input_texture  = false;
	_have_filter_texture = false;
	_have_final_texture  = false;
}

void dynamic_mask_instance::video_render(gs_effect_t* in_effect)
{
	obs_source_t* parent = obs_filter_get_parent(_self);
	obs_source_t* target = obs_filter_get_target(_self);
	uint32_t      width  = obs_source_get_base_width(target);
	uint32_t      height = obs_source_get_base_height(target);
	auto          input  = _input.lock();

	if (!_self || !parent || !target || !width || !height || !_input || !_input_capture || !_effect) {
		obs_source_skip_video_filter(_self);
		return;
	} else if (!input.width() || !input.height()) {
		obs_source_skip_video_filter(_self);
		return;
	}

#ifdef ENABLE_PROFILING
	streamfx::obs::gs::debug_marker gdmp{streamfx::obs::gs::debug_color_source, "Dynamic Mask '%s' on '%s'",
										 obs_source_get_name(_self), obs_source_get_name(obs_filter_get_parent(_self))};
#endif

	gs_effect_t* default_effect = obs_get_base_effect(obs_base_effect::OBS_EFFECT_DEFAULT);

	try { // Capture filter and input
		if (!_have_filter_texture) {
#ifdef ENABLE_PROFILING
			streamfx::obs::gs::debug_marker gdm{streamfx::obs::gs::debug_color_cache, "Cache"};
#endif

			if (obs_source_process_filter_begin(_self, GS_RGBA, OBS_ALLOW_DIRECT_RENDERING)) {
				auto op = _filter_rt->render(width, height);

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
				gs_ortho(0, static_cast<float>(width), 0, static_cast<float>(height), -1., 1.);

				obs_source_process_filter_end(_self, default_effect, width, height);

				gs_blend_state_pop();
			} else {
				throw std::runtime_error("Failed to render filter.");
			}

			_filter_texture      = _filter_rt->get_texture();
			_have_filter_texture = true;
		}

		if (!_have_input_texture) {
#ifdef ENABLE_PROFILING
			streamfx::obs::gs::debug_marker gdm{streamfx::obs::gs::debug_color_capture, "Capture '%s'",
												obs_source_get_name(_input_capture->get_object())};
#endif

			_input_texture      = _input_capture->render(input.width(), input.height());
			_have_input_texture = true;
		}

		// Draw source
		if (!_have_final_texture) {
#ifdef ENABLE_PROFILING
			streamfx::obs::gs::debug_marker gdm{streamfx::obs::gs::debug_color_convert, "Masking"};
#endif

			{
				auto op = _final_rt->render(width, height);

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
				gs_ortho(0, 1, 0, 1, -1., 1.);

				_effect.get_parameter("pMaskInputA").set_texture(_filter_texture);
				_effect.get_parameter("pMaskInputB").set_texture(_input_texture);

				_effect.get_parameter("pMaskBase").set_float4(_precalc.base);
				_effect.get_parameter("pMaskMatrix").set_matrix(_precalc.matrix);
				_effect.get_parameter("pMaskMultiplier").set_float4(_precalc.scale);

				while (gs_effect_loop(_effect.get(), "Mask")) {
					streamfx::gs_draw_fullscreen_tri();
				}

				gs_blend_state_pop();
			}

			_final_texture      = _final_rt->get_texture();
			_have_final_texture = true;
		}
	} catch (...) {
		obs_source_skip_video_filter(_self);
		return;
	}

	if (!_have_filter_texture || !_have_input_texture || !_have_final_texture) {
		obs_source_skip_video_filter(_self);
		return;
	}
	if (!_filter_texture->get_object() || !_input_texture->get_object() || !_final_texture->get_object()) {
		obs_source_skip_video_filter(_self);
		return;
	}

	// Draw source
	{
#ifdef ENABLE_PROFILING
		streamfx::obs::gs::debug_marker gdm{streamfx::obs::gs::debug_color_render, "Render"};
#endif

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
			DLOG_ERROR("<filter-dynamic-mask:%s> Failed to set image param.", obs_source_get_name(_self));
			obs_source_skip_video_filter(_self);
			return;
		} else {
			gs_effect_set_texture(param, _final_texture->get_object());
		}
		while (gs_effect_loop(final_effect, "Draw")) {
			gs_draw_sprite(0, 0, width, height);
		}
	}
}

void dynamic_mask_instance::enum_active_sources(obs_source_enum_proc_t enum_callback, void* param)
{
	if (_input)
		enum_callback(_self, _input.lock().get(), param);
}

void dynamic_mask_instance::enum_all_sources(obs_source_enum_proc_t enum_callback, void* param)
{
	if (_input)
		enum_callback(_self, _input.lock().get(), param);
}

void streamfx::filter::dynamic_mask::dynamic_mask_instance::show()
{
	if (!_input || !_self.showing() || !(_self.get_filter_parent().showing()))
		return;

	auto input = _input.lock();
	_input_vs  = ::streamfx::obs::source_showing_reference::add_showing_reference(input);
}

void streamfx::filter::dynamic_mask::dynamic_mask_instance::hide()
{
	_input_vs.reset();
}

void streamfx::filter::dynamic_mask::dynamic_mask_instance::activate()
{
	if (!_input || !_self.active() || !(_self.get_filter_parent().active()))
		return;

	auto input = _input.lock();
	_input_ac  = ::streamfx::obs::source_active_reference::add_active_reference(input);
}

void streamfx::filter::dynamic_mask::dynamic_mask_instance::deactivate()
{
	_input_ac.reset();
}

bool dynamic_mask_instance::acquire(std::string_view name)
try {
	// Prevent us from creating a circle.
	if (auto v = obs_source_get_name(obs_filter_get_parent(_self)); (v != nullptr) && (name == v)) {
		return false;
	}

	// Acquire a reference to the actual source.
	::streamfx::obs::source input = name;

	// Acquire a texture renderer for the source, with the parent source as the parent.
	auto capture = std::make_shared<streamfx::gfx::source_texture>(input, obs_filter_get_parent(_self));

	// Update our local storage.
	_input         = input;
	_input_capture = capture;

	// Do the necessary things.
	activate();
	show();

	return true;
} catch (const std::exception&) {
	release();
	return false;
} catch (...) {
	release();
	return false;
}

void dynamic_mask_instance::release()
{
	_input.reset();
	_input_capture.reset();

	deactivate();
	hide();
}

dynamic_mask_factory::dynamic_mask_factory()
{
	_info.id           = S_PREFIX "filter-dynamic-mask";
	_info.type         = OBS_SOURCE_TYPE_FILTER;
	_info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW;

	support_active_child_sources(true);
	support_child_sources(true);
	support_size(false);
	support_activity_tracking(true);
	support_visibility_tracking(true);
	finish_setup();
	register_proxy("obs-stream-effects-filter-dynamic-mask");
}

dynamic_mask_factory::~dynamic_mask_factory() {}

const char* dynamic_mask_factory::get_name()
{
	return D_TRANSLATE(ST_I18N);
}

void dynamic_mask_factory::get_defaults2(obs_data_t* data)
{
	obs_data_set_default_int(data, ST_KEY_CHANNEL, static_cast<int64_t>(channel::Red));
	for (auto kv : channel_translations) {
		obs_data_set_default_double(data, (std::string(ST_KEY_CHANNEL_VALUE) + "." + kv.second).c_str(), 1.0);
		obs_data_set_default_double(data, (std::string(ST_KEY_CHANNEL_MULTIPLIER) + "." + kv.second).c_str(), 1.0);
		for (auto kv2 : channel_translations) {
			obs_data_set_default_double(
				data, (std::string(ST_KEY_CHANNEL_INPUT) + "." + kv.second + "." + kv2.second).c_str(), 0.0);
		}
	}
}

obs_properties_t* dynamic_mask_factory::get_properties2(dynamic_mask_instance* data)
{
	obs_properties_t* props = obs_properties_create();
	obs_property_t*   p;

	_translation_cache.clear();

#ifdef ENABLE_FRONTEND
	{
		obs_properties_add_button2(props, S_MANUAL_OPEN, D_TRANSLATE(S_MANUAL_OPEN),
								   streamfx::filter::dynamic_mask::dynamic_mask_factory::on_manual_open, nullptr);
	}
#endif

	{ // Input
		p = obs_properties_add_list(props, ST_KEY_INPUT, D_TRANSLATE(ST_I18N_INPUT), OBS_COMBO_TYPE_LIST,
									OBS_COMBO_FORMAT_STRING);
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
			_translation_cache.push_back(translate_string(D_TRANSLATE(ST_I18N_CHANNEL_VALUE), D_TRANSLATE(pri_ch)));
			std::string buf = std::string(ST_KEY_CHANNEL_VALUE) + "." + pri_ch;
			p = obs_properties_add_float_slider(grp, buf.c_str(), _translation_cache.back().c_str(), -100.0, 100.0,
												0.01);
			obs_property_set_long_description(p, _translation_cache.back().c_str());
		}

		const char* sec_chs[] = {S_CHANNEL_RED, S_CHANNEL_GREEN, S_CHANNEL_BLUE, S_CHANNEL_ALPHA};
		for (auto sec_ch : sec_chs) {
			_translation_cache.push_back(translate_string(D_TRANSLATE(ST_I18N_CHANNEL_INPUT), D_TRANSLATE(sec_ch)));
			std::string buf = std::string(ST_KEY_CHANNEL_INPUT) + "." + pri_ch + "." + sec_ch;
			p = obs_properties_add_float_slider(grp, buf.c_str(), _translation_cache.back().c_str(), -100.0, 100.0,
												0.01);
			obs_property_set_long_description(p, _translation_cache.back().c_str());
		}

		{
			_translation_cache.push_back(
				translate_string(D_TRANSLATE(ST_I18N_CHANNEL_MULTIPLIER), D_TRANSLATE(pri_ch)));
			std::string buf = std::string(ST_KEY_CHANNEL_MULTIPLIER) + "." + pri_ch;
			p = obs_properties_add_float_slider(grp, buf.c_str(), _translation_cache.back().c_str(), -100.0, 100.0,
												0.01);
			obs_property_set_long_description(p, _translation_cache.back().c_str());
		}

		{
			_translation_cache.push_back(translate_string(D_TRANSLATE(ST_I18N_CHANNEL), D_TRANSLATE(pri_ch)));
			std::string buf = std::string(ST_KEY_CHANNEL) + "." + pri_ch;
			obs_properties_add_group(props, buf.c_str(), _translation_cache.back().c_str(),
									 obs_group_type::OBS_GROUP_NORMAL, grp);
		}
	}

	return props;
}

std::string dynamic_mask_factory::translate_string(const char* format, ...)
{
	va_list vargs;
	va_start(vargs, format);
	std::vector<char> buffer(2048);
	std::size_t       len = static_cast<size_t>(vsnprintf(buffer.data(), buffer.size(), format, vargs));
	va_end(vargs);
	return std::string(buffer.data(), buffer.data() + len);
}

#ifdef ENABLE_FRONTEND
bool dynamic_mask_factory::on_manual_open(obs_properties_t* props, obs_property_t* property, void* data)
try {
	streamfx::open_url(HELP_URL);
	return false;
} catch (const std::exception& ex) {
	D_LOG_ERROR("Failed to open manual due to error: %s", ex.what());
	return false;
} catch (...) {
	D_LOG_ERROR("Failed to open manual due to unknown error.", "");
	return false;
}
#endif

std::shared_ptr<dynamic_mask_factory> _filter_dynamic_mask_factory_instance = nullptr;

void streamfx::filter::dynamic_mask::dynamic_mask_factory::initialize()
try {
	if (!_filter_dynamic_mask_factory_instance)
		_filter_dynamic_mask_factory_instance = std::make_shared<dynamic_mask_factory>();
} catch (const std::exception& ex) {
	D_LOG_ERROR("Failed to initialize due to error: %s", ex.what());
} catch (...) {
	D_LOG_ERROR("Failed to initialize due to unknown error.", "");
}

void streamfx::filter::dynamic_mask::dynamic_mask_factory::finalize()
{
	_filter_dynamic_mask_factory_instance.reset();
}

std::shared_ptr<dynamic_mask_factory> streamfx::filter::dynamic_mask::dynamic_mask_factory::get()
{
	return _filter_dynamic_mask_factory_instance;
}
