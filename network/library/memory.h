#pragma once
#pragma comment(lib, "onecore.lib")
#include <malloc.h>
#include <utility>
#include <Windows.h>

namespace library {
	inline auto allocate(size_t const size) noexcept -> void* {
		return reinterpret_cast<void*>(::malloc(size));
	}
	inline auto allocate(size_t const size, size_t const align) noexcept -> void* {
		return reinterpret_cast<void*>(::_aligned_malloc(size, align));
	}
	template<typename type>
		requires (!std::is_void_v<type>)
	inline auto allocate(void) noexcept -> type* {
		if constexpr (16 >= alignof(type))
			return reinterpret_cast<type*>(::malloc(sizeof(type)));
		else
			return reinterpret_cast<type*>(::_aligned_malloc(sizeof(type), alignof(type)));
	}
	template<typename type>
		requires (!std::is_void_v<type>)
	inline auto allocate(size_t const count) noexcept -> type* {
		if constexpr (16 >= alignof(type))
			return reinterpret_cast<type*>(::malloc(sizeof(type) * count));
		else
			return reinterpret_cast<type*>(::_aligned_malloc(sizeof(type) * count, alignof(type)));
	}
	inline auto reallocate(void* pointer, size_t const size) noexcept -> void* {
		return reinterpret_cast<void*>(::realloc(pointer, size));
	}
	inline auto reallocate(void* pointer, size_t const size, size_t const align) noexcept -> void* {
		return reinterpret_cast<void*>(::_aligned_realloc(pointer, size, align));
	}
	template<typename type>
		requires (!std::is_void_v<type>)
	inline auto reallocate(type* pointer, size_t const count) noexcept -> type* {
		if constexpr (16 >= alignof(type))
			return reinterpret_cast<type*>(::realloc(pointer, sizeof(type) * count));
		else
			return reinterpret_cast<type*>(::_aligned_realloc(pointer, sizeof(type) * count, alignof(type)));
	}
	inline auto deallocate(void* const pointer) noexcept {
		::free(pointer);
	}
	inline auto deallocate(void* const pointer, size_t const align) noexcept {
		::_aligned_free(pointer);
	}
	template<typename type>
		requires (!std::is_void_v<type>)
	inline void deallocate(type* const pointer) noexcept {
		if constexpr (16 >= alignof(type))
			::free(pointer);
		else
			::_aligned_free(pointer);
	}

	inline auto memory_copy(void* const destine, void const* const source, size_t const size) noexcept -> void* {
		return ::memcpy(destine, source, size);
	}
	template<typename type>
		requires (!std::is_void_v<type>)
	inline auto memory_copy(type* const destine, type const* const source, size_t const count) noexcept -> type* {
		return reinterpret_cast<type*>(::memcpy(destine, source, sizeof(type) * count));
	}
	inline auto memory_move(void* const destine, void const* const source, size_t const size) noexcept -> void* {
		return ::memmove(destine, source, size);
	}
	template<typename type>
		requires (!std::is_void_v<type>)
	inline auto memory_move(type* const destine, type const* const source, size_t const count) noexcept -> type* {
		return reinterpret_cast<type*>(::memmove(destine, source, sizeof(type) * count));
	}
	inline auto memory_compare(void const* const buffer_1, void const* const buffer_2, size_t const size) noexcept -> int {
		return ::memcmp(buffer_1, buffer_2, size);
	}
	template<typename type>
		requires (!std::is_void_v<type>)
	inline auto memory_compare(type const* const buffer_1, type const* const buffer_2, size_t count) noexcept -> int {
		return ::memcmp(buffer_1, buffer_2, sizeof(type) * count);
	}
	inline auto memory_set(void* const destine, int value, size_t size) noexcept -> void* {
		return ::memset(destine, value, size);
	}

	template<typename type, typename... argument>
	inline auto construct(type& instance, argument&&... arg) noexcept {
		if constexpr (std::is_constructible_v<type, argument...>)
			if constexpr (std::is_class_v<type>)
				::new(reinterpret_cast<void*>(&instance)) type(std::forward<argument>(arg)...);
			else if constexpr (0 < sizeof...(arg))
#pragma warning(suppress: 6011)
				instance = type(std::forward<argument>(arg)...);
	}
	template<typename type>
	inline void destruct(type& instance) noexcept {
		if constexpr (std::is_destructible_v<type> && !std::is_trivially_destructible_v<type>)
			instance.~type();
	}

	inline auto get_large_page_minimum(void) noexcept -> size_t {
		return GetLargePageMinimum();
	}
	inline auto virtual_allocate(void* address, size_t size, unsigned long type, unsigned long protection) noexcept {
		return VirtualAlloc2(nullptr, address, size, type, protection, nullptr, 0);
	}
	inline auto virtual_deallocate(void* address, size_t size, unsigned long type) noexcept {
		VirtualFree(address, size, type);
	}
}