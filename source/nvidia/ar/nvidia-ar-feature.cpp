/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2020 Michael Fabian Dirks
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

#include "nvidia-ar-feature.hpp"

nvidia::ar::feature::feature(std::shared_ptr<::nvidia::ar::ar> ar, NvAR_FeatureID feature) : _ar(ar)
{
	NvAR_FeatureHandle feat;
	if (NvCV_Status res = _ar->create(feature, &feat); res != NVCV_SUCCESS) {
		throw std::runtime_error("Failed to create feature.");
	}

	_feature = std::shared_ptr<nvAR_Feature>{feat, [this](NvAR_FeatureHandle v) { _ar->destroy(v); }};
}

nvidia::ar::feature::~feature()
{
	_feature.reset();
}
