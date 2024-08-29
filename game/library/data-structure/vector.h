#pragma once
#include <initializer_list>
#include <utility>
#include <stdlib.h>

template<typename type>
class vector {
public:
	using iterator = type*;
public:
	inline explicit vector(void) noexcept = default;
	inline explicit vector(std::initializer_list<type> const& list) noexcept {
		reserve(list.size());
		for (auto& iter : list)
			emplace_back(iter);
	}
	inline explicit vector(iterator const& begin, iterator const& end) noexcept {
		reserve(end - begin);
		for (auto iter = begin; iter != end; ++iter)
			emplace_back(*iter);
	}
	inline explicit vector(vector const& rhs) noexcept
		: vector(rhs.begin(), rhs.end()) {
	}
	inline explicit vector(vector&& rhs) noexcept {
		swap(rhs);
	}
	inline ~vector(void) noexcept {
		clear();
		free(_array);
	}
public:
	template<typename universal>
	inline void push_back(universal&& value) noexcept {
		emplace_back(std::forward<universal>(value));
	}
	template<typename... argument>
	inline auto emplace_back(argument&&... arg) noexcept -> type& {
		if (_size >= _capacity) {
			_capacity = static_cast<size_t>(_capacity * 1.5f);
			if (_size >= _capacity)
				_capacity++;
			reserve(_capacity);
		}
		type* element = new(_array + _size) type(std::forward<argument>(arg)...);
		++_size;
		return *element;
	}
	inline void pop_back(void) noexcept {
		--_size;
		_array[_size].~type();
	}
public:
	inline auto front(void) const noexcept ->type& {
		return _array[0];
	}
	inline auto back(void) const noexcept ->type& {
		return _array[_size - 1];
	}
	inline auto operator[](size_t const idx) const noexcept ->type& {
		return _array[idx];
	}
public:
	inline auto begin(void) const noexcept -> iterator {
		return _array;
	}
	inline auto end(void) const noexcept -> iterator {
		return _array + _size;
	}
	inline auto data(void) noexcept -> type* {
		return _array;
	}
public:
	inline auto size(void) const noexcept -> size_t {
		return _size;
	}
	inline bool empty(void) const noexcept {
		return 0 == _size;
	}
	inline auto capacity(void) const noexcept -> size_t {
		return _capacity;
	}
	inline void reserve(size_t const& capacity) noexcept {
		if (_size > capacity)
			return;
		_capacity = capacity;
#pragma warning(suppress: 6308)
		_array = static_cast<type*>(realloc(_array, sizeof(type) * _capacity));
	}
	inline void resize(size_t const& size) noexcept {
		//if (_capacity < size)
		//	reserve(size);
		//for (; _size < size; ++_size)
		//	new(_array + _size) type(value);
	}
	inline void clear(void) noexcept {
		while (0 != _size)
			pop_back();
	}
	inline void swap(vector& rhs) noexcept {
		type* arr = _array;
		_array = rhs._array;
		rhs._array = arr;

		size_t size = _size;
		_size = rhs._size;
		rhs._size = size;

		size_t capacity = _capacity;
		_capacity = rhs._capacity;
		rhs._capacity = capacity;
	}
	inline void assign(size_t const size, type const& value) noexcept {
		clear();
		if (_capacity < size)
			reserve(size);
		for (; _size < size; ++_size)
			new(_array + _size) type(value);
	}
protected:
	type* _array = nullptr;
	size_t _size = 0;
	size_t _capacity = 0;
};