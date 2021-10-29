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

#include "nvidia-ar-feature.hpp"
#include "obs/gs/gs-helper.hpp"
#include "util/util-logging.hpp"
#include "util/util-platform.hpp"

#ifdef _DEBUG
#define ST_PREFIX "<%s> "
#define D_LOG_ERROR(x, ...) P_LOG_ERROR(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_WARNING(x, ...) P_LOG_WARN(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_INFO(x, ...) P_LOG_INFO(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_DEBUG(x, ...) P_LOG_DEBUG(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#else
#define ST_PREFIX "<nvidia::ar::feature> "
#define D_LOG_ERROR(...) P_LOG_ERROR(ST_PREFIX __VA_ARGS__)
#define D_LOG_WARNING(...) P_LOG_WARN(ST_PREFIX __VA_ARGS__)
#define D_LOG_INFO(...) P_LOG_INFO(ST_PREFIX __VA_ARGS__)
#define D_LOG_DEBUG(...) P_LOG_DEBUG(ST_PREFIX __VA_ARGS__)
#endif

streamfx::nvidia::ar::feature::~feature()
{
	D_LOG_DEBUG("Finalizing... (Addr: 0x%" PRIuPTR ")", this);
}

streamfx::nvidia::ar::feature::feature(feature_t feature)
	: _nvcuda(::streamfx::nvidia::cuda::obs::get()), _nvcv(::streamfx::nvidia::cv::cv::get()),
	  _nvar(::streamfx::nvidia::ar::ar::get()), _fx()
{
	D_LOG_DEBUG("Initializating... (Addr: 0x%" PRIuPTR ")", this);
	auto gctx = ::streamfx::obs::gs::context();
	auto cctx = cuda::obs::get()->get_context()->enter();

	// Create the Effect/Feature.
	::streamfx::nvidia::ar::handle_t handle;
	if (cv::result res = _nvar->NvAR_Create(feature, &handle); res != cv::result::SUCCESS) {
		throw cv::exception("Failed to create feature.", res);
	}
	_fx =
		std::shared_ptr<void>(handle, [this](::streamfx::nvidia::ar::handle_t handle) { _nvar->NvAR_Destroy(handle); });

	// Set CUDA stream and model directory.
	set(P_NVAR_CONFIG "CUDAStream", _nvcuda->get_stream());
	_model_path = _nvar->get_model_path().generic_u8string();
	set(P_NVAR_CONFIG "ModelDir", _model_path);
}

streamfx::nvidia::cv::result streamfx::nvidia::ar::feature::get(parameter_t param, std::string_view& value)
{
	const char* cvalue = nullptr;
	cv::result  res    = get(param, cvalue);
	if (res == cv::result::SUCCESS) {
		if (cvalue) {
			value.swap(std::string_view(cvalue));
		} else {
			value.swap(std::string_view());
		}
	}
	return res;
}

streamfx::nvidia::cv::result streamfx::nvidia::ar::feature::get(parameter_t param, std::string& value)
{
	const char* cvalue = nullptr;
	cv::result  res    = get(param, cvalue);
	if (res == cv::result::SUCCESS) {
		if (cvalue) {
			value = cvalue;
		} else {
			value.clear();
		}
	}
	return res;
}
