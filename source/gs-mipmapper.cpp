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

#include "gs-mipmapper.h"

extern "C" {
#pragma warning(push)
#pragma warning(disable : 4201)
#include <graphics/graphics.h>
#include <obs-module.h>
#include <obs.h>
#pragma warning(pop)
#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif
}

// Here be dragons!
// This is to add support for mipmap generation which is by default not possible with libobs.
// OBS hides a ton of possible things from us, which we'd have to simulate - or just hack around.
struct graphics_subsystem {
	void*        module;
	gs_device_t* device;
	// No other fields required.
};

#if defined(WIN32) || defined(WIN64)
#include <d3d11.h>
#include <dxgi.h>
#include <util/windows/ComPtr.hpp>

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
	vertexBuffer.reset();
	renderTarget.reset();
	effect.reset();
}

gs::mipmapper::mipmapper()
{
	vertexBuffer   = std::make_unique<gs::vertex_buffer>(6, 1);
	auto v0        = vertexBuffer->at(0);
	v0.position->x = 0;
	v0.position->y = 0;
	v0.uv[0]->x    = 0;
	v0.uv[0]->y    = 0;

	auto v1        = vertexBuffer->at(1);
	auto v4        = vertexBuffer->at(4);
	v4.position->x = v1.position->x = 1.0;
	v4.position->y = v1.position->y = 0;
	v4.uv[0]->x = v1.uv[0]->x = 1.0;
	v4.uv[0]->y = v1.uv[0]->y = 0;

	auto v2        = vertexBuffer->at(2);
	auto v3        = vertexBuffer->at(3);
	v3.position->x = v2.position->x = 0;
	v3.position->y = v2.position->y = 1.0;
	v3.uv[0]->x = v2.uv[0]->x = 0;
	v3.uv[0]->y = v2.uv[0]->y = 1.0;

	auto v5        = vertexBuffer->at(5);
	v5.position->x = 1.0;
	v5.position->y = 1.0;
	v5.uv[0]->x    = 1.0;
	v5.uv[0]->y    = 1.0;

	vertexBuffer->update();

	char* effect_file = obs_module_file("effects/mipgen.effect");
	effect            = std::make_unique<gs::effect>(effect_file);
	bfree(effect_file);
}

void gs::mipmapper::rebuild(std::shared_ptr<gs::texture> texture,
							gs::mipmapper::generator     generator = gs::mipmapper::generator::Bilinear,
							float_t                      strength  = 1.0)
{
	// Here be dragons! You have been warned.

	// Do nothing if there is no texture given.
	if (!texture) {
#ifdef _DEBUG
		assert(!texture);
#endif
		return;
	}

	obs_enter_graphics();

	// Test if we actually need to recreate the render target for a different format.
	bool recreate = false;
	if (renderTarget) {
		recreate = (texture->get_color_format() != renderTarget->get_color_format());
	} else {
		recreate = true;
	}

	// Re-create the render target if necessary.
	if (recreate) {
		renderTarget = std::make_unique<gs::rendertarget>(texture->get_color_format(), GS_ZS_NONE);
	}

	// Render
	graphics_t*      ctx         = gs_get_context();
	gs_d3d11_device* dev         = reinterpret_cast<gs_d3d11_device*>(ctx->device);
	int              device_type = gs_get_device_type();
	void*            obj         = gs_texture_get_obj(texture->get_object());
	std::string      technique   = "Draw";

	switch (generator) {
	case generator::Point:
		technique = "Point";
		break;
	case generator::Linear:
		technique = "Linear";
		break;
	case generator::Bilinear:
		technique = "Bilinear";
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

	gs_load_vertexbuffer(vertexBuffer->update());
	gs_load_indexbuffer(nullptr);

#if defined(WIN32) || defined(WIN64)
	if (device_type == GS_DEVICE_DIRECT3D_11) {
		// We definitely have a Direct3D11 resource.
		if (texture->get_type() == gs::texture::type::Normal) {
			D3D11_TEXTURE2D_DESC t2dsc;
			ID3D11Texture2D*     t2 = reinterpret_cast<ID3D11Texture2D*>(obj);

			t2->GetDesc(&t2dsc);

			// If we do not have any miplevels, just stop now.
			if (t2dsc.MipLevels == 0) {
				obs_leave_graphics();
				return;
			}

			size_t  texture_width  = texture->get_width();
			size_t  texture_height = texture->get_height();
			float_t texel_width    = 1.0 / texture_width;
			float_t texel_height   = 1.0 / texture_height;

			for (size_t mip = 1; mip < t2dsc.MipLevels; mip++) {
				texture_width /= 2;
				texture_height /= 2;

				// Draw mipmap layer
				{
					auto op = renderTarget->render(texture_width, texture_height);

					effect->get_parameter("image").set_texture(texture);
					effect->get_parameter("imageTexel").set_float2(texel_width, texel_height);
					effect->get_parameter("strength").set_float(strength);

					while (gs_effect_loop(effect->get_object(), technique.c_str())) {
						gs_draw(gs_draw_mode::GS_TRIS, 0, vertexBuffer->size());
					}
				}

				// Copy
				ID3D11Texture2D* rt =
					reinterpret_cast<ID3D11Texture2D*>(gs_texture_get_obj(renderTarget->get_object()));
				dev->context->CopySubresourceRegion(t2, mip, 0, 0, 0, rt, 0, nullptr);
			}
		}
	}
#endif
	if (device_type == GS_DEVICE_OPENGL) {
		// This is an OpenGL resource.
	}

	gs_load_indexbuffer(nullptr);
	gs_load_vertexbuffer(nullptr);

	obs_leave_graphics();
}
