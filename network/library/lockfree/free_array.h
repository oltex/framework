#pragma once
#include "../interlock.h"
#include "../memory.h"
#include "../function.h"

namespace library::lockfree {
	template<typename type>
	class free_array {
	protected:
		using size_type = unsigned int;
		struct node final {
			node* _next;
			type _value;
			inline explicit node(void) noexcept = delete;
			inline explicit node(node const&) noexcept = delete;
			inline explicit node(node&&) noexcept = delete;
			inline auto operator=(node const&) noexcept -> node & = delete;
			inline auto operator=(node&&) noexcept -> node & = delete;
			inline ~node(void) noexcept = delete;
		};
		using iterator = node*;

		unsigned long long _head;
		size_type _capacity;
		node* _array;
	public:
		inline explicit free_array(void) noexcept
			: _head(0), _capacity(0), _array(nullptr) {
		};
		template<typename... argument>
		inline explicit free_array(size_type const capacity, argument&&... arg)noexcept {
			_array = library::allocate<node>(capacity);
			_capacity = capacity;

			auto begin = _array;
			for (size_type index = 0; index < capacity - 1; ++index) {
				auto current = begin++;
				current->_next = begin;
				library::construct(current->_value, std::forward<argument>(arg)...);
			}
#pragma warning(suppress: 6011)
			begin->_next = nullptr;
			library::construct(begin->_value, std::forward<argument>(arg)...);

			_head = reinterpret_cast<unsigned long long>(_array);
		}
		inline explicit free_array(free_array const&) noexcept = delete;
		inline explicit free_array(free_array&&) noexcept = delete;
		inline auto operator=(free_array const&) noexcept -> free_array & = delete;
		inline auto operator=(free_array&&) noexcept -> free_array & = delete;
		inline ~free_array(void) noexcept {
			node* head = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & _head);
			while (nullptr != head)
				library::destruct<type>(library::exchange(head, head->_next)->_value);
			library::deallocate<node>(_array);
		};

		inline auto allocate(void) noexcept -> type* {
			for (;;) {
				unsigned long long head = _head;
				node* current = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & head);
				if (nullptr == current)
					return nullptr;
				unsigned long long next = reinterpret_cast<unsigned long long>(current->_next) + (0xFFFF800000000000ULL & head) + 0x0000800000000000ULL;
				if (head == library::interlock_compare_exhange(_head, next, head)) {
					return &current->_value;
				}
			}
		}
		inline void deallocate(type* value) noexcept {
			auto current = reinterpret_cast<node*>(reinterpret_cast<unsigned char*>(value) - offsetof(node, _value));
			for (;;) {
				unsigned long long head = _head;
				current->_next = reinterpret_cast<node*>(head & 0x00007FFFFFFFFFFFULL);
				unsigned long long next = reinterpret_cast<unsigned long long>(current) + (head & 0xFFFF800000000000ULL);
				if (head == library::interlock_compare_exhange(_head, next, head))
					break;
			}
		}

		inline auto begin(void) const noexcept -> iterator {
			return _array;
		}
		inline auto end(void) const noexcept -> iterator {
			return _array + _capacity;
		}
		inline auto operator[](size_type index) noexcept -> type& {
			return _array[index]._value;
		}
	};
}