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
#include <string_view>
#include "nvidia/ar/nvidia-ar.hpp"
#include "nvidia/cuda/nvidia-cuda-obs.hpp"

namespace streamfx::nvidia::ar {
	class feature {
		protected:
		std::shared_ptr<::streamfx::nvidia::cuda::obs>    _nvcuda;
		std::shared_ptr<::streamfx::nvidia::cv::cv>       _nvcvi;
		std::shared_ptr<::streamfx::nvidia::ar::ar>       _nvar;
		std::shared_ptr<void> _fx;

		public:
		~feature();
		feature(feature_t feature);

		::streamfx::nvidia::ar::handle_t get()
		{
			return _fx.get();
		}

		inline void load()
		{
			if (auto res = _nvar->NvAR_Load(get()); res != cv::result::SUCCESS)
				throw cv::exception("NvAR_Load", res);
		}

		inline void run()
		{
			if (auto res = _nvar->NvAR_Run(get()); res != cv::result::SUCCESS)
				throw cv::exception("NvAR_Run", res);
		}

		public:
		inline void set(std::string_view name, int32_t value)
		{
			if (auto res = _nvar->NvAR_SetS32(get(), name.data(), value); res != cv::result::SUCCESS)
				throw cv::exception("NvAR_SetS32", res);
		}

		inline void get(std::string_view name, int32_t* value)
		{
			if (auto res = _nvar->NvAR_GetS32(get(), name.data(), value); res != cv::result::SUCCESS)
				throw cv::exception("NvAR_GetS32", res);
		}

		inline void set(std::string_view name, uint32_t value)
		{
			if (auto res = _nvar->NvAR_SetU32(get(), name.data(), value); res != cv::result::SUCCESS)
				throw cv::exception("NvAR_SetU32", res);
		}

		inline void get(std::string_view name, uint32_t* value)
		{
			if (auto res = _nvar->NvAR_GetU32(get(), name.data(), value); res != cv::result::SUCCESS)
				throw cv::exception("NvAR_GetU32", res);
		}

		inline void set(std::string_view name, uint64_t value)
		{
			if (auto res = _nvar->NvAR_SetU64(get(), name.data(), value); res != cv::result::SUCCESS)
				throw cv::exception("NvAR_SetU64", res);
		}

		inline void get(std::string_view name, uint64_t* value)
		{
			if (auto res = _nvar->NvAR_GetU64(get(), name.data(), value); res != cv::result::SUCCESS)
				throw cv::exception("NvAR_GetU64", res);
		}

		inline void set(std::string_view name, float value)
		{
			if (auto res = _nvar->NvAR_SetF32(get(), name.data(), value); res != cv::result::SUCCESS)
				throw cv::exception("NvAR_SetF32", res);
		}

		inline void get(std::string_view name, float* value)
		{
			if (auto res = _nvar->NvAR_GetF32(get(), name.data(), value); res != cv::result::SUCCESS)
				throw cv::exception("NvAR_GetF32", res);
		}

		inline void set(std::string_view name, double value)
		{
			if (auto res = _nvar->NvAR_SetF64(get(), name.data(), value); res != cv::result::SUCCESS)
				throw cv::exception("NvAR_SetF64", res);
		}

		inline void get(std::string_view name, double* value)
		{
			if (auto res = _nvar->NvAR_GetF64(get(), name.data(), value); res != cv::result::SUCCESS)
				throw cv::exception("NvAR_GetF64", res);
		}

		inline void set(std::string_view name, std::vector<float>& value)
		{
			if (auto res =
					_nvar->NvAR_SetF32Array(get(), name.data(), value.data(), static_cast<int32_t>(value.size()));
				res != cv::result::SUCCESS)
				throw cv::exception("NvAR_SetF32Array", res);
		}

		inline void get(std::string_view name, std::vector<float>& value)
		{
			const float* data;
			int32_t      size;

			if (auto res = _nvar->NvAR_GetF32Array(get(), name.data(), &data, &size); res != cv::result::SUCCESS)
				throw cv::exception("NvAR_GetF32Array", res);

			value.resize(size);
			memcpy(value.data(), data, size * sizeof(float));
		}

		inline void set(std::string_view name, std::string_view value)
		{
			if (auto res = _nvar->NvAR_SetString(get(), name.data(), value.data()); res != cv::result::SUCCESS)
				throw cv::exception("NvAR_SetString", res);
		}

		inline void get(std::string_view name, std::string& value)
		{
			const char* data;
			if (auto res = _nvar->NvAR_GetString(get(), name.data(), &data); res != cv::result::SUCCESS)
				throw cv::exception("NvAR_GetString", res);

			if (data != nullptr)
				value = std::string(data);
			else
				value.clear();
		}

		inline void set(std::string_view name, std::shared_ptr<::streamfx::nvidia::cuda::stream> value)
		{
			if (auto res = _nvar->NvAR_SetCudaStream(get(), name.data(), value->get()); res != cv::result::SUCCESS)
				throw cv::exception("NvAR_SetCudaStream", res);
		}
		//void get(...);

		inline void set(std::string_view name, void* data, size_t size)
		{
			if (auto res = _nvar->NvAR_SetObject(get(), name.data(), data, static_cast<uint32_t>(size));
				res != cv::result::SUCCESS)
				throw cv::exception("NvAR_SetObject", res);
		}
		//void get(std::string_view name, void** data, size_t data_size);
	};
} // namespace streamfx::nvidia::ar
