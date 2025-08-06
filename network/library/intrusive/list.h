#pragma once
#include <utility>
#include <stdlib.h>
#include <malloc.h>
#include "../function.h"

namespace library::intrusive {
	template<size_t index>
	class list_hook {
	private:
		template<typename type, size_t>
		friend class list;
		list_hook* _prev, * _next;
	public:
		inline explicit list_hook(void) noexcept = default;
		inline explicit list_hook(list_hook const&) noexcept = default;
		inline explicit list_hook(list_hook&&) noexcept = default;
		inline auto operator=(list_hook const&) noexcept -> list_hook & = default;
		inline auto operator=(list_hook&&) noexcept -> list_hook & = default;
		inline ~list_hook(void) noexcept = default;
	};

	template<typename type, size_t index>
	//requires std::is_base_of<list_hook<index>, type>::value
	class list final {
		using size_type = unsigned int;
		using node = list_hook<index>;
		static_assert(std::is_base_of<node, type>::value);
		size_type _size;
		node _head;
	public:
		class iterator final {
		public:
			inline explicit iterator(node* const node = nullptr) noexcept
				: _node(node) {
			}
			inline iterator(iterator const&) noexcept = default;
			inline explicit iterator(iterator&&) noexcept = default;
			inline auto operator=(iterator const&) noexcept -> iterator & = default;
			inline auto operator=(iterator&&) noexcept -> iterator & = default;
			inline ~iterator() noexcept = default;

			inline auto operator*(void) const noexcept -> type& {
				return static_cast<type&>(*_node);
			}
			inline auto operator->(void) noexcept -> type* {
				return static_cast<type*>(_node);
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
		public:
			node* _node;
		};

		inline explicit list(void) noexcept
			: _size(0) {
			_head._next = _head._prev = &_head;
		}
		inline explicit list(list const&) noexcept;
		inline explicit list(list&&) noexcept;
		inline auto operator=(list const&) noexcept -> list&;
		inline auto operator=(list&&) noexcept -> list&;
		inline ~list(void) noexcept = default;

		inline void push_front(type& value) noexcept {
			insert(begin(), value);
		}
		inline void push_back(type& value) noexcept {
			insert(end(), value);
		}
		inline auto insert(iterator const& iter, type& value) noexcept -> iterator {
			auto current = static_cast<node*>(&value);
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
			return erase(static_cast<type&>(*iter._node));
		}
		inline auto erase(type& value) noexcept -> iterator {
			auto current = static_cast<node*>(&value);
			auto prev = current->_prev;
			auto next = current->_next;

			prev->_next = next;
			next->_prev = prev;

			--_size;
			return iterator(next);
		}

		inline auto front(void) const noexcept -> type& {
			return static_cast<type&>(*_head._next);
		}
		inline auto back(void) const noexcept -> type& {
			return static_cast<type&>(*_head._prev);
		}
		inline auto begin(void) const noexcept -> iterator {
			return iterator(_head._next);
		}
		inline auto end(void) noexcept -> iterator {
			return iterator(&_head);
		}

		inline void swap(list& rhs) noexcept {
			auto next = _head._next;
			next->_prev = _head._prev->_next = &rhs._head;
			next = rhs._head._next;
			next->_prev = rhs._head._prev->_next = &_head;

			library::swap(_head, rhs._head);
			library::swap(_size, rhs._size);
		}
		inline void clear(void) noexcept {
			_head._next = _head._prev = &_head;
			_size = 0;
		}
		inline auto size(void) const noexcept -> size_type {
			return _size;
		}
		inline bool empty(void) const noexcept {
			return 0 == _size;
		}
	};
}