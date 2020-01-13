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

#pragma once
#include <vector>

// Codec: HEVC
#define P_HEVC "Codec.HEVC"
#define P_HEVC_PROFILE "Codec.HEVC.Profile"
#define P_HEVC_TIER "Codec.HEVC.Tier"
#define P_HEVC_LEVEL "Codec.HEVC.Level"

namespace obsffmpeg {
	namespace codecs {
		namespace hevc {
			enum class profile {
				MAIN,
				MAIN10,
				RANGE_EXTENDED,
				UNKNOWN = -1,
			};

			enum class tier {
				MAIN,
				HIGH,
				UNKNOWN = -1,
			};

			enum class level {
				L1_0    = 30,
				L2_0    = 60,
				L2_1    = 63,
				L3_0    = 90,
				L3_1    = 93,
				L4_0    = 120,
				L4_1    = 123,
				L5_0    = 150,
				L5_1    = 153,
				L5_2    = 156,
				L6_0    = 180,
				L6_1    = 183,
				L6_2    = 186,
				UNKNOWN = -1,
			};

			void extract_header_sei(uint8_t* data, size_t sz_data, std::vector<uint8_t>& header,
			                        std::vector<uint8_t>& sei);

		} // namespace hevc
	}         // namespace codecs
} // namespace obsffmpeg
