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

#include "nvidia-cuda-context-stack.hpp"
#include <stdexcept>

nvidia::cuda::context_stack::context_stack(std::shared_ptr<::nvidia::cuda::cuda>    cuda,
										   std::shared_ptr<::nvidia::cuda::context> context)
	: _cuda(cuda), _ctx(context)
{
	using namespace ::nvidia::cuda;

	if (!cuda)
		throw std::invalid_argument("cuda");
	if (!context)
		throw std::invalid_argument("context");

	if (result res = _cuda->cuCtxPushCurrent(_ctx->get()); res != result::SUCCESS) {
		throw std::runtime_error("Failed to push context.");
	}
}

nvidia::cuda::context_stack::~context_stack()
{
	using namespace ::nvidia::cuda;

	context_t ctx;
	_cuda->cuCtxGetCurrent(&ctx);
	if (ctx == _ctx->get()) {
		_cuda->cuCtxPopCurrent(&ctx);
	}
}
