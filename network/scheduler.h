#pragma once
#include <functional>
#include "library/system-component/time/multimedia.h"
#include "library/system-component/wait_on_address.h"
#include "library/data-structure/lockfree/queue.h"



class function : public task {
	template <typename function_, typename... argument>
	inline explicit function(function_&& func, argument&&... arg) noexcept
		: _function(std::bind(std::forward<function_>(func), std::forward<argument>(arg)...)) {
	};
	inline explicit function(function const&) noexcept = delete;
	inline explicit function(function&&) noexcept = delete;
	inline auto operator=(function const&) noexcept -> function & = delete;
	inline auto operator=(function&&) noexcept -> function & = delete;
	inline ~function(void) noexcept = default;

	inline virtual bool excute(void) noexcept override {
		for (;;) {
			int time = _function();
			switch (time) {
			case 0:
				break;
			case -1:
				return false;
			default:
				_time += time;
				return true;
			}
		}
	}
private:
	std::function<int(void)> _function;
};

class scheduler final {
	class task;
	class task_queue final : protected data_structure::lockfree::queue<task*> {
	private:
		using base = data_structure::lockfree::queue<task*>;
	public:
		inline explicit task_queue(void) noexcept = default;
		inline explicit task_queue(task_queue const&) noexcept = delete;
		inline explicit task_queue(task_queue&&) noexcept = delete;
		inline auto operator=(task_queue const&) noexcept -> task_queue & = delete;
		inline auto operator=(task_queue&&) noexcept -> task_queue & = delete;
		inline ~task_queue(void) noexcept = default;

		inline void push(task& task_) noexcept {
			base::emplace(&task_);
			_InterlockedIncrement(&_size);
			system_component::wait_on_address::wake_single(&_size);
		}
		inline auto pop(void) noexcept -> task& {
			unsigned long long head = _head;
			node* address = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & head);
			unsigned long long next = address->_next;

			if (0x10000 > (0x00007FFFFFFFFFFFULL & next))
				__debugbreak();
			unsigned long long tail = _tail;
			if (tail == head)
				_InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_tail), next, tail);

			task& result = *reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & next)->_value;
			_head = next;
			_memory_pool::instance().deallocate(*address);
			_InterlockedDecrement(&_size);
			return result;
		}
		inline auto empty(void) const noexcept {
			unsigned long long head = _head;
			node* address = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & head);
			unsigned long long next = address->_next;
			if (_nullptr == (0x00007FFFFFFFFFFFULL & next))
				return true;
			return false;
		}
		inline void wake(void) noexcept {
			system_component::wait_on_address::wake_single(&_size);
		}
		inline bool wait(void* compare, unsigned long const wait_time) noexcept {
			return system_component::wait_on_address::wait(&_size, compare, sizeof(size_type), wait_time);
		}
	private:
		size_type _size = 0;
	};
};