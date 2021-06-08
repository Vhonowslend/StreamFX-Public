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

#ifdef WIN32

#include "d3d11.hpp"
#include <sstream>
#include <vector>
#include "obs/gs/gs-helper.hpp"

extern "C" {
#pragma warning(push)
#pragma warning(disable : 4191)
#pragma warning(disable : 4242)
#pragma warning(disable : 4244)
#pragma warning(disable : 4365)
#pragma warning(disable : 4986)
#include <libavutil/hwcontext_d3d11va.h>
#pragma warning(pop)
}

using namespace streamfx::ffmpeg::hwapi;

d3d11::d3d11() : _dxgi_module(0), _d3d11_module(0)
{
	_dxgi_module = LoadLibraryW(L"dxgi.dll");
	if (!_dxgi_module)
		throw std::runtime_error("Unable to load DXGI");

	_d3d11_module = LoadLibraryW(L"d3d11.dll");
	if (!_d3d11_module)
		throw std::runtime_error("Unable to load D3D11");

#pragma warning(push)
#pragma warning(disable : 4191)
	_CreateDXGIFactory  = reinterpret_cast<CreateDXGIFactory_t>(GetProcAddress(_dxgi_module, "CreateDXGIFactory"));
	_CreateDXGIFactory1 = reinterpret_cast<CreateDXGIFactory1_t>(GetProcAddress(_dxgi_module, "CreateDXGIFactory1"));
	_D3D11CreateDevice  = reinterpret_cast<D3D11CreateDevice_t>(GetProcAddress(_d3d11_module, "D3D11CreateDevice"));
#pragma warning(pop)

	if (!_CreateDXGIFactory && !_CreateDXGIFactory1)
		throw std::runtime_error("DXGI not supported");

	if (!_D3D11CreateDevice)
		throw std::runtime_error("D3D11 not supported");

	HRESULT hr = _CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&_dxgifactory);
	if (FAILED(hr)) {
		std::stringstream sstr;
		sstr << "Failed to create DXGI Factory (" << hr << ")";
		throw std::runtime_error(sstr.str());
	}
}

d3d11::~d3d11()
{
	FreeLibrary(_dxgi_module);
	FreeLibrary(_d3d11_module);
}

std::list<device> d3d11::enumerate_adapters()
{
	std::list<device> adapters;

	// Enumerate Adapters
	IDXGIAdapter1* dxgi_adapter = nullptr;
	for (UINT idx = 0; !FAILED(_dxgifactory->EnumAdapters1(idx, &dxgi_adapter)); idx++) {
		DXGI_ADAPTER_DESC1 desc = DXGI_ADAPTER_DESC1();
		dxgi_adapter->GetDesc1(&desc);

		std::vector<char> buf(1024);
		std::size_t       len =
			static_cast<size_t>(snprintf(buf.data(), buf.size(), "%ls (VEN_%04x/DEV_%04x/SUB_%04x/REV_%04x)",
										 desc.Description, desc.VendorId, desc.DeviceId, desc.SubSysId, desc.Revision));

		device dev;
		dev.name      = std::string(buf.data(), buf.data() + len);
		dev.id.first  = desc.AdapterLuid.HighPart;
		dev.id.second = desc.AdapterLuid.LowPart;

		adapters.push_back(dev);
	}

	return std::move(adapters);
}

std::shared_ptr<instance> d3d11::create(device target)
{
	std::shared_ptr<d3d11_instance>   inst;
	ATL::CComPtr<ID3D11Device>        device;
	ATL::CComPtr<ID3D11DeviceContext> context;
	IDXGIAdapter1*                    adapter = nullptr;

	// Find the correct "Adapter" (device).
	IDXGIAdapter1* dxgi_adapter = nullptr;
	for (UINT idx = 0; !FAILED(_dxgifactory->EnumAdapters1(idx, &dxgi_adapter)); idx++) {
		DXGI_ADAPTER_DESC1 desc = DXGI_ADAPTER_DESC1();
		dxgi_adapter->GetDesc1(&desc);

		if ((desc.AdapterLuid.LowPart == target.id.second) && (desc.AdapterLuid.HighPart == target.id.first)) {
			adapter = dxgi_adapter;
			break;
		}
	}

	// Create a D3D11 Device
	UINT                           device_flags   = D3D11_CREATE_DEVICE_VIDEO_SUPPORT;
	std::vector<D3D_FEATURE_LEVEL> feature_levels = {D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0,
													 D3D_FEATURE_LEVEL_11_1};

	if (FAILED(_D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_HARDWARE, NULL, device_flags, feature_levels.data(),
								  static_cast<UINT>(feature_levels.size()), D3D11_SDK_VERSION, &device, NULL,
								  &context))) {
		throw std::runtime_error("Failed to create D3D11 device for target.");
	}

	return std::make_shared<d3d11_instance>(device, context);
}

std::shared_ptr<instance> d3d11::create_from_obs()
{
	auto gctx = streamfx::obs::gs::context();

	if (GS_DEVICE_DIRECT3D_11 != gs_get_device_type()) {
		throw std::runtime_error("OBS Device is not a D3D11 Device.");
	}

	ATL::CComPtr<ID3D11Device> device =
		ATL::CComPtr<ID3D11Device>(reinterpret_cast<ID3D11Device*>(gs_get_device_obj()));
	ATL::CComPtr<ID3D11DeviceContext> context;
	device->GetImmediateContext(&context);

	return std::make_shared<d3d11_instance>(device, context);
}

struct D3D11AVFrame {
	ATL::CComPtr<ID3D11Texture2D> handle;
};

d3d11_instance::d3d11_instance(ATL::CComPtr<ID3D11Device> device, ATL::CComPtr<ID3D11DeviceContext> context)
{
	_device  = device;
	_context = context;
}

d3d11_instance::~d3d11_instance() {}

AVBufferRef* d3d11_instance::create_device_context()
{
	AVBufferRef* dctx_ref = av_hwdevice_ctx_alloc(AV_HWDEVICE_TYPE_D3D11VA);
	if (!dctx_ref)
		throw std::runtime_error("Failed to allocate AVHWDeviceContext.");

	AVHWDeviceContext*      dctx    = reinterpret_cast<AVHWDeviceContext*>(dctx_ref->data);
	AVD3D11VADeviceContext* d3d11va = reinterpret_cast<AVD3D11VADeviceContext*>(dctx->hwctx);

	// TODO: Determine if these need an additional reference.
	d3d11va->device = _device;
	d3d11va->device->AddRef();
	d3d11va->device_context = _context;
	d3d11va->device_context->AddRef();
	d3d11va->lock   = [](void*) { obs_enter_graphics(); };
	d3d11va->unlock = [](void*) { obs_leave_graphics(); };

	int ret = av_hwdevice_ctx_init(dctx_ref);
	if (ret < 0)
		throw std::runtime_error("Failed to initialize AVHWDeviceContext.");

	return dctx_ref;
}

std::shared_ptr<AVFrame> d3d11_instance::allocate_frame(AVBufferRef* frames)
{
	auto gctx = streamfx::obs::gs::context();

	// Allocate a frame.
	auto frame = std::shared_ptr<AVFrame>(av_frame_alloc(), [](AVFrame* frame) {
		av_frame_unref(frame);
		av_frame_free(&frame);
	});

	// Create the necessary buffers.
	if (av_hwframe_get_buffer(frames, frame.get(), 0) < 0) {
		throw std::runtime_error("Failed to create AVFrame.");
	}

	// Try to prevent this resource from ever leaving the GPU unless absolutely necessary.
	reinterpret_cast<ID3D11Texture2D*>(frame->data[0])->SetEvictionPriority(DXGI_RESOURCE_PRIORITY_MAXIMUM);

	return frame;
}

void d3d11_instance::copy_from_obs(AVBufferRef*, uint32_t handle, uint64_t lock_key, uint64_t* next_lock_key,
								   std::shared_ptr<AVFrame> frame)
{
	auto gctx = streamfx::obs::gs::context();

	// Attempt to acquire shared texture.
	ATL::CComPtr<ID3D11Texture2D> input;
	if (FAILED(_device->OpenSharedResource(reinterpret_cast<HANDLE>(static_cast<uintptr_t>(handle)),
										   __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&input)))) {
		throw std::runtime_error("Failed to open shared texture resource.");
	}

	// Attempt to acquire texture mutex.
	ATL::CComPtr<IDXGIKeyedMutex> mutex;
	if (FAILED(input->QueryInterface(__uuidof(IDXGIKeyedMutex), reinterpret_cast<void**>(&mutex)))) {
		throw std::runtime_error("Failed to retrieve mutex for texture resource.");
	}

	// Attempt to acquire texture lock.
	if (FAILED(mutex->AcquireSync(lock_key, 1000))) {
		throw std::runtime_error("Failed to acquire lock on input texture.");
	}

	// Set some parameters on the input texture, and get its description.
	UINT evict = input->GetEvictionPriority();
	input->SetEvictionPriority(DXGI_RESOURCE_PRIORITY_MAXIMUM);

	// Clone the content of the input texture.
	_context->CopyResource(reinterpret_cast<ID3D11Texture2D*>(frame->data[0]), input);

	// Restore original parameters on input.
	input->SetEvictionPriority(evict);

	// Release the acquired lock.
	if (FAILED(mutex->ReleaseSync(lock_key))) {
		throw std::runtime_error("Failed to release lock on input texture.");
	}

	// Release the lock on the next texture.
	// TODO: Determine if this is necessary.
	mutex->ReleaseSync(*next_lock_key);
}

std::shared_ptr<AVFrame> d3d11_instance::avframe_from_obs(AVBufferRef* frames, uint32_t handle, uint64_t lock_key,
														  uint64_t* next_lock_key)
{
	auto gctx = streamfx::obs::gs::context();

	auto frame = this->allocate_frame(frames);
	this->copy_from_obs(frames, handle, lock_key, next_lock_key, frame);
	return frame;
}

#endif
