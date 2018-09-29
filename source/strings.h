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
#include "plugin.h"

#define P_TRANSLATE(x) obs_module_text(x)
#define P_DESC(x) x ".Description"

#define S_ADVANCED "Advanced"

namespace plugin {
	namespace strings {
		static const char* Advanced = "Advanced";
		namespace MipGenerator {
			static const char* Name        = "MipGenerator";
			static const char* Description = "MipGenerator.Description";
			static const char* Point       = "MipGenerator.Point";
			static const char* Linear      = "MipGenerator.Linear";
			static const char* Sharpen     = "MipGenerator.Sharpen";
			static const char* Smoothen    = "MipGenerator.Smoothen";
			static const char* Bicubic     = "MipGenerator.Bicubic";
			static const char* Lanczos     = "MipGenerator.Lanczos";
			static const char* Strength    = "MipGenerator.Strength";
		} // namespace MipGenerator

	} // namespace strings
} // namespace plugin
