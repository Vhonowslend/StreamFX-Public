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

#include "nvidia-ar.hpp"
#include <filesystem>
#include <mutex>
#include "nvidia/cuda/nvidia-cuda-obs.hpp"
#include "obs/gs/gs-helper.hpp"
#include "util/util-logging.hpp"
#include "util/util-platform.hpp"

#ifdef _DEBUG
#define ST_PREFIX "<%s> "
#define D_LOG_ERROR(x, ...) P_LOG_ERROR(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_WARNING(x, ...) P_LOG_WARN(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_INFO(x, ...) P_LOG_INFO(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_DEBUG(x, ...) P_LOG_DEBUG(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#else
#define ST_PREFIX "<nvidia::ar::ar> "
#define D_LOG_ERROR(...) P_LOG_ERROR(ST_PREFIX __VA_ARGS__)
#define D_LOG_WARNING(...) P_LOG_WARN(ST_PREFIX __VA_ARGS__)
#define D_LOG_INFO(...) P_LOG_INFO(ST_PREFIX __VA_ARGS__)
#define D_LOG_DEBUG(...) P_LOG_DEBUG(ST_PREFIX __VA_ARGS__)
#endif

#ifdef WIN32
#include <KnownFolders.h>
#include <ShlObj.h>
#include <Windows.h>

#define ST_LIBRARY_NAME "nvARPose.dll"
#else
#define ST_LIBRARY_NAME "libnvARPose.so"
#endif

#define P_NVAR_LOAD_SYMBOL(NAME)                                                                \
	{                                                                                           \
		NAME = reinterpret_cast<decltype(NAME)>(_library->load_symbol(#NAME));                  \
		if (!NAME)                                                                              \
			throw std::runtime_error("Failed to load '" #NAME "' from '" ST_LIBRARY_NAME "'."); \
	}

streamfx::nvidia::ar::ar::~ar()
{
	D_LOG_DEBUG("Finalizing... (Addr: 0x%" PRIuPTR ")", this);
}

streamfx::nvidia::ar::ar::ar() : _library(), _model_path()
{
	std::filesystem::path sdk_path;
	auto                  gctx = ::streamfx::obs::gs::context();
	auto                  cctx = ::streamfx::nvidia::cuda::obs::get()->get_context()->enter();

	D_LOG_DEBUG("Initializating... (Addr: 0x%" PRIuPTR ")", this);

	// Figure out where the Augmented Reality SDK is, if it is installed.
#ifdef WIN32
	{
		// NVAR SDK only defines NVAR_MODEL_PATH, so we'll use that as our baseline.
		DWORD env_size = GetEnvironmentVariableW(L"NVAR_MODEL_PATH", nullptr, 0);
		if (env_size > 0) {
			std::vector<wchar_t> buffer(env_size + 1, 0);
			env_size    = GetEnvironmentVariableW(L"NVAR_MODEL_PATH", buffer.data(), buffer.size());
			_model_path = std::wstring(buffer.data(), buffer.size());
		}

		// If the environment variable wasn't set and our model path is still undefined, guess!
		if (_model_path.empty()) {
			PWSTR   str = nullptr;
			HRESULT res = SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, nullptr, &str);
			if (res == S_OK) {
				_model_path = std::wstring(str);
				_model_path /= "NVIDIA Corporation";
				_model_path /= "NVIDIA AR SDK";
				_model_path /= "models";
				CoTaskMemFree(str);
			}
		}

		// The SDK is location one directory "up" from the model path.
		sdk_path = std::filesystem::absolute(std::filesystem::path(_model_path) / "..");
	}
#else
	throw std::runtime_error("Not yet implemented.");
#endif

	// Check if any of the found paths are valid.
	if (!std::filesystem::exists(sdk_path)) {
		D_LOG_ERROR("No supported NVIDIA SDK is installed to provide '%s'.", ST_LIBRARY_NAME);
		throw std::runtime_error("Failed to load '" ST_LIBRARY_NAME "'.");
	}

#ifdef WIN32
	// On platforms where it is possible, modify the linker directories.
	//SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
	DLL_DIRECTORY_COOKIE ck = AddDllDirectory(sdk_path.wstring().c_str());
#endif

	// Try and load the libraries
	if (!_library) {
		// Load it by name.
		try {
			_library = ::streamfx::util::library::load(std::string_view(ST_LIBRARY_NAME));
		} catch (...) {
			// Load it by path.
			auto lib_path = sdk_path;
			lib_path /= ST_LIBRARY_NAME;
			try {
				_library = ::streamfx::util::library::load(::streamfx::util::platform::native_to_utf8(lib_path));
			} catch (std::exception const& ex) {
				D_LOG_ERROR("Failed to load '%s' from '%s' with error: %s", ST_LIBRARY_NAME,
							util::platform::native_to_utf8(lib_path).string().c_str(), ex.what());
				throw std::runtime_error("Failed to load '" ST_LIBRARY_NAME "'.");
			} catch (...) {
				D_LOG_ERROR("Failed to load '%s' from '%s'.", ST_LIBRARY_NAME,
							util::platform::native_to_utf8(lib_path).string().c_str());
				throw std::runtime_error("Failed to load '" ST_LIBRARY_NAME "'.");
			}
		}
	}

	{ // Load Symbols
		P_NVAR_LOAD_SYMBOL(NvAR_GetVersion);
		P_NVAR_LOAD_SYMBOL(NvAR_Create);
		P_NVAR_LOAD_SYMBOL(NvAR_Destroy);
		P_NVAR_LOAD_SYMBOL(NvAR_Run);
		P_NVAR_LOAD_SYMBOL(NvAR_Load);
		P_NVAR_LOAD_SYMBOL(NvAR_GetS32);
		P_NVAR_LOAD_SYMBOL(NvAR_SetS32);
		P_NVAR_LOAD_SYMBOL(NvAR_GetU32);
		P_NVAR_LOAD_SYMBOL(NvAR_SetU32);
		P_NVAR_LOAD_SYMBOL(NvAR_GetU64);
		P_NVAR_LOAD_SYMBOL(NvAR_SetU64);
		P_NVAR_LOAD_SYMBOL(NvAR_GetF32);
		P_NVAR_LOAD_SYMBOL(NvAR_SetF32);
		P_NVAR_LOAD_SYMBOL(NvAR_GetF64);
		P_NVAR_LOAD_SYMBOL(NvAR_SetF64);
		P_NVAR_LOAD_SYMBOL(NvAR_GetString);
		P_NVAR_LOAD_SYMBOL(NvAR_SetString);
		P_NVAR_LOAD_SYMBOL(NvAR_GetCudaStream);
		P_NVAR_LOAD_SYMBOL(NvAR_SetCudaStream);
		P_NVAR_LOAD_SYMBOL(NvAR_GetObject);
		P_NVAR_LOAD_SYMBOL(NvAR_SetObject);
		P_NVAR_LOAD_SYMBOL(NvAR_GetF32Array);
		P_NVAR_LOAD_SYMBOL(NvAR_SetF32Array);
	}

	{ // Assign proper GPU.
		auto cctx = ::streamfx::nvidia::cuda::obs::get()->get_context()->enter();
		NvAR_SetU32(nullptr, P_NVAR_CONFIG "GPU", 0);
	}
}

std::filesystem::path streamfx::nvidia::ar::ar::get_model_path()
{
	return _model_path;
}

std::shared_ptr<streamfx::nvidia::ar::ar> streamfx::nvidia::ar::ar::get()
{
	static std::shared_ptr<streamfx::nvidia::ar::ar> instance;
	static std::mutex                                lock;

	std::unique_lock<std::mutex> ul(lock);
	if (!instance) {
		instance = std::make_shared<streamfx::nvidia::ar::ar>();
	}
	return instance;
}
