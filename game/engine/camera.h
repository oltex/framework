#pragma once
#include "component.h"
#include "transform.h"
#include <DirectXMath.h>

namespace engine {
	class camera : public component {
	public:
		inline explicit camera(transform& view_transform, float field_of_view, float const aspect_ratio, float const near, float const far) noexcept
			: _view_transform(view_transform) {
			field_of_view = DirectX::XMConvertToRadians(field_of_view);
			DirectX::XMStoreFloat4x4(&_projection_matrix, DirectX::XMMatrixPerspectiveFovLH(field_of_view, aspect_ratio, near, far));

		}
		inline virtual ~camera(void) noexcept {
		}
	public:
		inline void render(void) noexcept {

		}
	private:
		transform& _view_transform;
		DirectX::XMFLOAT4X4 _projection_matrix;
	};
}