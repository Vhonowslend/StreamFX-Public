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

#include "nvidia-ar.hpp"
#include <stdexcept>
#include <util/bmem.h>
#include <util/platform.h>

#ifdef WIN32
#include <Shlobj.h>
#include <Windows.h>
#endif

#include <nvARProxy.cpp>
#define nvGetProcAddress nvGetProcAddressCV
#define nvFreeLibrary nvFreeLibraryCV
#include <nvCVImageProxy.cpp>
#undef nvGetProcAddress
#undef nvFreeLibrary

nvidia::ar::ar::ar()
{
	if (!getNvARLib())
		throw std::runtime_error("Failed to load NVIDIA AR SDK runtime.");
}

nvidia::ar::ar::~ar() {}

std::filesystem::path nvidia::ar::ar::get_ar_sdk_path()
{
	char* arsdk_path = getenv("NV_AR_SDK_PATH");
	if (arsdk_path) {
		return std::filesystem::path(std::string{arsdk_path});
	} else {
		std::filesystem::path res;
#ifdef WIN32
		std::vector<wchar_t> dll_path_w;
		dll_path_w.resize(65535);
		DWORD size_w = GetModuleFileNameW(getNvARLib(), dll_path_w.data(), static_cast<DWORD>(dll_path_w.size()));

		std::vector<char> dll_path;
		dll_path.resize(65535);
		std::size_t size = os_wcs_to_utf8(dll_path_w.data(), size_w, dll_path.data(), dll_path.size());

		std::filesystem::path dll = std::string{dll_path.data(), dll_path.data() + size};
		res                       = dll.remove_filename();
#endif
		return res;
	}
}

NvCV_Status nvidia::ar::ar::image_init(NvCVImage* im, unsigned width, unsigned height, int pitch, void* pixels,
									   NvCVImage_PixelFormat format, NvCVImage_ComponentType type, unsigned isPlanar,
									   unsigned onGPU)
{
	return NvCVImage_Init(im, width, height, pitch, pixels, format, type, isPlanar, onGPU);
}

void nvidia::ar::ar::image_init_view(NvCVImage* subImg, NvCVImage* fullImg, int x, int y, unsigned width,
									 unsigned height)
{
	NvCVImage_InitView(subImg, fullImg, x, y, width, height);
}

NvCV_Status nvidia::ar::ar::image_alloc(NvCVImage* im, unsigned width, unsigned height, NvCVImage_PixelFormat format,
										NvCVImage_ComponentType type, unsigned isPlanar, unsigned onGPU,
										unsigned alignment)
{
	return NvCVImage_Alloc(im, width, height, format, type, isPlanar, onGPU, alignment);
}

NvCV_Status nvidia::ar::ar::image_realloc(NvCVImage* im, unsigned width, unsigned height, NvCVImage_PixelFormat format,
										  NvCVImage_ComponentType type, unsigned isPlanar, unsigned onGPU,
										  unsigned alignment)
{
	return NvCVImage_Realloc(im, width, height, format, type, isPlanar, onGPU, alignment);
}

void nvidia::ar::ar::image_dealloc(NvCVImage* im)
{
	NvCVImage_Dealloc(im);
}

NvCV_Status nvidia::ar::ar::image_create(unsigned width, unsigned height, NvCVImage_PixelFormat format,
										 NvCVImage_ComponentType type, unsigned isPlanar, unsigned onGPU,
										 unsigned alignment, NvCVImage** out)
{
	return NvCVImage_Create(width, height, format, type, isPlanar, onGPU, alignment, out);
}

void nvidia::ar::ar::image_destroy(NvCVImage* im)
{
	NvCVImage_Destroy(im);
}

void nvidia::ar::ar::image_component_offsets(NvCVImage_PixelFormat format, int* rOff, int* gOff, int* bOff, int* aOff,
											 int* yOff)
{
	NvCVImage_ComponentOffsets(format, rOff, gOff, bOff, aOff, yOff);
}

NvCV_Status nvidia::ar::ar::image_transfer(const NvCVImage* src, NvCVImage* dst, float scale, CUstream_st* stream,
										   NvCVImage* tmp)
{
	return NvCVImage_Transfer(src, dst, scale, stream, tmp);
}

NvCV_Status nvidia::ar::ar::image_composite(const NvCVImage* src, const NvCVImage* mat, NvCVImage* dst)
{
	//return NvCVImage_Composite(src, mat, dst);
	throw std::runtime_error("Not implemented.");
}

NvCV_Status nvidia::ar::ar::image_composite_over_constant(const NvCVImage* src, const NvCVImage* mat,
														  const unsigned char bgColor[3], NvCVImage* dst)
{
	return NvCVImage_CompositeOverConstant(src, mat, bgColor, dst);
}

NvCV_Status nvidia::ar::ar::image_flipy(const NvCVImage* src, NvCVImage* dst)
{
	return NvCVImage_FlipY(src, dst);
}

NvCV_Status nvidia::ar::ar::create(NvAR_FeatureID featureID, NvAR_FeatureHandle* handle)
{
	return NvAR_Create(featureID, handle);
}

NvCV_Status nvidia::ar::ar::destroy(NvAR_FeatureHandle handle)
{
	return NvAR_Destroy(handle);
}

NvCV_Status nvidia::ar::ar::set_uint32(NvAR_FeatureHandle handle, const char* name, unsigned int val)
{
	return NvAR_SetU32(handle, name, val);
}

NvCV_Status nvidia::ar::ar::set_int32(NvAR_FeatureHandle handle, const char* name, int val)
{
	return NvAR_SetS32(handle, name, val);
}

NvCV_Status nvidia::ar::ar::set_float32(NvAR_FeatureHandle handle, const char* name, float val)
{
	return NvAR_SetF32(handle, name, val);
}

NvCV_Status nvidia::ar::ar::set_float64(NvAR_FeatureHandle handle, const char* name, double val)
{
	return NvAR_SetF64(handle, name, val);
}

NvCV_Status nvidia::ar::ar::set_uint64(NvAR_FeatureHandle handle, const char* name, unsigned long long val)
{
	return NvAR_SetU64(handle, name, val);
}

NvCV_Status nvidia::ar::ar::set_object(NvAR_FeatureHandle handle, const char* name, void* ptr, unsigned long typeSize)
{
	return NvAR_SetObject(handle, name, ptr, typeSize);
}

NvCV_Status nvidia::ar::ar::set_string(NvAR_FeatureHandle handle, const char* name, const char* str)
{
	return NvAR_SetString(handle, name, str);
}

NvCV_Status nvidia::ar::ar::set_cuda_stream(NvAR_FeatureHandle handle, const char* name, CUstream stream)
{
	return NvAR_SetCudaStream(handle, name, stream);
}

NvCV_Status nvidia::ar::ar::set_float32_array(NvAR_FeatureHandle handle, const char* name, float* val, int count)
{
	return NvAR_SetF32Array(handle, name, val, count);
}

NvCV_Status nvidia::ar::ar::get_uint32(NvAR_FeatureHandle handle, const char* name, unsigned int* val)
{
	return NvAR_GetU32(handle, name, val);
}

NvCV_Status nvidia::ar::ar::get_int32(NvAR_FeatureHandle handle, const char* name, int* val)
{
	return NvAR_GetS32(handle, name, val);
}

NvCV_Status nvidia::ar::ar::get_float32(NvAR_FeatureHandle handle, const char* name, float* val)
{
	return NvAR_GetF32(handle, name, val);
}

NvCV_Status nvidia::ar::ar::get_float64(NvAR_FeatureHandle handle, const char* name, double* val)
{
	return NvAR_GetF64(handle, name, val);
}

NvCV_Status nvidia::ar::ar::get_uint64(NvAR_FeatureHandle handle, const char* name, unsigned long long* val)
{
	return NvAR_GetU64(handle, name, val);
}

NvCV_Status nvidia::ar::ar::get_object(NvAR_FeatureHandle handle, const char* name, const void** ptr,
									   unsigned long typeSize)
{
	return NvAR_GetObject(handle, name, ptr, typeSize);
}

NvCV_Status nvidia::ar::ar::get_string(NvAR_FeatureHandle handle, const char* name, const char** str)
{
	return NvAR_GetString(handle, name, str);
}

NvCV_Status nvidia::ar::ar::get_cuda_stream(NvAR_FeatureHandle handle, const char* name, const CUstream* stream)
{
	return NvAR_GetCudaStream(handle, name, stream);
}

NvCV_Status nvidia::ar::ar::get_float32_array(NvAR_FeatureHandle handle, const char* name, const float** vals,
											  int* count)
{
	return NvAR_GetF32Array(handle, name, vals, count);
}

NvCV_Status nvidia::ar::ar::run(NvAR_FeatureHandle handle)
{
	return NvAR_Run(handle);
}

NvCV_Status nvidia::ar::ar::load(NvAR_FeatureHandle handle)
{
	return NvAR_Load(handle);
}

NvCV_Status nvidia::ar::ar::cuda_stream_create(CUstream* stream)
{
	return NvAR_CudaStreamCreate(stream);
}

NvCV_Status nvidia::ar::ar::cuda_stream_destroy(CUstream stream)
{
	return NvAR_CudaStreamDestroy(stream);
}

const char* nvidia::ar::ar::cv_get_error_string_from_code(NvCV_Status code)
{
	return NvCV_GetErrorStringFromCode(code);
}
