#pragma once
#include <memory>
#include <cassert>
#include "memory.h"
#include "function.h"
#include "template.h"

namespace detail {
	template <bool resize, size_t capacity_ = 128>
	class serialize_buffer;
	template<size_t capacity_>
	class serialize_buffer<false, capacity_> {
	protected:
		using byte = unsigned char;
		using size_type = unsigned int;
		size_type _front;
		size_type _rear;
		byte _array[capacity_];
	public:
		inline explicit serialize_buffer(void) noexcept
			: _front(0), _rear(0) {
		};
		inline explicit serialize_buffer(serialize_buffer const&) noexcept = default;
		inline explicit serialize_buffer(serialize_buffer&&) noexcept = default;
		inline auto operator=(serialize_buffer const&) noexcept -> serialize_buffer & = default;
		inline auto operator=(serialize_buffer&&) noexcept -> serialize_buffer & = default;
		inline ~serialize_buffer(void) noexcept = default;

		template<typename type>
			requires library::arithmetic_type<type>
		inline auto operator<<(type const value) noexcept -> serialize_buffer& {
			assert(sizeof(type) + _rear <= capacity_ && "not enough capacity");
			reinterpret_cast<type&>(_array[_rear]) = value;
			_rear += sizeof(type);
			return *this;
		}
		inline void push(byte* const buffer, size_type const length) noexcept {
			assert(length + _rear <= capacity_ && "not enough capacity");
			library::memory_copy(_array + _rear, buffer, length);
			_rear += length;
		}

		inline auto capacity(void) const noexcept -> size_type {
			return capacity_;
		}
	};
	template<size_t capacity_>
	class serialize_buffer<true, capacity_> {
	protected:
		using size_type = unsigned int;
		size_type _front;
		size_type _rear;
		size_type _capacity;
		byte* _array;
	public:
		inline explicit serialize_buffer(void) noexcept
			: _front(0), _rear(0), _capacity(0), _array(nullptr) {
			reserve(capacity_);
		};
		inline explicit serialize_buffer(serialize_buffer const& rhs) noexcept
			: _front(rhs._front), _rear(rhs._rear), _capacity(rhs._capacity), _array(reinterpret_cast<byte*>(library::allocate(_capacity))) {
			library::memory_copy(_array + _front, rhs._array + _front, _rear - _front);
		}
		inline explicit serialize_buffer(serialize_buffer&& rhs) noexcept
			: _front(rhs._front), _rear(rhs._rear), _capacity(rhs._capacity), _array(library::exchange(rhs._array, nullptr)) {
		}
		inline auto operator=(serialize_buffer const& rhs) noexcept -> serialize_buffer&;
		inline auto operator=(serialize_buffer&& rhs) noexcept -> serialize_buffer&;
		inline ~serialize_buffer(void) noexcept {
			library::deallocate<byte>(_array);
		};

		template<typename type>
			requires library::arithmetic_type<type>
		inline auto operator<<(type const value) noexcept -> serialize_buffer& {
			//if (sizeof(type) + _rear > _capacity) {
			//	reserve(library::maximum(static_cast<size_type>(_capacity * 1.5f), _size + 1));
			//}
			reinterpret_cast<type&>(_array[_rear]) = value;
			_rear += sizeof(type);
			return *this;
		}
		inline void push(byte* const buffer, size_type const length) noexcept {
			library::memory_copy(_array + _rear, buffer, length);
			_rear += length;
		}

		inline void reserve(size_type const& capacity) noexcept {
			if (_capacity < capacity) {
#pragma warning(suppress: 6308)
				_array = reinterpret_cast<byte*>(realloc(_array, capacity));
				_capacity = capacity;
			}
		}
		inline auto capacity(void) const noexcept -> size_type {
			return _capacity;
		}
	};
}

namespace library {
	template<bool resize, size_t capacity_ = 128>
	class serialize_buffer : public detail::serialize_buffer<resize, capacity_> {
		using size_type = detail::serialize_buffer<resize, capacity_>::size_type;
		using iterator = byte*;
		using detail::serialize_buffer<resize, capacity_>::_front;
		using detail::serialize_buffer<resize, capacity_>::_rear;
		using detail::serialize_buffer<resize, capacity_>::_array;
	public:
		template<typename type>
			requires library::arithmetic_type<type>
		inline auto operator>>(type& value) noexcept -> serialize_buffer& {
			value = reinterpret_cast<type&>(_array[_front]);
			_front += sizeof(type);
			return *this;
		}
		inline void peek(byte* const buffer, size_type const length) const noexcept {
			memcpy(buffer, _array + _front, length);
		}
		inline void pop(size_type const length) noexcept {
			_front += length;
		}

		inline auto begin(void) noexcept -> iterator {
			return _array + _front;
		}
		inline auto end(void) noexcept -> iterator {
			return _array + _rear;
		}

		inline void clear(void) noexcept {
			_front = _rear = 0;
		}
		inline auto size(void) const noexcept -> size_type {
			return _rear - _front;
		}
		inline auto front(void) const noexcept -> size_type {
			return _front;
		}
		inline auto rear(void) const noexcept -> size_type {
			return _rear;
		}
		inline void move_front(size_type const length) noexcept {
			_front += length;
		}
		inline void move_rear(size_type const length) noexcept {
			_rear += length;
		}
		inline auto data(void) const noexcept -> byte* {
			return _array;
		}
	};
}