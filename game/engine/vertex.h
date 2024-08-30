#pragma once
#include "graphic.h"
#include <d3dcompiler.h>

namespace engine {
	class vertex {
	public:
		inline explicit vertex(graphic& graphic,
			wchar_t const* const path, char const* const entry, unsigned int const flag, unsigned int flag2) noexcept {
			ID3D10Blob* code;
			D3DCompileFromFile(path, nullptr, nullptr, entry, "vs_5_0", flag, flag2, &code, nullptr);
			graphic.get_device().CreateVertexShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &_vertex_shader);
			code->Release();
		}
		inline ~vertex(void) noexcept {
			_vertex_shader->Release();
		}
		ID3D11VertexShader* _vertex_shader;
	};
}