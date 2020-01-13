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

#define PLUGIN_NAME "StreamFX"

#define D_TRANSLATE(x) obs_module_text(x)
#define D_DESC(x) x ".Description"

#define S_FILEFILTERS_IMAGE "*.png *.webp *.tga *.tiff *.jpeg *.jpg *.bmp"
#define S_FILEFILTERS_TEXTURE S_FILEFILTERS_IMAGE " *.dds"
#define S_FILEFILTERS_VIDEO "*.mkv *.webm *.mp4 *.mov *.flv"
#define S_FILEFILTERS_SOUND "*.ogg *.flac *.mp3 *.wav"
#define S_FILEFILTERS_EFFECT "*.effect *.txt"
#define S_FILEFILTERS_ANY "*.*"

#define S_VERSION "Version"
#define S_COMMIT "Commit"

#define S_ADVANCED "Advanced"

#define S_STATE_DEFAULT "State.Default"
#define S_STATE_DISABLED "State.Disabled"
#define S_STATE_ENABLED "State.Enabled"
#define S_STATE_MANUAL "State.Manual"
#define S_STATE_AUTOMATIC "State.Automatic"

#define S_FILETYPE_IMAGE "FileType.Image"
#define S_FILETYPE_IMAGES "FileType.Images"
#define S_FILETYPE_VIDEO "FileType.Video"
#define S_FILETYPE_VIDEOS "FileType.Videos"
#define S_FILETYPE_SOUND "FileType.Sound"
#define S_FILETYPE_SOUNDS "FileType.Sounds"
#define S_FILETYPE_EFFECT "FileType.Effect"
#define S_FILETYPE_EFFECTS "FileType.Effects"

#define S_SOURCETYPE_SOURCE "SourceType.Source"
#define S_SOURCETYPE_SCENE "SourceType.Scene"

#define S_ALIGNMENT_CENTER "Alignment.Center"
#define S_ALIGNMENT_LEFT "Alignment.Left"
#define S_ALIGNMENT_RIGHT "Alignment.Right"
#define S_ALIGNMENT_TOP "Alignment.Top"
#define S_ALIGNMENT_BOTTOM "Alignment.Bottom"

#define S_BLUR_TYPE_BOX "Blur.Type.Box"
#define S_BLUR_TYPE_BOX_LINEAR "Blur.Type.BoxLinear"
#define S_BLUR_TYPE_GAUSSIAN "Blur.Type.Gaussian"
#define S_BLUR_TYPE_GAUSSIAN_LINEAR "Blur.Type.GaussianLinear"
#define S_BLUR_TYPE_DUALFILTERING "Blur.Type.DualFiltering"

#define S_BLUR_SUBTYPE_AREA "Blur.Subtype.Area"
#define S_BLUR_SUBTYPE_DIRECTIONAL "Blur.Subtype.Directional"
#define S_BLUR_SUBTYPE_ROTATIONAL "Blur.Subtype.Rotational"
#define S_BLUR_SUBTYPE_ZOOM "Blur.Subtype.Zoom"

#define S_MIPGENERATOR "MipGenerator"
#define S_MIPGENERATOR_POINT "MipGenerator.Point"
#define S_MIPGENERATOR_LINEAR "MipGenerator.Linear"
#define S_MIPGENERATOR_SHARPEN "MipGenerator.Sharpen"
#define S_MIPGENERATOR_SMOOTHEN "MipGenerator.Smoothen"
#define S_MIPGENERATOR_BICUBIC "MipGenerator.Bicubic"
#define S_MIPGENERATOR_LANCZOS "MipGenerator.Lanczos"
#define S_MIPGENERATOR_INTENSITY "MipGenerator.Intensity"

#define S_CHANNEL_RED "Channel.Red"
#define S_CHANNEL_GREEN "Channel.Green"
#define S_CHANNEL_BLUE "Channel.Blue"
#define S_CHANNEL_ALPHA "Channel.Alpha"
