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
#include "plugin.hpp"

#define P_TRANSLATE(x) obs_module_text(x)
#define P_DESC(x) x ".Description"

#define S_ADVANCED "Advanced"
#define S_FILEFILTERS_IMAGES "FileFilters.Images"

#define S_MIPGENERATOR "MipGenerator"
#define S_MIPGENERATOR_POINT "MipGenerator.Point"
#define S_MIPGENERATOR_LINEAR "MipGenerator.Linear"
#define S_MIPGENERATOR_SHARPEN "MipGenerator.Sharpen"
#define S_MIPGENERATOR_SMOOTHEN "MipGenerator.Smoothen"
#define S_MIPGENERATOR_BICUBIC "MipGenerator.Bicubic"
#define S_MIPGENERATOR_LANCZOS "MipGenerator.Lanczos"
#define S_MIPGENERATOR_STRENGTH "MipGenerator.Strength"
