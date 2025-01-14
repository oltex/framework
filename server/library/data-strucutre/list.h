#pragma once
#include <utility>
#include <stdlib.h>
#include <malloc.h>
#include "memory_pool.h"

namespace data_structure {
	template<typename type, typename allocator = memory_pool<type>>
	class list final {
	private:
		using size_type = unsigned int;
		struct node final {
			inline explicit node(void) noexcept = delete;
			inline explicit node(node const&) noexcept = delete;
			inline explicit node(node&&) noexcept = delete;
			inline auto operator=(node const&) noexcept = delete;
			inline auto operator=(node&&) noexcept = delete;
			inline ~node(void) noexcept = delete;
			node* _prev, * _next;
			type _value;
		};
		using rebind_allocator = allocator::template rebind<node>;
	public:
		class iterator final {
		public:
			inline explicit iterator(node* const node = nullptr) noexcept
				: _node(node) {
			}
			inline iterator(iterator const& rhs) noexcept
				: _node(rhs._node) {
			}
			inline auto operator=(iterator const& rhs) noexcept -> iterator& {
				_node = rhs._node;
				return *this;
			}
		public:
			inline auto operator*(void) const noexcept -> type& {
				return _node->_value;
			}
			inline auto operator->(void) const noexcept -> type* {
				return &_node->_value;
			}
			inline auto operator++(void) noexcept -> iterator& {
				_node = _node->_next;
				return *this;
			}
			inline auto operator++(int) noexcept -> iterator {
				iterator iter(*this);
				_node = _node->_next;
				return iter;
			}
			inline auto operator--(void) noexcept -> iterator& {
				_node = _node->_prev;
				return *this;
			}
			inline auto operator--(int) noexcept -> iterator {
				iterator iter(*this);
				_node = _node->_prev;
				return iter;
			}
			inline bool operator==(iterator const& rhs) const noexcept {
				return _node == rhs._node;
			}
			inline bool operator!=(iterator const& rhs) const noexcept {
				return _node != rhs._node;
			}
		public:
			node* _node;
		};
	public:
		inline explicit list(void) noexcept
			: _head(reinterpret_cast<node*>(calloc(1, sizeof(node*) * 2))) {
#pragma warning(suppress: 6011)
			_head->_next = _head->_prev = _head;
		}
		inline explicit list(std::initializer_list<type> init_list) noexcept
			: list() {
			for (auto& iter : init_list)
				emplace_back(iter);
		}
		inline explicit list(iterator const& begin, iterator const& end) noexcept
			: list() {
			for (auto iter = begin; iter != end; ++iter)
				emplace_back(*iter);
		}
		inline list(list const& rhs) noexcept
			: list(rhs.begin(), rhs.end()) {
		}
		inline explicit list(list&& rhs) noexcept
			: list() {
			swap(rhs);
		}
		//not implemented
		inline auto operator=(list const& rhs) noexcept;
		//not implemented
		inline auto operator=(list&& rhs) noexcept;
		inline ~list(void) noexcept {
			clear();
			free(_head);
		}
	public:
		template<typename universal>
		inline void push_front(universal&& value) noexcept {
			emplace(begin(), std::forward<universal>(value));
		}
		template<typename universal>
		inline void push_back(universal&& value) noexcept {
			emplace(end(), std::forward<universal>(value));
		}
		template<typename universal>
		inline auto insert(iterator const& iter, universal&& value) noexcept -> iterator {
			return emplace(iter, std::forward<universal>(value));
		}
		template<typename... argument>
		inline auto emplace_front(argument&&... arg) noexcept -> type& {
			return *emplace(begin(), std::forward<argument>(arg)...);
		}
		template<typename... argument>
		inline auto emplace_back(argument&&... arg) noexcept -> type& {
			return *emplace(end(), std::forward<argument>(arg)...);
		}
		template<typename... argument>
		inline auto emplace(iterator const& iter, argument&&... arg) noexcept -> iterator {
			auto current = &_allocator.allocate();
			//auto current = reinterpret_cast<node*>(malloc(sizeof(node)));
			if constexpr (std::is_class_v<type>) {
				if constexpr (std::is_constructible_v<type, argument...>)
					::new(reinterpret_cast<void*>(&current->_value)) type(std::forward<argument>(arg)...);
			}
			else if constexpr (1 == sizeof...(arg))
#pragma warning(suppress: 6011)
				current->_value = type(std::forward<argument>(arg)...);
			auto next = iter._node;
			auto prev = next->_prev;

			prev->_next = current;
			current->_prev = prev;
			current->_next = next;
			next->_prev = current;

			++_size;
			return iterator(current);
		}
		inline void pop_front(void) noexcept {
			erase(begin());
		}
		inline void pop_back(void) noexcept {
			erase(--end());
		}
		inline auto erase(iterator const& iter) noexcept -> iterator {
			auto current = iter._node;
			auto prev = current->_prev;
			auto next = current->_next;

			prev->_next = next;
			next->_prev = prev;

			if constexpr (std::is_destructible_v<type> && !std::is_trivially_destructible_v<type>)
				current->_value.~type();
			_allocator.deallocate(*current);
			--_size;
			return iterator(next);
		}
	public:
		inline auto front(void) const noexcept -> type& {
			return _head->_next->_value;
		}
		inline auto back(void) const noexcept -> type& {
			return _head->_prev->_value;
		}
		inline auto begin(void) const noexcept -> iterator {
			return iterator(_head->_next);
		}
		inline auto end(void) const noexcept -> iterator {
			return iterator(_head);
		}
	public:
		inline void swap(list& rhs) noexcept {
			node* head = _head;
			_head = rhs._head;
			rhs._head = head;

			size_type size = _size;
			_size = rhs._size;
			rhs._size = size;

			_allocator.swap(rhs._allocator);
		}
		inline void clear(void) noexcept {
			auto current = _head->_next;
			for (auto next = current; current != _head; current = next) {
				next = next->_next;
				if constexpr (std::is_destructible_v<type> && !std::is_trivially_destructible_v<type>)
					current->_value.~type();
				_allocator.deallocate(*current);
			}
			_head->_next = _head->_prev = _head;
			_size = 0;
		}
		inline void splice(iterator const& _before, iterator const& _first, iterator const& _last) noexcept {
			node* before = _before._node;
			node* first = _first._node;
			node* last = _last._node;

			node* first_prev = first->_prev;
			first_prev->_next = last;
			node* last_prev = last->_prev;
			last_prev->_next = before;
			node* before_prev = before->_prev;
			before_prev->_next = first;

			before->_prev = last_prev;
			last->_prev = first_prev;
			first->_prev = before_prev;

			//return last;
		}
		inline auto size(void) const noexcept -> size_type {
			return _size;
		}
		inline bool empty(void) const noexcept {
			return 0 == _size;
		}
	private:
		node* _head;
		size_type _size = 0;
		rebind_allocator _allocator;
	};
}