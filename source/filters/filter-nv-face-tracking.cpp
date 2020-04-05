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

#include "filter-nv-face-tracking.hpp"
#include <algorithm>
#include <filesystem>
#include <util/platform.h>
#include "nvidia/cuda/nvidia-cuda-context-stack.hpp"
#include "obs/gs/gs-helper.hpp"
#include "utility.hpp"

#define ST "Filter.Nvidia.FaceTracking"
#define ST_ROI "Filter.Nvidia.FaceTracking.ROI"
#define ST_ROI_ZOOM "Filter.Nvidia.FaceTracking.ROI.Zoom"
#define SK_ROI_ZOOM "ROI.Zoom"
#define ST_ROI_OFFSET "Filter.Nvidia.FaceTracking.ROI.Offset"
#define ST_ROI_OFFSET_X "Filter.Nvidia.FaceTracking.ROI.Offset.X"
#define SK_ROI_OFFSET_X "ROI.Offset.X"
#define ST_ROI_OFFSET_Y "Filter.Nvidia.FaceTracking.ROI.Offset.Y"
#define SK_ROI_OFFSET_Y "ROI.Offset.Y"
#define ST_ROI_STABILITY "Filter.Nvidia.FaceTracking.ROI.Stability"
#define SK_ROI_STABILITY "ROI.Stability"

void nvar_deleter(NvAR_FeatureHandle v)
{
	filter::nvidia::face_tracking_factory::get()->get_ar()->destroy(v);
}

filter::nvidia::face_tracking_instance::face_tracking_instance(obs_data_t* settings, obs_source_t* self)
	: obs::source_instance(settings, self), _width(), _height(), _up_to_date(false), _rt(), _cfg_roi_zoom(1.0),
	  _cfg_roi_offset({0., 0.}), _cfg_roi_stability(1.0), _roi_center(), _roi_size(), _roi_geom(4, 1),
	  _cuda(face_tracking_factory::get()->get_cuda()), _cuda_ctx(face_tracking_factory::get()->get_cuda_context()),
	  _cuda_stream(), _cuda_mem(), _cuda_flush_cache(true), _ar(face_tracking_factory::get()->get_ar()),
	  _ar_models_path(), _ar_tracker(), _ar_ready(false), _ar_bboxes_data(), _ar_bboxes(), _ar_bboxes_confidence(),
	  _ar_image(), _ar_image_bgr(), _ar_image_temp()
{
	// Create rendertarget for parent source storage.
	{
		auto gctx = gs::context{};
		_rt       = std::make_shared<gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
	}

	// Figure out where the AR SDK Models are stored.
	{
		std::filesystem::path models_path = _ar->get_ar_sdk_path();
		models_path                       = models_path.append("models");
		models_path                       = std::filesystem::absolute(models_path);
		models_path.concat("\\");
		_ar_models_path = models_path.string();
	}

	// Initialize everything.
	{
		auto cctx    = std::make_shared<::nvidia::cuda::context_stack>(_cuda, _cuda_ctx);
		_cuda_stream = std::make_shared<::nvidia::cuda::stream>(_cuda);
		face_detection_initialize();
	}

#ifdef _DEBUG
	// Profiling
	_profile_capture       = util::profiler::create();
	_profile_cuda_register = util::profiler::create();
	_profile_cuda_copy     = util::profiler::create();
	_profile_ar_transfer   = util::profiler::create();
	_profile_ar_run        = util::profiler::create();
#endif
}

filter::nvidia::face_tracking_instance::~face_tracking_instance()
{
	_ar->image_dealloc(&_ar_image_temp);
	_ar->image_dealloc(&_ar_image_bgr);
}

void filter::nvidia::face_tracking_instance::face_detection_initialize()
{
	// Create
	NvAR_FeatureHandle fd_inst;
	if (NvCV_Status res = _ar->create(NvAR_Feature_FaceDetection, &fd_inst); res != NVCV_SUCCESS) {
		throw std::runtime_error("Failed to create Face Detection feature.");
	}
	_ar_tracker = std::shared_ptr<nvAR_Feature>{fd_inst, nvar_deleter};

	// Configuration
	if (NvCV_Status res = _ar->set_cuda_stream(fd_inst, NvAR_Parameter_Config(CUDAStream),
											   reinterpret_cast<CUstream>(_cuda_stream->get()));
		res != NVCV_SUCCESS) {
		throw std::runtime_error("");
	}
	if (NvCV_Status res = _ar->set_string(fd_inst, NvAR_Parameter_Config(ModelDir), _ar_models_path.c_str());
		res != NVCV_SUCCESS) {
		throw std::runtime_error("");
	}
	if (NvCV_Status res = _ar->set_uint32(fd_inst, NvAR_Parameter_Config(Temporal), 1); res != NVCV_SUCCESS) {
		throw std::runtime_error("");
	}

	// Create Bounding Boxes Data
	_ar_bboxes_data.assign(1, {0., 0., 0., 0.});
	_ar_bboxes.boxes     = _ar_bboxes_data.data();
	_ar_bboxes.max_boxes = std::clamp<std::uint8_t>(static_cast<std::uint8_t>(_ar_bboxes_data.size()), 0, 255);
	_ar_bboxes.num_boxes = 0;
	_ar_bboxes_confidence.resize(_ar_bboxes_data.size());

	if (NvCV_Status res =
			_ar->set_object(_ar_tracker.get(), NvAR_Parameter_Output(BoundingBoxes), &_ar_bboxes, sizeof(NvAR_BBoxes));
		res != NVCV_SUCCESS) {
		throw std::runtime_error("Failed to set BoundingBoxes for Face Tracking feature.");
	}

	if (NvCV_Status res =
			_ar->set_float32_array(_ar_tracker.get(), NvAR_Parameter_Output(BoundingBoxesConfidence),
								   _ar_bboxes_confidence.data(), static_cast<int>(_ar_bboxes_confidence.size()));
		res != NVCV_SUCCESS) {
		throw std::runtime_error("Failed to set BoundingBoxesConfidence for Face Tracking feature.");
	}

	// Push to extra thread to not block OBS Studio.
	obs_source_addref(_self);
	::get_global_threadpool()->push(std::bind(&filter::nvidia::face_tracking_instance::face_detection_initialize_thread,
											  this, std::placeholders::_1),
									nullptr);
}

void filter::nvidia::face_tracking_instance::face_detection_initialize_thread(std::shared_ptr<void> param)
{
	auto cctx = std::make_shared<::nvidia::cuda::context_stack>(_cuda, _cuda_ctx);
	if (NvCV_Status res = _ar->load(_ar_tracker.get()); res != NVCV_SUCCESS) {
		_ar_fail = true;
	}
	_ar_ready = true;
	obs_source_release(_self);
}

void filter::nvidia::face_tracking_instance::create_image_buffer(std::size_t width, std::size_t height)
{
	auto cctx = std::make_shared<::nvidia::cuda::context_stack>(_cuda, _cuda_ctx);

	// Create CUDA and AR interop.
	size_t pitch = width * 4;
	_cuda_mem    = std::make_shared<::nvidia::cuda::memory>(_cuda, pitch * height);
	_ar->image_init(&_ar_image, static_cast<unsigned int>(width), static_cast<unsigned int>(height),
					static_cast<int>(pitch), reinterpret_cast<void*>(_cuda_mem->get()), NVCV_RGBA, NVCV_U8,
					NVCV_INTERLEAVED, NVCV_CUDA);
	_ar->image_dealloc(&_ar_image_bgr);
	_ar->image_alloc(&_ar_image_bgr, static_cast<unsigned int>(width), static_cast<unsigned int>(height), NVCV_BGR,
					 NVCV_U8, NVCV_INTERLEAVED, NVCV_CUDA, 0);

	if (NvCV_Status res =
			_ar->set_object(_ar_tracker.get(), NvAR_Parameter_Input(Image), &_ar_image_bgr, sizeof(NvCVImage));
		res != NVCV_SUCCESS) {
		throw std::runtime_error("_ar_tracker NvAR_Parameter_Input(Image)");
	}
}

void filter::nvidia::face_tracking_instance::roi_refresh()
{
	double_t kalman_q = util::math::lerp<double_t>(1.0, 1e-6, _cfg_roi_stability);
	double_t kalman_r = util::math::lerp<double_t>(std::numeric_limits<double_t>::epsilon(), 1e+2, _cfg_roi_stability);

	_roi_filters[0] = util::math::kalman1D<double_t>{kalman_q, kalman_r, 1.0, _roi_center.first};
	_roi_filters[1] = util::math::kalman1D<double_t>{kalman_q, kalman_r, 1.0, _roi_center.second};
	_roi_filters[2] = util::math::kalman1D<double_t>{kalman_q, kalman_r, 1.0, _roi_size.first};
	_roi_filters[3] = util::math::kalman1D<double_t>{kalman_q, kalman_r, 1.0, _roi_size.second};
}

void filter::nvidia::face_tracking_instance::roi_reset()
{
	_roi_center.first  = static_cast<double_t>(_width) / 2.;
	_roi_center.second = static_cast<double_t>(_height) / 2.;
	_roi_size.first    = static_cast<double_t>(_width);
	_roi_size.second   = static_cast<double_t>(_height);

	roi_refresh();
}

void filter::nvidia::face_tracking_instance::load(obs_data_t* data)
{
	update(data);
}

void filter::nvidia::face_tracking_instance::migrate(obs_data_t* data, std::uint64_t version) {}

void filter::nvidia::face_tracking_instance::update(obs_data_t* data)
{
	_cfg_roi_zoom          = obs_data_get_double(data, SK_ROI_ZOOM) / 100.0;
	_cfg_roi_offset.first  = obs_data_get_double(data, SK_ROI_OFFSET_X) / 100.0;
	_cfg_roi_offset.second = obs_data_get_double(data, SK_ROI_OFFSET_Y) / 100.0;
	_cfg_roi_stability     = obs_data_get_double(data, SK_ROI_STABILITY) / 100.0;

	// Refresh the Region Of Interest
	roi_refresh();
}

void filter::nvidia::face_tracking_instance::video_tick(float seconds)
{
	if (!_ar_ready)
		return;

	// Update Buffers
	uint32_t width  = obs_source_get_base_width(obs_filter_get_target(_self));
	uint32_t height = obs_source_get_base_height(obs_filter_get_target(_self));
	if (((width != _width) || (height != _height)) && width && height)
		try {
			// Recreate things.
			create_image_buffer(width, height);
			_cuda_flush_cache = true;

			// Update Width/Height
			_width  = width;
			_height = height;

			// Reset ROI.
			roi_reset();
		} catch (const std::exception& ex) {
			LOG_ERROR("Error: %s", ex.what());
		}

	_up_to_date = false;
}

void filter::nvidia::face_tracking_instance::video_render(gs_effect_t* effect)
{
	gs::debug_marker gdm_main{gs::debug_color_source, "%s", obs_source_get_name(_self)};
	obs_source_t*    filter_parent  = obs_filter_get_parent(_self);
	obs_source_t*    filter_target  = obs_filter_get_target(_self);
	gs_effect_t*     default_effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);

	if (!filter_parent || !filter_target || !_width || !_height || !_ar_ready) {
		obs_source_skip_video_filter(_self);
		return;
	}

	if (!_up_to_date) {
		{ // Capture the filter stack "below" us.
#ifdef _DEBUG
			auto prof = _profile_capture->track();
#endif
			gs::debug_marker marker{gs::debug_color_render, "%s: Capture", obs_source_get_name(_self)};
			if (obs_source_process_filter_begin(_self, _rt->get_color_format(), OBS_ALLOW_DIRECT_RENDERING)) {
				auto op  = _rt->render(_width, _height);
				vec4 clr = {0., 0., 0., 0.};

				gs_ortho(0, static_cast<float_t>(_width), 0, static_cast<float_t>(_height), 0, 1);
				gs_clear(GS_CLEAR_COLOR, &clr, 0, 0);

				obs_source_process_filter_tech_end(_self, default_effect, _width, _height, "Draw");
			} else {
				obs_source_skip_video_filter(_self);
				return;
			}
		}

		{
			gs::debug_marker marker{gs::debug_color_render, "%s: Nvidia AR SDK", obs_source_get_name(_self)};
			auto             cctx = std::make_shared<::nvidia::cuda::context_stack>(_cuda, _cuda_ctx);

			if (_cuda_flush_cache) {
#ifdef _DEBUG
				auto prof = _profile_cuda_register->track();
#endif
				_cuda_rt_cache = std::make_shared<::nvidia::cuda::gstexture>(_cuda, _rt->get_texture());
				_cuda_rt_cache->map(_cuda_stream);
				_cuda_flush_cache = false;
			}

			{
#ifdef _DEBUG
				auto prof = _profile_cuda_copy->track();
#endif
				::nvidia::cuda::cu_memcpy2d_t mc;
				mc.src_x_in_bytes  = 0;
				mc.src_y           = 0;
				mc.src_memory_type = ::nvidia::cuda::cu_memory_type::ARRAY;
				mc.src_host        = nullptr;
				mc.src_device      = 0;
				mc.src_array       = _cuda_rt_cache->map(_cuda_stream);
				mc.src_pitch       = static_cast<size_t>(_ar_image.pitch);
				mc.dst_x_in_bytes  = 0;
				mc.dst_y           = 0;
				mc.dst_memory_type = ::nvidia::cuda::cu_memory_type::DEVICE;
				mc.dst_host        = 0;
				mc.dst_device      = reinterpret_cast<::nvidia::cuda::cu_device_ptr_t>(_ar_image.pixels);
				mc.dst_array       = 0;
				mc.dst_pitch       = static_cast<size_t>(_ar_image.pitch);
				mc.width_in_bytes  = static_cast<size_t>(_ar_image.pitch);
				mc.height          = _ar_image.height;

				if (::nvidia::cuda::cu_result res = _cuda->cuMemcpy2D(&mc); res != ::nvidia::cuda::cu_result::SUCCESS) {
					obs_source_skip_video_filter(_self);
					return;
				}
			}

			{
#ifdef _DEBUG
				auto prof = _profile_ar_transfer->track();
#endif
				if (NvCV_Status res =
						_ar->image_transfer(&_ar_image, &_ar_image_bgr, 1.0,
											reinterpret_cast<CUstream_st*>(_cuda_stream->get()), &_ar_image_temp);
					res != NVCV_SUCCESS) {
					obs_source_skip_video_filter(_self);
					return;
				}
			}

			{
#ifdef _DEBUG
				auto prof = _profile_ar_run->track();
#endif
				if (NvCV_Status res = _ar->run(_ar_tracker.get()); res != NVCV_SUCCESS) {
					obs_source_skip_video_filter(_self);
					return;
				}
			}
		}

		// Recalculate the region of interest.
		if (_ar_bboxes.num_boxes > 0) {
			double_t aspect = double_t(_width) / double_t(_height);

			// Store values and center.
			double_t bbox_w  = _ar_bboxes.boxes[0].width;
			double_t bbox_h  = _ar_bboxes.boxes[0].height;
			double_t bbox_cx = _ar_bboxes.boxes[0].x + bbox_w / 2.0;
			double_t bbox_cy = _ar_bboxes.boxes[0].y + bbox_h / 2.0;

			// Zoom, Aspect Ratio, Offset
			bbox_h = util::math::lerp<double_t>(_height, bbox_h, _cfg_roi_zoom);
			bbox_h = std::clamp(bbox_h, 10 * aspect, static_cast<double_t>(_height));
			bbox_w = bbox_h * aspect;
			bbox_cx += _ar_bboxes.boxes[0].width * _cfg_roi_offset.first;
			bbox_cy += _ar_bboxes.boxes[0].height * _cfg_roi_offset.second;

			// Fit back into the frame
			// - Above code guarantees that height is never bigger than the height of the frame.
			// - Which also guarantees that width is never bigger than the width of the frame.
			// Only cx and cy need to be adjusted now to always be in the frame.
			bbox_cx = std::clamp(bbox_cx, (bbox_w / 2.), static_cast<double_t>(_width) - (bbox_w / 2.));
			bbox_cy = std::clamp(bbox_cy, (bbox_h / 2.), static_cast<double_t>(_height) - (bbox_h / 2.));

			// Filter values
			auto size_w   = _roi_filters[2].filter(bbox_w);
			auto size_h   = _roi_filters[3].filter(bbox_h);
			auto center_x = _roi_filters[0].filter(bbox_cx);
			auto center_y = _roi_filters[1].filter(bbox_cy);

			// Fix NaN/Infinity
			if (std::isfinite(size_w) && std::isfinite(size_h) && std::isfinite(center_x) && std::isfinite(center_y)) {
				_roi_center.first  = center_x;
				_roi_center.second = center_y;
				_roi_size.first    = size_w;
				_roi_size.second   = size_h;
			} else {
				roi_refresh();
			}
		} else {
			// Todo: Time based return to full frame.
		}

		// Update Region of Interest Geometry.
		{
			auto v0 = _roi_geom.at(0);
			auto v1 = _roi_geom.at(1);
			auto v2 = _roi_geom.at(2);
			auto v3 = _roi_geom.at(3);

			*v0.color = 0xFFFFFFFF;
			*v1.color = 0xFFFFFFFF;
			*v2.color = 0xFFFFFFFF;
			*v3.color = 0xFFFFFFFF;

			vec3_set(v3.position, static_cast<float_t>(_width), static_cast<float_t>(_height), 0.);
			vec3_set(v2.position, v3.position->x, 0., 0.);
			vec3_set(v1.position, 0., v3.position->y, 0.);
			vec3_set(v0.position, 0., 0., 0.);

			vec4_set(
				v0.uv[0],
				static_cast<float_t>((_roi_center.first - _roi_size.first / 2.) / static_cast<double_t>(_width)),
				static_cast<float_t>((_roi_center.second - _roi_size.second / 2.) / static_cast<double_t>(_height)), 0.,
				0.);
			vec4_set(
				v1.uv[0],
				static_cast<float_t>((_roi_center.first - _roi_size.first / 2.) / static_cast<double_t>(_width)),
				static_cast<float_t>((_roi_center.second + _roi_size.second / 2.) / static_cast<double_t>(_height)), 0.,
				0.);
			vec4_set(
				v2.uv[0],
				static_cast<float_t>((_roi_center.first + _roi_size.first / 2.) / static_cast<double_t>(_width)),
				static_cast<float_t>((_roi_center.second - _roi_size.second / 2.) / static_cast<double_t>(_height)), 0.,
				0.);
			vec4_set(
				v3.uv[0],
				static_cast<float_t>((_roi_center.first + _roi_size.first / 2.) / static_cast<double_t>(_width)),
				static_cast<float_t>((_roi_center.second + _roi_size.second / 2.) / static_cast<double_t>(_height)), 0.,
				0.);

			_roi_geom.update();
		}

		_up_to_date = true;
	}

	// Draw Texture
	gs_effect_set_texture(gs_effect_get_param_by_name(effect ? effect : default_effect, "image"),
						  _rt->get_texture()->get_object());
	gs_load_vertexbuffer(_roi_geom.update());
	while (gs_effect_loop(effect ? effect : default_effect, "Draw")) {
		gs_draw(gs_draw_mode::GS_TRISTRIP, 0, _roi_geom.size());
	}
	gs_load_vertexbuffer(nullptr);
}

#ifdef _DEBUG
bool filter::nvidia::face_tracking_instance::button_profile(obs_properties_t* props, obs_property_t* property)
{
	LOG_INFO("Profiling (Total/Avg/99.9/95)");
	LOG_INFO("  %-12s: %8lldµs %8lldµs %8lldµs %8lldµs", "Capture",
			 std::chrono::duration_cast<std::chrono::microseconds>(_profile_capture->total_duration()).count(),
			 static_cast<std::int64_t>(_profile_capture->average_duration() / 1000.0),
			 std::chrono::duration_cast<std::chrono::microseconds>(_profile_capture->percentile(0.999)).count(),
			 std::chrono::duration_cast<std::chrono::microseconds>(_profile_capture->percentile(0.95)).count());
	LOG_INFO("  %-12s: %8lldµs %8lldµs %8lldµs %8lldµs", "Register",
			 std::chrono::duration_cast<std::chrono::microseconds>(_profile_cuda_register->total_duration()).count(),
			 static_cast<std::int64_t>(_profile_cuda_register->average_duration() / 1000.0),
			 std::chrono::duration_cast<std::chrono::microseconds>(_profile_cuda_register->percentile(0.999)).count(),
			 std::chrono::duration_cast<std::chrono::microseconds>(_profile_cuda_register->percentile(0.95)).count());
	LOG_INFO("  %-12s: %8lldµs %8lldµs %8lldµs %8lldµs", "Copy",
			 std::chrono::duration_cast<std::chrono::microseconds>(_profile_cuda_copy->total_duration()).count(),
			 static_cast<std::int64_t>(_profile_cuda_copy->average_duration() / 1000.0),
			 std::chrono::duration_cast<std::chrono::microseconds>(_profile_cuda_copy->percentile(0.999)).count(),
			 std::chrono::duration_cast<std::chrono::microseconds>(_profile_cuda_copy->percentile(0.95)).count());
	LOG_INFO("  %-12s: %8lldµs %8lldµs %8lldµs %8lldµs", "Transfer",
			 std::chrono::duration_cast<std::chrono::microseconds>(_profile_ar_transfer->total_duration()).count(),
			 static_cast<std::int64_t>(_profile_capture->average_duration() / 1000.0),
			 std::chrono::duration_cast<std::chrono::microseconds>(_profile_ar_transfer->percentile(0.999)).count(),
			 std::chrono::duration_cast<std::chrono::microseconds>(_profile_ar_transfer->percentile(0.95)).count());
	LOG_INFO("  %-12s: %8lldµs %8lldµs %8lldµs %8lldµs", "Run",
			 std::chrono::duration_cast<std::chrono::microseconds>(_profile_ar_run->total_duration()).count(),
			 static_cast<std::int64_t>(_profile_ar_run->average_duration() / 1000.0),
			 std::chrono::duration_cast<std::chrono::microseconds>(_profile_ar_run->percentile(0.999)).count(),
			 std::chrono::duration_cast<std::chrono::microseconds>(_profile_ar_run->percentile(0.95)).count());

	return false;
}
#endif

std::shared_ptr<filter::nvidia::face_tracking_factory> filter::nvidia::face_tracking_factory::factory_instance =
	nullptr;

filter::nvidia::face_tracking_factory::face_tracking_factory()
{
	// Try and load CUDA.
	_cuda = std::make_shared<::nvidia::cuda::cuda>();

	// Try and load AR.
	_ar = std::make_shared<::nvidia::ar::ar>();

	// Initialize CUDA
	{
		auto gctx = gs::context{};
#ifdef WIN32
		if (gs_get_device_type() == GS_DEVICE_DIRECT3D_11) {
			_cuda_ctx =
				std::make_shared<::nvidia::cuda::context>(_cuda, reinterpret_cast<ID3D11Device*>(gs_get_device_obj()));
		}
#endif
		if (gs_get_device_type() == GS_DEVICE_OPENGL) {
			throw std::runtime_error("OpenGL not supported.");
		}
	}

	// Info
	_info.id           = "streamfx-nvidia-face-tracking";
	_info.type         = OBS_SOURCE_TYPE_FILTER;
	_info.output_flags = OBS_SOURCE_VIDEO;

	set_resolution_enabled(false);
	finish_setup();
}

filter::nvidia::face_tracking_factory::~face_tracking_factory() {}

const char* filter::nvidia::face_tracking_factory::get_name()
{
	return D_TRANSLATE(ST);
}

void filter::nvidia::face_tracking_factory::get_defaults2(obs_data_t* data)
{
	obs_data_set_default_double(data, SK_ROI_ZOOM, 50.0);
	obs_data_set_default_double(data, SK_ROI_OFFSET_X, 0.0);
	obs_data_set_default_double(data, SK_ROI_OFFSET_Y, -15.0);
	obs_data_set_default_double(data, SK_ROI_STABILITY, 50.0);
}

obs_properties_t* filter::nvidia::face_tracking_factory::get_properties2(filter::nvidia::face_tracking_instance* data)
{
	obs_properties_t* pr = obs_properties_create();

	{
		auto grp = obs_properties_create();
		obs_properties_add_group(pr, ST_ROI, D_TRANSLATE(ST_ROI), OBS_GROUP_NORMAL, grp);
		{
			auto p =
				obs_properties_add_float_slider(grp, SK_ROI_STABILITY, D_TRANSLATE(ST_ROI_STABILITY), 0, 100.0, 0.01);
			obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_ROI_STABILITY)));
			obs_property_float_set_suffix(p, " %");
		}
		{
			auto p = obs_properties_add_float_slider(grp, SK_ROI_ZOOM, D_TRANSLATE(ST_ROI_ZOOM), 0, 200.0, 0.01);
			obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_ROI_ZOOM)));
			obs_property_float_set_suffix(p, " %");
		}
		{
			auto grp2 = obs_properties_create();
			obs_properties_add_group(grp, ST_ROI_OFFSET, D_TRANSLATE(ST_ROI_OFFSET), OBS_GROUP_NORMAL, grp2);

			{
				auto p = obs_properties_add_float_slider(grp2, SK_ROI_OFFSET_X, D_TRANSLATE(ST_ROI_OFFSET_X), -50.0,
														 50.0, 0.01);
				obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_ROI_OFFSET_X)));
				obs_property_float_set_suffix(p, " %");
			}
			{
				auto p = obs_properties_add_float_slider(grp2, SK_ROI_OFFSET_Y, D_TRANSLATE(ST_ROI_OFFSET_Y), -50.0,
														 50.0, 0.01);
				obs_property_set_long_description(p, D_TRANSLATE(D_DESC(ST_ROI_OFFSET_Y)));
				obs_property_float_set_suffix(p, " %");
			}
		}
	}
#ifdef _DEBUG
	{
		auto p = obs_properties_add_button2(
			pr, "Profile", "Profile",
			[](obs_properties_t* props, obs_property_t* property, void* data) {
				return reinterpret_cast<filter::nvidia::face_tracking_instance*>(data)->button_profile(props, property);
			},
			data);
	}
#endif

	return pr;
}

std::shared_ptr<::nvidia::cuda::cuda> filter::nvidia::face_tracking_factory::get_cuda()
{
	return _cuda;
}

std::shared_ptr<::nvidia::cuda::context> filter::nvidia::face_tracking_factory::get_cuda_context()
{
	return _cuda_ctx;
}

std::shared_ptr<::nvidia::ar::ar> filter::nvidia::face_tracking_factory::get_ar()
{
	return _ar;
}
