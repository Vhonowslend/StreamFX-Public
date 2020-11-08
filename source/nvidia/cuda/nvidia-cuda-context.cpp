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

#include "nvidia-cuda-context.hpp"
#include <cassert>
#include <stdexcept>
#include "util/util-logging.hpp"

#ifdef _DEBUG
#define ST_PREFIX "<%s> "
#define D_LOG_ERROR(x, ...) P_LOG_ERROR(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_WARNING(x, ...) P_LOG_WARN(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_INFO(x, ...) P_LOG_INFO(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_DEBUG(x, ...) P_LOG_DEBUG(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#else
#define ST_PREFIX "<nvidia::cuda::context> "
#define D_LOG_ERROR(...) P_LOG_ERROR(ST_PREFIX __VA_ARGS__)
#define D_LOG_WARNING(...) P_LOG_WARN(ST_PREFIX __VA_ARGS__)
#define D_LOG_INFO(...) P_LOG_INFO(ST_PREFIX __VA_ARGS__)
#define D_LOG_DEBUG(...) P_LOG_DEBUG(ST_PREFIX __VA_ARGS__)
#endif

#ifdef WIN32
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4191 4242 4244 4365 4777 4986 5039 5204)
#endif
#include <atlutil.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif

#define ENABLE_STACK_CHECKS

nvidia::cuda::context::~context()
{
	D_LOG_DEBUG("Finalizing... (Addr: 0x%" PRIuPTR ")", this);

	if (_has_device) {
		_cuda->cuDevicePrimaryCtxRelease(_device);
	}
	_cuda->cuCtxDestroy(_ctx);
}

nvidia::cuda::context::context() : _cuda(::nvidia::cuda::cuda::get()), _ctx(), _has_device(false), _device()
{
	D_LOG_DEBUG("Initializating... (Addr: 0x%" PRIuPTR ")", this);
}

#ifdef WIN32
nvidia::cuda::context::context(ID3D11Device* device) : context()
{
	using namespace nvidia::cuda;

	if (!device)
		throw std::invalid_argument("device");
	// Get DXGI Device
	IDXGIDevice* dxgi_device; // Don't use ATL::CComPtr
	device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgi_device);

	// Get DXGI Adapter
	ATL::CComPtr<IDXGIAdapter> dxgi_adapter;
	dxgi_device->GetAdapter(&dxgi_adapter);

	// Get Device Index
	if (result res = _cuda->cuD3D11GetDevice(&_device, dxgi_adapter); res != result::SUCCESS) {
		throw std::runtime_error("Failed to get device index for device.");
	}

	// Acquire Context
	if (result res = _cuda->cuDevicePrimaryCtxRetain(&_ctx, _device); res != result::SUCCESS) {
		throw std::runtime_error("Failed to acquire primary device context.");
	}

	_has_device = true;
}
#endif

::nvidia::cuda::context_t nvidia::cuda::context::get()
{
	return _ctx;
}

std::shared_ptr<::nvidia::cuda::context_stack> nvidia::cuda::context::enter()
{
	return std::make_shared<::nvidia::cuda::context_stack>(shared_from_this());
}

void nvidia::cuda::context::push()
{
	if (auto res = _cuda->cuCtxPushCurrent(_ctx); res != ::nvidia::cuda::result::SUCCESS) {
		throw ::nvidia::cuda::cuda_error(res);
	}
}

void nvidia::cuda::context::pop()
{
#ifdef ENABLE_STACK_CHECKS
	::nvidia::cuda::context_t ctx;
	if (_cuda->cuCtxGetCurrent(&ctx) == ::nvidia::cuda::result::SUCCESS)
		assert(ctx == _ctx);
#endif

	assert(_cuda->cuCtxPopCurrent(&ctx) == ::nvidia::cuda::result::SUCCESS);
}

void nvidia::cuda::context::synchronize()
{
	D_LOG_DEBUG("Synchronizing... (Addr: 0x%" PRIuPTR ")", this);

#ifdef ENABLE_STACK_CHECKS
	::nvidia::cuda::context_t ctx;
	if (_cuda->cuCtxGetCurrent(&ctx) == ::nvidia::cuda::result::SUCCESS)
		assert(ctx == _ctx);
#endif

	if (auto res = _cuda->cuCtxSynchronize(); res != ::nvidia::cuda::result::SUCCESS) {
		throw ::nvidia::cuda::cuda_error(res);
	}
}
