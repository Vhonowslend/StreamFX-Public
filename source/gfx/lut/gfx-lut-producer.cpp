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

#include "gfx-lut-producer.hpp"

#include "obs/gs/gs-helper.hpp"

gs_color_format format_from_depth(gfx::lut::color_depth depth)
{
	switch (depth) {
	case gfx::lut::color_depth::_2:
	case gfx::lut::color_depth::_4:
	case gfx::lut::color_depth::_6:
	case gfx::lut::color_depth::_8:
		return gs_color_format::GS_RGBA;
	case gfx::lut::color_depth::_10:
		return gs_color_format::GS_R10G10B10A2;
	case gfx::lut::color_depth::_12:
	case gfx::lut::color_depth::_14:
	case gfx::lut::color_depth::_16:
		return gs_color_format::GS_RGBA16;
	}
	return GS_RGBA32F;
}

gfx::lut::producer::producer()
{
	_data = gfx::lut::data::instance();
	if (!_data->producer_effect())
		throw std::runtime_error("Unable to get LUT producer effect.");
}

gfx::lut::producer::~producer() {}

std::shared_ptr<streamfx::obs::gs::texture> gfx::lut::producer::produce(gfx::lut::color_depth depth)
{
	auto gctx = streamfx::obs::gs::context();

	if (!_rt || (_rt->get_color_format() != format_from_depth((depth)))) {
		_rt = std::make_shared<streamfx::obs::gs::rendertarget>(format_from_depth(depth), GS_ZS_NONE);
	}

	auto effect = _data->producer_effect();

	int32_t idepth         = static_cast<int32_t>(depth);
	int32_t size           = static_cast<int32_t>(pow(2l, idepth));
	int32_t grid_size      = static_cast<int32_t>(pow(2l, (idepth / 2)));
	int32_t container_size = static_cast<int32_t>(pow(2l, (idepth + (idepth / 2))));

	{
		auto op = _rt->render(static_cast<uint32_t>(container_size), static_cast<uint32_t>(container_size));

		gs_blend_state_push();
		gs_enable_color(true, true, true, false);
		gs_enable_blending(false);
		gs_enable_stencil_test(false);
		gs_enable_stencil_write(false);
		gs_ortho(0, 1, 0, 1, 0, 1);

		if (streamfx::obs::gs::effect_parameter efp = effect->get_parameter("lut_params_0"); efp) {
			efp.set_int4(size, grid_size, container_size, 0l);
		}

		while (gs_effect_loop(effect->get_object(), "Draw")) {
			streamfx::gs_draw_fullscreen_tri();
		}

		gs_enable_color(true, true, true, true);
		gs_blend_state_pop();
	}

	return _rt->get_texture();
}
