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

#include "nvidia-cuda-gs-texture.hpp"
#include "obs/gs/gs-helper.hpp"

nvidia::cuda::gstexture::gstexture(std::shared_ptr<nvidia::cuda::cuda> cuda, std::shared_ptr<gs::texture> texture)
	: _cuda(cuda), _texture(texture), _resource(), _is_mapped(false), _pointer()
{
	if (!texture)
		throw std::invalid_argument("texture");
	if (!cuda)
		throw std::invalid_argument("cuda");

	gs::context gctx;
	int         dev_type = gs_get_device_type();

	if (dev_type == GS_DEVICE_OPENGL) {
		// ToDo
	}
#ifdef WIN32
	if (dev_type == GS_DEVICE_DIRECT3D_11) {
		ID3D11Resource* resource = nullptr;
		switch (_texture->get_type()) {
		case gs::texture::type::Cube:
		case gs::texture::type::Normal: {
			resource = static_cast<ID3D11Resource*>(gs_texture_get_obj(_texture->get_object()));
			break;
		}
		case gs::texture::type::Volume: {
			resource = static_cast<ID3D11Resource*>(gs_texture_get_obj(_texture->get_object()));
			break;
		}
		}

		if (!resource) {
			throw std::runtime_error("nvidia::cuda::gstexture: Failed to get resource from gs::texture.");
		}

		switch (_cuda->cuGraphicsD3D11RegisterResource(&_resource, resource, 0)) {
		case nvidia::cuda::result::SUCCESS:
			break;
		default:
			throw std::runtime_error("nvidia::cuda::gstexture: Failed to register resource.");
		}
	}
#endif
}

nvidia::cuda::gstexture::~gstexture()
{
	unmap();
	_cuda->cuGraphicsUnregisterResource(_resource);
}

nvidia::cuda::array_t nvidia::cuda::gstexture::map(std::shared_ptr<nvidia::cuda::stream> stream)
{
	if (_is_mapped) {
		return _pointer;
	}

	graphics_resource_t resources[] = {_resource};
	switch (_cuda->cuGraphicsMapResources(1, resources, stream->get())) {
	case nvidia::cuda::result::SUCCESS:
		break;
	default:
		throw std::runtime_error("nvidia::cuda::gstexture: Mapping failed.");
	}

	_stream    = stream;
	_is_mapped = true;

	switch (_cuda->cuGraphicsSubResourceGetMappedArray(&_pointer, _resource, 0, 0)) {
	case nvidia::cuda::result::SUCCESS:
		break;
	default:
		unmap();
		throw std::runtime_error("nvidia::cuda::gstexture: Mapping pointer failed.");
	}

	return _pointer;
}

void nvidia::cuda::gstexture::unmap()
{
	if (!_is_mapped)
		return;

	graphics_resource_t resources[] = {_resource};
	switch (_cuda->cuGraphicsUnmapResources(1, resources, _stream->get())) {
	case nvidia::cuda::result::SUCCESS:
		break;
	default:
		throw std::runtime_error("nvidia::cuda::gstexture: Unmapping failed.");
	}

	_is_mapped = false;
	_pointer   = nullptr;
	_stream.reset();
}
