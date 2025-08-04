#pragma once
#include <cassert>
#include "memory.h"
#include "function.h"
#include "template.h"

namespace detail {
	template<typename type = char, auto sso = 16> //small string optimization
		requires (std::is_same_v<type, char> || std::is_same_v<type, wchar_t>)
	class string final {
		using size_type = unsigned int;
		union buffer {
			type _array[sso];
			type* _pointer;
		};
		//using node = typename std::conditional<resize, union union_node, struct strcut_node>::type;

		size_type _size;
		size_type _capacity;
		buffer _buffer;
	public:
		using iterator = type*;

		inline explicit string(void) noexcept
			: _size(0), _capacity(sso), _buffer() {
		};
		template<typename argument>
			requires (library::same_type<library::remove_cp<argument>, type>)
		inline string(argument arg) noexcept
			: string() {
			insert(end(), arg);
		}
		inline string(string const& rhs) noexcept
			: string(const_cast<string&>(rhs).c_str()) {
		};
		inline explicit string(string&& rhs) noexcept
			: _size(library::exchange(rhs._size, 0)), _capacity(library::exchange(rhs._capacity, sso)), _buffer(rhs._buffer) {
		};
		inline auto operator=(string const& rhs) noexcept -> string& {
			assign(const_cast<string&>(rhs).c_str());
			return *this;
		};
		inline auto operator=(string&& rhs) noexcept -> string& {
			if (sso < _capacity)
				library::deallocate(_buffer._pointer);

			_size = library::exchange(rhs._size, 0);
			_capacity = library::exchange(rhs._capacity, sso);
			_buffer = rhs._buffer;
			return *this;
		};
		inline ~string(void) noexcept {
			if (sso < _capacity)
				library::deallocate(_buffer._pointer);
		};

		template<typename argument>
			requires (library::same_type<library::remove_cp<argument>, type>)
		inline auto insert(iterator iter, argument arg) noexcept -> iterator {
			size_type char_size;
			if constexpr (std::is_same_v<argument, type>)
				char_size = 1;
			else
				char_size = static_cast<size_type>(library::string_length(arg));

			if (_size + char_size >= _capacity) {
				auto index = iter - begin();
				reserve(library::maximum(static_cast<size_type>(_capacity * 1.5f), _size + char_size + 1));
				iter = begin() + index;
			}
			library::memory_move(iter + char_size, iter, end() - iter);

			if constexpr (std::is_same_v<argument, type>)
				*iter = arg;
			else
				library::memory_copy(iter, arg, char_size);
			_size += char_size;
			return iter;
		}
		inline auto push_back(type character) noexcept -> type& {
			return *insert(end(), character);
		}
		inline auto erase(iterator iter) noexcept -> iterator {
			assert(_size > 0 && "called on empty");
			library::memory_move(iter, iter + 1, end() - (iter + 1));
			--_size;
			return iter;
		}
		inline void pop_back(void) noexcept {
			erase(end() - 1);
		}
		inline auto operator=(type const* const character) noexcept -> string& {
			assign(character);
			return *this;
		}
		inline auto assign(type const* const character) noexcept -> string& {
			clear();
			append(character);
			return *this;
		}
		inline auto operator+=(type const* const character) noexcept -> string& {
			append(character);
			return *this;
		}
		template<typename argument>
			requires (library::same_type<library::remove_cp<argument>, type>)
		inline auto append(argument character) noexcept -> string& {
			insert(end(), character);
			return *this;
		}

		inline auto operator==(string const& rhs) const noexcept -> bool {
			if (_size != rhs._size)
				return false;
			return 0 == library::memory_compare(const_cast<string&>(*this).data(), const_cast<string&>(rhs).data(), _size);
		}
		template<typename argument>
			requires (library::same_type<library::remove_cp<argument>, type>)
		inline friend auto operator==(string const& lhs, argument rhs) noexcept -> bool {
			if constexpr (std::is_same_v<argument, type>) {
				if (lhs._size != 1)
					return false;
				return *const_cast<string&>(lhs).data() == rhs;
			}
			else {
				if (lhs._size != library::string_length(rhs))
					return false;
				return 0 == library::memory_compare(const_cast<string&>(lhs).data(), rhs, lhs._size);
			}
		};
		inline auto operator+(string const& rhs) const noexcept -> string {
			string result;
			result.reserve(_size + rhs._size + 1);
			result.append(const_cast<string&>(*this).c_str()).append(const_cast<string&>(rhs).c_str());
			return result;
		}
		template<typename argument>
			requires (library::same_type<library::remove_cp<argument>, type>)
		inline friend auto operator+(string const& lhs, argument rhs) noexcept -> string {
			string result;
			size_type char_size;
			if constexpr (std::is_same_v<argument, type>)
				char_size = 1;
			else
				char_size = static_cast<size_type>(library::string_length(rhs));
			result.reserve(lhs._size + char_size + 1);
			result.append(const_cast<string&>(lhs).c_str()).append(rhs);
			return result;
		}
		template<typename argument>
			requires (library::same_type<library::remove_cp<argument>, type>)
		inline friend auto operator+(argument lhs, string const& rhs) noexcept -> string {
			string result;
			size_type char_size;
			if constexpr (std::is_same_v<argument, type>)
				char_size = 1;
			else
				char_size = static_cast<size_type>(library::string_length(lhs));
			result.reserve(rhs._size + char_size + 1);
			result.append(lhs).append(const_cast<string&>(rhs).c_str());
			return result;
		}

		inline auto begin(void) noexcept -> iterator {
			return data();
		}
		inline auto end(void) noexcept -> iterator {
			return data() + _size;
		}
		inline auto front(void) const noexcept -> type& {
			assert(_size > 0 && "called on empty");
			return data()[0];
		}
		inline auto back(void) const noexcept -> type& {
			assert(_size > 0 && "called on empty");
			return data()[_size - 1];
		}
		inline auto operator[](size_type const index) const noexcept ->type& {
			assert(index < _size && "index out of range");
			return data()[index];
		}

		inline void reserve(size_type capacity) noexcept {
			if (_capacity < capacity) {
				if (sso >= _capacity)
					_buffer._pointer = reinterpret_cast<type*>(library::memory_copy(library::allocate<type>(capacity), _buffer._array, _size));
				else
					_buffer._pointer = reinterpret_cast<type*>(library::reallocate(_buffer._pointer, capacity));
				_capacity = capacity;
			}
		}
		inline void clear(void) noexcept {
			_size = 0;
		}
		inline void swap(string& rhs) noexcept {
			library::swap(_size, rhs._size);
			library::swap(_capacity, rhs._capacity);
			library::swap(_buffer, rhs._buffer);
		}
		inline bool empty(void) const noexcept {
			return 0 == _size;
		}
		inline auto size(void) const noexcept -> size_type {
			return _size;
		}
		inline auto data(void) noexcept -> type* {
			if (sso >= _capacity)
				return _buffer._array;
			else
				return _buffer._pointer;
		}
		inline auto c_str(void) noexcept -> type* {
			if (sso >= _capacity) {
				_buffer._array[_size] = '\0';
				return _buffer._array;
			}
			else {
				_buffer._pointer[_size] = '\0';
				return _buffer._pointer;
			}
		}
	};
}

namespace library {
	using string = typename detail::string<char>;
	using wstring = typename detail::string<wchar_t>;
}