#pragma once
#include "graphic.h"
#include <d3dcompiler.h>

namespace engine {
	class pixel {
	public:
		inline explicit pixel(graphic& graphic, wchar_t const* const path) noexcept
			: _context(graphic.get_context()) {
			ID3D10Blob* code;
			D3DReadFileToBlob(path, &code);
			graphic.get_device().CreatePixelShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &_pixel_shader);
			code->Release();
		}
		inline ~pixel(void) noexcept {
			_pixel_shader->Release();
		}
	public:

	private:
		ID3D11DeviceContext& _context;
		ID3D11PixelShader* _pixel_shader;
	};
}