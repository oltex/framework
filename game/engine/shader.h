#pragma once
#include "../library/design-pattern/singleton.h"
#include "graphic.h"
#include <d3dcompiler.h>
#include <unordered_map>
#include <string>

namespace engine {
	class shader final {
	public:
		inline explicit shader(void) noexcept {
			//D3D11_INPUT_ELEMENT_DESC desc[]{
			//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			//	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			//};
			//unsigned int size = static_cast<unsigned int>(sizeof(desc) / sizeof(D3D11_INPUT_ELEMENT_DESC));
			//D3DReadFileToBlob(L"vertex.cso", &binary);
			//graphic.get_device().CreateInputLayout(desc, size, binary->GetBufferPointer(), binary->GetBufferSize(), &layout);
			//binary->Release();
			//_layout.emplace(layout);
		}
		inline explicit shader(shader const& rhs) noexcept = delete;
		inline auto operator=(shader const& rhs) noexcept -> shader & = delete;
		inline explicit shader(shader&& rhs) noexcept = delete;
		inline auto operator=(shader&& rhs) noexcept -> shader & = delete;
		inline ~shader(void) noexcept {
			for (auto& iter : _rasterizer)
				iter.second->Release();
			for (auto& iter : _depth_stencil)
				iter.second->Release();
			for (auto& iter : _blend)
				iter.second->Release();

			for (auto& iter : _layout)
				iter.second->Release();
			for (auto& iter : _pixel)
				iter.second->Release();
			for (auto& iter : _vertex)
				iter.second->Release();
		}
	public:
		inline void create_vertex(std::string key, std::wstring path) noexcept {
			ID3D10Blob* binary;
			D3DReadFileToBlob(path.c_str(), &binary);
			ID3D11VertexShader* vertex;
			graphic::instance().get_device().CreateVertexShader(binary->GetBufferPointer(), binary->GetBufferSize(), nullptr, &vertex);
			binary->Release();
			_vertex.emplace(key, vertex);
		}
		inline void create_pixel(std::string key, std::wstring path) noexcept {
			ID3D10Blob* binary;
			D3DReadFileToBlob(path.c_str(), &binary);
			ID3D11PixelShader* pixel;
			graphic::instance().get_device().CreatePixelShader(binary->GetBufferPointer(), binary->GetBufferSize(), nullptr, &pixel);
			binary->Release();
			_pixel.emplace(key, pixel);
		}
		inline auto find_vertex(std::string key) noexcept -> ID3D11VertexShader* {
			auto iter = _vertex.find(key);
			return iter->second;
		}
		inline auto find_pixel(std::string key) noexcept -> ID3D11VertexShader* {
			auto iter = _vertex.find(key);
			return iter->second;
		}
	private:
		std::unordered_map<std::string, ID3D11VertexShader*> _vertex;
		std::unordered_map<std::string, ID3D11PixelShader*> _pixel;


		//ID3D11DepthStencilView
		//ID3D11RenderTargetView
		//ID3D11UnorderedAccessView
		//ID3D11ShaderResourceView

		//D3D11_VIEWPORT

		//ID3D11BlendState
		//ID3D11DepthStencilState
		//ID3D11RasterizerState
		
		//ID3D11InputLayout

		//ID3D11VertexShader
		//ID3D11PixelShader

		std::unordered_map<std::string, ID3D11BlendState*> _blend;
		std::unordered_map<std::string, ID3D11DepthStencilState*> _depth_stencil;
		std::unordered_map<std::string, ID3D11RasterizerState*> _rasterizer;


		std::unordered_map<std::string, ID3D11InputLayout*> _layout;
	};
}