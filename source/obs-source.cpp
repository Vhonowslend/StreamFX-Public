/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2018 Michael Fabian Dirks
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

#include "obs-source.hpp"

void obs::source::handle_destroy(void* p, calldata_t* calldata){
	obs::source* self = reinterpret_cast<obs::source*>(p);
	obs_source_t* source;
	if (calldata_get_ptr(calldata, "source", &source)) {
		if (self->self == source) {
			self->self = nullptr;
		}
	}
}

obs::source::~source()
{
	auto sh = obs_source_get_signal_handler(this->self);
	signal_handler_disconnect(sh, "destroy", obs::source::handle_destroy, this);
	if (this->track_ownership && this->self) {
		obs_source_release(this->self);
	}
	this->self = nullptr;
}

obs::source::source(std::string name, bool track_ownership, bool add_reference)
{
	this->self            = obs_get_source_by_name(name.c_str());
	this->track_ownership = track_ownership;
	if (!add_reference) {
		obs_source_release(this->self);
	}
	auto sh = obs_source_get_signal_handler(this->self);
	signal_handler_connect(sh, "destroy", obs::source::handle_destroy, this);
}

obs::source::source(obs_source_t* source, bool track_ownership, bool add_reference)
{
	this->self            = source;
	this->track_ownership = track_ownership;
	if (add_reference) {
		obs_source_addref(this->self);
	}
	auto sh = obs_source_get_signal_handler(this->self);
	signal_handler_connect(sh, "destroy", obs::source::handle_destroy, this);
}

obs::source& obs::source::operator=(const source& ref)
{
	if (this != &ref) {
		if (self) {
			if (track_ownership) {
				obs_source_release(self);
			}
		}
		self = ref.self;
		track_ownership = ref.track_ownership;
		if (track_ownership) {
			obs_source_addref(self);
		}
	}
	return *this;
}

obs::source& obs::source::operator=(source&& ref) noexcept
{
	if (this != &ref) {
		self     = ref.self;
		ref.self = nullptr;
	}
	return *this;
}

obs_source_type obs::source::type()
{
	if (!self) {
		return (obs_source_type)-1;
	}
	return obs_source_get_type(self);
}

void* obs::source::type_data()
{
	if (!self) {
		return nullptr;
	}
	return obs_source_get_type_data(self);
}

uint32_t obs::source::width()
{
	if (!self) {
		return 0;
	}
	return obs_source_get_width(self);
}

uint32_t obs::source::height()
{
	if (!self) {
		return 0;
	}
	return obs_source_get_height(self);
}

bool obs::source::destroyed()
{
	return self == nullptr;
}

void obs::source::clear()
{
	self = nullptr;
}

obs_source_t* obs::source::get()
{
	return self;
}
