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

#pragma once
#include "obs/gs/gs-effect.hpp"

#include "warning-disable.hpp"
#include <memory>
#include "warning-enable.hpp"

namespace streamfx::gfx::lut {
	class data {
		std::shared_ptr<streamfx::obs::gs::effect> _producer_effect;
		std::shared_ptr<streamfx::obs::gs::effect> _consumer_effect;

		public:
		static std::shared_ptr<data> instance();

		private:
		data();

		public:
		~data();

		inline std::shared_ptr<streamfx::obs::gs::effect> producer_effect()
		{
			return _producer_effect;
		};

		inline std::shared_ptr<streamfx::obs::gs::effect> consumer_effect()
		{
			return _consumer_effect;
		};
	};

	enum class color_depth {
		Invalid = 0,
		_2      = 2,
		_4      = 4,
		_6      = 6,
		_8      = 8,
		_10     = 10,
		_12     = 12,
		_14     = 14,
		_16     = 16,
	};
} // namespace streamfx::gfx::lut
