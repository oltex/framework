#pragma once
#include "graphic.h"

namespace engine {
	class blend final {
	public:
		inline explicit blend(graphic& graphic, wchar_t const* const path) noexcept {
			D3D11_BLEND_DESC desc{};
			desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			//desc.RenderTarget[0].

			//desc.AlphaToCoverageEnable
			//graphic.get_device().CreateBlendState(&desc, &_state);
		}
	private:
		ID3D11BlendState* _state;
	};
}

//SetRasterizerState(RS_Default);
//SetDepthStencilState(DSS_Default, 0);
//SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);