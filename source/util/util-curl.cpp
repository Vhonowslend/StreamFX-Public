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

#include "util-curl.hpp"
#include <sstream>

int32_t util::curl::debug_helper(CURL* handle, curl_infotype type, char* data, size_t size, util::curl* self)
{
	if (self->_debug_callback) {
		self->_debug_callback(handle, type, data, size);
	} else {
#ifdef _DEBUG_CURL
		std::stringstream hd;
		for (size_t n = 0; n < size; n++) {
			hd << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << static_cast<int32_t>(data[n])
			   << " ";
			if (n % 16 == 15) {
				hd << "\n  ";
			}
		}
		std::string hds = hd.str();

		switch (type) {
		case CURLINFO_TEXT:
			DLOG_DEBUG("<CURL> %.*s", size - 1, data);
			break;
		case CURLINFO_HEADER_IN:
			DLOG_DEBUG("<CURL> << Header: %.*s", size - 1, data);
			break;
		case CURLINFO_HEADER_OUT:
			DLOG_DEBUG("<CURL> >> Header: %.*s", size - 1, data);
			break;
		case CURLINFO_DATA_IN:
			DLOG_DEBUG("<CURL> << %lld bytes of data:\n  %s", size, hds.c_str());
			break;
		case CURLINFO_DATA_OUT:
			DLOG_DEBUG("<CURL> >> %lld bytes of data:\n  %s", size, hds.c_str());
			break;
		case CURLINFO_SSL_DATA_IN:
			DLOG_DEBUG("<CURL> << %lld bytes of SSL data:\n  %s", size, hds.c_str());
			break;
		case CURLINFO_SSL_DATA_OUT:
			DLOG_DEBUG("<CURL> >> %lld bytes of SSL data:\n  %s", size, hds.c_str());
			break;
		}
#endif
	}
	return 0;
}

size_t util::curl::read_helper(void* ptr, size_t size, size_t count, util::curl* self)
{
	if (self->_read_callback) {
		return self->_read_callback(ptr, size, count);
	} else {
		return size * count;
	}
}

size_t util::curl::write_helper(void* ptr, size_t size, size_t count, util::curl* self)
{
	if (self->_write_callback) {
		return self->_write_callback(ptr, size, count);
	} else {
		return size * count;
	}
}

int32_t util::curl::xferinfo_callback(util::curl* self, curl_off_t dlt, curl_off_t dln, curl_off_t ult, curl_off_t uln)
{
	if (self->_xferinfo_callback) {
		return self->_xferinfo_callback(static_cast<uint64_t>(dlt), static_cast<uint64_t>(dln),
										static_cast<uint64_t>(ult), static_cast<uint64_t>(uln));
	} else {
		return 0;
	}
}

util::curl::curl() : _curl(), _read_callback(), _write_callback(), _headers()
{
	_curl = curl_easy_init();
	set_read_callback(nullptr);
	set_write_callback(nullptr);
	set_xferinfo_callback(nullptr);
	set_debug_callback(nullptr);

	// Default settings.
	set_option(CURLOPT_NOPROGRESS, false);
	set_option(CURLOPT_PATH_AS_IS, false);
	set_option(CURLOPT_CRLF, false);
#ifdef _DEBUG
	set_option(CURLOPT_VERBOSE, true);
#else
	set_option(CURLOPT_VERBOSE, false);
#endif
}

util::curl::~curl()
{
	curl_easy_cleanup(_curl);
}

void util::curl::clear_headers()
{
	_headers.clear();
}

void util::curl::clear_header(std::string header)
{
	_headers.erase(header);
}

void util::curl::set_header(std::string header, std::string value)
{
	_headers.insert_or_assign(header, value);
}

size_t perform_get_kv_size(std::string a, std::string b)
{
	return a.size() + 2 + b.size() + 1;
};

CURLcode util::curl::perform()
{
	std::vector<char>  buffer;
	struct curl_slist* headers = nullptr;

	if (_headers.size() > 0) {
		// Calculate full buffer size.
		{
			size_t buffer_size = 0;
			for (auto kv : _headers) {
				buffer_size += perform_get_kv_size(kv.first, kv.second);
			}
			buffer.resize(buffer_size * 2);
		}
		// Create HTTP Headers.
		{
			size_t buffer_offset = 0;
			for (auto kv : _headers) {
				size_t size = perform_get_kv_size(kv.first, kv.second);

				snprintf(&buffer.at(buffer_offset), size, "%s: %s", kv.first.c_str(), kv.second.c_str());

				headers = curl_slist_append(headers, &buffer.at(buffer_offset));

				buffer_offset += size;
			}
		}
		set_option<struct curl_slist*>(CURLOPT_HTTPHEADER, headers);
	}

	CURLcode res = curl_easy_perform(_curl);

	if (headers) {
		set_option<struct curl_slist*>(CURLOPT_HTTPHEADER, nullptr);
		curl_slist_free_all(headers);
	}

	return res;
}

void util::curl::reset()
{
	curl_easy_reset(_curl);
}

CURLcode util::curl::set_read_callback(curl_io_callback_t cb)
{
	_read_callback = cb;
	if (CURLcode res = curl_easy_setopt(_curl, CURLOPT_READDATA, this); res != CURLE_OK)
		return res;
	return curl_easy_setopt(_curl, CURLOPT_READFUNCTION, &read_helper);
}

CURLcode util::curl::set_write_callback(curl_io_callback_t cb)
{
	_write_callback = cb;
	if (CURLcode res = curl_easy_setopt(_curl, CURLOPT_WRITEDATA, this); res != CURLE_OK)
		return res;
	return curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, &write_helper);
}

CURLcode util::curl::set_xferinfo_callback(curl_xferinfo_callback_t cb)
{
	_xferinfo_callback = cb;
	if (CURLcode res = curl_easy_setopt(_curl, CURLOPT_XFERINFODATA, this); res != CURLE_OK)
		return res;
	return curl_easy_setopt(_curl, CURLOPT_XFERINFOFUNCTION, &xferinfo_callback);
}

CURLcode util::curl::set_debug_callback(curl_debug_callback_t cb)
{
	_debug_callback = cb;
	if (CURLcode res = curl_easy_setopt(_curl, CURLOPT_DEBUGDATA, this); res != CURLE_OK)
		return res;
	return curl_easy_setopt(_curl, CURLOPT_DEBUGFUNCTION, &debug_helper);
}
