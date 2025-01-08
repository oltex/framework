#pragma once
#include "memory_pool.h"
#include <utility>
#include <optional>
#include <Windows.h>

namespace data_structure::lockfree {
	template <typename type>
		requires std::is_trivially_copy_constructible_v<type>&& std::is_trivially_destructible_v<type>
	class queue {
	protected:
		using size_type = unsigned int;
		struct node final {
			inline explicit node(void) noexcept = delete;
			inline explicit node(node const&) noexcept = delete;
			inline explicit node(node&&) noexcept = delete;
			inline auto operator=(node const&) noexcept = delete;
			inline auto operator=(node&&) noexcept = delete;
			inline ~node(void) noexcept = delete;
			type _value;
			unsigned long long _next;
		};
	public:
		inline explicit queue(void) noexcept {
			node* current = &_memory_pool.allocate();
			current->_next = _nullptr = _InterlockedIncrement(&_static_nullptr);
			_head = _tail = reinterpret_cast<unsigned long long>(current);
		}
		inline explicit queue(queue const& rhs) noexcept = delete;
		inline explicit queue(queue&& rhs) noexcept = delete;
		inline auto operator=(queue const& rhs) noexcept -> queue & = delete;
		inline auto operator=(queue&& rhs) noexcept -> queue & = delete;
		inline ~queue(void) noexcept {
			node* head = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & _head);
			while (_nullptr != reinterpret_cast<unsigned long long>(head)) {
				unsigned long long next = head->_next;
				if constexpr (std::is_destructible_v<type> && !std::is_trivially_destructible_v<type>)
					head->_value.~type();
				_memory_pool.deallocate(*head);
				head = reinterpret_cast<node*>(next);
			}
		};
	public:
		template<typename universal>
		inline void push(universal&& value) noexcept {
			emplace(std::forward<universal>(value));
		}
		template<typename... argument>
		inline void emplace(argument&&... arg) noexcept {
			node* current = &_memory_pool.allocate();
			current->_next = _nullptr;
			if constexpr (std::is_class_v<type>) {
				if constexpr (std::is_constructible_v<type, argument...>)
					::new(reinterpret_cast<void*>(&current->_value)) type(std::forward<argument>(arg)...);
			}
			else if constexpr (1 == sizeof...(arg))
#pragma warning(suppress: 6011)
				current->_value = type(std::forward<argument>(arg)...);

			for (;;) {
				unsigned long long tail = _tail;
				node* address = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & tail);
				unsigned long long next = address->_next;

				if (_nullptr != next)
					_InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_tail), next + (0xFFFF800000000000ULL & tail) + 0x0000800000000000ULL, tail);
				else if (_nullptr == _InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&address->_next), (unsigned long long)current, _nullptr))
					break;
			}
		}
		inline auto pop(void) noexcept -> std::optional<type> {
			for (;;) {
				unsigned long long head = _head;
				node* address = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & head);
				unsigned long long next = address->_next;

				if (_nullptr == next || 0 == next)
					return std::nullopt;
				else {
					unsigned long long tail = _tail;
					node* tail_address = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & tail);
					if (reinterpret_cast<unsigned long long>(tail_address) == reinterpret_cast<unsigned long long>(address)) {
						unsigned long long tail_next = tail_address->_next;
						if (_nullptr != tail_next)
							_InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_tail), tail_next + (0xFFFF800000000000ULL & tail) + 0x0000800000000000ULL, tail);
					}
					else {
						type result = reinterpret_cast<node*>(next)->_value; // next 다른 숫자일때 추가적인 작업이 필요함
						unsigned long long change = next + (0xFFFF800000000000ULL & head) + 0x0000800000000000ULL;
						if (head == _InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_head), change, head)) {
							if constexpr (std::is_destructible_v<type> && !std::is_trivially_destructible_v<type>)
								address->_value.~type();
							_memory_pool.deallocate(*address);
							return result;
						}
					}
				}
			}
		}
	protected:
		unsigned long long _head;
		unsigned long long _tail;
		unsigned long long _nullptr;
		inline static unsigned long long _static_nullptr = 0;
		memory_pool<node> _memory_pool;
	};
}