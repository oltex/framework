#pragma once


#include "../library/system-component/inputoutput_completion_port.h"
#include "../library/system-component/socket.h"
#include "../library/system-component/thread.h"
#include "../library/system-component/time/multimedia.h"
#include "../library/system-component/wait_on_address.h"

#include "../library/data-structure/vector.h"
#include "../library/design-pattern/singleton.h"

#include "scheduler/scheduler.h"

//#include "../library/system-component/interlocked.h"
//#include "../library/data-structure/intrusive/shared_pointer.h"
//#include "../library/data-structure/lockfree/queue.h"
//#include "../library/data-structure/thread-local/memory_pool.h"
//#include "../library/data-structure/serialize_buffer.h"

#include <functional>

class network : public design_pattern::singleton<network> {
private:
	friend class design_pattern::singleton<network>;
	friend class server;
	enum class type : unsigned char {
		close, destory_session, task, destory_group
	};

	inline explicit network(void) noexcept {
		system_component::time::multimedia::end_period(1);
		system_component::wsa_start_up();
		system_component::socket socket_(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		socket_.wsa_io_control_acccept_ex();
	};
	inline explicit network(network const&) noexcept = delete;
	inline explicit network(network&&) noexcept = delete;
	inline auto operator=(network const&) noexcept -> network & = delete;
	inline auto operator=(network&&) noexcept -> network & = delete;
	inline ~network(void) noexcept {
		system_component::wsa_clean_up();
		system_component::time::multimedia::end_period(1);
	};
public:
	inline void start(void) noexcept {
		_complation_port.create(0);
		for (unsigned short index = 0; index < 16; ++index)
			_worker_thread.emplace_back(&network::worker, 0, this);

		_scheduler.initialize();
		_scheduler._thread.begin(&network::schedule, 0, this);
	}
	inline void stop(void) noexcept {

	}
private:
	inline void worker(void) noexcept;
	inline void schedule(void) noexcept {
		scheduler::ready ready;
		unsigned long wait_time = INFINITE;
		while (0 == _scheduler._active || 0 != _scheduler._size) {
			if (_scheduler.wait(wait_time))
				while (!_scheduler._task_queue.empty())
					ready.push(&_scheduler._task_queue.pop());
			wait_time = INFINITE;
			unsigned long time = system_component::time::multimedia::get_time();
			while (!ready.empty()) {
				auto task_ = ready.top();
				if (time < task_->_time) {
					wait_time = static_cast<unsigned long>(task_->_time - time);
					break;
				}
				ready.pop();
				_complation_port.post_queue_state(0, static_cast<uintptr_t>(type::task), reinterpret_cast<OVERLAPPED*>(task_));
			}
		}
	}

	system_component::inputoutput_completion_port _complation_port;
	data_structure::vector<system_component::thread> _worker_thread;
	scheduler _scheduler;
};