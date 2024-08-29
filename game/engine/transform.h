#pragma once
#include "component.h"
#include <DirectXMath.h>

namespace engine {
	class transform final : public component {
	public:
		inline explicit transform(void) noexcept {
			DirectX::XMStoreFloat4x4(&_matrix, DirectX::XMMatrixIdentity());
		}
		inline virtual ~transform(void) noexcept {
		}
	public:
		inline auto get_matrix(void) const noexcept -> DirectX::XMMATRIX {
			return DirectX::XMLoadFloat4x4(&_matrix);
		}
		inline auto set_matrix(DirectX::XMMATRIX const& matrix) noexcept {
			DirectX::XMStoreFloat4x4(&_matrix, matrix);
		}
	private:
		DirectX::XMFLOAT4X4 _matrix;
	};
}