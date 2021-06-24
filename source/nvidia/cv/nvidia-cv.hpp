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

#pragma once
#include <cinttypes>
#include "nvidia/cuda/nvidia-cuda.hpp"
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

#define NVCVI_DEFINE_FUNCTION(name, ...)                                   \
	private:                                                               \
	typedef ::streamfx::nvidia::cv::result(__cdecl* t##name)(__VA_ARGS__); \
                                                                           \
	public:                                                                \
	t##name name = nullptr;

#define NVCVI_DEFINE_FUNCTION_EX(ret, name, ...) \
	private:                                     \
	typedef ret(__cdecl* t##name)(__VA_ARGS__);  \
                                                 \
	public:                                      \
	t##name name = nullptr;

namespace streamfx::nvidia::cv {
	enum class result {
		// NVIDIA uses negative codes, but we use positive.
		SUCCESS                 = 0,
		ERROR_GENERAL           = -1,
		ERROR_UNIMPLEMENTED     = -2,
		ERROR_MEMORY            = -3,
		ERROR_EFFECT            = -4,
		ERROR_SELECTOR          = -5,
		ERROR_BUFFER            = -6,
		ERROR_PARAMETER         = -7,
		ERROR_MISMATCH          = -8,
		ERROR_PIXELFORMAT       = -9,
		ERROR_MODEL             = -10,
		ERROR_LIBRARY           = -11,
		ERROR_INITIALIZATION    = -12,
		ERROR_FILE              = -13,
		ERROR_FEATURENOTFOUND   = -14,
		ERROR_MISSINGINPUT      = -15,
		ERROR_RESOLUTION        = -16,
		ERROR_UNSUPPORTEDGPU    = -17,
		ERROR_WRONGGPU          = -18,
		ERROR_UNSUPPORTEDDRIVER = -19,
		ERROR_MODELDEPENDENCIES = -20,
		ERROR_PARSE             = -21,
		ERROR_MODELSUBSTITUTION = -22,
		ERROR_READ              = -23,
		ERROR_WRITE             = -24,
		ERROR_PARAMREADONLY     = -25,
		ERROR_TRT_ENQUEUE       = -26,
		ERROR_TRT_BINDINGS      = -27,
		ERROR_TRT_CONTEXT       = -28,
		ERROR_TRT_INFER         = -29,
		ERROR_TRT_ENGINE        = -30,
		ERROR_NPP               = -31,
		ERROR_CONFIG            = -32,

		// Error from Graphics API
		ERROR_DIRECT3D = -99,

		// Error from CUDA
		ERROR_CUDA_BASE            = -100,
		ERROR_CUDA_VALUE           = -101,
		ERROR_CUDA_MEMORY          = -102,
		ERROR_CUDA_PITCH           = -112,
		ERROR_CUDA_INIT            = -127,
		ERROR_CUDA_LAUNCH          = -819,
		ERROR_CUDA_KERNEL          = -309,
		ERROR_CUDA_DRIVER          = -135,
		ERROR_CUDA_UNSUPPORTED     = -901,
		ERROR_CUDA_ILLEGAL_ADDRESS = -800,
		ERROR_CUDA                 = -1099,
	};

	enum class pixel_format {
		UNKNOWN = 0,
		Y       = 1,
		A       = 2,
		YA      = 3,
		RGB     = 4,
		BGR     = 5,
		RGBA    = 6,
		BGRA    = 7,
		ARGB    = 8,
		ABGR    = 9,
		YUV420  = 10,
		YUV422  = 11,
		YUV444  = 12,
	};

	enum class component_type {
		UKNOWN = 0,
		UINT8  = 1,
		UINT16 = 2,
		SINT16 = 3,
		FP16   = 4,
		UINT32 = 5,
		SINT   = 6,
		FP32   = 7,
		UINT64 = 8,
		SINT64 = 9,
		FP64   = 10,
	};

	enum class component_layout {
		INTERLEAVED = 0,
		PLANAR      = 1,
		UYVY        = 2,
		YUV         = 3,
		VYUY        = 4,
		YVU         = 5,
		YUYV        = 6,
		YCUV        = 7,
		YVYU        = 8,
		YCVU        = 9,
		CYUV        = 10,
		_RESERVED11 = 11,
		CYVU        = 12,
		CHUNKY      = INTERLEAVED,
		I420        = YUV,
		IYUV        = YUV,
		YV12        = YVU,
		NV12        = YCUV,
		NV21        = YCVU,
		YUY2        = YUYV,
		I444        = YUV,
		YM24        = YUV,
		YM42        = YVU,
		NV24        = YCUV,
		NV42        = YCVU,
	};

	enum class color_information {
		SPACE_BT_601                 = 0x00,
		SPACE_BT_709                 = 0x01,
		SPACE_BT_2020                = 0x02,
		RANGE_PARTIAL                = 0x00,
		RANGE_FULL                   = 0x04,
		CHROMA_LOCATION_COSITED      = 0x00,
		CHROMA_LOCATION_INTERSTITIAL = 0x08,
		CHROMA_LOCATION_TOPLEFT      = 0x10,
	};

	enum class memory_location {
		CPU        = 0,
		GPU        = 1,
		CPU_PINNED = 2,
		CUDA_ARRAY = 3,
	};

	struct image_t {
		uint32_t       width;
		uint32_t       height;
		int32_t        pitch;
		pixel_format   pxl_format;
		component_type comp_type;
		uint8_t        pixel_bytes;
		uint8_t        component_bytes;
		uint8_t        num_components;
		unsigned char  comp_layout;
		unsigned char  mem_location;
		unsigned char  color_info;
		uint8_t        reserved[2];
		void*          pixels;
		void*          delete_pointer;
		void (*delete_function)(void* delete_pointer);
		uint64_t buffer_bytes;
	};

	template<typename T>
	struct point {
		T x, y;
	};

	template<typename T>
	struct rect {
		T x, y;
		T w, h;
	};

	class cv {
		std::shared_ptr<::streamfx::util::library> _library;

		public:
		~cv();
		cv();

		public:
		NVCVI_DEFINE_FUNCTION(NvCVImage_Init, image_t* image, uint32_t width, uint32_t height, uint32_t pitch,
							  void* pixels, pixel_format format, component_type comp_type, component_layout comp_layout,
							  memory_location mem_location);
		NVCVI_DEFINE_FUNCTION(NvCVImage_InitView, image_t* sub_image, image_t* image, int32_t x, int32_t y,
							  uint32_t width, uint32_t height);
		NVCVI_DEFINE_FUNCTION(NvCVImage_Alloc, image_t* image, uint32_t width, uint32_t height, pixel_format format,
							  component_type comp_type, uint32_t comp_layout, uint32_t mem_location,
							  uint32_t alignment);
		NVCVI_DEFINE_FUNCTION(NvCVImage_Realloc, image_t* image, uint32_t width, uint32_t height, pixel_format format,
							  component_type comp_type, uint32_t comp_layout, uint32_t mem_location,
							  uint32_t alignment);
		NVCVI_DEFINE_FUNCTION_EX(void, NvCVImage_Dealloc, image_t* image);
		NVCVI_DEFINE_FUNCTION(NvCVImage_Create, uint32_t width, uint32_t height, pixel_format format,
							  component_type comp_type, component_layout comp_layout, memory_location mem_location,
							  uint32_t alignment, image_t** image);
		NVCVI_DEFINE_FUNCTION_EX(void, NvCVImage_Destroy, image_t* image);
		NVCVI_DEFINE_FUNCTION_EX(void, NvCVImage_ComponentOffsets, pixel_format format, int32_t* red_offset,
								 int32_t* green_offset, int32_t* blue_offset, int32_t* alpha_offset, int32_t* y_offset);
		NVCVI_DEFINE_FUNCTION(NvCVImage_Transfer, const image_t* source, image_t* destination, float scale,
							  ::streamfx::nvidia::cuda::stream_t stream, image_t* buffer);
		NVCVI_DEFINE_FUNCTION(NvCVImage_TransferRect, const image_t* source, const rect<int32_t>* source_rect,
							  image_t* destination, const point<int32_t>* destination_point, float scale,
							  ::streamfx::nvidia::cuda::stream_t stream, image_t* buffer);
		NVCVI_DEFINE_FUNCTION(NvCVImage_TransferFromYUV, const void* y, int32_t yPixBytes, int32_t yPitch,
							  const void* u, const void* v, int32_t uvPixBytes, int32_t uvPitch, pixel_format yuvFormat,
							  component_type yuvType, color_information yuvColorSpace, memory_location yuvMemSpace,
							  image_t* destination, const rect<int32_t>* destination_area, float scale,
							  ::streamfx::nvidia::cuda::stream_t stream, image_t* tmp);
		NVCVI_DEFINE_FUNCTION(NvCVImage_TransferToYUV, const image_t* source, const rect<int32_t>* source_area,
							  const void* y, int32_t yPixBytes, int32_t yPitch, const void* u, const void* v,
							  int uvPixBytes, int32_t uvPitch, pixel_format yuvFormat, component_type yuvType,
							  color_information yuvColorSpace, memory_location yuvMemSpace, float scale,
							  ::streamfx::nvidia::cuda::stream_t stream, image_t* tmp);
		NVCVI_DEFINE_FUNCTION(NvCVImage_MapResource, image_t* image, ::streamfx::nvidia::cuda::stream_t stream);
		NVCVI_DEFINE_FUNCTION(NvCVImage_UnmapResource, image_t* image, ::streamfx::nvidia::cuda::stream_t stream);
		NVCVI_DEFINE_FUNCTION(NvCVImage_Composite, const image_t* foreground, const image_t* background,
							  const image_t* matte, image_t* destination, ::streamfx::nvidia::cuda::stream_t stream);
		NVCVI_DEFINE_FUNCTION(NvCVImage_CompositeRect, const image_t* foreground,
							  const point<int32_t> foreground_origin, const image_t* background,
							  const point<int32_t> background_origin, const image_t* matte, uint32_t mode,
							  image_t* destination, const point<int32_t> destination_origin,
							  ::streamfx::nvidia::cuda::stream_t stream);
		NVCVI_DEFINE_FUNCTION(NvCVImage_CompositeOverConstant, const image_t* source, const image_t* matte,
							  const uint8_t background_color[3], image_t* destination);
		NVCVI_DEFINE_FUNCTION(NvCVImage_FlipY, const image_t* source, image_t* destination);
		NVCVI_DEFINE_FUNCTION(NvCVImage_GetYUVPointers, image_t* image, uint8_t** y, uint8_t** u, uint8_t** v,
							  int32_t* y_pixel_bytes, int32_t* c_pixel_bytes, int32_t* y_row_bytes,
							  int32_t* c_row_bytes);

		NVCVI_DEFINE_FUNCTION_EX(const char*, NvCV_GetErrorStringFromCode, result code);

#ifdef WIN32
		NVCVI_DEFINE_FUNCTION(NvCVImage_InitFromD3D11Texture, image_t* image, struct ID3D11Texture2D* texture);
		NVCVI_DEFINE_FUNCTION(NvCVImage_ToD3DFormat, pixel_format format, component_type comp_type,
							  component_layout comp_layout, DXGI_FORMAT* dxgi_format);
		NVCVI_DEFINE_FUNCTION(NvCVImage_FromD3DFormat, DXGI_FORMAT d3dFormat, pixel_format* format,
							  component_type* comp_type, component_layout* comp_layout);

#ifdef __dxgicommon_h__
		NVCVI_DEFINE_FUNCTION(NvCVImage_ToD3DColorSpace, color_information nvcvColorSpace,
							  DXGI_COLOR_SPACE_TYPE* pD3dColorSpace);
		NVCVI_DEFINE_FUNCTION(NvCVImage_FromD3DColorSpace, DXGI_COLOR_SPACE_TYPE d3dColorSpace,
							  color_information* pNvcvColorSpace);
#endif
#endif

		public:
		static std::shared_ptr<::streamfx::nvidia::cv::cv> get();
	};
} // namespace streamfx::nvidia::cv

P_ENABLE_BITMASK_OPERATORS(::streamfx::nvidia::cv::color_information);
