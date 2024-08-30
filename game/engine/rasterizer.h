#pragma once
#include "graphic.h"

namespace engine {
	class rasterizer final {
	public:
		inline explicit rasterizer(graphic& graphic) noexcept
			: _context(graphic.get_context()) {
			D3D11_RASTERIZER_DESC desc{};
			desc.FillMode = D3D11_FILL_SOLID;
			desc.CullMode = D3D11_CULL_FRONT;
			graphic.get_device().CreateRasterizerState(&desc, &_state);
		}
		inline ~rasterizer(void) noexcept {
			_state->Release();
		}
	public:
		inline void set(void) noexcept {
		}
	private:
		ID3D11DeviceContext& _context;
		ID3D11RasterizerState* _state;
	};
}