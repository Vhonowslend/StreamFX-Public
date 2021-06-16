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
#include <filesystem>
#include <functional>
#include <memory>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4138)
#endif
#include <nvAR.h>
#include <nvAR_defs.h>
#include <nvCVImage.h>
#include <nvCVStatus.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace streamfx::nvidia::ar {
	class ar {
		public:
		ar();
		~ar();

		std::filesystem::path get_ar_sdk_path();

		public:
		NvCV_Status image_init(NvCVImage* im, unsigned width, unsigned height, int pitch, void* pixels,
							   NvCVImage_PixelFormat format, NvCVImage_ComponentType type, unsigned isPlanar,
							   unsigned onGPU);
		void image_init_view(NvCVImage* subImg, NvCVImage* fullImg, int x, int y, unsigned width, unsigned height);
		NvCV_Status image_alloc(NvCVImage* im, unsigned width, unsigned height, NvCVImage_PixelFormat format,
								NvCVImage_ComponentType type, unsigned isPlanar, unsigned onGPU, unsigned alignment);
		NvCV_Status image_realloc(NvCVImage* im, unsigned width, unsigned height, NvCVImage_PixelFormat format,
								  NvCVImage_ComponentType type, unsigned isPlanar, unsigned onGPU, unsigned alignment);
		void        image_dealloc(NvCVImage* im);
		NvCV_Status image_create(unsigned width, unsigned height, NvCVImage_PixelFormat format,
								 NvCVImage_ComponentType type, unsigned isPlanar, unsigned onGPU, unsigned alignment,
								 NvCVImage** out);
		void        image_destroy(NvCVImage* im);
		void        image_component_offsets(NvCVImage_PixelFormat format, int* rOff, int* gOff, int* bOff, int* aOff,
											int* yOff);
		NvCV_Status image_transfer(const NvCVImage* src, NvCVImage* dst, float scale, CUstream_st* stream,
								   NvCVImage* tmp);
		NvCV_Status image_composite(const NvCVImage* src, const NvCVImage* mat, NvCVImage* dst);
		NvCV_Status image_composite_over_constant(const NvCVImage* src, const NvCVImage* mat,
												  const unsigned char bgColor[3], NvCVImage* dst);
		NvCV_Status image_flipy(const NvCVImage* src, NvCVImage* dst);
		NvCV_Status create(NvAR_FeatureID featureID, NvAR_FeatureHandle* handle);
		NvCV_Status destroy(NvAR_FeatureHandle handle);
		NvCV_Status set_uint32(NvAR_FeatureHandle handle, const char* name, unsigned int val);
		NvCV_Status set_int32(NvAR_FeatureHandle handle, const char* name, int val);
		NvCV_Status set_float32(NvAR_FeatureHandle handle, const char* name, float val);
		NvCV_Status set_float64(NvAR_FeatureHandle handle, const char* name, double val);
		NvCV_Status set_uint64(NvAR_FeatureHandle handle, const char* name, unsigned long long val);
		NvCV_Status set_object(NvAR_FeatureHandle handle, const char* name, void* ptr, unsigned long typeSize);
		NvCV_Status set_string(NvAR_FeatureHandle handle, const char* name, const char* str);
		NvCV_Status set_cuda_stream(NvAR_FeatureHandle handle, const char* name, CUstream stream);
		NvCV_Status set_float32_array(NvAR_FeatureHandle handle, const char* name, float* val, int count);
		NvCV_Status get_uint32(NvAR_FeatureHandle handle, const char* name, unsigned int* val);
		NvCV_Status get_int32(NvAR_FeatureHandle handle, const char* name, int* val);
		NvCV_Status get_float32(NvAR_FeatureHandle handle, const char* name, float* val);
		NvCV_Status get_float64(NvAR_FeatureHandle handle, const char* name, double* val);
		NvCV_Status get_uint64(NvAR_FeatureHandle handle, const char* name, unsigned long long* val);
		NvCV_Status get_object(NvAR_FeatureHandle handle, const char* name, const void** ptr, unsigned long typeSize);
		NvCV_Status get_string(NvAR_FeatureHandle handle, const char* name, const char** str);
		NvCV_Status get_cuda_stream(NvAR_FeatureHandle handle, const char* name, const CUstream* stream);
		NvCV_Status get_float32_array(NvAR_FeatureHandle handle, const char* name, const float** vals, int* count);
		NvCV_Status run(NvAR_FeatureHandle handle);
		NvCV_Status load(NvAR_FeatureHandle handle);
		NvCV_Status cuda_stream_create(CUstream* stream);
		NvCV_Status cuda_stream_destroy(CUstream stream);
		const char* cv_get_error_string_from_code(NvCV_Status code);
	};
} // namespace streamfx::nvidia::ar
