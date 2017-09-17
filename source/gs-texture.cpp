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

#include "gs-texture.h"
#include <stdexcept>
#include <sys/stat.h>
#include <fstream>
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/util/platform.h>
	#include <libobs/obs.h>
	#pragma warning( pop )
}

GS::Texture::Texture(uint32_t width, uint32_t height, gs_color_format format, uint32_t mip_levels, const uint8_t **mip_data, uint32_t flags) {
	if (width == 0)
		throw std::logic_error("width must be at least 1");
	if (height == 0)
		throw std::logic_error("height must be at least 1");
	if (mip_levels == 0)
		throw std::logic_error("mip_levels must be at least 1");
	if (!mip_data)
		throw std::logic_error("mip_data is invalid");

	if (mip_levels > 1 || flags & Flags::BuildMipMaps) {
		bool isPOT = (pow(2, (int64_t)floor(log(width) / log(2))) == width)
			&& (pow(2, (int64_t)floor(log(height) / log(2))) == height);
		if (!isPOT)
			throw std::logic_error("mip mapping requires power of two dimensions");
	}

	obs_enter_graphics();
	m_texture = gs_texture_create(width, height, format, mip_levels, mip_data, (flags & Flags::Dynamic) ? GS_DYNAMIC : 0 | (flags & Flags::BuildMipMaps) ? GS_BUILD_MIPMAPS : 0);
	obs_leave_graphics();

	if (!m_texture)
		throw std::runtime_error("Failed to create texture.");
}

GS::Texture::Texture(uint32_t width, uint32_t height, uint32_t depth, gs_color_format format, uint32_t mip_levels, const uint8_t **mip_data, uint32_t flags) {
	if (width == 0)
		throw std::logic_error("width must be at least 1");
	if (height == 0)
		throw std::logic_error("height must be at least 1");
	if (depth == 0)
		throw std::logic_error("depth must be at least 1");
	if (mip_levels == 0)
		throw std::logic_error("mip_levels must be at least 1");
	if (!mip_data)
		throw std::logic_error("mip_data is invalid");

	if (mip_levels > 1 || flags & Flags::BuildMipMaps) {
		bool isPOT = (pow(2, (int64_t)floor(log(width) / log(2))) == width)
			&& (pow(2, (int64_t)floor(log(height) / log(2))) == height)
			&& (pow(2, (int64_t)floor(log(depth) / log(2))) == depth);
		if (!isPOT)
			throw std::logic_error("mip mapping requires power of two dimensions");
	}

	obs_enter_graphics();
	m_texture = gs_voltexture_create(width, height, depth, format, mip_levels, mip_data, (flags & Flags::Dynamic) ? GS_DYNAMIC : 0 | (flags & Flags::BuildMipMaps) ? GS_BUILD_MIPMAPS : 0);
	obs_leave_graphics();

	if (!m_texture)
		throw std::runtime_error("Failed to create texture.");
}

GS::Texture::Texture(uint32_t size, gs_color_format format, uint32_t mip_levels, const uint8_t **mip_data, uint32_t flags) {
	if (size == 0)
		throw std::logic_error("size must be at least 1");
	if (mip_levels == 0)
		throw std::logic_error("mip_levels must be at least 1");
	if (!mip_data)
		throw std::logic_error("mip_data is invalid");

	if (mip_levels > 1 || flags & Flags::BuildMipMaps) {
		bool isPOT = (pow(2, (int64_t)floor(log(size) / log(2))) == size);
		if (!isPOT)
			throw std::logic_error("mip mapping requires power of two dimensions");
	}

	obs_enter_graphics();
	m_texture = gs_cubetexture_create(size, format, mip_levels, mip_data, (flags & Flags::Dynamic) ? GS_DYNAMIC : 0 | (flags & Flags::BuildMipMaps) ? GS_BUILD_MIPMAPS : 0);
	obs_leave_graphics();

	if (!m_texture)
		throw std::runtime_error("Failed to create texture.");
}

GS::Texture::Texture(std::string file) {
	struct stat st;
	if (os_stat(file.c_str(), &st) != 0)
		throw std::ios_base::failure(file);

	obs_enter_graphics();
	m_texture = gs_texture_create_from_file(file.c_str());
	obs_leave_graphics();

	if (!m_texture)
		throw std::runtime_error("Failed to load texture.");
}

GS::Texture::Texture(Texture& other) {
	throw std::logic_error("not yet implemented");
	//obs_enter_graphics();
	//switch (gs_get_texture_type(other.m_texture)) {
	//	case GS_TEXTURE_2D:
	//		uint32_t width = gs_texture_get_width(other.m_texture);
	//		uint32_t height = gs_texture_get_height(other.m_texture);
	//		gs_color_format format = gs_texture_get_color_format(other.m_texture);
	//		uint32_t mult = 0;
	//		switch (format) {
	//			case GS_A8:
	//			case GS_R8:
	//				mult = 1;
	//				break;
	//			case GS_R16:
	//			case GS_R16F:
	//				mult = 2;
	//				break;
	//			case GS_RG16F:
	//			case GS_R32F:
	//			case GS_BGRA:
	//			case GS_BGRX:
	//			case GS_RGBA:
	//			case GS_R10G10B10A2:
	//				mult = 4;
	//				break;
	//			case GS_RGBA16:
	//			case GS_RGBA16F:
	//			case GS_RG32F:
	//				mult = 8;
	//				break;
	//			case GS_RGBA32F:
	//				mult = 16;
	//				break;
	//			case GS_DXT1:
	//			case GS_DXT3:
	//			case GS_DXT5:
	//				mult = 8;
	//				break;
	//		}
	//		uint8_t* buf = new uint8_t[width * height * mult];
	//		m_texture = gs_texture_create(width, height, format,
	//			1, &buf);
	//		delete buf;
	//		break;
	//	case GS_TEXTURE_3D:
	//		uint32_t width = gs_voltexture_get_width(other.m_texture);
	//		uint32_t height = gs_voltexture_get_height(other.m_texture);
	//		uint32_t depth = gs_voltexture_get_height(other.m_texture);
	//		gs_color_format format = gs_voltexture_get_color_format(other.m_texture);
	//		break;
	//	case GS_TEXTURE_CUBE:
	//		uint32_t size = gs_cubetexture_get_size(other.m_texture);
	//		gs_color_format format = gs_cubetexture_get_color_format(other.m_texture);
	//		gs_copy_texture()
	//			break;
	//}
	//obs_leave_graphics();
}

GS::Texture::~Texture() {
	if (m_texture) {
		obs_enter_graphics();
		switch (gs_get_texture_type(m_texture)) {
			case GS_TEXTURE_2D:
				gs_texture_destroy(m_texture);
				break;
			case GS_TEXTURE_3D:
				gs_voltexture_destroy(m_texture);
				break;
			case GS_TEXTURE_CUBE:
				gs_cubetexture_destroy(m_texture);
				break;
		}
		obs_leave_graphics();
	}
	m_texture = nullptr;
}

void GS::Texture::Load(int unit) {
	obs_enter_graphics();
	gs_load_texture(m_texture, unit);
	obs_leave_graphics();
}

gs_texture_t* GS::Texture::GetObject() {
	return m_texture;
}
