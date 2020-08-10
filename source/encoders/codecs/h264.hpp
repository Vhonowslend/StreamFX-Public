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
#include "common.hpp"

// Codec: H264
#define P_H264 "Codec.H264"
#define P_H264_PROFILE "Codec.H264.Profile"
#define P_H264_LEVEL "Codec.H264.Level"

namespace streamfx::encoder::codec::h264 {
	enum class profile {
		CONSTRAINED_BASELINE,
		BASELINE,
		MAIN,
		HIGH,
		HIGH444_PREDICTIVE,
		UNKNOWN = -1,
	};

	enum class level {
		L1_0 = 10,
		L1_0b,
		L1_1,
		L1_2,
		L1_3,
		L2_0 = 20,
		L2_1,
		L2_2,
		L3_0 = 30,
		L3_1,
		L3_2,
		L4_0 = 40,
		L4_1,
		L4_2,
		L5_0 = 50,
		L5_1,
		L5_2,
		L6_0 = 60,
		L6_1,
		L6_2,
		UNKNOWN = -1,
	};
} // namespace streamfx::encoder::codec::h264
