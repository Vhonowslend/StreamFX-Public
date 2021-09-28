// Copyright (c) 2021 Michael Fabian Dirks <info@xaymar.com>
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

#include "nvidia-vfx-denoising.hpp"
#include <cmath>
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
#define ST_PREFIX "<nvidia::vfx::denoising::denoising> "
#define D_LOG_ERROR(...) P_LOG_ERROR(ST_PREFIX __VA_ARGS__)
#define D_LOG_WARNING(...) P_LOG_WARN(ST_PREFIX __VA_ARGS__)
#define D_LOG_INFO(...) P_LOG_INFO(ST_PREFIX __VA_ARGS__)
#define D_LOG_DEBUG(...) P_LOG_DEBUG(ST_PREFIX __VA_ARGS__)
#endif

streamfx::nvidia::vfx::denoising::~denoising()
{
	auto gctx = ::streamfx::obs::gs::context();
	auto cctx = ::streamfx::nvidia::cuda::obs::get()->get_context()->enter();

	_fx.reset();

	// Clean up state buffer.
	_nvcuda->get_cuda()->cuMemFree(_state);

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

streamfx::nvidia::vfx::denoising::denoising()
	: _nvcuda(::streamfx::nvidia::cuda::obs::get()), _nvcvi(::streamfx::nvidia::cv::cv::get()),
	  _nvvfx(::streamfx::nvidia::vfx::vfx::get()), _state(0), _state_size(0), _strength(1.), _input(), _source(),
	  _destination(), _convert_to_u8(), _output(), _tmp(), _dirty(true)
{
	// Enter Graphics and CUDA context.
	auto gctx = ::streamfx::obs::gs::context();
	auto cctx = ::streamfx::nvidia::cuda::obs::get()->get_context()->enter();

	{ // Try & Create the Denoising effect.
		::streamfx::nvidia::vfx::handle_t handle;
		if (auto res = _nvvfx->NvVFX_CreateEffect(::streamfx::nvidia::vfx::EFFECT_DENOISING, &handle);
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
	resize(160, 90);

	// Load the effect.
	load();
}

void streamfx::nvidia::vfx::denoising::set_strength(float strength)
{
	std::swap(_strength, strength);

	// If anything was changed, flag the effect as dirty.
	if (!::streamfx::util::math::is_close<float>(_strength, strength, 0.01f))
		_dirty = true;

	// Update Effect
	auto gctx = ::streamfx::obs::gs::context();
	auto cctx = _nvcuda->get_context()->enter();
	if (auto res = _nvvfx->NvVFX_SetF32(_fx.get(), ::streamfx::nvidia::vfx::PARAMETER_STRENGTH, _strength);
		res != ::streamfx::nvidia::cv::result::SUCCESS) {
		D_LOG_ERROR("Failed to set '%s' to %1.3f.", ::streamfx::nvidia::vfx::PARAMETER_STRENGTH, _strength);
	};
}

float streamfx::nvidia::vfx::denoising::strength()
{
	return _strength;
}

void streamfx::nvidia::vfx::denoising::size(std::pair<uint32_t, uint32_t>& size)
{
	constexpr uint32_t min_width  = 142;
	constexpr uint32_t min_height = 80;
	uint32_t           max_width  = 1920;
	uint32_t           max_height = 1080;

	// Calculate Size
	if (size.first > size.second) {
		// Dominant Width
		double ar   = static_cast<double>(size.second) / static_cast<double>(size.first);
		size.first  = std::clamp<uint32_t>(size.first, min_width, max_width);
		size.second = std::clamp<uint32_t>(static_cast<uint32_t>(std::lround(static_cast<double>(size.first) * ar)),
										   min_height, max_height);
	} else {
		// Dominant Height
		double ar   = static_cast<double>(size.first) / static_cast<double>(size.second);
		size.second = std::clamp<uint32_t>(size.second, min_height, max_height);
		size.first  = std::clamp<uint32_t>(static_cast<uint32_t>(std::lround(static_cast<double>(size.second) * ar)),
                                          min_width, max_width);
	}
}

std::shared_ptr<::streamfx::obs::gs::texture>
	streamfx::nvidia::vfx::denoising::process(std::shared_ptr<::streamfx::obs::gs::texture> in)
{
	// Enter Graphics and CUDA context.
	auto gctx = ::streamfx::obs::gs::context();
	auto cctx = _nvcuda->get_context()->enter();

#ifdef ENABLE_PROFILING
	::streamfx::obs::gs::debug_marker profiler1{::streamfx::obs::gs::debug_color_magenta, "NvVFX Denoising"};
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
		if (auto res = _nvcvi->NvCVImage_Transfer(_input->get_image(), _convert_to_fp32->get_image(), 1.f / 255.f,
												  _nvcuda->get_stream()->get(), _tmp->get_image());
			res != ::streamfx::nvidia::cv::result::SUCCESS) {
			D_LOG_ERROR("Failed to transfer input to processing source due to error: %s",
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
		if (auto res = _nvcvi->NvCVImage_Transfer(_destination->get_image(), _convert_to_u8->get_image(), 255.f,
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

void streamfx::nvidia::vfx::denoising::resize(uint32_t width, uint32_t height)
{
	auto gctx = ::streamfx::obs::gs::context();
	auto cctx = ::streamfx::nvidia::cuda::obs::get()->get_context()->enter();

	if (!_tmp) {
		_tmp = std::make_shared<::streamfx::nvidia::cv::image>(
			width, height, ::streamfx::nvidia::cv::pixel_format::RGBA, ::streamfx::nvidia::cv::component_type::UINT8,
			::streamfx::nvidia::cv::component_layout::PLANAR, ::streamfx::nvidia::cv::memory_location::GPU, 1);
	}

	// Input Size was changed.
	if (!_input || !_source || !_destination || !_output || !_state || (width != _input->get_texture()->get_width())
		|| (height != _input->get_texture()->get_height()) || (width != _output->get_texture()->get_width())
		|| (height != _output->get_texture()->get_height())) {
		if (_input) {
			_input->resize(width, height);
		} else {
			_input = std::make_shared<::streamfx::nvidia::cv::texture>(width, height, GS_RGBA_UNORM);
		}

		if (_convert_to_fp32) {
			_convert_to_fp32->resize(width, height);
		} else {
			_convert_to_fp32 = std::make_shared<::streamfx::nvidia::cv::image>(
				width, height, ::streamfx::nvidia::cv::pixel_format::RGBA, ::streamfx::nvidia::cv::component_type::FP32,
				::streamfx::nvidia::cv::component_layout::PLANAR, ::streamfx::nvidia::cv::memory_location::GPU, 1);
		}

		if (_source) {
			_source->resize(width, height);
		} else {
			_source = std::make_shared<::streamfx::nvidia::cv::image>(
				width, height, ::streamfx::nvidia::cv::pixel_format::BGR, ::streamfx::nvidia::cv::component_type::FP32,
				::streamfx::nvidia::cv::component_layout::PLANAR, ::streamfx::nvidia::cv::memory_location::GPU, 1);
		}

		if (auto res = _nvvfx->NvVFX_SetImage(_fx.get(), ::streamfx::nvidia::vfx::PARAMETER_INPUT_IMAGE_0,
											  _source->get_image());
			res != ::streamfx::nvidia::cv::result::SUCCESS) {
			D_LOG_ERROR("Failed to set input image due to error: %s", _nvcvi->NvCV_GetErrorStringFromCode(res));
			throw std::runtime_error("SetImage failed.");
		}

		if (_destination) {
			_destination->resize(width, height);
		} else {
			_destination = std::make_shared<::streamfx::nvidia::cv::image>(
				width, height, ::streamfx::nvidia::cv::pixel_format::BGR, ::streamfx::nvidia::cv::component_type::FP32,
				::streamfx::nvidia::cv::component_layout::PLANAR, ::streamfx::nvidia::cv::memory_location::GPU, 1);
		}

		if (_convert_to_u8) {
			_convert_to_u8->resize(width, height);
		} else {
			_convert_to_u8 = std::make_shared<::streamfx::nvidia::cv::image>(
				width, height, ::streamfx::nvidia::cv::pixel_format::RGBA,
				::streamfx::nvidia::cv::component_type::UINT8, ::streamfx::nvidia::cv::component_layout::INTERLEAVED,
				::streamfx::nvidia::cv::memory_location::GPU, 1);
		}

		if (_output) {
			_output->resize(width, height);
		} else {
			_output = std::make_shared<::streamfx::nvidia::cv::texture>(width, height, GS_RGBA_UNORM);
		}

		if (auto res = _nvvfx->NvVFX_SetImage(_fx.get(), ::streamfx::nvidia::vfx::PARAMETER_OUTPUT_IMAGE_0,
											  _destination->get_image());
			res != ::streamfx::nvidia::cv::result::SUCCESS) {
			D_LOG_ERROR("Failed to set output image due to error: %s", _nvcvi->NvCV_GetErrorStringFromCode(res));
			throw std::runtime_error("SetImage failed.");
		}

		{ // Reallocate and clean state.
			_nvvfx->NvVFX_GetU32(_fx.get(), ::streamfx::nvidia::vfx::PARAMETER_STATE_SIZE, &_state_size);
			if (_state) {
				//_nvcuda->get_cuda()->cuMemFree(_state);
			}
			_nvcuda->get_cuda()->cuMemAlloc(&_state, _state_size);
			_nvcuda->get_cuda()->cuMemsetD8(_state, 0, _state_size);

			_states[0] = reinterpret_cast<void*>(_state);
			if (auto res = _nvvfx->NvVFX_SetObject(_fx.get(), ::streamfx::nvidia::vfx::PARAMETER_STATE,
												   reinterpret_cast<void*>(_states));
				res != ::streamfx::nvidia::cv::result::SUCCESS) {
				D_LOG_ERROR("Failed to set state due to error: %s", _nvcvi->NvCV_GetErrorStringFromCode(res));
				throw std::runtime_error("SetObject failed.");
			}
		}

		_dirty = true;
	}
}

void streamfx::nvidia::vfx::denoising::load()
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
