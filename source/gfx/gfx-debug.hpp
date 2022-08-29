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

#include "obs/gs/gs-effect.hpp"
#include "obs/gs/gs-vertexbuffer.hpp"

#include "warning-disable.hpp"
#include <memory>
#include "warning-enable.hpp"

namespace streamfx::gfx {
	class debug {
		std::shared_ptr<::streamfx::obs::gs::effect>        _effect;
		std::shared_ptr<::streamfx::obs::gs::vertex_buffer> _point_vb;
		std::shared_ptr<::streamfx::obs::gs::vertex_buffer> _line_vb;
		std::shared_ptr<::streamfx::obs::gs::vertex_buffer> _arrow_vb;
		std::shared_ptr<::streamfx::obs::gs::vertex_buffer> _quad_vb;

		public /* Singleton */:
		static std::shared_ptr<streamfx::gfx::debug> get();

		private:
		debug();

		public:
		~debug();

		void draw_point(float x, float y, uint32_t color = 0xFFFFFFFF);

		void draw_line(float x, float y, float x2, float y2, uint32_t color = 0xFFFFFFFF);

		void draw_arrow(float x, float y, float x2, float y2, float w = 0., uint32_t color = 0xFFFFFFFF);

		void draw_rectangle(float x, float y, float w, float h, bool frame, uint32_t color = 0xFFFFFFFF);
	};
} // namespace streamfx::gfx
