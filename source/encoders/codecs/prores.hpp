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

// Codec: ProRes
#define P_PRORES "Codec.ProRes"
#define P_PRORES_PROFILE "Codec.ProRes.Profile"
#define P_PRORES_PROFILE_APCS "Codec.ProRes.Profile.APCS"
#define P_PRORES_PROFILE_APCO "Codec.ProRes.Profile.APCO"
#define P_PRORES_PROFILE_APCN "Codec.ProRes.Profile.APCN"
#define P_PRORES_PROFILE_APCH "Codec.ProRes.Profile.APCH"
#define P_PRORES_PROFILE_AP4H "Codec.ProRes.Profile.AP4H"
#define P_PRORES_PROFILE_AP4X "Codec.ProRes.Profile.AP4X"

namespace encoder::codec::prores {
	enum class profile : std::int32_t {
		APCO = 0,
		Y422_PROXY = APCO,
		APCS = 1,
		Y422_LT = APCS,
		APCN = 2,
		Y422 = APCN,
		APCH = 3,
		Y422_HQ = APCH,
		AP4H = 4,
		Y4444 = AP4H,
		AP4X = 5,
		Y4444_XQ = AP4X,
		_COUNT,
	};
}
