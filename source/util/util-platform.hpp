// Copyright 2020 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once
#include <filesystem>
#include <string>

namespace streamfx::util::platform {
#ifdef WIN32
	std::string           native_to_utf8(std::wstring const& v);
	std::filesystem::path native_to_utf8(std::filesystem::path const& v);

	std::wstring          utf8_to_native(std::string const& v);
	std::filesystem::path utf8_to_native(std::filesystem::path const& v);
#else
	inline std::string native_to_utf8(std::string const& v)
	{
		return std::string(v);
	};
	inline std::filesystem::path native_to_utf8(std::filesystem::path const& v)
	{
		return std::filesystem::path(v);
	};

	inline std::string utf8_to_native(std::string const& v)
	{
		return std::string(v);
	};
	inline std::filesystem::path utf8_to_native(std::filesystem::path const& v)
	{
		return std::filesystem::path(v);
	};
#endif
} // namespace streamfx::util::platform
