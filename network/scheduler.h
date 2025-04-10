#pragma once
#include "library/system-component/time/multimedia.h"
#include "library/system-component/wait_on_address.h"
#include "library/system-component/thread.h"
#include "library/data-structure/lockfree/queue.h"
#include "library/data-structure/priority_queue.h"

class scheduler final {
public:
	class task {
	public:
		inline explicit task(void) noexcept
			: _time(library::system_component::time::multimedia::get_time()) {
		};
		inline explicit task(task const&) noexcept = delete;
		inline explicit task(task&&) noexcept = delete;
		inline auto operator=(task const&) noexcept -> task & = delete;
		inline auto operator=(task&&) noexcept -> task & = delete;
		inline ~task(void) noexcept = default;

		inline virtual bool excute(void) noexcept = 0;

		unsigned long _time;
	};
	class queue final : protected library::data_structure::lockfree::queue<task*, false> {
	private:
		using base = library::data_structure::lockfree::queue<task*>;
	public:
		inline explicit queue(void) noexcept = default;
		inline explicit queue(queue const&) noexcept = delete;
		inline explicit queue(queue&&) noexcept = delete;
		inline auto operator=(queue const&) noexcept -> queue & = delete;
		inline auto operator=(queue&&) noexcept -> queue & = delete;
		inline ~queue(void) noexcept = default;

		inline void push(task& task_) noexcept {
			_InterlockedIncrement(&_size);
			base::emplace(&task_);
			library::system_component::wait_on_address::wake_single(&_size);
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
			library::system_component::wait_on_address::wake_single(&_size);
		}
		inline bool wait(void* compare, unsigned long const wait_time) noexcept {
			return library::system_component::wait_on_address::wait(&_size, compare, sizeof(size_type), wait_time);
		}
	private:
		size_type _size = 0;
	};
	inline static auto less(task* const& source, task* const& destination) noexcept -> std::strong_ordering {
		return source->_time <=> destination->_time;
	}
	using ready = library::data_structure::priority_queue<task*, less>;
public:
	inline explicit scheduler(void) noexcept = default;
	inline explicit scheduler(scheduler const&) noexcept = delete;
	inline explicit scheduler(scheduler&&) noexcept = delete;
	inline auto operator=(scheduler const&) noexcept -> scheduler & = delete;
	inline auto operator=(scheduler&&) noexcept -> scheduler & = delete;
	inline ~scheduler(void) noexcept = default;

	inline void initialize(void) noexcept {
		_active = 0;
		_size = 0;
	}
	inline void finalize(void) noexcept {
		_InterlockedExchange(&_active, -1);
		_task_queue.wake();
		_thread.wait_for_single(INFINITE);
		_thread.close();
	}
	inline void push(task& task_) noexcept {
		_task_queue.push(task_);
	}
	inline bool wait(unsigned long const wait_time) noexcept {
		return _task_queue.wait(&_active, wait_time);
	}

	library::system_component::thread _thread;
	queue _task_queue;
	unsigned int _active;
	unsigned int _size;
};