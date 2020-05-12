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
#include <functional>
#include <memory>
#include "utility.hpp"

#ifdef WIN32
#pragma warning(push)
#pragma warning(disable : 4365)
#pragma warning(disable : 5204)
#include <d3d11.h>
#include <dxgi.h>
#pragma warning(pop)
#endif

#define CUDA_DEFINE_FUNCTION(name, ...)                        \
	private:                                                   \
	typedef ::nvidia::cuda::cu_result (*t##name)(__VA_ARGS__); \
                                                               \
	public:                                                    \
	t##name name;

namespace nvidia::cuda {
	enum class cu_result : std::size_t {
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

	enum class cu_memory_type : std::uint32_t {
		HOST    = 1,
		DEVICE  = 2,
		ARRAY   = 3,
		UNIFIED = 4,
	};

	enum class cu_array_format : std::uint32_t {
		UNSIGNED_INT8  = 0b00000001,
		UNSIGNED_INT16 = 0b00000010,
		UNSIGNED_INT32 = 0b00000011,
		SIGNED_INT8    = 0b00001000,
		SIGNED_INT16   = 0b00001001,
		SIGNED_INT32   = 0b00001010,
		HALF           = 0b00010000,
		FLOAT          = 0b00100000,
	};

	enum class cu_context_flags : std::uint32_t {
		SCHEDULER_AUTO                 = 0x0,
		SCHEDULER_SPIN                 = 0x1,
		SCHEDULER_YIELD                = 0x2,
		SCHEDULER_BLOCKING_SYNC        = 0x4,
		MAP_HOST                       = 0x8,
		LOCAL_MEMORY_RESIZE_TO_MAXIMUM = 0x10,
	};

	enum class cu_stream_flags : std::uint32_t {
		DEFAULT      = 0x0,
		NON_BLOCKING = 0x1,
	};

	typedef void*         cu_array_t;
	typedef void*         cu_context_t;
	typedef std::uint64_t cu_device_ptr_t;
	typedef void*         cu_graphics_resource_t;
	typedef void*         cu_stream_t;
	typedef std::int32_t  cu_device_t;

	struct cu_memcpy2d_t {
		std::size_t src_x_in_bytes;
		std::size_t src_y;

		cu_memory_type  src_memory_type;
		const void*     src_host;
		cu_device_ptr_t src_device;
		cu_array_t      src_array;
		std::size_t     src_pitch;

		std::size_t dst_x_in_bytes;
		std::size_t dst_y;

		cu_memory_type  dst_memory_type;
		const void*     dst_host;
		cu_device_ptr_t dst_device;
		cu_array_t      dst_array;
		std::size_t     dst_pitch;

		std::size_t width_in_bytes;
		std::size_t height;
	};

	struct cu_array_descriptor_t {
		std::size_t     width;
		std::size_t     height;
		std::uint32_t   num_channels;
		cu_array_format format;
	};

	class cuda {
		private:
		void* _library;

		public:
		cuda();
		~cuda();

		public:
		// Initialization
		CUDA_DEFINE_FUNCTION(cuInit, std::int32_t flags);

		// Version Management
		CUDA_DEFINE_FUNCTION(cuDriverGetVersion, std::int32_t* driverVersion);

		// Device Management
		// cuDeviceGet
		// cuDeviceGetAttribute
		// cuDeviceGetCount
		// cuDeviceGetLuid
		// cuDeviceGetName
		// cuDeviceGetNvSciSyncAttributes
		// cuDeviceGetUuid
		// cuDeviceTotalMem_v2

		// Primary Context Management
		// cuDevicePrimaryCtxGetState
		CUDA_DEFINE_FUNCTION(cuDevicePrimaryCtxRelease, cu_device_t device);
		// cuDevicePrimaryCtxReset_v2
		CUDA_DEFINE_FUNCTION(cuDevicePrimaryCtxRetain, cu_context_t* ctx, cu_device_t device);
		CUDA_DEFINE_FUNCTION(cuDevicePrimaryCtxSetFlags, cu_device_t device, cu_context_flags flags);

		// Context Management
		CUDA_DEFINE_FUNCTION(cuCtxCreate, cu_context_t* ctx, cu_context_flags flags, cu_device_t device);
		CUDA_DEFINE_FUNCTION(cuCtxDestroy, cu_context_t ctx);
		// cuCtxGetApiVersion
		// cuCtxGetCacheConfig
		CUDA_DEFINE_FUNCTION(cuCtxGetCurrent, cu_context_t* ctx);
		// cuCtxGetDevice
		// cuCtxGetFlags
		// cuCtxGetLimit
		// cuCtxGetSharedMemConfig
		CUDA_DEFINE_FUNCTION(cuCtxGetStreamPriorityRange, std::int32_t* lowestPriority, std::int32_t* highestPriority);
		CUDA_DEFINE_FUNCTION(cuCtxPopCurrent, cu_context_t* ctx);
		CUDA_DEFINE_FUNCTION(cuCtxPushCurrent, cu_context_t ctx);
		// cuCtxSetCacheConfig
		CUDA_DEFINE_FUNCTION(cuCtxSetCurrent, cu_context_t ctx);
		// cuCtxSetLimit
		// cuCtxSetSharedMemConfig
		// cuCtxSynchronize
		CUDA_DEFINE_FUNCTION(cuCtxSynchronize);
		// UNDOCUMENTED? cuCtxResetPersistingL2Cache

		// Module Management
		// cuLinkAddData
		// cuLinkAddFile
		// cuLinkComplete
		// cuLinkCreate
		// cuLinkDestroy
		// cuModuleGetFunction
		// cuModuleGetGlobal
		// cuModuleGetSurfRef
		// cuModuleGetTexRef
		// cuModuleLoad
		// cuModuleLoadData
		// cuModuleLoadDataEx
		// cuModuleLoadFatBinary
		// cuModuleUnload

		// Memory Management
		// cuArray3DCreate_v2
		// cuArray3DGetDescripter_v2
		// cuArrayCreate_v2
		// cuArrayDestroy
		CUDA_DEFINE_FUNCTION(cuArrayGetDescriptor, cu_array_descriptor_t* pArrayDescripter, cu_array_t array);
		// cuArrayGetDescriptor_v2
		// cuDeviceGetByPCIBusId
		// cuDeviceGetPCIBusId
		// cuIpcCloseMemHandle
		// cuIpcGetEventHandle
		// cuIpcGetMemHandle
		// cuIpcOpenEventHandle
		// cuIpcOpenMemHandle
		CUDA_DEFINE_FUNCTION(cuMemAlloc, cu_device_ptr_t* ptr, std::size_t bytes);
		// cuMemAllocHost_v2
		// cuMemAllocManaged
		CUDA_DEFINE_FUNCTION(cuMemAllocPitch, cu_device_ptr_t* ptr, std::size_t* pitch, std::size_t width_in_bytes,
							 std::size_t height, std::uint32_t element_size_bytes);
		CUDA_DEFINE_FUNCTION(cuMemFree, cu_device_ptr_t ptr);
		// cuMemFreeHost
		// cuMemGetAddressRange_v2
		// cuMemGetInfo_v2
		// cuMemHostAlloc
		CUDA_DEFINE_FUNCTION(cuMemHostGetDevicePointer, cu_device_ptr_t* devptr, void* ptr, std::uint32_t flags);
		// cuMemHostGetFlags
		// cuMemHostRegister_v2
		// cuMemHostUnregister
		CUDA_DEFINE_FUNCTION(cuMemcpy, cu_device_ptr_t dst, cu_device_ptr_t src, std::size_t bytes);
		CUDA_DEFINE_FUNCTION(cuMemcpy2D, const cu_memcpy2d_t* copy);
		CUDA_DEFINE_FUNCTION(cuMemcpy2DAsync, const cu_memcpy2d_t* copy, cu_stream_t stream);
		// cuMemcpy2DUnaligned_v2 / _v2_ptds
		// cuMemcpy3D_v2 / _v2_ptds
		// cuMemcpy3DAsync_v2 / _v2_ptsz
		// cuMemcpy3DPeer / _ptds
		// cuMemcpy3DPeerAsync_v2 / _v2_ptsz
		// cuMemcpyAsync / _ptsz
		CUDA_DEFINE_FUNCTION(cuMemcpyAtoA, cu_array_t dst, std::size_t dstOffset, cu_array_t src, std::size_t srcOffset,
							 std::size_t byteCount);
		CUDA_DEFINE_FUNCTION(cuMemcpyAtoD, cu_device_ptr_t dst, cu_array_t src, std::size_t srcOffset,
							 std::size_t byteCount);
		CUDA_DEFINE_FUNCTION(cuMemcpyAtoH, void* dst, cu_array_t src, std::size_t srcOffset, std::size_t byteCount);
		CUDA_DEFINE_FUNCTION(cuMemcpyAtoHAsync, void* dst, cu_array_t src, std::size_t srcOffset,
							 std::size_t byteCount);
		CUDA_DEFINE_FUNCTION(cuMemcpyDtoA, cu_array_t dst, std::size_t dstOffset, cu_device_ptr_t src,
							 std::size_t byteCount);
		CUDA_DEFINE_FUNCTION(cuMemcpyDtoD, cu_device_ptr_t dst, cu_array_t srcArray, std::size_t byteCount);
		CUDA_DEFINE_FUNCTION(cuMemcpyDtoH, void* dst, cu_array_t src, std::size_t byteCount);
		CUDA_DEFINE_FUNCTION(cuMemcpyDtoHAsync, void* dst, cu_array_t src, std::size_t byteCount);
		CUDA_DEFINE_FUNCTION(cuMemcpyHtoA, cu_array_t dst, std::size_t dstOffset, void* src, std::size_t byteCount);
		CUDA_DEFINE_FUNCTION(cuMemcpyHtoAAsync, cu_array_t dst, std::size_t dstOffset, void* src,
							 std::size_t byteCount);
		CUDA_DEFINE_FUNCTION(cuMemcpyHtoD, cu_device_ptr_t dst, void* src, std::size_t byteCount);
		CUDA_DEFINE_FUNCTION(cuMemcpyHtoDAsync, cu_device_ptr_t dst, void* src, std::size_t byteCount);
		// cuMemcpyPeer / _ptds
		// cuMemcpyPeerAsync / _ptsz
		// cuMemsetD16
		// cuMemsetD16Async
		// cuMemsetD2D16
		// cuMemsetD2D16Async
		// cuMemsetD2D32
		// cuMemsetD2D32Async
		// cuMemsetD2D8
		// cuMemsetD2D8Async
		// cuMemsetD32
		// cuMemsetD32Async
		// cuMemsetD8
		// cuMemsetD8Async
		// cuMipmappedArrayCreate
		// cuMipmappedArrayDestroy
		// cuMipmappedArrayGetLevel

		// Virtual Memory Management
		// cuMemAddressFree
		// cuMemAddressReserve
		// cuMemCreate
		// cuMemExportToShareableHandle
		// cuMemGetAccess
		// cuMemGetAllocationGranularity
		// cuMemGetAllocationPropertiesFromHandle
		// cuMemImportFromShareableHandle
		// cuMemMap
		// cuMemRelease
		// cuMemSetAccess
		// cuMemUnmap

		// Unified Addressing
		// cuMemAdvise
		// cuMemPrefetchAsync
		// cuMemRangeGetAttribute
		// cuMemRangeGetAttributes
		// cuPointerGetAttribute
		// cuPointerGetAttributes
		// cuPointerSetAttribute

		// Stream Managment
		// cuStreamAddCallback
		// cuStreamAttachMemAsync
		// cuStreamBeginCapture_v2
		CUDA_DEFINE_FUNCTION(cuStreamCreate, cu_stream_t* stream, cu_stream_flags flags);
		CUDA_DEFINE_FUNCTION(cuStreamCreateWithPriority, cu_stream_t* stream, cu_stream_flags flags,
							 std::int32_t priority);
		CUDA_DEFINE_FUNCTION(cuStreamDestroy, cu_stream_t stream);
		// cuStreamEndCapture
		// cuStreamGetCaptureInfo
		// cuStreamGetCtx
		// cuStreamGetFlags
		// cuStreamGetPriority
		// cuStreamIsCapturing
		// cuStreamQuery
		CUDA_DEFINE_FUNCTION(cuStreamSynchronize, cu_stream_t stream);
		// cuStreamWaitEvent
		// cuThreadExchangeStreamCaptureMode

		// Event Management
		// cuEventCreate
		// cuEventDestroy_v2
		// cuEventElapsedTime
		// cuEventQuery
		// cuEventRecord
		// cuEventSynchronize

		// External Resource Interoperability
		// cuDestroyExternalMemory
		// cuDestroyExternalSemaphore
		// cuExternalMemoryGetMappedBuffer
		// cuExternalMemoryGetMappedMipmappedArray
		// cuImportExternalMemory
		// cuImportExternalSemaphore
		// cuSignalExternalSemaphoresAsync
		// cuWaitExternalSemaphoresAsync

		// Stream Memory Operations
		// cuStreamBatchMemOp
		// cuStreamWaitValue32
		// cuStreamWaitValue64
		// cuStreamWriteValue32
		// cuStreamWriteValue64

		// Execution Control
		// cuFuncGetAttribute
		// cuFuncSetAttribute
		// cuFuncSetCacheConfig
		// cuFuncSetSharedMemConfig
		// cuLaunchCooperativeKernel
		// cuLaunchCooperativeKernelMultiDevice
		// cuLaunchHostFunc
		// cuLaunchKernel

		// Graph Management
		// Todo!

		// Occupancy
		// Todo

		// Texture Object Management
		// Todo

		// Surface Object Management
		// Todo

		// Peer Context Memory Access
		// Todo

		// Graphics Interoperability
		CUDA_DEFINE_FUNCTION(cuGraphicsMapResources, std::uint32_t count, cu_graphics_resource_t* resources,
							 cu_stream_t stream);
		// cuGraphicsResourcesGetMappedMipmappedArray
		// cuGraphicsResourcesGetMappedPointer_v2
		// cuGraphicsResourcesSetMapFlags_v2
		CUDA_DEFINE_FUNCTION(cuGraphicsSubResourceGetMappedArray, cu_array_t* array, cu_graphics_resource_t resource,
							 std::uint32_t index, std::uint32_t level);
		CUDA_DEFINE_FUNCTION(cuGraphicsUnmapResources, std::uint32_t count, cu_graphics_resource_t* resources,
							 cu_stream_t stream);
		CUDA_DEFINE_FUNCTION(cuGraphicsUnregisterResource, cu_graphics_resource_t resource);

		// Profile Control
		// Todo

		// OpenGL Interoperability
		// cuGLGetDevices
		// cuGraphcisGLRegisterBuffer
		// cuGraphcisGLRegisterImage
#ifdef WIN32
		// cuWGLGetDevice

		// Direct3D9 Interopability
		// cuD3D9CtxCreate
		// cuD3D9CtxCreateOnDevice
		// cuD3D9CtxGetDevice
		// cuD3D9CtxGetDevices
		// cuD3D9GetDirect3DDevice
		// cuGraphicsD3D9RegisterResource

		// Direct3D10 Interopability
		// cuD3D10GetDevice
		// cuD3D10GetDevices
		// cuGraphicsD3D10RegisterResource

		// Direct3D11 Interopability
		CUDA_DEFINE_FUNCTION(cuD3D11GetDevice, cu_device_t* device, IDXGIAdapter* adapter);
		// cuD3D11GetDevices
		CUDA_DEFINE_FUNCTION(cuGraphicsD3D11RegisterResource, cu_graphics_resource_t* resource,
							 ID3D11Resource* d3dresource, std::uint32_t flags);
#endif
	};
} // namespace nvidia::cuda

P_ENABLE_BITMASK_OPERATORS(::nvidia::cuda::cu_context_flags)
P_ENABLE_BITMASK_OPERATORS(::nvidia::cuda::cu_stream_flags)
