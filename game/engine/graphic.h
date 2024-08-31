#pragma once
#pragma comment(lib,"d3d11.lib")
#include "../library/design-pattern/singleton.h"
#include "../library/window/window.h"
#include <d3d11.h>

namespace engine {
	class graphic final : public design_pattern::singleton<graphic, design_pattern::member_static<graphic>> {
		friend class singleton<graphic, design_pattern::member_static<graphic>>;
	public:
		inline static auto constructor(window::window& window) noexcept -> graphic& {
			_instance = new graphic(window);
			return *_instance;
		}
	private:
		inline explicit graphic(window::window& window) noexcept {
			unsigned int flag = 0;
#ifdef _DEBUG
			flag = D3D11_CREATE_DEVICE_DEBUG;
#endif
			D3D_FEATURE_LEVEL freature_level{};
			if (S_OK != D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, flag, nullptr, 0, D3D11_SDK_VERSION, &_device, &freature_level, &_context))
				DebugBreak();


			{	// swap chain
				IDXGIDevice* device;
				_device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&device));
				IDXGIAdapter* adapter;
				device->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&adapter));
				IDXGIFactory* factory;
				adapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&factory));
				DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
				swap_chain_desc.BufferDesc.Width = window.get_client_rect().right;
				swap_chain_desc.BufferDesc.Height = window.get_client_rect().bottom;
				swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
				swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
				swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
				swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
				swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
				swap_chain_desc.SampleDesc.Quality = 0;
				swap_chain_desc.SampleDesc.Count = 1;
				swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				swap_chain_desc.BufferCount = 1;
				swap_chain_desc.OutputWindow = window.data();
				swap_chain_desc.Windowed = true; //전체화면 모드 어떻게 얻는지 확인필요함
				swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
				if (S_OK != factory->CreateSwapChain(device, &swap_chain_desc, &_swap_chain))
					DebugBreak();
				factory->Release();
				adapter->Release();
				device->Release();
			}
			{	//render target view
				ID3D11Texture2D* texture = nullptr;
				_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&texture));
				_device->CreateRenderTargetView(texture, nullptr, (ID3D11RenderTargetView**)&_render_target_view);
				texture->Release();
			}
			{	// depth stencil view
				ID3D11Texture2D* texture = nullptr;
				D3D11_TEXTURE2D_DESC texture_desc{};
				texture_desc.Width = window.get_client_rect().right;
				texture_desc.Height = window.get_client_rect().bottom;
				texture_desc.MipLevels = 1;
				texture_desc.ArraySize = 1;
				texture_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
				texture_desc.SampleDesc.Count = 1;
				texture_desc.SampleDesc.Quality = 0;
				texture_desc.Usage = D3D11_USAGE_DEFAULT;
				texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
				texture_desc.CPUAccessFlags = 0;
				texture_desc.MiscFlags = 0;
				if (S_OK != _device->CreateTexture2D(&texture_desc, nullptr, &texture))
					DebugBreak();
				if (S_OK != _device->CreateDepthStencilView(texture, nullptr, &_depth_stencil_view))
					DebugBreak();
				texture->Release();
			}
		}
		inline explicit graphic(graphic const& rhs) noexcept = delete;
		inline auto operator=(graphic const& rhs) noexcept -> graphic & = delete;
		inline explicit graphic(graphic&& rhs) noexcept = delete;
		inline auto operator=(graphic&& rhs) noexcept -> graphic & = delete;
		inline ~graphic(void) noexcept {
			_depth_stencil_view->Release();
			_render_target_view->Release();
			_swap_chain->Release();

			_context->Release();
			_device->Release();
		}
	public:
		inline void begin_render(void) noexcept {
			_context->OMSetRenderTargets(1, (ID3D11RenderTargetView**)&_render_target_view, _depth_stencil_view);
			_context->ClearRenderTargetView((ID3D11RenderTargetView*)_render_target_view, _color);
			_context->ClearDepthStencilView(_depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
		}
		inline void end_render(void) noexcept {
			_swap_chain->Present(0, 0);
		}
	public:
		inline auto get_device(void) noexcept -> ID3D11Device& {
			return *_device;
		}
		inline auto get_context(void) noexcept -> ID3D11DeviceContext& {
			return *_context;
		}
	private:
		ID3D11Device* _device;
		ID3D11DeviceContext* _context;

		IDXGISwapChain* _swap_chain;
		ID3D11View* _render_target_view;
		ID3D11DepthStencilView* _depth_stencil_view;

		float _color[4] = { 0.f, 0.f, 0.5f, 1.f };
	};
}


//#include <DirectXMath.h>
//#include <DirectXPackedVector.h>
//#include <DirectXCollision.h>
//#include <d3dcompiler.h>
//d3d11.lib
//dinput8.lib
//dxguid.lib
//Effects11d.lib
//DirectXTKd.lib
//assimp - vc143 - mtd.lib
//fmod_vc.lib
//fmodL_vc.lib
