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

#include "nvidia-vfx-effect.hpp"
#include "obs/gs/gs-helper.hpp"
#include "util/util-logging.hpp"

#ifdef _DEBUG
#define ST_PREFIX "<%s> "
#define D_LOG_ERROR(x, ...) P_LOG_ERROR(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_WARNING(x, ...) P_LOG_WARN(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_INFO(x, ...) P_LOG_INFO(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_DEBUG(x, ...) P_LOG_DEBUG(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#else
#define ST_PREFIX "<nvidia::vfx::feature> "
#define D_LOG_ERROR(...) P_LOG_ERROR(ST_PREFIX __VA_ARGS__)
#define D_LOG_WARNING(...) P_LOG_WARN(ST_PREFIX __VA_ARGS__)
#define D_LOG_INFO(...) P_LOG_INFO(ST_PREFIX __VA_ARGS__)
#define D_LOG_DEBUG(...) P_LOG_DEBUG(ST_PREFIX __VA_ARGS__)
#endif

using namespace ::streamfx::nvidia;

vfx::effect::~effect()
{
	auto gctx = ::streamfx::obs::gs::context();
	auto cctx = cuda::obs::get()->get_context()->enter();

	_fx.reset();
	_nvvfx.reset();
	_nvcvi.reset();
	_nvcuda.reset();
}

vfx::effect::effect(effect_t effect)
	: _nvcuda(cuda::obs::get()), _nvcvi(cv::cv::get()), _nvvfx(vfx::vfx::get()), _fx(), _fx_dirty(true)
{
	auto gctx = ::streamfx::obs::gs::context();
	auto cctx = cuda::obs::get()->get_context()->enter();

	// Create the Effect/Feature.
	::vfx::handle_t handle;
	if (cv::result res = _nvvfx->NvVFX_CreateEffect(effect, &handle); res != cv::result::SUCCESS) {
		D_LOG_ERROR("Unable to create effect: %s", _nvcvi->NvCV_GetErrorStringFromCode(res));
		throw std::runtime_error("Unable to create effect.");
	}

	_fx = std::shared_ptr<void>(handle, [](::vfx::handle_t handle) { ::vfx::vfx::get()->NvVFX_DestroyEffect(handle); });
}

cv::result vfx::effect::get(parameter_t param, std::string_view& value)
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

cv::result vfx::effect::get(parameter_t param, std::string& value)
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
