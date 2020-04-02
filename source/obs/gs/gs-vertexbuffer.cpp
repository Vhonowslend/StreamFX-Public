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

#include "gs-vertexbuffer.hpp"
#include <stdexcept>
#include "obs/gs/gs-helper.hpp"
#include "utility.hpp"

void gs::vertex_buffer::initialize(size_t capacity, size_t layers)
{
	if (capacity > MAXIMUM_VERTICES) {
		throw std::out_of_range("capacity too large");
	}
	if (layers > MAXIMUM_UVW_LAYERS) {
		throw std::out_of_range("too many layers");
	}

	// Allocate memory for data.
	_data          = gs_vbdata_create();
	_data->num     = _capacity;
	_data->num_tex = _layers;
	_data->points = _positions = (vec3*)util::malloc_aligned(16, sizeof(vec3) * _capacity);
	_data->normals = _normals = (vec3*)util::malloc_aligned(16, sizeof(vec3) * _capacity);
	_data->tangents = _tangents = (vec3*)util::malloc_aligned(16, sizeof(vec3) * _capacity);
	_data->colors = _colors = (uint32_t*)util::malloc_aligned(16, sizeof(uint32_t) * _capacity);
	if (_layers > 0) {
		_data->tvarray = _layer_data = (gs_tvertarray*)util::malloc_aligned(16, sizeof(gs_tvertarray) * _layers);
		for (size_t n = 0; n < _layers; n++) {
			_layer_data[n].array = _uvs[n] = (vec4*)util::malloc_aligned(16, sizeof(vec4) * _capacity);
			_layer_data[n].width           = 4;
			memset(_uvs[n], 0, sizeof(vec4) * _capacity);
		}
	} else {
		_data->tvarray = nullptr;
	}
}

gs::vertex_buffer::~vertex_buffer()
{
	if (_positions) {
		util::free_aligned(_positions);
		_positions = nullptr;
	}
	if (_normals) {
		util::free_aligned(_normals);
		_normals = nullptr;
	}
	if (_tangents) {
		util::free_aligned(_tangents);
		_tangents = nullptr;
	}
	if (_colors) {
		util::free_aligned(_colors);
		_colors = nullptr;
	}
	for (size_t n = 0; n < _layers; n++) {
		if (_uvs[n]) {
			util::free_aligned(_uvs[n]);
			_uvs[n] = nullptr;
		}
	}
	if (_layer_data) {
		util::free_aligned(_layer_data);
		_layer_data = nullptr;
	}
	if (_data) {
		memset(_data, 0, sizeof(gs_vb_data));
		if (!_buffer) {
			gs_vbdata_destroy(_data);
			_data = nullptr;
		}
	}
	if (_buffer) {
		auto gctx = gs::context();
		gs_vertexbuffer_destroy(_buffer);
		_buffer = nullptr;
	}
}

gs::vertex_buffer::vertex_buffer() : vertex_buffer(MAXIMUM_VERTICES, MAXIMUM_UVW_LAYERS) {}

gs::vertex_buffer::vertex_buffer(uint32_t vertices) : vertex_buffer(vertices, MAXIMUM_UVW_LAYERS) {}

gs::vertex_buffer::vertex_buffer(uint32_t vertices, uint8_t uvlayers)
	: _size(vertices), _capacity(vertices), _layers(uvlayers), _positions(nullptr), _normals(nullptr),
	  _tangents(nullptr), _colors(nullptr), _data(nullptr), _buffer(nullptr), _layer_data(nullptr)
{
	initialize(vertices, uvlayers);

	if (vertices > MAXIMUM_VERTICES) {
		throw std::out_of_range("vertices out of range");
	}
	if (uvlayers > MAXIMUM_UVW_LAYERS) {
		throw std::out_of_range("uvlayers out of range");
	}

	// Allocate GPU
	auto gctx = gs::context();
	_buffer   = gs_vertexbuffer_create(_data, GS_DYNAMIC);
	memset(_data, 0, sizeof(gs_vb_data));
	_data->num     = _capacity;
	_data->num_tex = _layers;
	if (!_buffer) {
		throw std::runtime_error("Failed to create vertex buffer.");
	}
}

// cppcheck-suppress uninitMemberVar
gs::vertex_buffer::vertex_buffer(gs_vertbuffer_t* vb)
	: _size(0), _capacity(0), _layers(0), _positions(nullptr), _normals(nullptr), _tangents(nullptr), _colors(nullptr),
	  _uvs(), _data(nullptr), _buffer(nullptr), _layer_data(nullptr)
{
	auto        gctx = gs::context();
	gs_vb_data* vbd  = gs_vertexbuffer_get_data(vb);
	if (!vbd)
		throw std::runtime_error("vertex buffer with no data");

	initialize(vbd->num, vbd->num_tex);

	if (_positions && vbd->points)
		memcpy(_positions, vbd->points, vbd->num * sizeof(vec3));
	if (_normals && vbd->normals)
		memcpy(_normals, vbd->normals, vbd->num * sizeof(vec3));
	if (_tangents && vbd->tangents)
		memcpy(_tangents, vbd->tangents, vbd->num * sizeof(vec3));
	if (_colors && vbd->colors)
		memcpy(_colors, vbd->colors, vbd->num * sizeof(uint32_t));
	if (vbd->tvarray != nullptr) {
		for (size_t n = 0; n < vbd->num_tex; n++) {
			if (vbd->tvarray[n].array != nullptr && vbd->tvarray[n].width <= 4 && vbd->tvarray[n].width > 0) {
				if (vbd->tvarray[n].width == 4) {
					memcpy(_uvs[n], vbd->tvarray[n].array, vbd->num * sizeof(vec4));
				} else if (vbd->tvarray[n].width < 4) {
					for (size_t idx = 0; idx < _capacity; idx++) {
						float* mem = reinterpret_cast<float*>(vbd->tvarray[n].array) + (idx * vbd->tvarray[n].width);
						// cppcheck-suppress memsetClassFloat
						memset(&_uvs[n][idx], 0, sizeof(vec4));
						memcpy(&_uvs[n][idx], mem, vbd->tvarray[n].width);
					}
				}
			}
		}
	}
}

// cppcheck-suppress uninitMemberVar
gs::vertex_buffer::vertex_buffer(vertex_buffer const& other) : vertex_buffer(other._capacity)
{
	// Copy Constructor
	memcpy(_positions, other._positions, _capacity * sizeof(vec3));
	memcpy(_normals, other._normals, _capacity * sizeof(vec3));
	memcpy(_tangents, other._tangents, _capacity * sizeof(vec3));
	memcpy(_colors, other._colors, _capacity * sizeof(vec3));
	for (size_t n = 0; n < MAXIMUM_UVW_LAYERS; n++) {
		memcpy(_uvs[n], other._uvs[n], _capacity * sizeof(vec3));
	}
}

gs::vertex_buffer::vertex_buffer(vertex_buffer const&& other) noexcept : _uvs()
{
	// Move Constructor
	_capacity  = other._capacity;
	_size      = other._size;
	_layers    = other._layers;
	_positions = other._positions;
	_normals   = other._normals;
	_tangents  = other._tangents;
	_colors    = other._colors;
	for (size_t n = 0; n < MAXIMUM_UVW_LAYERS; n++) {
		_uvs[n] = other._uvs[n];
	}
	_data       = other._data;
	_buffer     = other._buffer;
	_layer_data = other._layer_data;
}

void gs::vertex_buffer::operator=(vertex_buffer const&& other) noexcept
{
	// Move Assignment
	/// First self-destruct (semi-destruct itself).
	if (_positions) {
		util::free_aligned(_positions);
		_positions = nullptr;
	}
	if (_normals) {
		util::free_aligned(_normals);
		_normals = nullptr;
	}
	if (_tangents) {
		util::free_aligned(_tangents);
		_tangents = nullptr;
	}
	if (_colors) {
		util::free_aligned(_colors);
		_colors = nullptr;
	}
	for (size_t n = 0; n < MAXIMUM_UVW_LAYERS; n++) {
		if (_uvs[n]) {
			util::free_aligned(_uvs[n]);
			_uvs[n] = nullptr;
		}
	}
	if (_layer_data) {
		util::free_aligned(_layer_data);
		_layer_data = nullptr;
	}
	if (_data) {
		memset(_data, 0, sizeof(gs_vb_data));
		if (!_buffer) {
			gs_vbdata_destroy(_data);
			_data = nullptr;
		}
	}
	if (_buffer) {
		auto gctx = gs::context();
		gs_vertexbuffer_destroy(_buffer);
		_buffer = nullptr;
	}

	/// Then assign new values.
	_capacity  = other._capacity;
	_size      = other._size;
	_layers    = other._layers;
	_positions = other._positions;
	_normals   = other._normals;
	_tangents  = other._tangents;
	for (size_t n = 0; n < MAXIMUM_UVW_LAYERS; n++) {
		_uvs[n] = other._uvs[n];
	}
	_data       = other._data;
	_buffer     = other._buffer;
	_layer_data = other._layer_data;
}

void gs::vertex_buffer::resize(uint32_t new_size)
{
	if (new_size > _capacity) {
		throw std::out_of_range("new_size out of range");
	}
	_size = new_size;
}

uint32_t gs::vertex_buffer::size()
{
	return _size;
}

bool gs::vertex_buffer::empty()
{
	return _size == 0;
}

const gs::vertex gs::vertex_buffer::at(uint32_t idx)
{
	if (idx >= _size) {
		throw std::out_of_range("idx out of range");
	}

	gs::vertex vtx(&_positions[idx], &_normals[idx], &_tangents[idx], &_colors[idx], nullptr);
	for (size_t n = 0; n < _layers; n++) {
		vtx.uv[n] = &_uvs[n][idx];
	}
	return vtx;
}

const gs::vertex gs::vertex_buffer::operator[](uint32_t const pos)
{
	return at(pos);
}

void gs::vertex_buffer::set_uv_layers(uint32_t layers)
{
	_layers = layers;
}

uint32_t gs::vertex_buffer::get_uv_layers()
{
	return _layers;
}

vec3* gs::vertex_buffer::get_positions()
{
	return _positions;
}

vec3* gs::vertex_buffer::get_normals()
{
	return _normals;
}

vec3* gs::vertex_buffer::get_tangents()
{
	return _tangents;
}

uint32_t* gs::vertex_buffer::get_colors()
{
	return _colors;
}

vec4* gs::vertex_buffer::get_uv_layer(size_t idx)
{
	if (idx >= _layers) {
		throw std::out_of_range("idx out of range");
	}
	return _uvs[idx];
}

gs_vertbuffer_t* gs::vertex_buffer::update(bool refreshGPU)
{
	if (!refreshGPU)
		return _buffer;

	if (_size > _capacity)
		throw std::out_of_range("size is larger than capacity");

	// Update VertexBuffer data.
	auto gctx = gs::context();
	_data     = gs_vertexbuffer_get_data(_buffer);
	memset(_data, 0, sizeof(gs_vb_data));
	_data->num      = _capacity;
	_data->points   = _positions;
	_data->normals  = _normals;
	_data->tangents = _tangents;
	_data->colors   = _colors;
	_data->num_tex  = _layers;
	_data->tvarray  = _layer_data;
	for (size_t n = 0; n < _layers; n++) {
		_layer_data[n].array = _uvs[n];
		_layer_data[n].width = 4;
	}

	// Update GPU
	gs_vertexbuffer_flush(_buffer);

	// WORKAROUND: OBS Studio 20.x and below incorrectly deletes data that it doesn't own.
	memset(_data, 0, sizeof(gs_vb_data));
	_data->num     = _capacity;
	_data->num_tex = _layers;
	for (uint32_t n = 0; n < _layers; n++) {
		_layer_data[n].width = 4;
	}

	return _buffer;
}

gs_vertbuffer_t* gs::vertex_buffer::update()
{
	return update(true);
}
