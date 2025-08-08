#pragma once
#include <new>
#include "memory.h"

namespace library {
	template <typename type>
	class storage {
	public:
		alignas(alignof(type)) unsigned char _buffer[sizeof(type)];
	public:
		inline explicit storage(void) noexcept = default;
		inline explicit storage(storage const&) noexcept = delete;
		inline explicit storage(storage&&) noexcept = delete;
		inline auto operator=(storage const&) noexcept -> storage & = delete;
		inline auto operator=(storage&&) noexcept -> storage & = delete;
		inline ~storage(void) noexcept = default;

		template<typename... argument>
		inline void construct(argument&&... arg) noexcept {
			library::construct<type>(*reinterpret_cast<type*>(_buffer), std::forward<argument>(arg)...);
		}
		inline auto operator=(type& rhs) noexcept -> type& {
			return get() = rhs;
		}
		inline auto operator=(type&& rhs) noexcept -> type& {
			return get() = std::move(rhs);
		}
		inline void destruct(void) noexcept {
			library::destruct<type>(get());
		}
		inline auto get(void) noexcept -> type& {
			return *reinterpret_cast<type*>(_buffer);
		}
		inline auto operator*(void) noexcept -> type& {
			return get();
		}
		inline auto operator->(void) noexcept -> type* {
			return &get();
		}

		inline void relocate(type& rhs) noexcept {
			library::memory_copy(&get(), &rhs, 1);
		}
	};
}