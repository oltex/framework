#pragma once
#include "graphic.h"
#include <d3d11.h>

namespace engine {
	class layout final {
	public:
		inline explicit layout(graphic& graphic) noexcept {
			D3D11_INPUT_ELEMENT_DESC desc[] {
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			unsigned int size  = static_cast<unsigned int>(sizeof(desc) / sizeof(D3D11_INPUT_ELEMENT_DESC));

			int a = graphic.get_device().CreateInputLayout(desc, size, nullptr, 0, &_layout);
			int b = 10;
		}
	private:
		ID3D11InputLayout* _layout;
	};
}