/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2017 Michael Fabian Dirks
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "gs-mipmapper.hpp"
#include <sstream>
#include <stdexcept>
#include "obs/gs/gs-helper.hpp"
#include "plugin.hpp"

// Direct3D 11
#ifdef _WIN32
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4191 4242 4244 4365 4777 4986 5039 5204)
#endif
#include <Windows.h>
#include <atlutil.h>
#include <d3d11.h>
#include <dxgi.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif

// OpenGL
#include "glad/gl.h"

#ifdef _WIN32
struct d3d_info {
	ID3D11Device*        device  = nullptr;
	ID3D11DeviceContext* context = nullptr;
	ID3D11Resource*      source  = nullptr;
	ID3D11Resource*      target  = nullptr;
};

void d3d_initialize(d3d_info& info, std::shared_ptr<streamfx::obs::gs::texture> source,
					std::shared_ptr<streamfx::obs::gs::texture> target)
{
	info.source = reinterpret_cast<ID3D11Resource*>(gs_texture_get_obj(source->get_object()));
	info.target = reinterpret_cast<ID3D11Resource*>(gs_texture_get_obj(target->get_object()));
	info.device = reinterpret_cast<ID3D11Device*>(gs_get_device_obj());
	info.device->GetImmediateContext(&info.context);
}

void d3d_copy_subregion(d3d_info& info, std::shared_ptr<streamfx::obs::gs::texture> source, uint32_t mip_level,
						uint32_t width, uint32_t height)
{
	D3D11_BOX box = {0, 0, 0, width, height, 1};
	info.context->CopySubresourceRegion(info.target, mip_level, 0, 0, 0, info.source, 0, &box);
}

#endif

struct opengl_info {
	GLuint source = 0;
	GLuint target = 0;
	GLuint fbo    = 0;
};

std::string opengl_translate_error(GLenum error)
{
#define TRANSLATE_CASE(X) \
	case X:               \
		return #X;

	switch (error) {
		TRANSLATE_CASE(GL_NO_ERROR);
		TRANSLATE_CASE(GL_INVALID_ENUM);
		TRANSLATE_CASE(GL_INVALID_VALUE);
		TRANSLATE_CASE(GL_INVALID_OPERATION);
		TRANSLATE_CASE(GL_STACK_OVERFLOW);
		TRANSLATE_CASE(GL_STACK_UNDERFLOW);
		TRANSLATE_CASE(GL_OUT_OF_MEMORY);
		TRANSLATE_CASE(GL_INVALID_FRAMEBUFFER_OPERATION);
	}

	return std::to_string(error);
#undef TRANSLATE_CASE
}

std::string opengl_translate_framebuffer_status(GLenum error)
{
#define TRANSLATE_CASE(X) \
	case X:               \
		return #X;

	switch (error) {
		TRANSLATE_CASE(GL_FRAMEBUFFER_COMPLETE);
		TRANSLATE_CASE(GL_FRAMEBUFFER_UNDEFINED);
		TRANSLATE_CASE(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
		TRANSLATE_CASE(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
		TRANSLATE_CASE(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER);
		TRANSLATE_CASE(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER);
		TRANSLATE_CASE(GL_FRAMEBUFFER_UNSUPPORTED);
		TRANSLATE_CASE(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE);
		TRANSLATE_CASE(GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS);
	}

	return std::to_string(error);
#undef TRANSLATE_CASE
}

#define D_OPENGL_CHECK_ERROR(FUNCTION)                            \
	if (auto err = glGetError(); err != GL_NO_ERROR) {            \
		std::stringstream sstr;                                   \
		sstr << opengl_translate_error(err) << " = " << FUNCTION; \
		throw std::runtime_error(sstr.str());                     \
	}

#define D_OPENGL_CHECK_FRAMEBUFFERSTATUS(BUFFER, FUNCTION)                             \
	if (auto err = glCheckFramebufferStatus(BUFFER); err != GL_FRAMEBUFFER_COMPLETE) { \
		std::stringstream sstr;                                                        \
		sstr << opengl_translate_framebuffer_status(err) << " = " << FUNCTION;         \
		throw std::runtime_error(sstr.str());                                          \
	}

void opengl_initialize(opengl_info& info, std::shared_ptr<streamfx::obs::gs::texture> source,
					   std::shared_ptr<streamfx::obs::gs::texture> target)
{
	info.source = *reinterpret_cast<GLuint*>(gs_texture_get_obj(source->get_object()));
	info.target = *reinterpret_cast<GLuint*>(gs_texture_get_obj(target->get_object()));

	glGenFramebuffers(1, &info.fbo);
}

void opengl_finalize(opengl_info& info)
{
	glDeleteFramebuffers(1, &info.fbo);
}

void opengl_copy_subregion(opengl_info& info, std::shared_ptr<streamfx::obs::gs::texture> source_tex,
						   uint32_t mip_level, uint32_t width, uint32_t height)
{
	GLuint source = *reinterpret_cast<GLuint*>(gs_texture_get_obj(source_tex->get_object()));

	// Source -> Texture Unit 0, Read Color Framebuffer
	glActiveTexture(GL_TEXTURE0);
	D_OPENGL_CHECK_ERROR("glActiveTexture(GL_TEXTURE0);");
	glBindTexture(GL_TEXTURE_2D, source);
	D_OPENGL_CHECK_ERROR("glBindTexture(GL_TEXTURE_2D, origin);");
	glBindFramebuffer(GL_READ_FRAMEBUFFER, info.fbo);
	D_OPENGL_CHECK_ERROR("glBindFramebuffer(GL_READ_FRAMEBUFFER, info.fbo);");
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, source,
						   0); // Origin is a render target, not a texture
	D_OPENGL_CHECK_ERROR(
		"glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, origin, mip_level);");
	D_OPENGL_CHECK_FRAMEBUFFERSTATUS(
		GL_READ_FRAMEBUFFER,
		"glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, origin, mip_level);");

	// Target -> Texture Unit 1
	glActiveTexture(GL_TEXTURE1);
	D_OPENGL_CHECK_ERROR("glActiveTexture(GL_TEXTURE1);");
	glBindTexture(GL_TEXTURE_2D, info.target);
	D_OPENGL_CHECK_ERROR("glBindTexture(GL_TEXTURE_2D, info.target);");

	// Copy Data
	glCopyTexSubImage2D(GL_TEXTURE_2D, mip_level, 0, 0, 0, 0, width, height);
	D_OPENGL_CHECK_ERROR("glCopyTexSubImage2D(GL_TEXTURE_2D, mip_level, 0, 0, 0, 0, width, height);");

	// Target -/-> Texture Unit 1
	glActiveTexture(GL_TEXTURE1);
	D_OPENGL_CHECK_ERROR("glActiveTexture(GL_TEXTURE1);");
	glBindTexture(GL_TEXTURE_2D, 0);
	D_OPENGL_CHECK_ERROR("glBindTexture(GL_TEXTURE_2D, 0);");

	// Source -/-> Texture Unit 0, Read Color Framebuffer
	glActiveTexture(GL_TEXTURE0);
	D_OPENGL_CHECK_ERROR("glActiveTexture(GL_TEXTURE0);");
	glBindTexture(GL_TEXTURE_2D, 0);
	D_OPENGL_CHECK_ERROR("glBindTexture(GL_TEXTURE_2D, 0);");
	glBindFramebuffer(GL_READ_FRAMEBUFFER, info.fbo);
	D_OPENGL_CHECK_ERROR("glBindFramebuffer(GL_READ_FRAMEBUFFER, info.fbo);");
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
	D_OPENGL_CHECK_ERROR("glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);");
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	D_OPENGL_CHECK_ERROR("glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);");
}

streamfx::obs::gs::mipmapper::~mipmapper()
{
	_rt.reset();
	_effect.reset();
}

streamfx::obs::gs::mipmapper::mipmapper()
{
	auto gctx = streamfx::obs::gs::context();

	{
		auto file = streamfx::data_file_path("effects/mipgen.effect");
		try {
			_effect = streamfx::obs::gs::effect::create(file);
		} catch (const std::exception& ex) {
			DLOG_ERROR("Error loading '%s': %s", file.generic_u8string().c_str(), ex.what());
		}
	}
}

uint32_t streamfx::obs::gs::mipmapper::calculate_max_mip_level(uint32_t width, uint32_t height)
{
	return static_cast<uint32_t>(1 + std::lroundl(floor(log2(std::max<GLint>(width, height)))));
}

void streamfx::obs::gs::mipmapper::rebuild(std::shared_ptr<streamfx::obs::gs::texture> source,
										   std::shared_ptr<streamfx::obs::gs::texture> target)
{
	{ // Validate arguments and structure.
		if (!source || !target)
			return; // Do nothing if source or target are missing.

		if (!_effect)
			return; // Do nothing if the necessary data failed to load.

		// Ensure texture sizes match
		if ((source->get_width() != target->get_width()) || (source->get_height() != target->get_height())) {
			throw std::invalid_argument("source and target must have same size");
		}

		// Ensure texture types match
		if ((source->get_type() != target->get_type())) {
			throw std::invalid_argument("source and target must have same type");
		}

		// Ensure texture formats match
		if ((source->get_color_format() != target->get_color_format())) {
			throw std::invalid_argument("source and target must have same format");
		}
	}

	// Get a unique lock on the graphics context.
	auto gctx = streamfx::obs::gs::context();

	// Do we need to recreate the render target for a different format?
	if ((!_rt) || (source->get_color_format() != _rt->get_color_format())) {
		_rt = std::make_unique<streamfx::obs::gs::rendertarget>(source->get_color_format(), GS_ZS_NONE);
	}

	// Initialize API Handlers.
	opengl_info oglinfo;
	if (gs_get_device_type() == GS_DEVICE_OPENGL) {
		opengl_initialize(oglinfo, source, target);
	}
#ifdef _WIN32
	d3d_info d3dinfo;
	if (gs_get_device_type() == GS_DEVICE_DIRECT3D_11) {
		d3d_initialize(d3dinfo, source, target);
	}
#endif

	// Use different methods for different types of textures.
	if (source->get_type() == streamfx::obs::gs::texture::type::Normal) {
		uint32_t width         = source->get_width();
		uint32_t height        = source->get_height();
		size_t   max_mip_level = calculate_max_mip_level(width, height);

		{
#ifdef ENABLE_PROFILING
			auto cctr = streamfx::obs::gs::debug_marker(streamfx::obs::gs::debug_color_azure_radiance,
														"Mip Level %" PRId64 "", 0);
#endif

			// Retrieve maximum mip map level.
#ifdef _WIN32
			if (gs_get_device_type() == GS_DEVICE_DIRECT3D_11) {
				d3d_copy_subregion(d3dinfo, source, 0, width, height);
			}
#endif
			if (gs_get_device_type() == GS_DEVICE_OPENGL) {
				opengl_copy_subregion(oglinfo, source, 0, width, height);
			}
		}

		// Render each mip map level.
		for (size_t mip = 1; mip < max_mip_level; mip++) {
#ifdef ENABLE_PROFILING
			auto cctr = streamfx::obs::gs::debug_marker(streamfx::obs::gs::debug_color_azure_radiance,
														"Mip Level %" PRIuMAX, mip);
#endif

			uint32_t cwidth  = std::max<uint32_t>(width >> mip, 1);
			uint32_t cheight = std::max<uint32_t>(height >> mip, 1);
			float_t  iwidth  = 1.f / static_cast<float_t>(cwidth);
			float_t  iheight = 1.f / static_cast<float_t>(cheight);

			// Set up rendering state.
			gs_blend_state_push();
			gs_reset_blend_state();
			gs_enable_blending(false);
			gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);
			gs_enable_color(true, true, true, true);
			gs_enable_depth_test(false);
			gs_enable_stencil_test(false);
			gs_enable_stencil_write(false);
			gs_set_cull_mode(GS_NEITHER);
			try {
				auto op = _rt->render(width, height);
				gs_set_viewport(0, 0, static_cast<int>(cwidth), static_cast<int>(cheight));
				gs_ortho(0, 1, 0, 1, 0, 1);

				vec4 black = {1., 1., 1., 1};
				gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &black, 0, 0);

				_effect.get_parameter("image").set_texture(target);
				_effect.get_parameter("imageTexel").set_float2(iwidth, iheight);
				_effect.get_parameter("level").set_int(int32_t(mip - 1));
				while (gs_effect_loop(_effect.get_object(), "Draw")) {
					streamfx::gs_draw_fullscreen_tri();
				}
			} catch (...) {
			}

			// Clean up rendering state.
			gs_blend_state_pop();

			// Copy from the render target to the target mip level.
#ifdef _WIN32
			if (gs_get_device_type() == GS_DEVICE_DIRECT3D_11) {
				d3d_copy_subregion(d3dinfo, _rt->get_texture(), mip, cwidth, cheight);
			}
#endif
			if (gs_get_device_type() == GS_DEVICE_OPENGL) {
				opengl_copy_subregion(oglinfo, _rt->get_texture(), mip, cwidth, cheight);
			}
		}

	} else {
		throw std::runtime_error("Only 2D Textures support Mip-mapping.");
	}

	// Finalize API handlers.
	if (gs_get_device_type() == GS_DEVICE_OPENGL) {
		opengl_finalize(oglinfo);
	}
#ifdef _WIN32
	if (gs_get_device_type() == GS_DEVICE_DIRECT3D_11) {
		//d3d_finalize(d3dinfo);
	}
#endif
}
