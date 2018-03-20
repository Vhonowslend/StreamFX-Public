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

#include "gs-sampler.h"

gs::sampler::sampler() {
	m_dirty = true;
	m_samplerInfo = { GS_FILTER_LINEAR, GS_ADDRESS_WRAP, GS_ADDRESS_WRAP, GS_ADDRESS_WRAP, 1, 0 };
	m_samplerState = nullptr;
}

gs::sampler::~sampler() {
	if (m_samplerState)
		gs_samplerstate_destroy(m_samplerState);
}

void gs::sampler::set_filter(gs_sample_filter v) {
	m_dirty = true;
	m_samplerInfo.filter = v;
}

gs_sample_filter gs::sampler::get_filter() {
	return m_samplerInfo.filter;
}

void gs::sampler::set_address_mode_u(gs_address_mode v) {
	m_dirty = true;
	m_samplerInfo.address_u = v;
}

gs_address_mode gs::sampler::get_address_mode_u() {
	return m_samplerInfo.address_u;
}

void gs::sampler::set_address_mode_v(gs_address_mode v) {
	m_dirty = true;
	m_samplerInfo.address_v = v;
}

gs_address_mode gs::sampler::get_address_mode_v() {
	return m_samplerInfo.address_v;
}

void gs::sampler::set_address_mode_w(gs_address_mode v) {
	m_dirty = true;
	m_samplerInfo.address_w = v;
}

gs_address_mode gs::sampler::get_address_mode_w() {
	return m_samplerInfo.address_w;
}

void gs::sampler::set_max_anisotropy(int v) {
	m_dirty = true;
	m_samplerInfo.max_anisotropy = v;
}

int gs::sampler::get_max_anisotropy() {
	return m_samplerInfo.max_anisotropy;
}

void gs::sampler::set_border_color(uint32_t v) {
	m_dirty = true;
	m_samplerInfo.border_color = v;
}

void gs::sampler::set_border_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	m_dirty = true;
	m_samplerInfo.border_color = a << 24 | r << 16 | g << 8 | b;
}

uint32_t gs::sampler::get_border_color() {
	return m_samplerInfo.border_color;
}

uint8_t gs::sampler::get_border_color(bool r, bool g, bool b, bool a) {
	if (a)
		return (m_samplerInfo.border_color >> 24) & 0xFF;
	if (r)
		return (m_samplerInfo.border_color >> 16) & 0xFF;
	if (g)
		return (m_samplerInfo.border_color >> 8) & 0xFF;
	if (b)
		return m_samplerInfo.border_color & 0xFF;
	return 0;
}

gs_sampler_state* gs::sampler::refresh() {
	gs_samplerstate_destroy(m_samplerState);
	m_samplerState = gs_samplerstate_create(&m_samplerInfo);
	m_dirty = false;
	return m_samplerState;
}

gs_sampler_state* gs::sampler::get_object() {
	if (m_dirty)
		return refresh();
	return m_samplerState;
}
