#pragma once
#include "graphic.h"
#include <d3dcompiler.h>

namespace engine {
	class pixel {
	public:
		inline explicit pixel(graphic& graphic,
			wchar_t const* const path, char const* const entry, unsigned int const flag, unsigned int flag2) noexcept {
			ID3D10Blob* code;
			D3DCompileFromFile(path, nullptr, nullptr, entry, "ps_5_0", flag, flag2, &code, nullptr);
			graphic.get_device().CreatePixelShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &_pixelShader);
			code->Release();
		}
		ID3D11PixelShader* _pixelShader;
	};
}