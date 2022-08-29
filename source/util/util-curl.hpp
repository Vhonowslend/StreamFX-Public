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

#pragma once
#include "warning-disable.hpp"
#include <cinttypes>
#include <cstring>
#include <functional>
#include <map>
#include <string>
#include <vector>
#include "warning-enable.hpp"

extern "C" {
#include "warning-disable.hpp"
#include <curl/curl.h>
#include "warning-enable.hpp"
}

namespace streamfx::util {
	typedef std::function<size_t(void*, size_t, size_t)>                   curl_io_callback_t;
	typedef std::function<int32_t(uint64_t, uint64_t, uint64_t, uint64_t)> curl_xferinfo_callback_t;
	typedef std::function<void(CURL*, curl_infotype, char*, size_t)>       curl_debug_callback_t;

	class curl {
		CURL*                              _curl;
		curl_io_callback_t                 _read_callback;
		curl_io_callback_t                 _write_callback;
		curl_xferinfo_callback_t           _xferinfo_callback;
		curl_debug_callback_t              _debug_callback;
		std::map<std::string, std::string> _headers;

		static int32_t debug_helper(CURL* handle, curl_infotype type, char* data, size_t size,
									streamfx::util::curl* userptr);
		static size_t  read_helper(void*, size_t, size_t, streamfx::util::curl*);
		static size_t  write_helper(void*, size_t, size_t, streamfx::util::curl*);
		static int32_t xferinfo_callback(streamfx::util::curl*, curl_off_t, curl_off_t, curl_off_t, curl_off_t);

		public:
		curl();
		~curl();

		template<typename _Ty1>
		CURLcode set_option(CURLoption opt, _Ty1 value)
		{
			return curl_easy_setopt(_curl, opt, value);
		};

		CURLcode set_option(CURLoption opt, const bool value)
		{
			// CURL does not seem to accept boolean, so we err on the side of safety here.
			return curl_easy_setopt(_curl, opt, value ? 1 : 0);
		};

		CURLcode set_option(CURLoption opt, const std::string value)
		{
			return curl_easy_setopt(_curl, opt, value.c_str());
		};

		CURLcode set_option(CURLoption opt, const std::string_view value)
		{
			return curl_easy_setopt(_curl, opt, value.data());
		};

		template<typename _Ty1>
		CURLcode get_info(CURLINFO info, _Ty1& value)
		{
			return curl_easy_getinfo(_curl, info, &value);
		};

		CURLcode get_info(CURLINFO info, std::vector<char>& value)
		{
			char* buffer;
			if (CURLcode res = curl_easy_getinfo(_curl, info, &buffer); res != CURLE_OK) {
				return res;
			}

			size_t buffer_len = strnlen(buffer, size_t(0xFFFF));
			value.resize(buffer_len);

			memcpy(value.data(), buffer, value.size());
			return CURLE_OK;
		};

		CURLcode get_info(CURLINFO info, std::string& value)
		{
			std::vector<char> buffer;
			if (CURLcode res = get_info(info, buffer); res != CURLE_OK) {
				return res;
			}
			value = std::string(buffer.data(), buffer.data() + strlen(buffer.data()));
			return CURLE_OK;
		};

		void clear_headers();

		void clear_header(std::string_view header);

		void set_header(std::string header, std::string value);

		CURLcode perform();

		void reset();

		public /* Helpers */:
		CURLcode set_read_callback(curl_io_callback_t cb);

		CURLcode set_write_callback(curl_io_callback_t cb);

		CURLcode set_xferinfo_callback(curl_xferinfo_callback_t cb);

		CURLcode set_debug_callback(curl_debug_callback_t cb);
	};
} // namespace streamfx::util
