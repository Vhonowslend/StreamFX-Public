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
#include <memory>
#include "nvidia-cuda.hpp"

namespace nvidia::cuda {
	class stream {
		std::shared_ptr<::nvidia::cuda::cuda> _cuda;
		::nvidia::cuda::stream_t           _stream;

		public:
		stream(std::shared_ptr<::nvidia::cuda::cuda> cuda,
			   ::nvidia::cuda::stream_flags       flags    = ::nvidia::cuda::stream_flags::DEFAULT,
			   std::int32_t                          priority = 0);
		~stream();

		::nvidia::cuda::stream_t get();
	};
} // namespace nvidia::cuda
