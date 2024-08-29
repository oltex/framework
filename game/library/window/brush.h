#pragma once
#include "object.h"
#include <utility>

namespace window {
	class brush final : public object {
	public:
		inline explicit brush(COLORREF const color) noexcept
			: object(CreateSolidBrush(color)) {
		};
		inline explicit brush(brush const& rhs) noexcept = delete;
		inline auto operator=(brush const& rhs) noexcept -> brush & = delete;
		inline explicit brush(brush&& rhs) noexcept
			: object(std::move(rhs)) {
		};
		inline auto operator=(brush&& rhs) noexcept -> brush& {
			object::operator=(std::move(rhs));
		};
		inline virtual ~brush(void) noexcept override = default;
	};
}