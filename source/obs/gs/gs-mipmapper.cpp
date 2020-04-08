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
#include <stdexcept>
#include "obs/gs/gs-helper.hpp"
#include "plugin.hpp"

#if defined(WIN32) || defined(WIN64)
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 5039)
#endif
#include <Windows.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif

// Here be dragons!
// This is to add support for mipmap generation which is by default not possible with libobs.
// OBS hides a ton of possible things from us, which we'd have to simulate - or just hack around.
struct graphics_subsystem {
	void*        module;
	gs_device_t* device;
	// No other fields required.
};

#if defined(WIN32) || defined(WIN64)
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201 4365)
#endif
#include <d3d11.h>
#include <dxgi.h>
#include <util/windows/ComPtr.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

// Slaughtered copy of d3d11-subsystem.hpp gs_device. We only need up to device and context, the rest is "unknown" to us.
struct gs_d3d11_device {
	ComPtr<IDXGIFactory1>       factory;
	ComPtr<IDXGIAdapter1>       adapter;
	ComPtr<ID3D11Device>        device;
	ComPtr<ID3D11DeviceContext> context;
	// No other fields required.
};
#endif

gs::mipmapper::~mipmapper()
{
	_vb.reset();
	_rt.reset();
	_effect.reset();
}

gs::mipmapper::mipmapper()
{
	_vb            = std::make_unique<gs::vertex_buffer>(uint32_t(6u), std::uint8_t(1u));
	auto v0        = _vb->at(0);
	v0.position->x = 0;
	v0.position->y = 0;
	v0.uv[0]->x    = 0;
	v0.uv[0]->y    = 0;

	auto v1        = _vb->at(1);
	auto v4        = _vb->at(4);
	v4.position->x = v1.position->x = 1.0;
	v4.position->y = v1.position->y = 0;
	v4.uv[0]->x = v1.uv[0]->x = 1.0;
	v4.uv[0]->y = v1.uv[0]->y = 0;

	auto v2        = _vb->at(2);
	auto v3        = _vb->at(3);
	v3.position->x = v2.position->x = 0;
	v3.position->y = v2.position->y = 1.0;
	v3.uv[0]->x = v2.uv[0]->x = 0;
	v3.uv[0]->y = v2.uv[0]->y = 1.0;

	auto v5        = _vb->at(5);
	v5.position->x = 1.0;
	v5.position->y = 1.0;
	v5.uv[0]->x    = 1.0;
	v5.uv[0]->y    = 1.0;

	_vb->update();

	char* effect_file = obs_module_file("effects/mipgen.effect");
	_effect           = gs::effect::create(effect_file);
	bfree(effect_file);
}

void gs::mipmapper::rebuild(std::shared_ptr<gs::texture> source, std::shared_ptr<gs::texture> target,
							gs::mipmapper::generator generator = gs::mipmapper::generator::Linear,
							float_t                  strength  = 1.0)
{
	// Here be dragons! You have been warned.

	// Do nothing if there is no texture given.
	if (!source) {
#ifdef _DEBUG
		assert(!source);
#endif
		return;
	}
	if (!target) {
#ifdef _DEBUG
		assert(!target);
#endif
		return;
	}

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

	auto gctx = gs::context();

	// Copy original texture
	//gs_copy_texture(target->get_object(), source->get_object());

	// Test if we actually need to recreate the render target for a different format or at all.
	if ((!_rt) || (source->get_color_format() != _rt->get_color_format())) {
		_rt = std::make_unique<gs::rendertarget>(source->get_color_format(), GS_ZS_NONE);
	}

	// Render
#if defined(WIN32) || defined(WIN64)
	graphics_t*      ctx = gs_get_context();
	gs_d3d11_device* dev = reinterpret_cast<gs_d3d11_device*>(ctx->device);
#endif
	int         device_type = gs_get_device_type();
	std::string technique   = "Draw";

	switch (generator) {
	case generator::Point:
		technique = "Point";
		break;
	case generator::Linear:
		technique = "Linear";
		break;
	case generator::Sharpen:
		technique = "Sharpen";
		break;
	case generator::Smoothen:
		technique = "Smoothen";
		break;
	case generator::Bicubic:
		technique = "Bicubic";
		break;
	case generator::Lanczos:
		technique = "Lanczos";
		break;
	}

	gs_load_vertexbuffer(_vb->update());
	gs_load_indexbuffer(nullptr);

	if (source->get_type() == gs::texture::type::Normal) {
		std::size_t texture_width  = source->get_width();
		std::size_t texture_height = source->get_height();
		float_t     texel_width    = 1.0f / texture_width;
		float_t     texel_height   = 1.0f / texture_height;
		std::size_t mip_levels     = 1;

#if defined(WIN32) || defined(WIN64)
		ID3D11Texture2D* target_t2 = nullptr;
		ID3D11Texture2D* source_t2 = nullptr;
		if (device_type == GS_DEVICE_DIRECT3D_11) {
			// We definitely have a Direct3D11 resource.
			D3D11_TEXTURE2D_DESC target_t2desc;
			target_t2 = reinterpret_cast<ID3D11Texture2D*>(gs_texture_get_obj(target->get_object()));
			source_t2 = reinterpret_cast<ID3D11Texture2D*>(gs_texture_get_obj(source->get_object()));
			target_t2->GetDesc(&target_t2desc);
			dev->context->CopySubresourceRegion(target_t2, 0, 0, 0, 0, source_t2, 0, nullptr);
			mip_levels = target_t2desc.MipLevels;
		}
#endif
		if (device_type == GS_DEVICE_OPENGL) {
			// This is an OpenGL resource.
		}

		// If we do not have any miplevels, just stop now.
		if (mip_levels == 1) {
			return;
		}

		for (std::size_t mip = 1; mip < mip_levels; mip++) {
			texture_width /= 2;
			texture_height /= 2;
			if (texture_width == 0) {
				texture_width = 1;
			}
			if (texture_height == 0) {
				texture_height = 1;
			}

			texel_width  = 1.0f / texture_width;
			texel_height = 1.0f / texture_height;

			// Draw mipmap layer
			try {
				auto op = _rt->render(uint32_t(texture_width), uint32_t(texture_height));

				gs_set_cull_mode(GS_NEITHER);
				gs_reset_blend_state();
				gs_blend_function_separate(gs_blend_type::GS_BLEND_ONE, gs_blend_type::GS_BLEND_ZERO,
										   gs_blend_type::GS_BLEND_ONE, gs_blend_type::GS_BLEND_ZERO);
				gs_enable_depth_test(false);
				gs_enable_stencil_test(false);
				gs_enable_stencil_write(false);
				gs_enable_color(true, true, true, true);
				gs_ortho(0, 1, 0, 1, -1, 1);

				vec4 black;
				vec4_zero(&black);
				gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &black, 0, 0);

				_effect.get_parameter("image").set_texture(target);
				_effect.get_parameter("level").set_int(int32_t(mip - 1));
				_effect.get_parameter("imageTexel").set_float2(texel_width, texel_height);
				_effect.get_parameter("strength").set_float(strength);

				while (gs_effect_loop(_effect.get_object(), technique.c_str())) {
					gs_draw(gs_draw_mode::GS_TRIS, 0, _vb->size());
				}
			} catch (...) {
				LOG_ERROR("Failed to render mipmap layer.");
			}

#if defined(WIN32) || defined(WIN64)
			if (device_type == GS_DEVICE_DIRECT3D_11) {
				// Copy
				ID3D11Texture2D* rt    = reinterpret_cast<ID3D11Texture2D*>(gs_texture_get_obj(_rt->get_object()));
				std::uint32_t    level = uint32_t(D3D11CalcSubresource(UINT(mip), 0, UINT(mip_levels)));
				dev->context->CopySubresourceRegion(target_t2, level, 0, 0, 0, rt, 0, NULL);
			}
#endif
		}
	}

	gs_load_indexbuffer(nullptr);
	gs_load_vertexbuffer(nullptr);
}
