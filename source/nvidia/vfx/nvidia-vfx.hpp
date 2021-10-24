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
#include "nvidia/cv/nvidia-cv.hpp"

#define NVVFX_DEFINE_FUNCTION(name, ...)                                   \
	private:                                                               \
	typedef ::streamfx::nvidia::cv::result(__cdecl* t##name)(__VA_ARGS__); \
                                                                           \
	public:                                                                \
	t##name name = nullptr;

namespace streamfx::nvidia::vfx {
	typedef const char* effect_t;
	typedef const char* parameter_t;
	typedef void*       object_t;
	typedef object_t    handle_t;

	static constexpr effect_t EFFECT_TRANSFER           = "Transfer";
	static constexpr effect_t EFFECT_GREEN_SCREEN       = "GreenScreen";
	static constexpr effect_t EFFECT_BACKGROUND_BLUR    = "BackgroundBlur";
	static constexpr effect_t EFFECT_ARTIFACT_REDUCTION = "ArtifactReduction";
	static constexpr effect_t EFFECT_SUPERRESOLUTION    = "SuperRes";
	static constexpr effect_t EFFECT_UPSCALE            = "Upscale";
	static constexpr effect_t EFFECT_DENOISING          = "Denoising";

	static constexpr parameter_t PARAMETER_INPUT_IMAGE_0   = "SrcImage0";
	static constexpr parameter_t PARAMETER_INPUT_IMAGE_1   = "SrcImage1";
	static constexpr parameter_t PARAMETER_OUTPUT_IMAGE_0  = "DstImage0";
	static constexpr parameter_t PARAMETER_MODEL_DIRECTORY = "ModelDir";
	static constexpr parameter_t PARAMETER_CUDA_STREAM     = "CudaStream";
	static constexpr parameter_t PARAMETER_INFO            = "Info";
	static constexpr parameter_t PARAMETER_SCALE           = "Scale";
	static constexpr parameter_t PARAMETER_STRENGTH        = "Strength";
	static constexpr parameter_t PARAMETER_STRENGTH_LEVELS = "StrengthLevels";
	static constexpr parameter_t PARAMETER_MODE            = "Mode";
	static constexpr parameter_t PARAMETER_TEMPORAL        = "Temporal";
	static constexpr parameter_t PARAMETER_GPU             = "GPU";
	static constexpr parameter_t PARAMETER_BATCH_SIZE      = "BatchSize";
	static constexpr parameter_t PARAMETER_MODEL_BATCH     = "ModelBatch";
	static constexpr parameter_t PARAMETER_STATE           = "State";
	static constexpr parameter_t PARAMETER_STATE_SIZE      = "StateSize";

	class vfx {
		std::shared_ptr<::streamfx::util::library> _library;
#ifdef WIN32
		void* _extra;
#endif
		std::filesystem::path _model_path;

		public:
		~vfx();
		vfx();

		std::filesystem::path model_path();

		public:
		NVVFX_DEFINE_FUNCTION(NvVFX_GetVersion, uint32_t* version);
		NVVFX_DEFINE_FUNCTION(NvVFX_CreateEffect, effect_t effect, handle_t* handle);
		NVVFX_DEFINE_FUNCTION(NvVFX_DestroyEffect, handle_t handle);
		NVVFX_DEFINE_FUNCTION(NvVFX_SetU32, handle_t effect, parameter_t paramName, uint32_t val);
		NVVFX_DEFINE_FUNCTION(NvVFX_SetS32, handle_t effect, parameter_t paramName, int32_t val);
		NVVFX_DEFINE_FUNCTION(NvVFX_SetF32, handle_t effect, parameter_t paramName, float val);
		NVVFX_DEFINE_FUNCTION(NvVFX_SetF64, handle_t effect, parameter_t paramName, double val);
		NVVFX_DEFINE_FUNCTION(NvVFX_SetU64, handle_t effect, parameter_t paramName, uint64_t val);
		NVVFX_DEFINE_FUNCTION(NvVFX_SetObject, handle_t effect, parameter_t paramName, void* ptr);
		NVVFX_DEFINE_FUNCTION(NvVFX_SetCudaStream, handle_t effect, parameter_t paramName,
							  ::streamfx::nvidia::cuda::stream_t stream);
		NVVFX_DEFINE_FUNCTION(NvVFX_SetImage, handle_t effect, parameter_t paramName,
							  ::streamfx::nvidia::cv::image_t* im);
		NVVFX_DEFINE_FUNCTION(NvVFX_SetString, handle_t effect, parameter_t paramName, const char* str);
		NVVFX_DEFINE_FUNCTION(NvVFX_GetU32, handle_t effect, parameter_t paramName, uint32_t* val);
		NVVFX_DEFINE_FUNCTION(NvVFX_GetS32, handle_t effect, parameter_t paramName, int32_t* val);
		NVVFX_DEFINE_FUNCTION(NvVFX_GetF32, handle_t effect, parameter_t paramName, float* val);
		NVVFX_DEFINE_FUNCTION(NvVFX_GetF64, handle_t effect, parameter_t paramName, double* val);
		NVVFX_DEFINE_FUNCTION(NvVFX_GetU64, handle_t effect, parameter_t paramName, uint64_t* val);
		NVVFX_DEFINE_FUNCTION(NvVFX_GetObject, handle_t effect, parameter_t paramName, void** ptr);
		NVVFX_DEFINE_FUNCTION(NvVFX_GetCudaStream, handle_t effect, parameter_t paramName,
							  ::streamfx::nvidia::cuda::stream_t stream);
		NVVFX_DEFINE_FUNCTION(NvVFX_GetImage, handle_t effect, parameter_t paramName,
							  ::streamfx::nvidia::cv::image_t* im);
		NVVFX_DEFINE_FUNCTION(NvVFX_GetString, handle_t effect, parameter_t paramName, const char** str);
		NVVFX_DEFINE_FUNCTION(NvVFX_Run, handle_t effect, int32_t async);
		NVVFX_DEFINE_FUNCTION(NvVFX_Load, handle_t effect);

		public:
		static std::shared_ptr<::streamfx::nvidia::vfx::vfx> get();
	};
} // namespace streamfx::nvidia::vfx
