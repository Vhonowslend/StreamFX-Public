// Modern effects for a modern Streamer
// Copyright (C) 2019 Michael Fabian Dirks
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

#include "gfx-blur-base.hpp"

#include "warning-disable.hpp"
#include <stdexcept>
#include "warning-enable.hpp"

void streamfx::gfx::blur::base::set_step_scale_x(double_t v)
{
	this->set_step_scale(v, this->get_step_scale_y());
}

void streamfx::gfx::blur::base::set_step_scale_y(double_t v)
{
	this->set_step_scale(this->get_step_scale_x(), v);
}

double_t streamfx::gfx::blur::base::get_step_scale_x()
{
	double_t x, y;
	this->get_step_scale(x, y);
	return x;
}

double_t streamfx::gfx::blur::base::get_step_scale_y()
{
	double_t x, y;
	this->get_step_scale(x, y);
	return y;
}

void streamfx::gfx::blur::base_center::set_center_x(double_t v)
{
	this->set_center(v, this->get_center_y());
}

void streamfx::gfx::blur::base_center::set_center_y(double_t v)
{
	this->set_center(this->get_center_x(), v);
}

double_t streamfx::gfx::blur::base_center::get_center_x()
{
	double_t x, y;
	this->get_center(x, y);
	return x;
}

double_t streamfx::gfx::blur::base_center::get_center_y()
{
	double_t x, y;
	this->get_center(x, y);
	return y;
}
