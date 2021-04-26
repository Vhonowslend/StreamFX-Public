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

#include "nvidia-cuda.hpp"
#include <mutex>
#include "util/util-logging.hpp"

#ifdef _DEBUG
#define ST_PREFIX "<%s> "
#define D_LOG_ERROR(x, ...) P_LOG_ERROR(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_WARNING(x, ...) P_LOG_WARN(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_INFO(x, ...) P_LOG_INFO(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_DEBUG(x, ...) P_LOG_DEBUG(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#else
#define ST_PREFIX "<nvidia::cuda::cuda> "
#define D_LOG_ERROR(...) P_LOG_ERROR(ST_PREFIX __VA_ARGS__)
#define D_LOG_WARNING(...) P_LOG_WARN(ST_PREFIX __VA_ARGS__)
#define D_LOG_INFO(...) P_LOG_INFO(ST_PREFIX __VA_ARGS__)
#define D_LOG_DEBUG(...) P_LOG_DEBUG(ST_PREFIX __VA_ARGS__)
#endif

#if defined(_WIN32) || defined(_WIN64)
#define CUDA_NAME "nvcuda.dll"
#else
#define CUDA_NAME "libcuda.so.1"
#endif

#define CUDA_LOAD_SYMBOL(NAME)                                                            \
	{                                                                                     \
		NAME = reinterpret_cast<decltype(NAME)>(_library->load_symbol(#NAME));            \
		if (!NAME)                                                                        \
			throw std::runtime_error("Failed to load '" #NAME "' from '" CUDA_NAME "'."); \
	}
#define CUDA_LOAD_SYMBOL_V2(NAME)                                                         \
	{                                                                                     \
		NAME = reinterpret_cast<decltype(NAME)>(_library->load_symbol(#NAME "_v2"));      \
		if (!NAME)                                                                        \
			throw std::runtime_error("Failed to load '" #NAME "' from '" CUDA_NAME "'."); \
	}
#define CUDA_LOAD_SYMBOL_EX(NAME, OVERRIDE)                                               \
	{                                                                                     \
		NAME = reinterpret_cast<decltype(NAME)>(_library->load_symbol(#OVERRIDE));        \
		if (!NAME)                                                                        \
			throw std::runtime_error("Failed to load '" #NAME "' from '" CUDA_NAME "'."); \
	}

nvidia::cuda::cuda::~cuda()
{
	D_LOG_DEBUG("Finalizing... (Addr: 0x%" PRIuPTR ")", this);
}

nvidia::cuda::cuda::cuda() : _library()
{
	D_LOG_DEBUG("Initialization... (Addr: 0x%" PRIuPTR ")", this);

	_library = util::library::load(std::string_view(CUDA_NAME));

	// Initialization
	CUDA_LOAD_SYMBOL(cuInit);

	// Version Management
	CUDA_LOAD_SYMBOL(cuDriverGetVersion);

	// Primary Context Management
	CUDA_LOAD_SYMBOL(cuDevicePrimaryCtxRetain);
	CUDA_LOAD_SYMBOL_V2(cuDevicePrimaryCtxRelease);
	CUDA_LOAD_SYMBOL_V2(cuDevicePrimaryCtxSetFlags);

	// Context Management
	CUDA_LOAD_SYMBOL_V2(cuCtxCreate);
	CUDA_LOAD_SYMBOL_V2(cuCtxDestroy);
	CUDA_LOAD_SYMBOL(cuCtxGetCurrent);
	CUDA_LOAD_SYMBOL(cuCtxGetStreamPriorityRange);
	CUDA_LOAD_SYMBOL_V2(cuCtxPopCurrent);
	CUDA_LOAD_SYMBOL_V2(cuCtxPushCurrent);
	CUDA_LOAD_SYMBOL(cuCtxSetCurrent);
	CUDA_LOAD_SYMBOL(cuCtxSynchronize);

	// Memory Management
	CUDA_LOAD_SYMBOL_V2(cuArrayGetDescriptor);
	CUDA_LOAD_SYMBOL_V2(cuMemAlloc);
	CUDA_LOAD_SYMBOL_V2(cuMemAllocPitch);
	CUDA_LOAD_SYMBOL_V2(cuMemFree);
	CUDA_LOAD_SYMBOL_V2(cuMemHostGetDevicePointer);
	CUDA_LOAD_SYMBOL(cuMemcpy);
	CUDA_LOAD_SYMBOL_V2(cuMemcpy2D);
	CUDA_LOAD_SYMBOL_V2(cuMemcpy2DAsync);
	CUDA_LOAD_SYMBOL_V2(cuMemcpyAtoA);
	CUDA_LOAD_SYMBOL_V2(cuMemcpyAtoD);
	CUDA_LOAD_SYMBOL_V2(cuMemcpyAtoH);
	CUDA_LOAD_SYMBOL_V2(cuMemcpyAtoHAsync);
	CUDA_LOAD_SYMBOL_V2(cuMemcpyDtoA);
	CUDA_LOAD_SYMBOL_V2(cuMemcpyDtoD);
	CUDA_LOAD_SYMBOL_V2(cuMemcpyDtoH);
	CUDA_LOAD_SYMBOL_V2(cuMemcpyDtoHAsync);
	CUDA_LOAD_SYMBOL_V2(cuMemcpyHtoA);
	CUDA_LOAD_SYMBOL_V2(cuMemcpyHtoAAsync);
	CUDA_LOAD_SYMBOL_V2(cuMemcpyHtoD);
	CUDA_LOAD_SYMBOL_V2(cuMemcpyHtoDAsync);

	// Stream Managment
	CUDA_LOAD_SYMBOL(cuStreamCreate);
	CUDA_LOAD_SYMBOL(cuStreamCreateWithPriority);
	CUDA_LOAD_SYMBOL_V2(cuStreamDestroy);
	CUDA_LOAD_SYMBOL(cuStreamSynchronize);

	// Graphics Interoperability
	CUDA_LOAD_SYMBOL(cuGraphicsMapResources);
	CUDA_LOAD_SYMBOL(cuGraphicsSubResourceGetMappedArray);
	CUDA_LOAD_SYMBOL(cuGraphicsUnmapResources);
	CUDA_LOAD_SYMBOL(cuGraphicsUnregisterResource);

#ifdef WIN32
	// Direct3D11 Interopability
	CUDA_LOAD_SYMBOL(cuD3D11GetDevice);
	CUDA_LOAD_SYMBOL(cuGraphicsD3D11RegisterResource);
#endif

	// Log found CUDA version.
	{
		int32_t cuda_version = 0;
		if (cuDriverGetVersion(&cuda_version) == result::SUCCESS) {
			int32_t major = cuda_version / 1000;
			int32_t minor = (cuda_version % 1000) / 10;
			int32_t patch = (cuda_version % 10);
			D_LOG_INFO("Driver reported CUDA version: %" PRId32 ".%" PRId32 ".%" PRId32, major, minor, patch);
		} else {
			D_LOG_WARNING("Failed to query NVIDIA CUDA Driver for version.", 0);
		}
	}

	// Initialize CUDA
	cuInit(0);
}

std::shared_ptr<nvidia::cuda::cuda> nvidia::cuda::cuda::get()
{
	static std::weak_ptr<nvidia::cuda::cuda> instance;
	static std::mutex                        lock;

	std::unique_lock<std::mutex> ul(lock);
	if (instance.expired()) {
		auto hard_instance = std::make_shared<nvidia::cuda::cuda>();
		instance           = hard_instance;
		return hard_instance;
	}
	return instance.lock();
}
