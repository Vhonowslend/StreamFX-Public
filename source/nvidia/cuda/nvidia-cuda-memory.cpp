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

#include "nvidia-cuda-memory.hpp"
#include <stdexcept>

nvidia::cuda::memory::memory(std::shared_ptr<::nvidia::cuda::cuda> cuda, std::size_t size)
	: _cuda(cuda), _pointer(), _size(size)
{
	::nvidia::cuda::result res = _cuda->cuMemAlloc(&_pointer, size);
	switch (res) {
	case ::nvidia::cuda::result::SUCCESS:
		break;
	default:
		throw std::runtime_error("nvidia::cuda::memory: cuMemAlloc failed.");
	}
}

nvidia::cuda::memory::~memory()
{
	_cuda->cuMemFree(_pointer);
}

nvidia::cuda::device_ptr_t nvidia::cuda::memory::get()
{
	return _pointer;
}

std::size_t nvidia::cuda::memory::size()
{
	return _size;
}
