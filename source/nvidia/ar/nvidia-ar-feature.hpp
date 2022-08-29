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
#include "nvidia/ar/nvidia-ar.hpp"
#include "nvidia/cuda/nvidia-cuda-obs.hpp"
#include "nvidia/cv/nvidia-cv-image.hpp"
#include "nvidia/cv/nvidia-cv-texture.hpp"
#include "nvidia/cv/nvidia-cv.hpp"

#include "warning-disable.hpp"
#include <string_view>
#include "warning-enable.hpp"

namespace streamfx::nvidia::ar {
	class feature {
		protected:
		std::shared_ptr<::streamfx::nvidia::cuda::obs> _nvcuda;
		std::shared_ptr<::streamfx::nvidia::cv::cv>    _nvcv;
		std::shared_ptr<::streamfx::nvidia::ar::ar>    _nvar;
		std::shared_ptr<void>                          _fx;
		std::string                                    _model_path;

		public:
		~feature();
		feature(feature_t feature);

		::streamfx::nvidia::ar::handle_t get()
		{
			return _fx.get();
		}

		public /* Int32 */:
		inline cv::result set(parameter_t param, uint32_t const value)
		{
			return _nvar->NvAR_SetU32(_fx.get(), param, value);
		}
		inline cv::result get(parameter_t param, uint32_t* value)
		{
			return _nvar->NvAR_GetU32(_fx.get(), param, value);
		}

		inline cv::result set(parameter_t param, int32_t const value)
		{
			return _nvar->NvAR_SetS32(_fx.get(), param, value);
		}
		inline cv::result get(parameter_t param, int32_t* value)
		{
			return _nvar->NvAR_GetS32(_fx.get(), param, value);
		}

		public /* Int64 */:
		inline cv::result set(parameter_t param, uint64_t const value)
		{
			return _nvar->NvAR_SetU64(_fx.get(), param, value);
		}
		inline cv::result get(parameter_t param, uint64_t* value)
		{
			return _nvar->NvAR_GetU64(_fx.get(), param, value);
		}

		public /* Float32 */:
		inline cv::result set(parameter_t param, float const value)
		{
			return _nvar->NvAR_SetF32(_fx.get(), param, value);
		}
		inline cv::result get(parameter_t param, float* value)
		{
			return _nvar->NvAR_GetF32(_fx.get(), param, value);
		}

		inline cv::result set(parameter_t param, float* const value, int32_t size)
		{
			return _nvar->NvAR_SetF32Array(_fx.get(), param, value, static_cast<int32_t>(size));
		}
		inline cv::result get(parameter_t param, const float* value, int32_t size)
		{
			return _nvar->NvAR_GetF32Array(_fx.get(), param, &value, &size);
		}

		inline cv::result set(parameter_t param, std::vector<float> const& value)
		{
			return _nvar->NvAR_SetF32Array(_fx.get(), param, value.data(), static_cast<int32_t>(value.size()));
		}
		inline cv::result get(parameter_t param, std::vector<float>& value)
		{
			const float* data;
			int32_t      size;
			cv::result   result;

			result = _nvar->NvAR_GetF32Array(_fx.get(), param, &data, &size);

			value.resize(static_cast<size_t>(size));
			memcpy(value.data(), data, size * sizeof(float));

			return result;
		}

		public /* Float64 */:
		inline cv::result set(parameter_t param, double const value)
		{
			return _nvar->NvAR_SetF64(_fx.get(), param, value);
		}
		inline cv::result get(parameter_t param, double* value)
		{
			return _nvar->NvAR_GetF64(_fx.get(), param, value);
		}

		public /* String */:
		inline cv::result set(parameter_t param, const char* const value)
		{
			return _nvar->NvAR_SetString(_fx.get(), param, value);
		};
		inline cv::result get(parameter_t param, const char*& value)
		{
			return _nvar->NvAR_GetString(_fx.get(), param, &value);
		};

		inline cv::result set(parameter_t param, std::string_view const value)
		{
			return _nvar->NvAR_SetString(_fx.get(), param, value.data());
		};
		cv::result get(parameter_t param, std::string_view& value);

		inline cv::result set(parameter_t param, std::string const& value)
		{
			return _nvar->NvAR_SetString(_fx.get(), param, value.c_str());
		};
		cv::result get(parameter_t param, std::string& value);

		public /* CUDA Stream */:
		inline cv::result set(parameter_t param, cuda::stream_t const value)
		{
			return _nvar->NvAR_SetCudaStream(_fx.get(), param, value);
		};
		inline cv::result get(parameter_t param, cuda::stream_t& value)
		{
			return _nvar->NvAR_GetCudaStream(_fx.get(), param, &value);
		};

		inline cv::result set(parameter_t param, std::shared_ptr<::streamfx::nvidia::cuda::stream> const value)
		{
			return _nvar->NvAR_SetCudaStream(_fx.get(), param, value->get());
		}
		//inline cv::result get(parameter_t param, std::shared_ptr<::streamfx::nvidia::cuda::stream> value);

		public /* CV Image */:
		inline cv::result set(parameter_t param, cv::image_t& value)
		{
			return _nvar->NvAR_SetObject(_fx.get(), param, &value, sizeof(cv::image_t));
		};
		inline cv::result get(parameter_t param, cv::image_t*& value)
		{
			return _nvar->NvAR_GetObject(_fx.get(), param, reinterpret_cast<object_t*>(&value), sizeof(cv::image_t));
		};

		inline cv::result set(parameter_t param, std::shared_ptr<cv::image> const value)
		{
			return _nvar->NvAR_SetObject(_fx.get(), param, value->get_image(), sizeof(cv::image_t));
		};
		//inline cv::result get(parameter_t param, std::shared_ptr<cv::image>& value);

		public /* CV Texture */:
		inline cv::result set(parameter_t param, std::shared_ptr<cv::texture> const value)
		{
			return _nvar->NvAR_SetObject(_fx.get(), param, value->get_image(), sizeof(cv::image_t));
		};
		//inline cv::result get(parameter_t param, std::shared_ptr<cv::image>& value);

		public /* Objects */:
		inline cv::result set_object(parameter_t param, void* const data, size_t size)
		{
			return _nvar->NvAR_SetObject(_fx.get(), param, data, static_cast<uint32_t>(size));
		}
		inline cv::result get_object(parameter_t param, void*& data, size_t size)
		{
			return _nvar->NvAR_GetObject(_fx.get(), param, &data, static_cast<uint32_t>(size));
		}

		public /* Control */:
		inline cv::result load()
		{
			return _nvar->NvAR_Load(_fx.get());
		}

		inline cv::result run()
		{
			return _nvar->NvAR_Run(_fx.get());
		}
	};
} // namespace streamfx::nvidia::ar
