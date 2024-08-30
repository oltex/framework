#pragma once
#include "graphic.h"
#include <d3d11.h>

namespace engine {
	class layout final {
	public:
		inline explicit layout(graphic& graphic, D3D11_INPUT_ELEMENT_DESC* const desc, unsigned int const size, wchar_t const* const path) noexcept
			: _context(graphic.get_context()) {
			ID3D10Blob* code;
			D3DReadFileToBlob(path, &code);

			D3D11_INPUT_ELEMENT_DESC desc2[]{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			HRESULT hr = graphic.get_device().CreateInputLayout(desc2, 2, code->GetBufferPointer(), code->GetBufferSize(), &_layout);
			code.re

		}
		inline ~layout(void) noexcept {
			_layout->Release();
		}
	public:
		inline void set(void) noexcept {
			_context.IASetInputLayout(_layout);
		}
	private:
		ID3D11DeviceContext& _context;
		ID3D11InputLayout* _layout;
	};
}