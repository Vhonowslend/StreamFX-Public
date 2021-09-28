/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2020 Michael Fabian Dirks
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

#include "filter-autoframing.hpp"
#include "obs/gs/gs-helper.hpp"
#include "util/util-logging.hpp"

#ifdef _DEBUG
#define ST_PREFIX "<%s> "
#define D_LOG_ERROR(x, ...) P_LOG_ERROR(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_WARNING(x, ...) P_LOG_WARN(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_INFO(x, ...) P_LOG_INFO(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_DEBUG(x, ...) P_LOG_DEBUG(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#else
#define ST_PREFIX "<filter::autoframing> "
#define D_LOG_ERROR(...) P_LOG_ERROR(ST_PREFIX __VA_ARGS__)
#define D_LOG_WARNING(...) P_LOG_WARN(ST_PREFIX __VA_ARGS__)
#define D_LOG_INFO(...) P_LOG_INFO(ST_PREFIX __VA_ARGS__)
#define D_LOG_DEBUG(...) P_LOG_DEBUG(ST_PREFIX __VA_ARGS__)
#endif

// Tracking
// - Mode
//   - "Solo": Track only a single person, but drastically improve temporal coherency.
//   - "Group": Track up to 8 people, but make no guarantee about which boundary is which face.
//     - May need to fix the random aspect of it in the future.
// - Stability: Slider from 0% to 100%.
//   - 0%: Smoothing, with over and undershoot.
//   - 100%: Pinpoint accuracy, always on the face.

// Framing
// - Modes
//   - Mode 0: Combine all tracked frames into a single large one.
//   - Mode 1: Keep individual frames per person, place them in some sort of predefined pattern (selectable).
//   - Mode 2: Frame each one individually, and if frames would overlap, merge their cells in the selectable pattern into one large one.
//     - Animate the merge, or having it be instant?
// - Padding in X and Y, either as % of total, or as px.

/* options
Tracking
- Mode
  - Solo: Track a single person only.
  - Group: Track up to 8 people.
- Stability: 0 - 100% Slider
  - 0%: Predict and smooth changes as much as possible.
  - 100%: No smoothing, always spot on tracking.


Advanced
- Engine
  - Automatic (whichever is available)
  - NVIDIA BROADCAST

*/

#define ST_I18N "Filter.AutoFraming"

#define ST_KEY_ADVANCED S_ADVANCED
#define ST_I18N_ADVANCED ST_I18N ".Advanced"
#define ST_KEY_ADVANCED_PROVIDER ST_KEY_ADVANCED ".Provider"
#define ST_I18N_ADVANCED_PROVIDER ST_I18N_ADVANCED ".Provider"
#define ST_I18N_ADVANCED_PROVIDER_NVIDIA_FACEDETECTION ST_I18N_ADVANCED_PROVIDER ".NVIDIA.FaceDetection"

#define ST_MAXIMUM_REGIONS 8

using streamfx::filter::autoframing::autoframing_factory;
using streamfx::filter::autoframing::autoframing_instance;
using streamfx::filter::autoframing::autoframing_provider;

static constexpr std::string_view HELP_URL = "https://github.com/Xaymar/obs-StreamFX/wiki/Filter-Auto-Framing";

static autoframing_provider provider_priority[] = {
	autoframing_provider::NVIDIA_FACEDETECTION,
};

inline std::pair<bool, double_t> parse_text_as_size(const char* text)
{
	double_t v = 0;
	if (sscanf(text, "%lf", &v) == 1) {
		const char* prc_chr = strrchr(text, '%');
		if (prc_chr && (*prc_chr == '%')) {
			return {true, v / 100.0};
		} else {
			return {false, v};
		}
	} else {
		return {true, 1.0};
	}
}

const char* streamfx::filter::autoframing::cstring(autoframing_provider provider)
{
	switch (provider) {
	case autoframing_provider::INVALID:
		return "N/A";
	case autoframing_provider::AUTOMATIC:
		return D_TRANSLATE(S_STATE_AUTOMATIC);
	case autoframing_provider::NVIDIA_FACEDETECTION:
		return D_TRANSLATE(ST_I18N_ADVANCED_PROVIDER_NVIDIA_FACEDETECTION);
	default:
		throw std::runtime_error("Missing Conversion Entry");
	}
}

std::string streamfx::filter::autoframing::string(autoframing_provider provider)
{
	return cstring(provider);
}

autoframing_instance::~autoframing_instance()
{
	D_LOG_DEBUG("Finalizing... (Addr: 0x%" PRIuPTR ")", this);

	// TODO: Make this asynchronous.
	std::unique_lock<std::mutex> ul(_provider_lock);
	switch (_provider) {
#ifdef ENABLE_FILTER_DENOISING_NVIDIA
	case autoframing_provider::NVIDIA_FACEDETECTION:
		nvar_facedetection_unload();
		break;
#endif
	default:
		break;
	}
}

autoframing_instance::autoframing_instance(obs_data_t* data, obs_source_t* self)
	: source_instance(data, self), _size(), _provider(INVALID), _provider_ui(INVALID), _provider_ready(false),
	  _provider_lock(), _provider_task(), _input(), _dirty(true)
{
	D_LOG_DEBUG("Initializating... (Addr: 0x%" PRIuPTR ")", this);

	{
		::streamfx::obs::gs::context gctx;

		// Create the render target for the input buffering.
		_input = std::make_shared<::streamfx::obs::gs::rendertarget>(GS_RGBA_UNORM, GS_ZS_NONE);
		_input->render(1, 1); // Preallocate the RT on the driver and GPU.

		// Create the Vertex Buffer for rendering.
		_vb = std::make_shared<::streamfx::obs::gs::vertex_buffer>(4, 1);
		vec3_set(_vb->at(0).position, 0, 0, 0);
		vec3_set(_vb->at(1).position, 1, 0, 0);
		vec3_set(_vb->at(2).position, 0, 1, 0);
		vec3_set(_vb->at(3).position, 1, 1, 0);
		_vb->update(true);
	}

	if (data) {
		load(data);
	}
}

void autoframing_instance::load(obs_data_t* data)
{
	// Update from passed data.
	update(data);
}

void autoframing_instance::migrate(obs_data_t* data, uint64_t version)
{
	if (version < STREAMFX_MAKE_VERSION(0, 10, 0, 0)) {
		// Change engine setting back to automatic if loading of a specified engine failed.
		obs_data_unset_user_value(data, ST_KEY_ADVANCED_PROVIDER);
	}

	if (version < STREAMFX_MAKE_VERSION(0, 11, 0, 0)) {
		obs_data_unset_user_value(data, ST_KEY_ADVANCED_PROVIDER);
	}
}

void autoframing_instance::update(obs_data_t* data)
{
	// Check if the user changed which Denoising provider we use.
	autoframing_provider provider = static_cast<autoframing_provider>(obs_data_get_int(data, ST_KEY_ADVANCED_PROVIDER));
	if (provider == autoframing_provider::AUTOMATIC) {
		provider = autoframing_factory::get()->find_ideal_provider();
	}

	// Check if the provider was changed, and if so switch.
	if (provider != _provider) {
		_provider_ui = provider;
		switch_provider(provider);
	}

	if (_provider_ready) {
		std::unique_lock<std::mutex> ul(_provider_lock);

		switch (_provider) {
#ifdef ENABLE_FILTER_UPSCALING_NVIDIA
		case autoframing_provider::NVIDIA_FACEDETECTION:
			nvar_facedetection_update(data);
			break;
#endif
		default:
			break;
		}
	}
}

void streamfx::filter::autoframing::autoframing_instance::properties(obs_properties_t* properties)
{
	switch (_provider_ui) {
#ifdef ENABLE_FILTER_AUTOFRAMING_NVIDIA
	case autoframing_provider::NVIDIA_FACEDETECTION:
		nvar_facedetection_properties(properties);
		break;
#endif
	default:
		break;
	}
}

uint32_t autoframing_instance::get_width()
{
	return std::max<uint32_t>(_size.first, 1);
}

uint32_t autoframing_instance::get_height()
{
	return std::max<uint32_t>(_size.second, 1);
}

void autoframing_instance::video_tick(float_t seconds)
{
	auto target = obs_filter_get_target(_self);
	auto width  = obs_source_get_base_width(target);
	auto height = obs_source_get_base_height(target);
	_size       = {width, height};

	_dirty = true;
}

void autoframing_instance::video_render(gs_effect_t* effect)
{
	auto parent = obs_filter_get_parent(_self);
	auto target = obs_filter_get_target(_self);
	auto width  = obs_source_get_base_width(target);
	auto height = obs_source_get_base_height(target);
	vec4 blank  = vec4{0, 0, 0, 0};

	// Ensure we have the bare minimum of valid information.
	target = target ? target : parent;
	effect = effect ? effect : obs_get_base_effect(OBS_EFFECT_DEFAULT);

	// Skip the filter if:
	// - The Provider isn't ready yet.
	// - We don't have a target.
	// - The width/height of the next filter in the chain is empty.
	if (!_provider_ready || !target || (width == 0) || (height == 0)) {
		obs_source_skip_video_filter(_self);
		return;
	}

#ifdef ENABLE_PROFILING
	::streamfx::obs::gs::debug_marker profiler0{::streamfx::obs::gs::debug_color_source, "StreamFX Auto-Framing"};
	::streamfx::obs::gs::debug_marker profiler0_0{::streamfx::obs::gs::debug_color_gray, "'%s' on '%s'",
												  obs_source_get_name(_self), obs_source_get_name(parent)};
#endif

	if (_dirty) {
		// Capture the input.
		if (obs_source_process_filter_begin(_self, GS_RGBA, OBS_ALLOW_DIRECT_RENDERING)) {
			auto op = _input->render(width, height);

			// Clear the buffer
			gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &blank, 0, 0);

			// Set GPU state
			gs_blend_state_push();
			gs_enable_color(true, true, true, true);
			gs_enable_blending(false);
			gs_enable_depth_test(false);
			gs_enable_stencil_test(false);
			gs_set_cull_mode(GS_NEITHER);

			// Render
			bool srgb = gs_framebuffer_srgb_enabled();
			gs_enable_framebuffer_srgb(gs_get_linear_srgb());
			obs_source_process_filter_end(_self, obs_get_base_effect(OBS_EFFECT_DEFAULT), width, height);
			gs_enable_framebuffer_srgb(srgb);

			// Reset GPU state
			gs_blend_state_pop();
		} else {
			obs_source_skip_video_filter(_self);
			return;
		}

		// Lock & Process the captured input with the provider.
		std::unique_lock<std::mutex> ul(_provider_lock);
		switch (_provider) {
#ifdef ENABLE_FILTER_DENOISING_NVIDIA
		case autoframing_provider::NVIDIA_FACEDETECTION:
			nvar_facedetection_process();
			break;
#endif
		default:
			obs_source_skip_video_filter(_self);
			return;
		}
	}

	{ // Draw the result for the next filter to use.
#ifdef ENABLE_PROFILING
		::streamfx::obs::gs::debug_marker profiler1{::streamfx::obs::gs::debug_color_render, "Render"};
#endif
		gs_effect_set_texture(gs_effect_get_param_by_name(effect, "image"), _input->get_object());
		while (gs_effect_loop(effect, "Draw")) {
			gs_draw_sprite(nullptr, 0, _size.first, _size.second);
		}
	}

	/*
	if (!_capture_fresh) { // Only re-capture if we ticked before.
		if (obs_source_process_filter_begin(_self, _capture->get_color_format(), OBS_ALLOW_DIRECT_RENDERING)) {
			auto op  = _capture->render(_size.first, _size.second);
			vec4 clr = {0., 0., 0., 0.};

			gs_ortho(0., _size.first, 0., _size.second, -1., 1.);
			gs_clear(GS_CLEAR_COLOR, &clr, 0., 0);
			gs_enable_color(true, true, true, true);
			gs_enable_blending(false);

			obs_source_process_filter_tech_end(_self, input_effect, _size.first, _size.second, "Draw");
		} else {
			obs_source_skip_video_filter(_self);
			return;
		}

		// Update Tracking
		perform();

		_capture_fresh = true;
	}

	{ // Adjust UVs
		{
			auto v     = _vb->at(0);
			v.uv[0]->x = _frame.x / _size.first;
			v.uv[0]->y = _frame.y / _size.second;
		}
		{
			auto v     = _vb->at(1);
			v.uv[0]->x = (_frame.x + _frame.w) / _size.first;
			v.uv[0]->y = _frame.y / _size.second;
		}
		{
			auto v     = _vb->at(2);
			v.uv[0]->x = _frame.x / _size.first;
			v.uv[0]->y = (_frame.y + _frame.h) / _size.second;
		}
		{
			auto v     = _vb->at(3);
			v.uv[0]->x = (_frame.x + _frame.w) / _size.first;
			v.uv[0]->y = (_frame.y + _frame.h) / _size.second;
		}
	}
	gs_load_vertexbuffer(_vb->update(true));

	gs_matrix_push();
	gs_matrix_scale3f(_size.first, _size.second, 0);

	gs_effect_set_texture(gs_effect_get_param_by_name(default_effect, "image"), _capture->get_texture()->get_object());
	while (gs_effect_loop(default_effect, "Draw")) {
		gs_draw(GS_TRISTRIP, 0, _vb->size());
	}

	gs_matrix_pop();
	gs_load_vertexbuffer(nullptr);
	*/
}

struct switch_provider_data_t {
	autoframing_provider provider;
};

void streamfx::filter::autoframing::autoframing_instance::switch_provider(autoframing_provider provider)
{
	std::unique_lock<std::mutex> ul(_provider_lock);

	// Safeguard against calls made from unlocked memory.
	if (provider == _provider) {
		return;
	}

	// This doesn't work correctly.
	// - Need to allow multiple switches at once because OBS is weird.
	// - Doesn't guarantee that the task is properly killed off.

	// Log information.
	D_LOG_INFO("Instance '%s' is switching provider from '%s' to '%s'.", obs_source_get_name(_self), cstring(_provider),
			   cstring(provider));

	// 1.If there is an existing task, attempt to cancel it.
	if (_provider_task) {
		streamfx::threadpool()->pop(_provider_task);
	}

	// 2. Build data to pass into the task.
	auto spd      = std::make_shared<switch_provider_data_t>();
	spd->provider = _provider;
	_provider     = provider;

	// 3. Then spawn a new task to switch provider.
	_provider_task = streamfx::threadpool()->push(
		std::bind(&autoframing_instance::task_switch_provider, this, std::placeholders::_1), spd);
}

void streamfx::filter::autoframing::autoframing_instance::task_switch_provider(util::threadpool_data_t data)
{
	std::shared_ptr<switch_provider_data_t> spd = std::static_pointer_cast<switch_provider_data_t>(data);

	// 1. Mark the provider as no longer ready.
	_provider_ready = false;

	// 2. Lock the provider from being used.
	std::unique_lock<std::mutex> ul(_provider_lock);

	try {
		// 3. Unload the previous provider.
		switch (spd->provider) {
#ifdef ENABLE_FILTER_AUTOFRAMING_NVIDIA
		case autoframing_provider::NVIDIA_FACEDETECTION:
			nvar_facedetection_unload();
			break;
#endif
		default:
			break;
		}

		// 4. Load the new provider.
		switch (_provider) {
#ifdef ENABLE_FILTER_AUTOFRAMING_NVIDIA
		case autoframing_provider::NVIDIA_FACEDETECTION:
			nvar_facedetection_load();
			break;
#endif
		default:
			break;
		}

		// Log information.
		D_LOG_INFO("Instance '%s' switched provider from '%s' to '%s'.", obs_source_get_name(_self),
				   cstring(spd->provider), cstring(_provider));

		_provider_ready = true;
	} catch (std::exception const& ex) {
		// Log information.
		D_LOG_ERROR("Instance '%s' failed switching provider with error: %s", obs_source_get_name(_self), ex.what());
	}
}

#ifdef ENABLE_FILTER_AUTOFRAMING_NVIDIA
void streamfx::filter::autoframing::autoframing_instance::nvar_facedetection_load() {}

void streamfx::filter::autoframing::autoframing_instance::nvar_facedetection_unload() {}

void streamfx::filter::autoframing::autoframing_instance::nvar_facedetection_process() {}

void streamfx::filter::autoframing::autoframing_instance::nvar_facedetection_properties(obs_properties_t* props) {}

void streamfx::filter::autoframing::autoframing_instance::nvar_facedetection_update(obs_data_t* data) {}
#endif

autoframing_factory::autoframing_factory()
{
	bool any_available = false;

	// 1. Try and load any configured providers.
#ifdef ENABLE_FILTER_AUTOFRAMING_NVIDIA
	try {
		// Load CVImage and Video Effects SDK.
		_nvcuda           = ::streamfx::nvidia::cuda::obs::get();
		_nvcvi            = ::streamfx::nvidia::cv::cv::get();
		_nvar             = ::streamfx::nvidia::ar::ar::get();
		_nvidia_available = true;
		any_available |= _nvidia_available;
	} catch (const std::exception& ex) {
		_nvidia_available = false;
		_nvar.reset();
		_nvcvi.reset();
		_nvcuda.reset();
		D_LOG_WARNING("Failed to make NVIDIA providers available due to error: %s", ex.what());
	} catch (...) {
		_nvidia_available = false;
		_nvar.reset();
		_nvcvi.reset();
		_nvcuda.reset();
		D_LOG_WARNING("Failed to make NVIDIA providers available with unknown error.", nullptr);
	}
#endif

	// 2. Check if any of them managed to load at all.
	if (!any_available) {
		D_LOG_ERROR("All supported providers failed to initialize, disabling effect.", 0);
		return;
	}

	// Register initial source.
	_info.id           = S_PREFIX "filter-autoframing";
	_info.type         = OBS_SOURCE_TYPE_FILTER;
	_info.output_flags = OBS_SOURCE_VIDEO;

	set_resolution_enabled(false);
	finish_setup();

	// Register proxy identifiers.
	register_proxy("streamfx-filter-nvidia-face-tracking");
	register_proxy("streamfx-nvidia-face-tracking");
}

autoframing_factory::~autoframing_factory() {}

const char* autoframing_factory::get_name()
{
	return D_TRANSLATE(ST_I18N);
}

void autoframing_factory::get_defaults2(obs_data_t* data)
{
	// Advanced
	obs_data_set_default_int(data, ST_KEY_ADVANCED_PROVIDER, static_cast<int64_t>(autoframing_provider::AUTOMATIC));
}

static bool modified_provider(obs_properties_t* props, obs_property_t*, obs_data_t* settings) noexcept
try {
	return true;
} catch (const std::exception& ex) {
	DLOG_ERROR("Unexpected exception in function '%s': %s.", __FUNCTION_NAME__, ex.what());
	return false;
} catch (...) {
	DLOG_ERROR("Unexpected exception in function '%s'.", __FUNCTION_NAME__);
	return false;
}

obs_properties_t* autoframing_factory::get_properties2(autoframing_instance* data)
{
	obs_properties_t* pr = obs_properties_create();

#ifdef ENABLE_FRONTEND
	{
		obs_properties_add_button2(pr, S_MANUAL_OPEN, D_TRANSLATE(S_MANUAL_OPEN), autoframing_factory::on_manual_open,
								   nullptr);
	}
#endif

	if (data) {
		data->properties(pr);
	}

	{ // Advanced Settings
		auto grp = obs_properties_create();
		obs_properties_add_group(pr, S_ADVANCED, D_TRANSLATE(S_ADVANCED), OBS_GROUP_NORMAL, grp);

		{
			auto p = obs_properties_add_list(grp, ST_KEY_ADVANCED_PROVIDER, D_TRANSLATE(ST_I18N_ADVANCED_PROVIDER),
											 OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
			obs_property_set_modified_callback(p, modified_provider);
			obs_property_list_add_int(p, D_TRANSLATE(S_STATE_AUTOMATIC),
									  static_cast<int64_t>(autoframing_provider::AUTOMATIC));
			obs_property_list_add_int(p, D_TRANSLATE(ST_I18N_ADVANCED_PROVIDER_NVIDIA_FACEDETECTION),
									  static_cast<int64_t>(autoframing_provider::NVIDIA_FACEDETECTION));
		}
	}

	return pr;
}

#ifdef ENABLE_FRONTEND
bool streamfx::filter::autoframing::autoframing_factory::on_manual_open(obs_properties_t* props,
																		obs_property_t* property, void* data)
{
	streamfx::open_url(HELP_URL);
	return false;
}
#endif

bool streamfx::filter::autoframing::autoframing_factory::is_provider_available(autoframing_provider provider)
{
	switch (provider) {
#ifdef ENABLE_FILTER_AUTOFRAMING_NVIDIA
	case autoframing_provider::NVIDIA_FACEDETECTION:
		return _nvidia_available;
#endif
	default:
		return false;
	}
}

autoframing_provider streamfx::filter::autoframing::autoframing_factory::find_ideal_provider()
{
	for (auto v : provider_priority) {
		if (is_provider_available(v)) {
			return v;
			break;
		}
	}
	return autoframing_provider::INVALID;
}

std::shared_ptr<autoframing_factory> _filter_autoframing_factory_instance = nullptr;

void autoframing_factory::initialize()
try {
	if (!_filter_autoframing_factory_instance)
		_filter_autoframing_factory_instance = std::make_shared<autoframing_factory>();
} catch (const std::exception& ex) {
	D_LOG_ERROR("Failed to initialize due to error: %s", ex.what());
} catch (...) {
	D_LOG_ERROR("Failed to initialize due to unknown error.", "");
}

void autoframing_factory::finalize()
{
	_filter_autoframing_factory_instance.reset();
}

std::shared_ptr<autoframing_factory> autoframing_factory::get()
{
	return _filter_autoframing_factory_instance;
}
