#pragma once
#include <type_traits>

namespace design_pattern {
	template<typename type>
	class singleton {
	protected:
		inline explicit singleton(void) noexcept = default;
		inline ~singleton(void) noexcept = default;
	private:
		inline explicit singleton(singleton const& rhs) noexcept = delete;
		inline explicit singleton(singleton&& rhs) noexcept = delete;
		inline auto operator=(singleton const& rhs) noexcept -> singleton & = delete;
		inline auto operator=(singleton&& rhs) noexcept -> singleton & = delete;
	public:
		inline static auto instance(void) noexcept -> type& {
			static type _instance;
			return _instance;
		}
	};
}