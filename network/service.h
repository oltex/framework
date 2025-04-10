#pragma once
#include "library/system-component/inputoutput_completion_port.h"
#include "library/system-component/thread.h"
#include "library/system-component/socket.h"
#include "library/system-component/time/multimedia.h"
//
//#include "library/data-structure/serialize_buffer.h"
//#include "library/data-structure/thread-local/memory_pool.h"
//#include "library/data-structure/intrusive/shared_pointer.h"
//#include "library/data-structure/intrusive/list.h"
//#include "library/data-structure/lockfree/queue.h"
//#include "library/data-structure/priority_queue.h"
#include "library/data-structure/vector.h"
//
//#include "library/database/mysql.h"
//#include "library/database/redis.h"
//
#include "library/design-pattern/singleton.h"
//
//#include <optional>
//#include <iostream>
//#include <intrin.h>
//#include <functional>

#include "scheduler.h"

namespace framework::network {
	class server : public library::design_pattern::singleton<server> {
	private:
		friend class library::design_pattern::singleton<server>;
		enum class type : unsigned char {
			stop, task, destory_session, destory_group
		};

		inline explicit server(void) noexcept {
			library::system_component::time::multimedia::end_period(1);
			library::system_component::wsa_start_up();
			library::system_component::socket socket_(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			socket_.wsa_io_control_acccept_ex();
		};
		inline explicit server(server const&) noexcept = delete;
		inline explicit server(server&&) noexcept = delete;
		inline auto operator=(server const&) noexcept -> server & = delete;
		inline auto operator=(server&&) noexcept -> server & = delete;
		inline ~server(void) noexcept {
			library::system_component::wsa_clean_up();
			library::system_component::time::multimedia::end_period(1);
		};
	public:
		inline void start(unsigned long const concurrent_thread, unsigned short worker_thread) noexcept {
			_complation_port.create(concurrent_thread);
			for (unsigned short index = 0; index < worker_thread; ++index)
				_worker_thread.emplace_back(&server::worker, 0, this);

			//_scheduler.initialize();
			//_scheduler._thread.begin(&server::schedule, 0, this);
		}
		inline void stop(void) noexcept {
			for (unsigned short index = 0; index < _worker_thread.size(); ++index)
				_complation_port.post_queue_state(0, static_cast<uintptr_t>(type::stop), nullptr);
			library::system_component::handle::wait_for_multiple(_worker_thread, true, INFINITE);

		}
	private:
		inline void worker(void) noexcept {
			for (;;) {
				auto [result, transferred, key, overlapped] = _complation_port.get_queue_state(INFINITE);
				switch (static_cast<type>(key)) {
					using enum type;
				case stop:
					return;
				case task:
					break;
				default:
					break;
					//reinterpret_cast<server*>(0x00007FFFFFFFFFFFULL & key)->worker(result, transferred, key, overlapped);
				}
			}
		}
		inline void schedule(void) noexcept {
			scheduler::ready ready;
			unsigned long wait_time = INFINITE;
			while (0 == _scheduler._active || 0 != _scheduler._size) {
				if (_scheduler.wait(wait_time))
					while (!_scheduler._queue.empty())
						ready.push(&_scheduler._queue.pop());
				wait_time = INFINITE;
				unsigned long time = library::system_component::time::multimedia::get_time();
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

		library::system_component::inputoutput_completion_port _complation_port;
		library::data_structure::vector<library::system_component::thread> _worker_thread;
		scheduler _scheduler;
	};
}