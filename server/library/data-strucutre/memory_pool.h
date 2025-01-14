#pragma once
#include <memory>

namespace data_structure {
	template<typename type>
	class memory_pool final {
	private:
		union node final {
			inline explicit node(void) noexcept = delete;
			inline explicit node(node const&) noexcept = delete;
			inline explicit node(node&&) noexcept = delete;
			inline auto operator=(node const&) noexcept = delete;
			inline auto operator=(node&&) noexcept = delete;
			inline ~node(void) noexcept = delete;
			node* _next;
			type _value;
		};
	public:
		template <typename other>
		using rebind = memory_pool<other>;
	public:
		inline explicit memory_pool(void) noexcept = default;
		inline explicit memory_pool(memory_pool const&) noexcept = delete;
		inline explicit memory_pool(memory_pool&& rhs) noexcept
			: _head(rhs._head) {
			rhs._head = nullptr;
		};
		inline auto operator=(memory_pool const&) noexcept = delete;
		inline auto operator=(memory_pool&& rhs) noexcept {
			_head = rhs._head;
			rhs._head = nullptr;
		}
		inline ~memory_pool(void) noexcept {
			while (nullptr != _head) {
#pragma warning(suppress: 6001)
				node* next = _head->_next;
				free(_head);
				_head = next;
			}
		}
	public:
		template<typename... argument>
		inline auto allocate(argument&&... arg) noexcept -> type& {
			node* current;
			if (nullptr == _head)
				current = static_cast<node*>(malloc(sizeof(node)));
			else {
				current = _head;
				_head = current->_next;
			}

			if constexpr (std::is_class_v<type>) {
				if constexpr (std::is_constructible_v<type, argument...>)
					::new(reinterpret_cast<void*>(&current->_value)) type(std::forward<argument>(arg)...);
			}
			else if constexpr (1 == sizeof...(arg))
#pragma warning(suppress: 6011)
				current->_value = type(std::forward<argument>(arg)...);
#pragma warning(suppress: 6011)
			return current->_value;
		}
		inline void deallocate(type& value) noexcept {
			if constexpr (std::is_destructible_v<type> && !std::is_trivially_destructible_v<type>)
				value.~type();
			reinterpret_cast<node*>(&value)->_next = _head;
			_head = reinterpret_cast<node*>(&value);
		}
	public:
		inline void swap(memory_pool& rhs) noexcept {
			auto head = _head;
			_head = rhs._head;
			rhs._head = head;
		}
	private:
		node* _head = nullptr;
	};
}