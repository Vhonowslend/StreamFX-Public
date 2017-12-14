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

#include "gs-rendertarget.h"
#include <stdexcept>
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/obs.h>
	#pragma warning( pop )
}

GS::RenderTarget::RenderTarget(gs_color_format colorFormat, gs_zstencil_format zsFormat) {
	m_isBeingRendered = false;
	obs_enter_graphics();
	m_renderTarget = gs_texrender_create(colorFormat, zsFormat);
	obs_leave_graphics();
}

GS::RenderTarget::~RenderTarget() {
	obs_enter_graphics();
	gs_texrender_destroy(m_renderTarget);
	obs_leave_graphics();
}

GS::RenderTargetOp GS::RenderTarget::Render(uint32_t width, uint32_t height) {
	return { this, width, height };
}

gs_texture_t* GS::RenderTarget::GetTextureObject() {
	obs_enter_graphics();
	gs_texture_t* tex = gs_texrender_get_texture(m_renderTarget);
	obs_leave_graphics();
	return tex;
}

GS::RenderTargetOp::RenderTargetOp(GS::RenderTarget* rt, uint32_t width, uint32_t height) : m_renderTarget(rt) {
	if (m_renderTarget == nullptr)
		throw std::invalid_argument("rt");
	if (m_renderTarget->m_isBeingRendered)
		throw std::logic_error("Can't start rendering to the same render target twice.");
	obs_enter_graphics();
	gs_texrender_reset(m_renderTarget->m_renderTarget);
	if (!gs_texrender_begin(m_renderTarget->m_renderTarget, width, height)) {
		obs_leave_graphics();
		throw std::runtime_error("Failed to begin rendering to render target.");
	}
	obs_leave_graphics();
	m_renderTarget->m_isBeingRendered = true;
}

GS::RenderTargetOp::RenderTargetOp(GS::RenderTargetOp&& r) {
	this->m_renderTarget = r.m_renderTarget;
	r.m_renderTarget = nullptr;
}

GS::RenderTargetOp::~RenderTargetOp() {
	if (m_renderTarget == nullptr)
		return;
	obs_enter_graphics();
	gs_texrender_end(m_renderTarget->m_renderTarget);
	obs_leave_graphics();
	m_renderTarget->m_isBeingRendered = false;
}
