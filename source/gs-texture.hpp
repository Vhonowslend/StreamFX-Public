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

#pragma once
#include <cinttypes>
#include <string>
#include "utility.hpp"

// OBS
#pragma warning(push)
#pragma warning(disable : 4201)
#include <graphics/graphics.h>
#pragma warning(pop)

namespace gs {
	class texture {
		public:
		enum class type : uint8_t { Normal, Volume, Cube };

		enum class flags : uint8_t {
			None,
			Dynamic,
			BuildMipMaps,
		};

		protected:
		gs_texture_t* m_texture;
		bool          m_isOwner     = true;
		type          m_textureType = type::Normal;

		public:
		~texture();

		/*!
		 * \brief Create a 2D Texture
		 *
		 * \param width Width of the 2D Texture
		 * \param height Height of the 2D Texture
		 * \param format Color Format to use
		 * \param mip_levels Number of Mip Levels available
		 * \param mip_data Texture data including mipmaps
		 * \param texture_flags Texture Flags
		 */
		texture(uint32_t width, uint32_t height, gs_color_format format, uint32_t mip_levels, const uint8_t** mip_data,
				gs::texture::flags texture_flags);

		/*!
		 * \brief Create a 3D Texture
		 *
		 * \param width Width of the 3D Texture
		 * \param height Height of the 3D Texture
		 * \param depth Depth of the 3D Texture
		 * \param format Color Format to use
		 * \param mip_levels Number of Mip Levels available
		 * \param mip_data Texture data including mipmaps
		 * \param texture_flags Texture Flags
		 */
		texture(uint32_t width, uint32_t height, uint32_t depth, gs_color_format format, uint32_t mip_levels,
				const uint8_t** mip_data, gs::texture::flags texture_flags);

		/*!
		 * \brief Create a Cube Texture
		 *
		 * \param size Size of each Cube Maps face
		 * \param format Color Format to use
		 * \param mip_levels Number of Mip Levels available
		 * \param mip_data Texture data including mipmaps
		 * \param texture_flags Texture Flags
		 */
		texture(uint32_t size, gs_color_format format, uint32_t mip_levels, const uint8_t** mip_data,
				gs::texture::flags texture_flags);

		/*!
		* \brief Load a texture from a file
		*
		* Creates a new #GS::Texture from a file located on disk. If the
		* file can not be found, accessed or read, a #Plugin::file_not_found_error
		* will be thrown. If there is an error reading the file, a
		* #Plugin::io_error will be thrown.
		*
		* \param file File to create the texture from.
		*/
		texture(std::string file);

		/*!
		* \brief Create a texture from an existing gs_texture_t object.
		*/
		texture(gs_texture_t* tex, bool takeOwnership = false) : m_texture(tex), m_isOwner(takeOwnership) {}

		void load(int unit);

		gs_texture_t* get_object();

		uint32_t get_width();

		uint32_t get_height();

		uint32_t get_depth();

		gs::texture::type get_type();

		gs_color_format get_color_format();
	};

	ENABLE_BITMASK_OPERATORS(gs::texture::flags)
} // namespace gs
