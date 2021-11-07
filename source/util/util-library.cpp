// Copyright (c) 2020 Michael Fabian Dirks <info@xaymar.com>
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

#include "util-library.hpp"
#include <unordered_map>
#include "util-platform.hpp"

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__) // Windows
#define ST_WINDOWS
#else
#define ST_UNIX
#endif

#if defined(ST_WINDOWS)
#include <Windows.h>
#elif defined(ST_UNIX)
#include <dlfcn.h>
#endif

streamfx::util::library::library(std::filesystem::path file) : _library(nullptr)
{
#if defined(ST_WINDOWS)
	SetLastError(ERROR_SUCCESS);
	auto wfile = ::streamfx::util::platform::utf8_to_native(file.u8string());
	if (file.is_absolute()) {
		_library = reinterpret_cast<void*>(LoadLibraryExW(wfile.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR));
	} else {
		_library = reinterpret_cast<void*>(LoadLibraryExW(wfile.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS));
	}
	if (!_library) {
		DWORD error = GetLastError();
		if (error != ERROR_PROC_NOT_FOUND) {
			PSTR        message = NULL;
			std::string ex      = "Failed to load library.";
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
						   NULL, error, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPSTR)&message, 0, NULL);
			if (message) {
				ex = message;
				LocalFree(message);
				throw std::runtime_error(ex);
			}
		}
		throw std::runtime_error("Failed to load library.");
	}
#elif defined(ST_UNIX)
	_library = dlopen(file.u8string().c_str(), RTLD_LAZY);
	if (!_library) {
		if (char* error = dlerror(); error)
			throw std::runtime_error(error);
		else
			throw std::runtime_error("Failed to load library.");
	}
#endif
}

streamfx::util::library::~library()
{
#if defined(ST_WINDOWS)
	FreeLibrary(reinterpret_cast<HMODULE>(_library));
#elif defined(ST_UNIX)
	dlclose(_library);
#endif
}

void* streamfx::util::library::load_symbol(std::string_view name)
{
#if defined(ST_WINDOWS)
	return reinterpret_cast<void*>(GetProcAddress(reinterpret_cast<HMODULE>(_library), name.data()));
#elif defined(ST_UNIX)
	return reinterpret_cast<void*>(dlsym(_library, name.data()));
#endif
}

static std::unordered_map<std::string, std::weak_ptr<::streamfx::util::library>> libraries;

std::shared_ptr<::streamfx::util::library> streamfx::util::library::load(std::filesystem::path file)
{
	auto kv = libraries.find(file.u8string());
	if (kv != libraries.end()) {
		if (auto ptr = kv->second.lock(); ptr)
			return ptr;
		libraries.erase(kv);
	}

	auto ptr = std::make_shared<::streamfx::util::library>(file);
	libraries.emplace(file.u8string(), ptr);

	return ptr;
}

std::shared_ptr<::streamfx::util::library> streamfx::util::library::load(std::string_view name)
{
	return load(std::filesystem::u8path(name));
}
