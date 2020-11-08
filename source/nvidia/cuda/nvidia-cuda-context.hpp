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
	class context_stack;

	class context : public std::enable_shared_from_this<::nvidia::cuda::context> {
		std::shared_ptr<::nvidia::cuda::cuda> _cuda;
		::nvidia::cuda::context_t             _ctx;
		bool                                  _has_device;
		::nvidia::cuda::device_t              _device;

		private:
		context();

		public:
		~context();

#ifdef WIN32
		context(ID3D11Device* device);
#endif

		::nvidia::cuda::context_t get();

		void push();
		void pop();

		void synchronize();

		public:
		std::shared_ptr<::nvidia::cuda::context_stack> enter();
	};

	class context_stack {
		std::shared_ptr<::nvidia::cuda::context> _ctx;

		public:
		inline ~context_stack()
		{
			_ctx->pop();
		}
		inline context_stack(std::shared_ptr<::nvidia::cuda::context> ctx) : _ctx(ctx)
		{
			_ctx->push();
		}
	};
} // namespace nvidia::cuda
