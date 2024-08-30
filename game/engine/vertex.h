#pragma once
#include "graphic.h"
#include <d3dcompiler.h>
#include <vector>

namespace engine {
	class vertex {
	public:
		inline explicit vertex(graphic& graphic, wchar_t const* const path) noexcept
			: _context(graphic.get_context()) {
			ID3D10Blob* code;
			D3DReadFileToBlob(path, &code);
			//auto& a = graphic::instance().get_device();
			graphic.get_device().CreateVertexShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &_vertex_shader);
			code->Release();
		}
		inline ~vertex(void) noexcept {
			_vertex_shader->Release();
		}
	public:
		inline void ma(void) noexcept {
		}
		inline void set(void) noexcept {
			//graphic::instance().get_context().VSSetShader();
			_context.VSSetShader(_vertex_shader, nullptr, 0);
		}
	private:
		ID3D11DeviceContext& _context;
		ID3D11VertexShader* _vertex_shader;
	};
}
//D3DCompileFromFile(L"vertex.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &code, nullptr);

//std::ifstream file(path, std::ios::binary | std::ios::ate);
//size_t size = file.tellg();
//file.seekg(0, std::ios::beg);
//ID3D10Blob* code;
//D3DCreateBlob(size, &code);
//file.read((char*)code->GetBufferPointer(), size);
//file.close();