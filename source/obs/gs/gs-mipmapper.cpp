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

gs::mipmapper::~mipmapper()
{
	_vb.reset();
	_rt.reset();
	_effect.reset();
}

gs::mipmapper::mipmapper()
{
	_vb = std::make_unique<gs::vertex_buffer>(uint32_t(3u), uint8_t(1u));

	{
		auto vtx        = _vb->at(0);
		vtx.position->x = 0;
		vtx.position->y = 0;
		vtx.uv[0]->x    = 0;
		vtx.uv[0]->y    = 0;
	}
	{
		auto vtx        = _vb->at(1);
		vtx.position->x = 0.;
		vtx.position->y = 2.;
		vtx.uv[0]->x    = 0.;
		vtx.uv[0]->y    = 2.;
	}
	{
		auto vtx        = _vb->at(2);
		vtx.position->x = 2.;
		vtx.position->y = 0.;
		vtx.uv[0]->x    = 2.;
		vtx.uv[0]->y    = 0.;
	}

	_vb->update();

	_effect = gs::effect::create(streamfx::data_file_path("effects/mipgen.effect").u8string());
}

void gs::mipmapper::rebuild(std::shared_ptr<gs::texture> source, std::shared_ptr<gs::texture> target)
{
	{ // Validate arguments and structure.
		if (!source || !target)
			return; // Do nothing if source or target are missing.

		if (!_vb || !_effect)
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
	auto gctx = gs::context();

	// Do we need to recreate the render target for a different format?
	if ((!_rt) || (source->get_color_format() != _rt->get_color_format())) {
		_rt = std::make_unique<gs::rendertarget>(source->get_color_format(), GS_ZS_NONE);
	}

	// Grab API related information.
#ifdef _WIN32
	ID3D11Device*        d3d_device  = nullptr;
	ID3D11DeviceContext* d3d_context = nullptr;
	ID3D11Resource*      d3d_source  = nullptr;
	ID3D11Resource*      d3d_target  = nullptr;
	if (gs_get_device_type() == GS_DEVICE_DIRECT3D_11) {
		d3d_source = reinterpret_cast<ID3D11Resource*>(gs_texture_get_obj(source->get_object()));
		d3d_target = reinterpret_cast<ID3D11Resource*>(gs_texture_get_obj(target->get_object()));
		d3d_device = reinterpret_cast<ID3D11Device*>(gs_get_device_obj());
		d3d_device->GetImmediateContext(&d3d_context);
	}
#endif
	if (gs_get_device_type() == GS_DEVICE_OPENGL) {
		// FixMe! Implement OpenGL
	}

	// Use different methods for different types of textures.
	if (source->get_type() == gs::texture::type::Normal) {
		while (true) {
			uint32_t width         = source->get_width();
			uint32_t height        = source->get_height();
			size_t   max_mip_level = 1;

			{
#ifdef ENABLE_PROFILING
				auto cctr = gs::debug_marker(gs::debug_color_azure_radiance, "Mip Level %" PRId64 "", 0);
#endif

#ifdef _WIN32
				if (gs_get_device_type() == GS_DEVICE_DIRECT3D_11) {
					{ // Retrieve maximum mip map level.
						D3D11_TEXTURE2D_DESC td;
						static_cast<ID3D11Texture2D*>(d3d_target)->GetDesc(&td);
						max_mip_level = td.MipLevels;
					}

					// Copy mip level 0 across textures.
					d3d_context->CopySubresourceRegion(d3d_target, 0, 0, 0, 0, d3d_source, 0, nullptr);
				}
#endif
				if (gs_get_device_type() == GS_DEVICE_OPENGL) {
					// FixMe! Implement OpenGL
				}
			}

			// Do we even need to do anything here?
			if (max_mip_level == 1)
				break;

			// Render each mip map level.
			for (size_t mip = 1; mip < max_mip_level; mip++) {
#ifdef ENABLE_PROFILING
				auto cctr = gs::debug_marker(gs::debug_color_azure_radiance, "Mip Level %" PRIuMAX, mip);
#endif

				uint32_t cwidth  = std::max<uint32_t>(width >> mip, 1);
				uint32_t cheight = std::max<uint32_t>(height >> mip, 1);
				float_t  iwidth  = 1.f / static_cast<float_t>(cwidth);
				float_t  iheight = 1.f / static_cast<float_t>(cheight);

				// Set up rendering state.
				gs_load_vertexbuffer(_vb->update(false));
				gs_load_indexbuffer(nullptr);
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
						gs_draw(gs_draw_mode::GS_TRIS, 0, _vb->size());
					}
				} catch (...) {
				}

				// Clean up rendering state.
				gs_load_indexbuffer(nullptr);
				gs_load_vertexbuffer(nullptr);
				gs_blend_state_pop();

				// Copy from the render target to the target mip level.
#ifdef _WIN32
				if (gs_get_device_type() == GS_DEVICE_DIRECT3D_11) {
					ID3D11Texture2D* rtt =
						reinterpret_cast<ID3D11Texture2D*>(gs_texture_get_obj(_rt->get_texture()->get_object()));
					uint32_t level = uint32_t(D3D11CalcSubresource(UINT(mip), 0, UINT(max_mip_level)));

					D3D11_BOX box = {0, 0, 0, cwidth, cheight, 1};
					d3d_context->CopySubresourceRegion(d3d_target, level, 0, 0, 0, rtt, 0, &box);
				}
#endif
				if (gs_get_device_type() == GS_DEVICE_OPENGL) {
					// FixMe! Implement OpenGL
				}
			}

			break;
		}
	} else {
		throw std::runtime_error("Texture type is not supported by mipmapping yet.");
	}
}
