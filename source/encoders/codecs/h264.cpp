// FFMPEG Video Encoder Integration for OBS Studio
// Copyright (c) 2019 Michael Fabian Dirks <info@xaymar.com>
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

#include "h264.hpp"

uint8_t* is_nal_start(uint8_t* ptr, uint8_t* end_ptr, size_t& size)
{
	// Ensure that the remaining space actually can contain a prefix and NAL header.
	if ((ptr + (3 + 1)) >= end_ptr)
		return nullptr;

	if (*ptr != 0x0)
		return nullptr;
	if (*(ptr + 1) != 0x0)
		return nullptr;

	// 3-Byte NAL prefix.
	if (*(ptr + 2) == 0x1) {
		size = 3;
		return ptr + 3;
	}

	// 4-Byte NAL Prefix
	if ((ptr + (4 + 1)) >= end_ptr)
		return nullptr;
	if (*(ptr + 2) != 0x0)
		return nullptr;
	if (*(ptr + 3) != 0x01)
		return nullptr;

	size = 4;
	return ptr + 4;
}

uint8_t* streamfx::encoder::codec::h264::find_closest_nal(uint8_t* ptr, uint8_t* end_ptr, size_t& size)
{
	for (uint8_t* seek_ptr = ptr; seek_ptr < end_ptr; seek_ptr++) {
		if (auto nal_ptr = is_nal_start(seek_ptr, end_ptr, size); nal_ptr != nullptr)
			return nal_ptr;
	}
	return nullptr;
}

uint32_t streamfx::encoder::codec::h264::get_packet_reference_count(uint8_t* ptr, uint8_t* end_ptr)
{
	size_t   nal_ptr_prefix = 0;
	uint8_t* nal_ptr        = find_closest_nal(ptr, end_ptr, nal_ptr_prefix);
	while ((nal_ptr != nullptr) && (nal_ptr < end_ptr)) {
		// Try and figure out the actual size of the NAL.
		size_t   nal_end_ptr_prefix = 0;
		uint8_t* nal_end_ptr        = find_closest_nal(nal_ptr, end_ptr, nal_end_ptr_prefix);
		size_t   nal_size           = (nal_end_ptr ? nal_end_ptr : end_ptr) - nal_ptr - nal_end_ptr_prefix;

		// Try and figure out the ideal priority.
		switch (static_cast<nal_unit_type>((*nal_ptr) & 0x5)) {
		case nal_unit_type::CODED_SLICE_NONIDR:
			return static_cast<uint32_t>((*nal_ptr >> 5) & 0x2);
		case nal_unit_type::CODED_SLICE_IDR:
			return static_cast<uint32_t>((*nal_ptr >> 5) & 0x2);
		default:
			break;
		}

		// Update our NAL pointer.
		nal_ptr = nal_end_ptr;
	}

	return std::numeric_limits<uint32_t>::max();
}
