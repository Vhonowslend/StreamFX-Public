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
#include <string>
#include "nvidia-ar.hpp"
#include "nvidia/cuda/nvidia-cuda-stream.hpp"

namespace nvidia::ar {
	class feature {
		std::shared_ptr<::nvidia::ar::ar> _ar;
		std::shared_ptr<nvAR_Feature>     _feature;

		public:
		feature(std::shared_ptr<::nvidia::ar::ar> ar, NvAR_FeatureID feature);
		~feature();

		public:
		template<typename T>
		inline NvCV_Status set(std::string name, T value);
		template<typename T>
		inline NvCV_Status get(std::string name, T& value);

		template<>
		inline NvCV_Status set(std::string name, int32_t value)
		{
			return _ar->set_int32(_feature.get(), name.c_str(), value);
		}

		template<>
		inline NvCV_Status get(std::string name, int32_t& value)
		{
			return _ar->get_int32(_feature.get(), name.c_str(), &value);
		}

		template<>
		inline NvCV_Status set(std::string name, uint32_t value)
		{
			return _ar->set_uint32(_feature.get(), name.c_str(), value);
		}

		template<>
		inline NvCV_Status get(std::string name, uint32_t& value)
		{
			return _ar->get_uint32(_feature.get(), name.c_str(), &value);
		}

		template<>
		inline NvCV_Status set(std::string name, uint64_t value)
		{
			return _ar->set_uint64(_feature.get(), name.c_str(), value);
		}

		template<>
		inline NvCV_Status get(std::string name, uint64_t& value)
		{
			return _ar->get_uint64(_feature.get(), name.c_str(), &value);
		}

		template<>
		inline NvCV_Status set(std::string name, std::float_t value)
		{
			return _ar->set_float32(_feature.get(), name.c_str(), value);
		}

		template<>
		inline NvCV_Status get(std::string name, std::float_t& value)
		{
			return _ar->get_float32(_feature.get(), name.c_str(), &value);
		}

		template<>
		inline NvCV_Status set(std::string name, std::vector<std::float_t> value)
		{
			return _ar->set_float32_array(_feature.get(), name.c_str(), value.data(),
										  static_cast<int32_t>(value.size()));
		}

		template<>
		inline NvCV_Status get(std::string name, std::vector<std::float_t>& value)
		{
			// ToDo: Validate this.
			const float* vals      = nullptr;
			int          val_count = 0;
			NvCV_Status  res       = _ar->get_float32_array(_feature.get(), name.c_str(), &vals, &val_count);
			if (res != NVCV_SUCCESS) {
				return res;
			} else {
				value.resize(static_cast<size_t>(val_count));
				for (std::size_t idx = 0; idx < static_cast<std::size_t>(val_count); idx++) {
					value[idx] = *vals;
					vals++;
				}
				return res;
			}
		}

		template<>
		inline NvCV_Status set(std::string name, std::double_t value)
		{
			return _ar->set_float64(_feature.get(), name.c_str(), value);
		}

		template<>
		inline NvCV_Status get(std::string name, std::double_t& value)
		{
			return _ar->get_float64(_feature.get(), name.c_str(), &value);
		}

		template<>
		inline NvCV_Status set(std::string name, std::string value)
		{
			return _ar->set_string(_feature.get(), name.c_str(), value.c_str());
		}

		template<>
		inline NvCV_Status get(std::string name, std::string& value)
		{
			// ToDo: Validate this.
			const char* buf;
			NvCV_Status res = _ar->get_string(_feature.get(), name.c_str(), &buf);
			if (res == NVCV_SUCCESS) {
				value = std::string(buf, buf + strlen(buf));
				return res;
			} else {
				return res;
			}
		}

		template<>
		inline NvCV_Status set(std::string name, std::shared_ptr<::nvidia::cuda::stream> value)
		{
			return _ar->set_cuda_stream(_feature.get(), name.c_str(), reinterpret_cast<CUstream>(value->get()));
		}

		template<>
		inline NvCV_Status get(std::string name, std::shared_ptr<::nvidia::cuda::stream>& value)
		{}
	};
} // namespace nvidia::ar
