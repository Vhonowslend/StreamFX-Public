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

#pragma once
#include <cstddef>
#include <tuple>
#include "util/util-bitmask.hpp"
#include "util/util-library.hpp"

#ifdef WIN32
#pragma warning(push)
#pragma warning(disable : 4365)
#pragma warning(disable : 5204)
#include <d3d11.h>
#include <dxgi.h>
#pragma warning(pop)
#endif

#define CUDA_DEFINE_FUNCTION(name, ...)                     \
	private:                                                \
	typedef ::nvidia::cuda::result (*t##name)(__VA_ARGS__); \
                                                            \
	public:                                                 \
	t##name name = nullptr;

namespace nvidia::cuda {
	enum class result : std::size_t {
		SUCCESS                  = 0,
		INVALID_VALUE            = 1,
		OUT_OF_MEMORY            = 2,
		NOT_INITIALIZED          = 3,
		DEINITIALIZED            = 4,
		NO_DEVICE                = 100,
		INVALID_DEVICE           = 101,
		INVALID_CONTEXT          = 201,
		MAP_FAILED               = 205,
		UNMAP_FAILED             = 206,
		ARRAY_IS_MAPPED          = 207,
		ALREADY_MAPPED           = 208,
		NOT_MAPPED               = 211,
		INVALID_GRAPHICS_CONTEXT = 219,
		// Still missing some.
	};

	enum class memory_type : uint32_t {
		HOST    = 1,
		DEVICE  = 2,
		ARRAY   = 3,
		UNIFIED = 4,
	};

	enum class array_format : uint32_t {
		UNSIGNED_INT8  = 0b00000001,
		UNSIGNED_INT16 = 0b00000010,
		UNSIGNED_INT32 = 0b00000011,
		SIGNED_INT8    = 0b00001000,
		SIGNED_INT16   = 0b00001001,
		SIGNED_INT32   = 0b00001010,
		HALF           = 0b00010000,
		FLOAT          = 0b00100000,
	};

	enum class context_flags : uint32_t {
		SCHEDULER_AUTO                 = 0x0,
		SCHEDULER_SPIN                 = 0x1,
		SCHEDULER_YIELD                = 0x2,
		SCHEDULER_BLOCKING_SYNC        = 0x4,
		MAP_HOST                       = 0x8,
		LOCAL_MEMORY_RESIZE_TO_MAXIMUM = 0x10,
	};

	enum class external_memory_handle_type : uint32_t {
		INVALID                      = 0,
		FILE_DESCRIPTOR              = 1,
		WIN32_SHARED_HANDLE          = 2,
		WIN32_GLOBAL_SHARED_HANDLE   = 3,
		D3D12_HEAP                   = 4,
		D3D12_RESOURCE               = 5,
		D3D11_SHARED_RESOURCE        = 6,
		D3D11_GLOBAL_SHARED_RESOURCE = 7,
		NVSCIBUF                     = 8,
	};

	enum class stream_flags : uint32_t {
		DEFAULT      = 0x0,
		NON_BLOCKING = 0x1,
	};

	typedef void*    array_t;
	typedef void*    context_t;
	typedef uint64_t device_ptr_t;
	typedef void*    external_memory_t;
	typedef void*    graphics_resource_t;
	typedef void*    stream_t;
	typedef int32_t  device_t;

	struct memcpy2d_v2_t {
		std::size_t src_x_in_bytes;
		std::size_t src_y;

		memory_type  src_memory_type;
		const void*  src_host;
		device_ptr_t src_device;
		array_t      src_array;
		std::size_t  src_pitch;

		std::size_t dst_x_in_bytes;
		std::size_t dst_y;

		memory_type  dst_memory_type;
		const void*  dst_host;
		device_ptr_t dst_device;
		array_t      dst_array;
		std::size_t  dst_pitch;

		std::size_t width_in_bytes;
		std::size_t height;
	};

	struct array_descriptor_v2_t {
		std::size_t  width;
		std::size_t  height;
		uint32_t     num_channels;
		array_format format;
	};

	struct external_memory_buffer_info_v1_t {
		uint64_t offset;
		uint64_t size;
		uint32_t flags;
		uint32_t reserved[16];
	};

	struct external_memory_handle_info_v1_t {
		external_memory_handle_type type;
		union {
			int32_t file;
			struct {
				void*       handle;
				const void* name;
			};
			const void* nvscibuf;
		};
		uint64_t size;
		uint32_t flags;
		uint32_t reserved[16];
	};

	class cuda_error : public std::exception {
		::nvidia::cuda::result _code;

		public:
		~cuda_error(){};
		cuda_error(::nvidia::cuda::result code) : _code(code) {}

		::nvidia::cuda::result code()
		{
			return _code;
		}
	};

	class cuda {
		std::shared_ptr<streamfx::util::library> _library;

		public:
		~cuda();
		cuda();

		int32_t version();

		public:
		// Initialization
		CUDA_DEFINE_FUNCTION(cuInit, int32_t flags);

		// Version Management
		CUDA_DEFINE_FUNCTION(cuDriverGetVersion, int32_t* driverVersion);

		// Device Management
		// - Not yet needed.

		// Primary Context Management
		CUDA_DEFINE_FUNCTION(cuDevicePrimaryCtxRelease, device_t device);
		CUDA_DEFINE_FUNCTION(cuDevicePrimaryCtxRetain, context_t* ctx, device_t device);
		CUDA_DEFINE_FUNCTION(cuDevicePrimaryCtxSetFlags, device_t device, context_flags flags);

		// Context Management
		CUDA_DEFINE_FUNCTION(cuCtxCreate, context_t* ctx, context_flags flags, device_t device);
		CUDA_DEFINE_FUNCTION(cuCtxDestroy, context_t ctx);
		CUDA_DEFINE_FUNCTION(cuCtxGetCurrent, context_t* ctx);
		CUDA_DEFINE_FUNCTION(cuCtxGetStreamPriorityRange, int32_t* lowestPriority, int32_t* highestPriority);
		CUDA_DEFINE_FUNCTION(cuCtxPopCurrent, context_t* ctx);
		CUDA_DEFINE_FUNCTION(cuCtxPushCurrent, context_t ctx);
		CUDA_DEFINE_FUNCTION(cuCtxSetCurrent, context_t ctx);
		CUDA_DEFINE_FUNCTION(cuCtxSynchronize);

		// Module Management
		// - Not yet needed.

		// Memory Management
		CUDA_DEFINE_FUNCTION(cuArrayGetDescriptor, array_descriptor_v2_t* pArrayDescripter, array_t array);
		CUDA_DEFINE_FUNCTION(cuMemAlloc, device_ptr_t* ptr, std::size_t bytes);
		CUDA_DEFINE_FUNCTION(cuMemAllocPitch, device_ptr_t* ptr, std::size_t* pitch, std::size_t width_in_bytes,
							 std::size_t height, uint32_t element_size_bytes);
		CUDA_DEFINE_FUNCTION(cuMemFree, device_ptr_t ptr);
		CUDA_DEFINE_FUNCTION(cuMemHostGetDevicePointer, device_ptr_t* devptr, void* ptr, uint32_t flags);
		CUDA_DEFINE_FUNCTION(cuMemcpy, device_ptr_t dst, device_ptr_t src, std::size_t bytes);
		CUDA_DEFINE_FUNCTION(cuMemcpy2D, const memcpy2d_v2_t* copy);
		CUDA_DEFINE_FUNCTION(cuMemcpy2DAsync, const memcpy2d_v2_t* copy, stream_t stream);
		CUDA_DEFINE_FUNCTION(cuMemcpyAtoA, array_t dst, std::size_t dstOffset, array_t src, std::size_t srcOffset,
							 std::size_t byteCount);
		CUDA_DEFINE_FUNCTION(cuMemcpyAtoD, device_ptr_t dst, array_t src, std::size_t srcOffset, std::size_t byteCount);
		CUDA_DEFINE_FUNCTION(cuMemcpyAtoH, void* dst, array_t src, std::size_t srcOffset, std::size_t byteCount);
		CUDA_DEFINE_FUNCTION(cuMemcpyAtoHAsync, void* dst, array_t src, std::size_t srcOffset, std::size_t byteCount);
		CUDA_DEFINE_FUNCTION(cuMemcpyDtoA, array_t dst, std::size_t dstOffset, device_ptr_t src, std::size_t byteCount);
		CUDA_DEFINE_FUNCTION(cuMemcpyDtoD, device_ptr_t dst, array_t srcArray, std::size_t byteCount);
		CUDA_DEFINE_FUNCTION(cuMemcpyDtoH, void* dst, array_t src, std::size_t byteCount);
		CUDA_DEFINE_FUNCTION(cuMemcpyDtoHAsync, void* dst, array_t src, std::size_t byteCount);
		CUDA_DEFINE_FUNCTION(cuMemcpyHtoA, array_t dst, std::size_t dstOffset, void* src, std::size_t byteCount);
		CUDA_DEFINE_FUNCTION(cuMemcpyHtoAAsync, array_t dst, std::size_t dstOffset, void* src, std::size_t byteCount);
		CUDA_DEFINE_FUNCTION(cuMemcpyHtoD, device_ptr_t dst, void* src, std::size_t byteCount);
		CUDA_DEFINE_FUNCTION(cuMemcpyHtoDAsync, device_ptr_t dst, void* src, std::size_t byteCount);

		// Virtual Memory Management
		// - Not yet needed.

		// Stream Ordered Memory Allocator
		// - Not yet needed.

		// Unified Addressing
		// - Not yet needed.

		// Stream Managment
		CUDA_DEFINE_FUNCTION(cuStreamCreate, stream_t* stream, stream_flags flags);
		CUDA_DEFINE_FUNCTION(cuStreamCreateWithPriority, stream_t* stream, stream_flags flags, int32_t priority);
		CUDA_DEFINE_FUNCTION(cuStreamDestroy, stream_t stream);
		CUDA_DEFINE_FUNCTION(cuStreamSynchronize, stream_t stream);
		CUDA_DEFINE_FUNCTION(cuStreamGetPriority, stream_t stream, int32_t* priority);

		// Event Management
		// - Not yet needed.

		// External Resource Interoperability (CUDA 11.1+)
		// - Not yet needed.

		// Stream Memory Operations
		// - Not yet needed.

		// Execution Control
		// - Not yet needed.

		// Graph Management
		// - Not yet needed.

		// Occupancy
		// - Not yet needed.

		// Texture Object Management
		// - Not yet needed.

		// Surface Object Management
		// - Not yet needed.

		// Peer Context Memory Access
		// - Not yet needed.

		// Graphics Interoperability
		CUDA_DEFINE_FUNCTION(cuGraphicsMapResources, uint32_t count, graphics_resource_t* resources, stream_t stream);
		CUDA_DEFINE_FUNCTION(cuGraphicsSubResourceGetMappedArray, array_t* array, graphics_resource_t resource,
							 uint32_t index, uint32_t level);
		CUDA_DEFINE_FUNCTION(cuGraphicsUnmapResources, uint32_t count, graphics_resource_t* resources, stream_t stream);
		CUDA_DEFINE_FUNCTION(cuGraphicsUnregisterResource, graphics_resource_t resource);

		// Driver Entry Point Access
		// - Not yet needed.

		// Profiler Control
		// - Not yet needed.

		// OpenGL Interoperability
		// - Not yet needed.

		// VDPAU Interoperability
		// - Not yet needed.

		// EGL Interoperability
		// - Not yet needed.

#ifdef WIN32
		// Direct3D9 Interoperability
		// - Not yet needed.

		// Direct3D10 Interoperability
		CUDA_DEFINE_FUNCTION(cuD3D10GetDevice, device_t* device, IDXGIAdapter* adapter);
		CUDA_DEFINE_FUNCTION(cuGraphicsD3D10RegisterResource, graphics_resource_t* resource,
							 ID3D10Resource* d3dresource, uint32_t flags);

		// Direct3D11 Interoperability
		CUDA_DEFINE_FUNCTION(cuD3D11GetDevice, device_t* device, IDXGIAdapter* adapter);
		CUDA_DEFINE_FUNCTION(cuGraphicsD3D11RegisterResource, graphics_resource_t* resource,
							 ID3D11Resource* d3dresource, uint32_t flags);
#endif
		public:
		static std::shared_ptr<::nvidia::cuda::cuda> get();
	};
} // namespace nvidia::cuda

P_ENABLE_BITMASK_OPERATORS(::nvidia::cuda::context_flags)
P_ENABLE_BITMASK_OPERATORS(::nvidia::cuda::stream_flags)
