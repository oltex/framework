#pragma once
#include "object.h"
#include <utility>

namespace window {
	class pen final : public object {
	public:
		inline explicit pen(int const iStyle, int const cWidth, COLORREF const color) noexcept
			: object(CreatePen(iStyle, cWidth, color)) {
		}
		inline explicit pen(pen const& rhs) noexcept = delete;
		inline auto operator=(pen const& rhs) noexcept -> pen & = delete;
		inline explicit pen(pen&& rhs) noexcept
			: object(std::move(rhs)) {
		};
		inline auto operator=(pen&& rhs) noexcept -> pen& {
			object::operator=(std::move(rhs));
		};
		inline virtual ~pen(void) noexcept override = default;
	};
}