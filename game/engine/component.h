#pragma once

namespace engine {
	class component {
	public:
		inline explicit component(void) noexcept {
		}
		inline virtual auto clone(void) noexcept -> component* = 0;
		inline explicit component(component const& rhs) noexcept {
		}
		inline virtual ~component(void) noexcept {
		}
	private:
		bool _active;
		bool _destory;
	};
}