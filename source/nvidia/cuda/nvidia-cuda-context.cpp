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
#include <stdexcept>

#ifdef WIN32
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4191 4365 4777 5039 5204)
#endif
#include <atlutil.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif

nvidia::cuda::context::context(std::shared_ptr<::nvidia::cuda::cuda> cuda)
	: _cuda(cuda), _ctx(), _has_device(false), _device()
{
	if (!cuda)
		throw std::invalid_argument("cuda");
}

nvidia::cuda::context::~context()
{
	if (_has_device) {
		_cuda->cuDevicePrimaryCtxRelease(_device);
	}
	_cuda->cuCtxDestroy(_ctx);
}

#ifdef WIN32
nvidia::cuda::context::context(std::shared_ptr<::nvidia::cuda::cuda> cuda, ID3D11Device* device) : context(cuda)
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
