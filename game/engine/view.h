#pragma once
#include "graphic.h"

namespace engine {
	class view final {
	public:
		inline explicit view(graphic& graphic) noexcept {
			graphic.get_device().CreateRenderTargetView
		}
		inline explicit view(view const& rhs) noexcept = delete;
		inline auto operator=(view const& rhs) noexcept -> view & = delete;
		inline explicit view(view&& rhs) noexcept = delete;
		inline auto operator=(view&& rhs) noexcept -> view & = delete;
		inline ~view(void) noexcept {

		}
	private:
		ID3D11RenderTargetView* pRTVs[8] = { nullptr };
		ID3D11View* _diffuse_render;
		ID3D11View* _diffuse_render;
		ID3D11View* _diffuse_render;
	};
}

//ZeroMemory(&Texture2D_Desc, sizeof(D3D11_TEXTURE2D_DESC));
//Texture2D_Desc.Width = static_cast<_uint>(Viewport.Width); Texture2D_Desc.Height = static_cast<_uint>(Viewport.Height);
//Texture2D_Desc.MipLevels = 1; Texture2D_Desc.ArraySize = 1; Texture2D_Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//Texture2D_Desc.SampleDesc.Count = 1; Texture2D_Desc.SampleDesc.Quality = 0; Texture2D_Desc.Usage = D3D11_USAGE_DEFAULT;
//Texture2D_Desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
//Texture2D_Desc.CPUAccessFlags = 0; Texture2D_Desc.MiscFlags = 0;
//hr |= Add_Target(TEXT("Diffuse"), CTarget::Create(m_pDevice, m_pContext, nullptr, &Texture2D_Desc, nullptr, nullptr, _float4{ 0.f, 0.f, 0.f, 0.f }));
//
///* For.Target_Diffuse */
//if (FAILED(m_pTarget_Manager->Add_RenderTarget(m_pDevice, m_pContext, TEXT("Target_Diffuse"), ViewPort.Width, ViewPort.Height, DXGI_FORMAT_B8G8R8A8_UNORM, _float4(1.f, 0.0f, 0.0f, 0.f))))
//return E_FAIL;
//
//m_vClearColor = vClearColor;
///* 렌더타겟을 생성한다. */
//
//
///* 1.텍스쳐를 생성하낟.  */
//
//
//D3D11_TEXTURE2D_DESC		TextureDesc;
//ZeroMemory(&TextureDesc, sizeof(D3D11_TEXTURE2D_DESC));
//
//TextureDesc.Width = iWidth;
//TextureDesc.Height = iHeight;
//TextureDesc.MipLevels = 1;
//TextureDesc.ArraySize = 1;
//TextureDesc.Format = eFormat;
//
//TextureDesc.SampleDesc.Quality = 0;
//TextureDesc.SampleDesc.Count = 1;
//
//TextureDesc.Usage = D3D11_USAGE_DEFAULT;
//TextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
//TextureDesc.CPUAccessFlags = 0;
//TextureDesc.MiscFlags = 0;
//
//if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &m_pTexture)))
//return E_FAIL;
//
///* 2.텍스쳐의 정보를 활용하여 렌더타겟 뷰를 생성한다.  */
//if (FAILED(m_pDevice->CreateRenderTargetView(m_pTexture, nullptr, &m_pRTV)))
//return E_FAIL;
//
///* 3.셰이더 리소스 뷰를 생성한다.   */
//if (FAILED(m_pDevice->CreateShaderResourceView(m_pTexture, nullptr, &m_pSRV)))
//return E_FAIL;
//
//return S_OK;