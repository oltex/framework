#pragma once
#include "../memory.h"
#include "../function.h"
#include <memory>
#include <Windows.h>

namespace library::lockfree {
	template<typename type, bool placement = true, bool compress = true>
	class pool final {
		union union_node final {
			union_node* _next;
			type _value;
			inline explicit union_node(void) noexcept = delete;
			inline explicit union_node(union_node const&) noexcept = delete;
			inline explicit union_node(union_node&&) noexcept = delete;
			inline auto operator=(union_node const&) noexcept = delete;
			inline auto operator=(union_node&&) noexcept = delete;
			inline ~union_node(void) noexcept = delete;
		};
		struct strcut_node final {
			strcut_node* _next;
			type _value;
			inline explicit strcut_node(void) noexcept = delete;
			inline explicit strcut_node(strcut_node const&) noexcept = delete;
			inline explicit strcut_node(strcut_node&&) noexcept = delete;
			inline auto operator=(strcut_node const&) noexcept = delete;
			inline auto operator=(strcut_node&&) noexcept = delete;
			inline ~strcut_node(void) noexcept = delete;
		};
		using node = typename std::conditional<compress, union union_node, struct strcut_node>::type;
		alignas(64) unsigned long long _head;
	public:
		inline explicit pool(void) noexcept
			: _head(0) {
		};
		inline explicit pool(pool const&) noexcept = delete;
		inline explicit pool(pool&& rhs) noexcept
			: _head(exchange(rhs._head, nullptr)) {
		};
		inline auto operator=(pool const&) noexcept = delete;
		inline auto operator=(pool&& rhs) noexcept -> pool& {
			node* head = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & _head);
			while (nullptr != head)
				library::deallocate<node>(exchange(head, head->_next));
			_head = exchange(rhs._head, nullptr);
			return *this;
		}
		inline ~pool(void) noexcept {
			node* head = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & _head);
			while (nullptr != head)
				library::deallocate<node>(exchange(head, head->_next));
		}

		template<typename... argument>
		inline auto allocate(argument&&... arg) noexcept -> type* {
			node* current;
			for (;;) {
				unsigned long long head = _head;
				current = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & head);
				if (nullptr == current) {
					current = library::allocate<node>();
					break;
				}
				unsigned long long next = reinterpret_cast<unsigned long long>(current->_next) + (0xFFFF800000000000ULL & head);
				if (head == _InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_head), next, head))
					break;
			}

			if constexpr (true == placement)
				library::construct<type, argument...>(current->_value, std::forward<argument>(arg)...);
			return &current->_value;
		}
		inline void deallocate(type* value) noexcept {
			if constexpr (true == placement)
				library::destruct<type>(*value);
			auto current = reinterpret_cast<node*>(reinterpret_cast<unsigned char*>(value) - offsetof(node, _value));
			for (;;) {
				unsigned long long head = _head;
				current->_next = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & head);
				unsigned long long next = reinterpret_cast<unsigned long long>(current) + (0xFFFF800000000000ULL & head) + 0x0000800000000000ULL;
				if (head == _InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_head), next, head))
					break;
			}
		}
	};
}