#pragma once
#include "../library/design-pattern/singleton.h"
#include "graphic.h"
#include <d3dcompiler.h>
#include <unordered_map>
#include <string>

namespace engine {
	class shader final {
	public:
		inline explicit shader(graphic& graphic) noexcept {
			ID3D10Blob* binary;
			ID3D11VertexShader* vertex;
			ID3D11PixelShader* pixel;
			ID3D11InputLayout* layout;

			D3DReadFileToBlob(L"vertex.cso", &binary);
			graphic.get_device().CreateVertexShader(binary->GetBufferPointer(), binary->GetBufferSize(), nullptr, &vertex);
			binary->Release();
			_vertex.emplace(vertex);

			D3DReadFileToBlob(L"pixel.cso", &binary);
			graphic.get_device().CreatePixelShader(binary->GetBufferPointer(), binary->GetBufferSize(), nullptr, &pixel);
			binary->Release();
			_pixel.emplace(pixel);

			D3D11_INPUT_ELEMENT_DESC desc[]{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			unsigned int size = static_cast<unsigned int>(sizeof(desc) / sizeof(D3D11_INPUT_ELEMENT_DESC));
			D3DReadFileToBlob(L"vertex.cso", &binary);
			graphic.get_device().CreateInputLayout(desc, size, binary->GetBufferPointer(), binary->GetBufferSize(), &layout);
			binary->Release();
			_layout.emplace(layout);
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
	private:
		std::unordered_map<std::string, ID3D11VertexShader*> _vertex;
		std::unordered_map<std::string, ID3D11PixelShader*> _pixel;
		std::unordered_map<std::string, ID3D11InputLayout*> _layout;

		std::unordered_map<std::string, ID3D11BlendState*> _blend;
		std::unordered_map<std::string, ID3D11DepthStencilState*> _depth_stencil;
		std::unordered_map<std::string, ID3D11RasterizerState*> _rasterizer;
	};
}