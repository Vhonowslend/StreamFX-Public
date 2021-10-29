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

#include "gfx-debug.hpp"
#include <mutex>
#include "graphics/matrix4.h"
#include "obs/gs/gs-helper.hpp"
#include "plugin.hpp"
#include "util/util-logging.hpp"

#ifdef _DEBUG
#define ST_PREFIX "<%s> "
#define D_LOG_ERROR(x, ...) P_LOG_ERROR(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_WARNING(x, ...) P_LOG_WARN(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_INFO(x, ...) P_LOG_INFO(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#define D_LOG_DEBUG(x, ...) P_LOG_DEBUG(ST_PREFIX##x, __FUNCTION_SIG__, __VA_ARGS__)
#else
#define ST_PREFIX "<gfx::debug> "
#define D_LOG_ERROR(...) P_LOG_ERROR(ST_PREFIX __VA_ARGS__)
#define D_LOG_WARNING(...) P_LOG_WARN(ST_PREFIX __VA_ARGS__)
#define D_LOG_INFO(...) P_LOG_INFO(ST_PREFIX __VA_ARGS__)
#define D_LOG_DEBUG(...) P_LOG_DEBUG(ST_PREFIX __VA_ARGS__)
#endif

std::shared_ptr<streamfx::gfx::debug> streamfx::gfx::debug::get()
{
	static std::weak_ptr<streamfx::gfx::debug> instance;
	static std::mutex                          lock;

	std::unique_lock<std::mutex> ul(lock);
	if (instance.expired()) {
		auto hard_instance = std::shared_ptr<streamfx::gfx::debug>(new streamfx::gfx::debug());
		instance           = hard_instance;
		return hard_instance;
	}
	return instance.lock();
}

streamfx::gfx::debug::debug()
{
	{
		std::filesystem::path file = ::streamfx::data_file_path("effects/standard.effect");
		try {
			_effect = std::make_shared<::streamfx::obs::gs::effect>(file);
		} catch (...) {
			D_LOG_ERROR("Failed to load '%s'.", file.generic_u8string().c_str());
		}
	}
}

streamfx::gfx::debug::~debug()
{
	obs::gs::context gctx{};

	_point_vb.reset();
	_line_vb.reset();
	_arrow_vb.reset();
	_quad_vb.reset();
}

void streamfx::gfx::debug::draw_point(float x, float y, uint32_t color)
{
	obs::gs::context gctx{};

	if (!_point_vb) {
		_point_vb = std::make_shared<obs::gs::vertex_buffer>(1u, 1u);
	}

	{
		auto vtx = _point_vb->at(0);
		vec3_set(vtx.position, x, y, 0.);
		*vtx.color = color;
	}

	gs_load_indexbuffer(nullptr);
	gs_load_vertexbuffer(_point_vb->update(true));
	while (gs_effect_loop(_effect->get_object(), "Color")) {
		gs_draw(GS_POINTS, 0, 1);
	}
	gs_load_vertexbuffer(nullptr);
}

void streamfx::gfx::debug::draw_line(float x, float y, float x2, float y2, uint32_t color /*= 0xFFFFFFFF*/)
{
	obs::gs::context gctx{};

	if (!_line_vb) {
		_line_vb = std::make_shared<obs::gs::vertex_buffer>(2u, 1u);
	}

	{
		auto vtx = _line_vb->at(0);
		vec3_set(vtx.position, x, y, 0.);
		*vtx.color = color;
	}
	{
		auto vtx = _line_vb->at(1);
		vec3_set(vtx.position, x2, y2, 0.);
		*vtx.color = color;
	}

	gs_load_indexbuffer(nullptr);
	gs_load_vertexbuffer(_line_vb->update(true));
	while (gs_effect_loop(_effect->get_object(), "Color")) {
		gs_draw(GS_LINES, 0, 2);
	}
	gs_load_vertexbuffer(nullptr);
}

void streamfx::gfx::debug::draw_arrow(float x, float y, float x2, float y2, float w /*= 0.*/,
									  uint32_t color /*= 0xFFFFFFFF*/)
{
	obs::gs::context gctx{};

	if (!_arrow_vb) {
		_arrow_vb = std::make_shared<obs::gs::vertex_buffer>(5u, 1u);
	}

	float dx  = x2 - x;
	float dy  = y2 - y;
	float ang = atan2(-dx, dy);
	float len = sqrt(dx * dx + dy * dy);

	if (abs(w) <= 1) {
		w = len / 3.;
	}

	matrix4 rotator;
	matrix4_identity(&rotator);
	matrix4_rotate_aa4f(&rotator, &rotator, 0, 0, 1, ang);
	vec3 offset;
	vec3_set(&offset, x, y, 0.);

	{
		auto vtx = _arrow_vb->at(0);
		vec3_set(vtx.position, 0, 0, 0.);
		vec3_transform(vtx.position, vtx.position, &rotator);
		vec3_add(vtx.position, vtx.position, &offset);
		*vtx.color = color;
	}
	{
		auto vtx = _arrow_vb->at(1);
		vec3_set(vtx.position, 0, len, 0.);
		vec3_transform(vtx.position, vtx.position, &rotator);
		vec3_add(vtx.position, vtx.position, &offset);
		*vtx.color = color;
	}
	{
		auto vtx = _arrow_vb->at(2);
		vec3_set(vtx.position, -w, len - w, 0.);
		vec3_transform(vtx.position, vtx.position, &rotator);
		vec3_add(vtx.position, vtx.position, &offset);
		*vtx.color = color;
	}
	{
		auto vtx = _arrow_vb->at(3);
		vec3_set(vtx.position, w, len - w, 0.);
		vec3_transform(vtx.position, vtx.position, &rotator);
		vec3_add(vtx.position, vtx.position, &offset);
		*vtx.color = color;
	}
	{
		auto vtx = _arrow_vb->at(4);
		vec3_set(vtx.position, 0, len, 0.);
		vec3_transform(vtx.position, vtx.position, &rotator);
		vec3_add(vtx.position, vtx.position, &offset);
		*vtx.color = color;
	}

	gs_load_indexbuffer(nullptr);
	gs_load_vertexbuffer(_arrow_vb->update(true));
	while (gs_effect_loop(_effect->get_object(), "Color")) {
		gs_draw(GS_LINESTRIP, 0, 5);
	}
	gs_load_vertexbuffer(nullptr);
}

void streamfx::gfx::debug::draw_rectangle(float x, float y, float w, float h, bool frame,
										  uint32_t color /*= 0xFFFFFFFF*/)
{
	obs::gs::context gctx{};

	if (!_quad_vb) {
		_quad_vb = std::make_shared<obs::gs::vertex_buffer>(5u, 1u);
	}

	if (frame) {
		{
			auto vtx = _quad_vb->at(0);
			vec3_set(vtx.position, x, y, 0.);
			*vtx.color = color;
		}
		{
			auto vtx = _quad_vb->at(1);
			vec3_set(vtx.position, x + w, y, 0.);
			*vtx.color = color;
		}
		{
			auto vtx = _quad_vb->at(2);
			vec3_set(vtx.position, x + w, y + h, 0.);
			*vtx.color = color;
		}
		{
			auto vtx = _quad_vb->at(3);
			vec3_set(vtx.position, x, y + h, 0.);
			*vtx.color = color;
		}
		{
			auto vtx = _quad_vb->at(4);
			vec3_set(vtx.position, x, y, 0.);
			*vtx.color = color;
		}

		gs_load_indexbuffer(nullptr);
		gs_load_vertexbuffer(_quad_vb->update(true));
		while (gs_effect_loop(_effect->get_object(), "Color")) {
			gs_draw(GS_LINESTRIP, 0, 5);
		}
		gs_load_vertexbuffer(nullptr);
	} else {
		{
			auto vtx = _quad_vb->at(0);
			vec3_set(vtx.position, x, y, 0.);
			*vtx.color = color;
		}
		{
			auto vtx = _quad_vb->at(1);
			vec3_set(vtx.position, x + w, y, 0.);
			*vtx.color = color;
		}
		{
			auto vtx = _quad_vb->at(2);
			vec3_set(vtx.position, x, y + h, 0.);
			*vtx.color = color;
		}
		{
			auto vtx = _quad_vb->at(3);
			vec3_set(vtx.position, x + w, y + h, 0.);
			*vtx.color = color;
		}

		gs_load_indexbuffer(nullptr);
		gs_load_vertexbuffer(_quad_vb->update(true));
		while (gs_effect_loop(_effect->get_object(), "Color")) {
			gs_draw(GS_TRISTRIP, 0, 4);
		}
		gs_load_vertexbuffer(nullptr);
	}
}
