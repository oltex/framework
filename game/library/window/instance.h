#pragma once
#include <Windows.h>

namespace window {
	class instance final {
	public:
		inline explicit instance(HINSTANCE const hinstance) noexcept
			: _hinstance(hinstance) {
		}
		inline explicit instance(instance const& rhs) noexcept = delete;
		inline auto operator=(instance const& rhs) noexcept -> instance & = delete;
		inline explicit instance(instance&& rhs) noexcept
			: _hinstance(rhs._hinstance) {
			rhs._hinstance = nullptr;
		}
		inline auto operator=(instance&& rhs) noexcept -> instance& {
			_hinstance = rhs._hinstance;
			rhs._hinstance = nullptr;
			return *this;
		}
		inline ~instance(void) noexcept = default;
	public:
		inline auto data(void) const noexcept -> HINSTANCE {
			return _hinstance;
		}
	private:
		HINSTANCE _hinstance;
	};
}