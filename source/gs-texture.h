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
#include <inttypes.h>
#include <string>
extern "C" {
#pragma warning( push )
#pragma warning( disable: 4201 )
#include <libobs/graphics/graphics.h>
#pragma warning( pop )
}

namespace GS {
	class Texture {
		protected:
		gs_texture_t * m_texture;
		bool m_isOwner = true;

		public:
		enum Type : uint8_t {
			Normal,
			Volume,
			Cube
		};

		enum Flags : uint32_t {
			Dynamic,
			BuildMipMaps,
		};

		public:
		/*!
		 * \brief Create a new texture from data
		 *
		 *
		 *
		 * \param width
		 * \param height
		 * \param format
		 * \param mip_levels
		 * \param mip_data
		 * \param flags
		 */
		Texture(uint32_t width, uint32_t height, gs_color_format format, uint32_t mip_levels,
			const uint8_t **mip_data, uint32_t flags);

		/*!
		 * \brief Create a new volume texture from data
		 *
		 *
		 *
		 * \param width
		 * \param height
		 * \param depth
		 * \param format
		 * \param mip_levels
		 * \param mip_data
		 * \param flags
		 */
		Texture(uint32_t width, uint32_t height, uint32_t depth, gs_color_format format, uint32_t mip_levels,
			const uint8_t **mip_data, uint32_t flags);

		/*!
		 * \brief Create a new cube texture from data
		 *
		 *
		 *
		 * \param size
		 * \param format
		 * \param mip_levels
		 * \param mip_data
		 * \param flags
		 */
		Texture(uint32_t size, gs_color_format format, uint32_t mip_levels, const uint8_t **mip_data,
			uint32_t flags);

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
		Texture(std::string file);

		/*!
		* \brief Create a texture from an existing gs_texture_t object.
		*/
		Texture(gs_texture_t* tex, bool takeOwnership = false) : m_texture(tex), m_isOwner(takeOwnership) {}

		/*!
		* \brief Copy an existing texture
		*
		* Create a Texture instance from an existing texture.
		* This will not take ownership of the underlying gs_texture_t object.
		*
		* \param other
		* \return
		*/
		Texture(Texture& other);

		/*!
		* \brief Default constructor
		*/
		Texture() : m_texture(nullptr) {}

		/*!
		 * \brief Destructor
		 *
		 *
		 *
		 * \return
		 */
		virtual ~Texture();

		/*!
		 * \brief
		 *
		 *
		 *
		 * \param unit
		 * \return void
		 */
		void Load(int unit);

		/*!
		 * \brief
		 *
		 *
		 *
		 * \return gs_texture_t*
		 */
		gs_texture_t* GetObject();
	};
}
