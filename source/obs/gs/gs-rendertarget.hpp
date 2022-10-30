// Copyright (c) 2017-2022 Michael Fabian Dirks <info@xaymar.com>
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
#include "gs-texture.hpp"

namespace streamfx::obs::gs {
	class rendertarget_op;

	class rendertarget {
		friend class rendertarget_op;

		protected:
		gs_texrender_t* _render_target;
		bool            _is_being_rendered;

		gs_color_format    _color_format;
		gs_zstencil_format _zstencil_format;

		public:
		~rendertarget();

		rendertarget(gs_color_format colorFormat, gs_zstencil_format zsFormat);

		gs_texture_t* get_object();

		std::shared_ptr<streamfx::obs::gs::texture> get_texture();

		void get_texture(streamfx::obs::gs::texture& tex);

		void get_texture(std::shared_ptr<streamfx::obs::gs::texture>& tex);

		void get_texture(std::unique_ptr<streamfx::obs::gs::texture>& tex);

		gs_color_format get_color_format();

		gs_zstencil_format get_zstencil_format();

		streamfx::obs::gs::rendertarget_op render(uint32_t width, uint32_t height);

		streamfx::obs::gs::rendertarget_op render(uint32_t width, uint32_t height, gs_color_space cs);
	};

	class rendertarget_op {
		streamfx::obs::gs::rendertarget* parent;

		public:
		~rendertarget_op();

		rendertarget_op(streamfx::obs::gs::rendertarget* rt, uint32_t width, uint32_t height);

		rendertarget_op(streamfx::obs::gs::rendertarget* rt, uint32_t width, uint32_t height, gs_color_space cs);

		// Move Constructor
		rendertarget_op(streamfx::obs::gs::rendertarget_op&&) noexcept;

		// Copy Constructor
		rendertarget_op(const streamfx::obs::gs::rendertarget_op&) = delete;

		rendertarget_op& operator=(const streamfx::obs::gs::rendertarget_op& r) = delete;
	};
} // namespace streamfx::obs::gs
