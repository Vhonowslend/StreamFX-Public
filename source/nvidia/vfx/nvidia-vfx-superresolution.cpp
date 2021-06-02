// Copyright (c) 2020 Michael Fabian Dirks <info@xaymar.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "nvidia-vfx-superresolution.hpp"
#include <utility>
#include "obs/gs/gs-helper.hpp"
#include "util/util-logging.hpp"
#include "util/utility.hpp"

#ifdef _DEBUG
#define ST_PREFIX "<%s> "
#define D_LOG_ERROR(x, ...) P_LOG_ERROR(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_WARNING(x, ...) P_LOG_WARN(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_INFO(x, ...) P_LOG_INFO(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_DEBUG(x, ...) P_LOG_DEBUG(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#else
#define ST_PREFIX "<nvidia::vfx::superresolution::superresolution> "
#define D_LOG_ERROR(...) P_LOG_ERROR(ST_PREFIX __VA_ARGS__)
#define D_LOG_WARNING(...) P_LOG_WARN(ST_PREFIX __VA_ARGS__)
#define D_LOG_INFO(...) P_LOG_INFO(ST_PREFIX __VA_ARGS__)
#define D_LOG_DEBUG(...) P_LOG_DEBUG(ST_PREFIX __VA_ARGS__)
#endif

streamfx::nvidia::vfx::superresolution::~superresolution()
{
	auto gctx = ::streamfx::obs::gs::context();
	auto cctx = ::streamfx::nvidia::cuda::obs::get()->get_context()->enter();

	_fx.reset();

	// Clean up any CUDA resources in use.
	_input.reset();
	_convert_to_fp32.reset();
	_source.reset();
	_destination.reset();
	_convert_to_u8.reset();
	_output.reset();
	_tmp.reset();

	// Release CUDA, CVImage, and Video Effects SDK.
	_nvvfx.reset();
	_nvcvi.reset();
	_nvcuda.reset();
}

streamfx::nvidia::vfx::superresolution::superresolution()
	: _nvcuda(::streamfx::nvidia::cuda::obs::get()), _nvcvi(::streamfx::nvidia::cv::cv::get()),
	  _nvvfx(::streamfx::nvidia::vfx::vfx::get()), _strength(1.), _scale(1.5), _input(), _source(), _destination(),
	  _convert_to_u8(), _output(), _tmp(), _dirty(true)
{
	// Enter Graphics and CUDA context.
	auto gctx = ::streamfx::obs::gs::context();
	auto cctx = ::streamfx::nvidia::cuda::obs::get()->get_context()->enter();

	{ // Try & Create the Super-Resolution effect.
		::streamfx::nvidia::vfx::handle_t handle;
		if (auto res = _nvvfx->NvVFX_CreateEffect(::streamfx::nvidia::vfx::EFFECT_SUPERRESOLUTION, &handle);
			res != ::streamfx::nvidia::cv::result::SUCCESS) {
			D_LOG_ERROR("Failed to create effect due to error: %s", _nvcvi->NvCV_GetErrorStringFromCode(res));
			throw std::runtime_error("CreateEffect failed.");
		}

		_fx = std::shared_ptr<void>(handle, [](::streamfx::nvidia::vfx::handle_t handle) {
			::streamfx::nvidia::vfx::vfx::get()->NvVFX_DestroyEffect(handle);
		});
	}

	// Assign the appropriate CUDA stream.
	if (auto res = _nvvfx->NvVFX_SetCudaStream(_fx.get(), ::streamfx::nvidia::vfx::PARAMETER_CUDA_STREAM,
											   _nvcuda->get_stream()->get());
		res != ::streamfx::nvidia::cv::result::SUCCESS) {
		D_LOG_ERROR("Failed to set CUDA stream due to error: %s", _nvcvi->NvCV_GetErrorStringFromCode(res));
		throw std::runtime_error("SetCudaStream failed.");
	}

	// Set the proper model directory.
	if (auto res = _nvvfx->NvVFX_SetString(_fx.get(), ::streamfx::nvidia::vfx::PARAMETER_MODEL_DIRECTORY,
										   _nvvfx->model_path().generic_u8string().c_str());
		res != ::streamfx::nvidia::cv::result::SUCCESS) {
		D_LOG_ERROR("Failed to set model directory due to error: %s", _nvcvi->NvCV_GetErrorStringFromCode(res));
		throw std::runtime_error("SetString failed.");
	}

	// Set the strength, scale and buffers.
	set_strength(_strength);
	set_scale(_scale);
	resize(160, 90);

	// Load the effect.
	load();
}

void streamfx::nvidia::vfx::superresolution::set_strength(float strength)
{
	strength = (strength >= .5f) ? 1.f : 0.f;
	std::swap(_strength, strength);

	// If anything was changed, flag the effect as dirty.
	if (!::streamfx::util::math::is_close<float>(_strength, strength, 0.01))
		_dirty = true;

	// Update Effect
	uint32_t value = (_strength >= .5f) ? 1 : 0;
	auto     gctx  = ::streamfx::obs::gs::context();
	auto     cctx  = ::streamfx::nvidia::cuda::obs::get()->get_context()->enter();
	if (auto res = _nvvfx->NvVFX_SetU32(_fx.get(), ::streamfx::nvidia::vfx::PARAMETER_STRENGTH, value);
		res != ::streamfx::nvidia::cv::result::SUCCESS) {
		D_LOG_ERROR("Failed to set '%s' to %lu.", ::streamfx::nvidia::vfx::PARAMETER_STRENGTH, value);
	};
}

float streamfx::nvidia::vfx::superresolution::strength()
{
	return _strength;
}

void streamfx::nvidia::vfx::superresolution::set_scale(float scale)
{
	// Limit to acceptable range.
	scale = std::clamp<float>(scale, 1., 4.);

	// Match to nearest scale.
	std::pair<float, float> minimal = {0., std::numeric_limits<float>::max()};
	std::vector<float>      deltas{
        1. + (1. / 3.), 1.5, 2.0, 3.0, 4.0,
    };
	for (float delta : deltas) {
		float value = abs(delta - scale);
		if (minimal.second > value) {
			minimal.first  = delta;
			minimal.second = value;
		}
	}

	// If anything was changed, flag the effect as dirty.
	if (!::streamfx::util::math::is_close<float>(_scale, minimal.first, 0.01))
		_dirty = true;

	_scale = minimal.first;
}

float streamfx::nvidia::vfx::superresolution::scale()
{
	return _scale;
}

void streamfx::nvidia::vfx::superresolution::size(std::pair<uint32_t, uint32_t> const& size,
												  std::pair<uint32_t, uint32_t>&       input_size,
												  std::pair<uint32_t, uint32_t>&       output_size)
{
	constexpr uint32_t min_width  = 160;
	constexpr uint32_t min_height = 90;
	uint32_t           max_width  = 0;
	uint32_t           max_height = 0;

	if (_scale > 3.0) {
		max_width  = 960;
		max_height = 540;
	} else if (_scale > 2.0) {
		max_width  = 1280;
		max_height = 720;
	} else {
		max_width  = 1920;
		max_height = 1080;
	}

	// Calculate Input Size
	if (input_size.first > input_size.second) {
		// Dominant Width
		double ar         = static_cast<double>(input_size.second) / static_cast<double>(input_size.first);
		input_size.first  = std::clamp<uint32_t>(input_size.first, min_width, max_width);
		input_size.second = std::clamp<uint32_t>(
			static_cast<uint32_t>(round(static_cast<double>(input_size.first) * ar)), min_height, max_height);
	} else {
		// Dominant Height
		double ar         = static_cast<double>(input_size.first) / static_cast<double>(input_size.second);
		input_size.second = std::clamp<uint32_t>(input_size.second, min_height, max_height);
		input_size.first  = std::clamp<uint32_t>(
            static_cast<uint32_t>(round(static_cast<double>(input_size.second) * ar)), min_width, max_width);
	}

	// Calculate Output Size.
	output_size.first  = static_cast<uint32_t>(input_size.first * _scale);
	output_size.second = static_cast<uint32_t>(input_size.second * _scale);
}

std::shared_ptr<::streamfx::obs::gs::texture>
	streamfx::nvidia::vfx::superresolution::process(std::shared_ptr<::streamfx::obs::gs::texture> in)
{
	// Enter Graphics and CUDA context.
	auto gctx = ::streamfx::obs::gs::context();
	auto cctx = _nvcuda->get_context()->enter();

#ifdef ENABLE_PROFILING
	::streamfx::obs::gs::debug_marker profiler1{::streamfx::obs::gs::debug_color_magenta, "NvVFX Super-Resolution"};
#endif

	// Resize if the size or scale was changed.
	resize(in->get_width(), in->get_height());

	// Reload effect if dirty.
	if (_dirty) {
		load();
	}

	{ // Copy parameter to input.
#ifdef ENABLE_PROFILING
		::streamfx::obs::gs::debug_marker profiler1{::streamfx::obs::gs::debug_color_copy, "Copy In -> Input"};
#endif
		gs_copy_texture(_input->get_texture()->get_object(), in->get_object());
	}

	{ // Convert Input to Source format
#ifdef ENABLE_PROFILING
		::streamfx::obs::gs::debug_marker profiler1{::streamfx::obs::gs::debug_color_convert,
													"Convert Input -> Source"};
#endif
		if (auto res = _nvcvi->NvCVImage_Transfer(_input->get_image(), _convert_to_fp32->get_image(), 1.f,
												  _nvcuda->get_stream()->get(), _tmp->get_image());
			res != ::streamfx::nvidia::cv::result::SUCCESS) {
			D_LOG_ERROR("Failed to transfer processing result to output due to error: %s",
						_nvcvi->NvCV_GetErrorStringFromCode(res));
			throw std::runtime_error("Transfer failed.");
		}
	}

	{ // Copy input to source.
#ifdef ENABLE_PROFILING
		::streamfx::obs::gs::debug_marker profiler1{::streamfx::obs::gs::debug_color_copy, "Copy Input -> Source"};
#endif
		if (auto res = _nvcvi->NvCVImage_Transfer(_convert_to_fp32->get_image(), _source->get_image(), 1.f,
												  _nvcuda->get_stream()->get(), _tmp->get_image());
			res != ::streamfx::nvidia::cv::result::SUCCESS) {
			D_LOG_ERROR("Failed to transfer input to processing source due to error: %s",
						_nvcvi->NvCV_GetErrorStringFromCode(res));
			throw std::runtime_error("Transfer failed.");
		}
	}

	{ // Process source to destination.
#ifdef ENABLE_PROFILING
		::streamfx::obs::gs::debug_marker profiler1{::streamfx::obs::gs::debug_color_cache, "Process"};
#endif
		if (auto res = _nvvfx->NvVFX_Run(_fx.get(), 0); res != ::streamfx::nvidia::cv::result::SUCCESS) {
			D_LOG_ERROR("Failed to process due to error: %s", _nvcvi->NvCV_GetErrorStringFromCode(res));
			throw std::runtime_error("Run failed.");
		}
	}

	{ // Convert Destination to Output format
#ifdef ENABLE_PROFILING
		::streamfx::obs::gs::debug_marker profiler1{::streamfx::obs::gs::debug_color_convert,
													"Convert Destination -> Output"};
#endif
		if (auto res = _nvcvi->NvCVImage_Transfer(_destination->get_image(), _convert_to_u8->get_image(), 1.f,
												  _nvcuda->get_stream()->get(), _tmp->get_image());
			res != ::streamfx::nvidia::cv::result::SUCCESS) {
			D_LOG_ERROR("Failed to transfer processing result to output due to error: %s",
						_nvcvi->NvCV_GetErrorStringFromCode(res));
			throw std::runtime_error("Transfer failed.");
		}
	}

	{ // Copy destination to output.
#ifdef ENABLE_PROFILING
		::streamfx::obs::gs::debug_marker profiler1{::streamfx::obs::gs::debug_color_copy,
													"Copy Destination -> Output"};
#endif
		if (auto res = _nvcvi->NvCVImage_Transfer(_convert_to_u8->get_image(), _output->get_image(), 1.,
												  _nvcuda->get_stream()->get(), _tmp->get_image());
			res != ::streamfx::nvidia::cv::result::SUCCESS) {
			D_LOG_ERROR("Failed to transfer processing result to output due to error: %s",
						_nvcvi->NvCV_GetErrorStringFromCode(res));
			throw std::runtime_error("Transfer failed.");
		}
	}

	// Return output.
	return _output->get_texture();
}

void streamfx::nvidia::vfx::superresolution::resize(uint32_t width, uint32_t height)
{
	uint32_t out_width  = static_cast<uint32_t>(width * _scale);
	uint32_t out_height = static_cast<uint32_t>(height * _scale);

	auto gctx = ::streamfx::obs::gs::context();
	auto cctx = ::streamfx::nvidia::cuda::obs::get()->get_context()->enter();

	if (!_tmp) {
		_tmp = std::make_shared<::streamfx::nvidia::cv::image>(
			out_width, out_height, ::streamfx::nvidia::cv::pixel_format::RGBA,
			::streamfx::nvidia::cv::component_type::UINT8, ::streamfx::nvidia::cv::component_layout::PLANAR,
			::streamfx::nvidia::cv::memory_location::GPU, 1);
	}

	// Input Size was changed.
	if (!_input || !_source || (width != _input->get_texture()->get_width())
		|| (height != _input->get_texture()->get_height())) {
		if (_input) {
			_input->resize(width, height);
		} else {
			_input = std::make_shared<::streamfx::nvidia::cv::texture>(width, height, GS_RGBA_UNORM);
		}

		if (_source) {
			_source->resize(width, height);
		} else {
			_source = std::make_shared<::streamfx::nvidia::cv::image>(
				width, height, ::streamfx::nvidia::cv::pixel_format::BGR, ::streamfx::nvidia::cv::component_type::FP32,
				::streamfx::nvidia::cv::component_layout::PLANAR, ::streamfx::nvidia::cv::memory_location::GPU, 1);
		}

		if (_convert_to_fp32) {
			_convert_to_fp32->reallocate(out_width, out_height, ::streamfx::nvidia::cv::pixel_format::RGBA,
										 ::streamfx::nvidia::cv::component_type::FP32,
										 ::streamfx::nvidia::cv::component_layout::PLANAR,
										 ::streamfx::nvidia::cv::memory_location::GPU, 1);
		} else {
			_convert_to_fp32 = std::make_shared<::streamfx::nvidia::cv::image>(
				out_width, out_height, ::streamfx::nvidia::cv::pixel_format::RGBA,
				::streamfx::nvidia::cv::component_type::FP32, ::streamfx::nvidia::cv::component_layout::PLANAR,
				::streamfx::nvidia::cv::memory_location::GPU, 1);
		}

		if (auto res = _nvvfx->NvVFX_SetImage(_fx.get(), ::streamfx::nvidia::vfx::PARAMETER_INPUT_IMAGE_0,
											  _source->get_image());
			res != ::streamfx::nvidia::cv::result::SUCCESS) {
			D_LOG_ERROR("Failed to set input image due to error: %s", _nvcvi->NvCV_GetErrorStringFromCode(res));
			throw std::runtime_error("SetImage failed.");
		}

		_dirty = true;
	}

	// Input Size or Scale was changed.
	if (!_destination || !_output || (out_width != _output->get_texture()->get_width())
		|| (out_height != _output->get_texture()->get_height())) {
		if (_destination) {
			_destination->resize(out_width, out_height);
		} else {
			_destination = std::make_shared<::streamfx::nvidia::cv::image>(
				out_width, out_height, ::streamfx::nvidia::cv::pixel_format::BGR,
				::streamfx::nvidia::cv::component_type::FP32, ::streamfx::nvidia::cv::component_layout::PLANAR,
				::streamfx::nvidia::cv::memory_location::GPU, 1);
		}

		if (_output) {
			_output->resize(out_width, out_height);
		} else {
			_output = std::make_shared<::streamfx::nvidia::cv::texture>(out_width, out_height, GS_RGBA_UNORM);
		}

		if (_convert_to_u8) {
			_convert_to_u8->reallocate(out_width, out_height, ::streamfx::nvidia::cv::pixel_format::RGBA,
									   ::streamfx::nvidia::cv::component_type::UINT8,
									   ::streamfx::nvidia::cv::component_layout::INTERLEAVED,
									   ::streamfx::nvidia::cv::memory_location::GPU, 1);
		} else {
			_convert_to_u8 = std::make_shared<::streamfx::nvidia::cv::image>(
				out_width, out_height, ::streamfx::nvidia::cv::pixel_format::RGBA,
				::streamfx::nvidia::cv::component_type::UINT8, ::streamfx::nvidia::cv::component_layout::INTERLEAVED,
				::streamfx::nvidia::cv::memory_location::GPU, 1);
		}

		if (auto res = _nvvfx->NvVFX_SetImage(_fx.get(), ::streamfx::nvidia::vfx::PARAMETER_OUTPUT_IMAGE_0,
											  _destination->get_image());
			res != ::streamfx::nvidia::cv::result::SUCCESS) {
			D_LOG_ERROR("Failed to set output image due to error: %s", _nvcvi->NvCV_GetErrorStringFromCode(res));
			throw std::runtime_error("SetImage failed.");
		}

		_dirty = true;
	}
}

void streamfx::nvidia::vfx::superresolution::load()
{
	auto gctx = ::streamfx::obs::gs::context();
	{
		auto cctx = ::streamfx::nvidia::cuda::obs::get()->get_context()->enter();
		if (auto res = _nvvfx->NvVFX_SetCudaStream(_fx.get(), ::streamfx::nvidia::vfx::PARAMETER_CUDA_STREAM,
												   _nvcuda->get_stream()->get());
			res != ::streamfx::nvidia::cv::result::SUCCESS) {
			D_LOG_ERROR("Failed to set CUDA stream due to error: %s", _nvcvi->NvCV_GetErrorStringFromCode(res));
			throw std::runtime_error("SetCudaStream failed.");
		}
	}

	{
		auto cctx = ::streamfx::nvidia::cuda::obs::get()->get_context()->enter();
		if (auto res = _nvvfx->NvVFX_Load(_fx.get()); res != ::streamfx::nvidia::cv::result::SUCCESS) {
			D_LOG_ERROR("Failed to initialize effect due to error: %s", _nvcvi->NvCV_GetErrorStringFromCode(res));
			throw std::runtime_error("Load failed.");
		}
	}

	_dirty = false;
}
