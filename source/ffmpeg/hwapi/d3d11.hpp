// FFMPEG Video Encoder Integration for OBS Studio
// Copyright (c) 2019 Michael Fabian Dirks <info@xaymar.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once
#include "base.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4191 4242 4244 4365 4777 4986 5039 5204)
#endif
#include <atlutil.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <dxgi.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace streamfx::ffmpeg::hwapi {
	class d3d11 : public streamfx::ffmpeg::hwapi::base {
		typedef HRESULT(__stdcall* CreateDXGIFactory_t)(REFIID, void**);
		typedef HRESULT(__stdcall* CreateDXGIFactory1_t)(REFIID, void**);
		typedef HRESULT(__stdcall* D3D11CreateDevice_t)(IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT,
														CONST D3D_FEATURE_LEVEL*, UINT, UINT, ID3D11Device**,
														D3D_FEATURE_LEVEL*, ID3D11DeviceContext**);

		HMODULE              _dxgi_module;
		CreateDXGIFactory_t  _CreateDXGIFactory;
		CreateDXGIFactory1_t _CreateDXGIFactory1;

		HMODULE             _d3d11_module;
		D3D11CreateDevice_t _D3D11CreateDevice;

		ATL::CComPtr<IDXGIFactory1> _dxgifactory;

		public:
		d3d11();
		virtual ~d3d11();

		virtual std::list<hwapi::device> enumerate_adapters() override;

		virtual std::shared_ptr<hwapi::instance> create(hwapi::device target) override;

		virtual std::shared_ptr<hwapi::instance> create_from_obs() override;
	};

	class d3d11_instance : public streamfx::ffmpeg::hwapi::instance {
		ATL::CComPtr<ID3D11Device>        _device;
		ATL::CComPtr<ID3D11DeviceContext> _context;

		public:
		d3d11_instance(ATL::CComPtr<ID3D11Device> device, ATL::CComPtr<ID3D11DeviceContext> context);
		virtual ~d3d11_instance();

		virtual AVBufferRef* create_device_context() override;

		virtual std::shared_ptr<AVFrame> allocate_frame(AVBufferRef* frames) override;

		virtual void copy_from_obs(AVBufferRef* frames, uint32_t handle, uint64_t lock_key, uint64_t* next_lock_key,
								   std::shared_ptr<AVFrame> frame) override;

		virtual std::shared_ptr<AVFrame> avframe_from_obs(AVBufferRef* frames, uint32_t handle, uint64_t lock_key,
														  uint64_t* next_lock_key) override;
	};
} // namespace streamfx::ffmpeg::hwapi
