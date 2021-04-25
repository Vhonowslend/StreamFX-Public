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

#include "nvidia-cuda-obs.hpp"
#include "obs/gs/gs-helper.hpp"
#include "util/util-logging.hpp"

#ifdef _DEBUG
#define ST_PREFIX "<%s> "
#define D_LOG_ERROR(x, ...) P_LOG_ERROR(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_WARNING(x, ...) P_LOG_WARN(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_INFO(x, ...) P_LOG_INFO(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_DEBUG(x, ...) P_LOG_DEBUG(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#else
#define ST_PREFIX "<nvidia::cuda::obs> "
#define D_LOG_ERROR(...) P_LOG_ERROR(ST_PREFIX __VA_ARGS__)
#define D_LOG_WARNING(...) P_LOG_WARN(ST_PREFIX __VA_ARGS__)
#define D_LOG_INFO(...) P_LOG_INFO(ST_PREFIX __VA_ARGS__)
#define D_LOG_DEBUG(...) P_LOG_DEBUG(ST_PREFIX __VA_ARGS__)
#endif

nvidia::cuda::obs::~obs()
{
	D_LOG_DEBUG("Finalizing... (Addr: 0x%" PRIuPTR ")", this);

	auto gctx = gs::context{};
	{
		auto stack = _context->enter();
		_stream->synchronize();
		_context->synchronize();
	}
	_context.reset();
	_cuda.reset();
}

nvidia::cuda::obs::obs() : _cuda(::nvidia::cuda::cuda::get()), _context()
{
	D_LOG_DEBUG("Initializating... (Addr: 0x%" PRIuPTR ")", this);

	auto gctx = gs::context{};

	// Create Context
#ifdef WIN32
	if (gs_get_device_type() == GS_DEVICE_DIRECT3D_11) {
		_context = std::make_shared<::nvidia::cuda::context>(reinterpret_cast<ID3D11Device*>(gs_get_device_obj()));
	}
#endif
	if (gs_get_device_type() == GS_DEVICE_OPENGL) {
		throw std::runtime_error("Not yet implemented.");
	}

	// Create Stream
	auto stack = _context->enter();
	_stream    = std::make_shared<::nvidia::cuda::stream>();
}

std::shared_ptr<nvidia::cuda::obs> nvidia::cuda::obs::get()
{
	static std::weak_ptr<nvidia::cuda::obs> instance;
	static std::mutex                       lock;

	std::unique_lock<std::mutex> ul(lock);
	if (instance.expired()) {
		std::shared_ptr<nvidia::cuda::obs> hard_instance;
		hard_instance = std::make_shared<nvidia::cuda::obs>();
		instance      = hard_instance;
		return hard_instance;
	}
	return instance.lock();
}

std::shared_ptr<nvidia::cuda::cuda> nvidia::cuda::obs::get_cuda()
{
	return _cuda;
}

std::shared_ptr<nvidia::cuda::context> nvidia::cuda::obs::get_context()
{
	return _context;
}

std::shared_ptr<nvidia::cuda::stream> nvidia::cuda::obs::get_stream()
{
	return _stream;
}
