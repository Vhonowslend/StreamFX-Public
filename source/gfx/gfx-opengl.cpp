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

#include "gfx-opengl.hpp"
#include "plugin.hpp"
#include "util/util-logging.hpp"

#include "warning-disable.hpp"
#include <mutex>
#include "warning-enable.hpp"

// OpenGL
#include "warning-disable.hpp"
#include "glad/gl.h"
#ifdef D_PLATFORM_WINDOWS
#include "glad/wgl.h"
#endif
#ifdef D_PLATFORM_LINUX
#include "glad/glx.h"
#endif
#include "warning-enable.hpp"

#ifdef _DEBUG
#define ST_PREFIX "<%s> "
#define D_LOG_ERROR(x, ...) P_LOG_ERROR(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_WARNING(x, ...) P_LOG_WARN(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_INFO(x, ...) P_LOG_INFO(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_DEBUG(x, ...) P_LOG_DEBUG(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#else
#define ST_PREFIX "<gfx::opengl> "
#define D_LOG_ERROR(...) P_LOG_ERROR(ST_PREFIX __VA_ARGS__)
#define D_LOG_WARNING(...) P_LOG_WARN(ST_PREFIX __VA_ARGS__)
#define D_LOG_INFO(...) P_LOG_INFO(ST_PREFIX __VA_ARGS__)
#define D_LOG_DEBUG(...) P_LOG_DEBUG(ST_PREFIX __VA_ARGS__)
#endif

std::shared_ptr<streamfx::gfx::opengl> streamfx::gfx::opengl::get()
{
	static std::weak_ptr<streamfx::gfx::opengl> instance;
	static std::mutex                           lock;

	std::unique_lock<std::mutex> ul(lock);
	if (instance.expired()) {
		auto hard_instance = std::shared_ptr<streamfx::gfx::opengl>(new streamfx::gfx::opengl());
		instance           = hard_instance;
		return hard_instance;
	}
	return instance.lock();
}

streamfx::gfx::opengl::opengl()
{
	int version = gladLoaderLoadGL();
#ifdef D_PLATFORM_WINDOWS
	// ToDo: Figure out the HDC for which we need to load.
	//gladLoaderLoadWGL();
#endif
#ifdef D_PLATFORM_LINUX
	//gladLoaderLoadGLX();
#endif // D_PLATFORM_LINUX
	D_LOG_INFO("Version %d.%d initialized.", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
}

streamfx::gfx::opengl::~opengl()
{
	gladLoaderUnloadGL();
#ifdef D_PLATFORM_WINDOWS
	// Does not appear to exist.
	//gladLoaderUnloadWGL();
#endif
#ifdef D_PLATFORM_LINUX
	//gladLoaderUnloadGLX();
#endif
	D_LOG_INFO("Finalized.", "");
}
