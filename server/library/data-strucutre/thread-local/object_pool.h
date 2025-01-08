#pragma once
#include "../../design-pattern/thread-local/singleton.h"
#include "../lockfree/memory_pool.h"
#include "../pair.h"

namespace data_structure::_thread_local {
	template<typename type, size_t bucket_size = 1024>
	class object_pool final : public design_pattern::_thread_local::singleton<object_pool<type, bucket_size>> {
	private:
		friend class design_pattern::_thread_local::singleton<object_pool<type, bucket_size>>;
		using size_type = unsigned int;
		struct node final {
			inline explicit node(void) noexcept = delete;
			inline explicit node(node const&) noexcept = delete;
			inline explicit node(node&&) noexcept = delete;
			inline auto operator=(node const&) noexcept = delete;
			inline auto operator=(node&&) noexcept = delete;
			inline ~node(void) noexcept = delete;
			node* _next;
			type _value;
		};
		class stack final {
		private:
			struct bucket final {
				inline explicit bucket(void) noexcept = delete;
				inline explicit bucket(bucket const&) noexcept = delete;
				inline explicit bucket(bucket&&) noexcept = delete;
				inline auto operator=(bucket const&) noexcept = delete;
				inline auto operator=(bucket&&) noexcept = delete;
				inline ~bucket(void) noexcept = delete;
				bucket* _next;
				node* _value;
				size_type _size;
			};
		public:
			inline explicit stack(void) noexcept
				: _head(0) {
			}
			inline explicit stack(stack const& rhs) noexcept = delete;
			inline explicit stack(stack&& rhs) noexcept = delete;
			inline auto operator=(stack const& rhs) noexcept -> stack & = delete;
			inline auto operator=(stack&& rhs) noexcept -> stack & = delete;
			inline ~stack(void) noexcept {
				bucket* head = reinterpret_cast<bucket*>(0x00007FFFFFFFFFFFULL & _head);
				while (nullptr != head) {
					bucket* next = head->_next;
					_memory_pool.deallocate(*head);
					head = next;
				}
			};
		public:
			inline void push(node* value, size_type size) noexcept {
				bucket* current = &_memory_pool.allocate();
				current->_value = value;
				current->_size = size;

				for (;;) {
					unsigned long long head = _head;
					current->_next = reinterpret_cast<bucket*>(0x00007FFFFFFFFFFFULL & head);
					unsigned long long next = reinterpret_cast<unsigned long long>(current) + (0xFFFF800000000000ULL & head);
					if (head == _InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_head), next, head))
						break;
				}
			}
			template<typename... argument>
			inline auto pop(argument&&... arg) noexcept -> pair<node*, size_type> {
				for (;;) {
					unsigned long long head = _head;
					bucket* address = reinterpret_cast<bucket*>(0x00007FFFFFFFFFFFULL & head);
					if (nullptr == address) {
						pair<node*, size_type> result{ reinterpret_cast<node*>(malloc(sizeof(node) * bucket_size)), bucket_size };

						node* current = result._first;
						node* next = current + 1;
						for (size_type index = 0; index < bucket_size - 1; ++index, current = next++) {
							current->_next = next;
							if constexpr (std::is_class_v<type>) {
								if constexpr (std::is_constructible_v<type, argument...>)
									::new(reinterpret_cast<void*>(&current->_value)) type(std::forward<argument>(arg)...);
							}
							else if constexpr (1 == sizeof...(arg))
								current->_value = type(std::forward<argument>(arg)...);

						}
						current->_next = nullptr;
						if constexpr (std::is_class_v<type>) {
							if constexpr (std::is_constructible_v<type, argument...>)
								::new(reinterpret_cast<void*>(&current->_value)) type(std::forward<argument>(arg)...);
						}
						else if constexpr (1 == sizeof...(arg))
							current->_value = type(std::forward<argument>(arg)...);


						return result;
					}
					unsigned long long next = reinterpret_cast<unsigned long long>(address->_next) + (0xFFFF800000000000ULL & head) + 0x0000800000000000ULL;
					if (head == _InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_head), next, head)) {
						pair<node*, size_type> result{ address->_value, address->_size };
						_memory_pool.deallocate(*address);
						return result;
					}
				}
			}
		private:
			unsigned long long _head;
			lockfree::memory_pool<bucket> _memory_pool;
		};
	private:
		inline explicit object_pool(void) noexcept = default;
		inline explicit object_pool(object_pool const& rhs) noexcept = delete;
		inline explicit object_pool(object_pool&& rhs) noexcept = delete;
		inline auto operator=(object_pool const& rhs) noexcept -> object_pool & = delete;
		inline auto operator=(object_pool&& rhs) noexcept -> object_pool & = delete;
		inline ~object_pool(void) noexcept {
			if (_size > bucket_size) {
				_stack.push(_head, bucket_size);
				_head = _break;
				_size -= bucket_size;
			}
			if (_size != bucket_size)
				__debugbreak();
			_stack.push(_head, _size);
		};
	public:
		template<typename... argument>
		inline auto allocate(argument&&... arg) noexcept -> type& {
			auto [value, size] = _stack.pop(std::forward<argument>(arg)...);
			_head = value;
			_size = size;
			return acquire();
		}
		inline auto acquire(void) noexcept -> type& {
			node* current = _head;
			_head = current->_next;
			--_size;
			return current->_value;
		}
		inline void release(type& value) noexcept {
			node* current = reinterpret_cast<node*>(reinterpret_cast<uintptr_t*>(&value) - 1);
			current->_next = _head;
			_head = current;
			++_size;
			if (bucket_size == _size)
				_break = _head;
			else if (bucket_size * 2 == _size) {
				_stack.push(_head, bucket_size);
				_head = _break;
				_size -= bucket_size;
			}
		}
		inline bool empty(void) const noexcept {
			return 0 == _size;
		}
	private:
		inline static stack _stack;
		node* _head = nullptr;
		node* _break = nullptr;
		size_type _size = 0;
	};
}