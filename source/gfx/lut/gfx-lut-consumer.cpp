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

#include "gfx-lut-consumer.hpp"

#include "obs/gs/gs-helper.hpp"

gfx::lut::consumer::consumer()
{
	_data = gfx::lut::data::instance();
	if (!_data->consumer_effect())
		throw std::runtime_error("Unable to get LUT consumer effect.");
}

gfx::lut::consumer::~consumer() {}

std::shared_ptr<gs::effect> gfx::lut::consumer::prepare(gfx::lut::color_depth depth, std::shared_ptr<gs::texture> lut)
{
	auto gctx = gs::context();

	auto effect = _data->consumer_effect();

	int32_t idepth         = static_cast<int32_t>(depth);
	int32_t size           = static_cast<int32_t>(pow(2l, idepth));
	int32_t grid_size      = static_cast<int32_t>(pow(2l, (idepth / 2)));
	int32_t container_size = static_cast<int32_t>(pow(2l, (idepth + (idepth / 2))));

	if (gs::effect_parameter efp = effect->get_parameter("lut_params_0"); efp) {
		efp.set_int4(size, grid_size, container_size, 0l);
	}

	if (gs::effect_parameter efp = effect->get_parameter("lut_params_1"); efp) {
		float inverse_size           = 1.f / static_cast<float>(size);
		float inverse_z_size         = 1.f / static_cast<float>(grid_size);
		float inverse_container_size = 1.f / static_cast<float>(container_size);
		float half_texel             = inverse_container_size / 2.f;
		efp.set_float4(inverse_size, inverse_z_size, inverse_container_size, half_texel);
	}

	if (gs::effect_parameter efp = effect->get_parameter("lut"); efp) {
		efp.set_texture(lut);
	}

	return effect;
}

void gfx::lut::consumer::consume(gfx::lut::color_depth depth, std::shared_ptr<gs::texture> lut,
								 std::shared_ptr<gs::texture> texture)
{
	auto gctx = gs::context();

	auto effect = prepare(depth, lut);

	if (gs::effect_parameter efp = effect->get_parameter("image"); efp) {
		efp.set_texture(texture->get_object());
	}

	// Draw a simple quad.
	while (gs_effect_loop(effect->get_object(), "Draw")) {
		gs_draw_sprite(nullptr, 0, 1, 1);
	}
}
