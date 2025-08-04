#pragma once
#include "library/inputoutput_complet_port.h"
#include "library/thread.h"
#include "library/vector.h"

namespace framework {
	class iocp final {
		using size_type = unsigned int;
		library::inputoutput_complet_port _complet_port;
		library::vector<library::thread> _worker_thread;
	public:
		class object {
		public:
			inline virtual void worker(bool result, unsigned long transferred, uintptr_t key, OVERLAPPED* overlapped) noexcept = 0;
		};

		inline void start(size_type concurrent_thread, size_type worker_thread) noexcept {
			_complet_port.create(concurrent_thread);
			for (size_type index = 0; index < worker_thread; ++index)
				_worker_thread.emplace_back(&iocp::worker, 0, this);
		}
		inline void stop(void) noexcept {
			for (size_type index = 0; index < _worker_thread.size(); ++index)
				_complet_port.post_queue_state(0, 0, nullptr);
			HANDLE handle[128];
			for (unsigned int index = 0; index < _worker_thread.size(); ++index)
				handle[index] = _worker_thread[index].data();
			library::handle::wait_for_multiple(_worker_thread.size(), handle, true, INFINITE);
			_worker_thread.clear();
			_complet_port.close();
		}
		inline void worker(void) noexcept {
			for (;;) {
				auto [result, transferred, key, overlapped] = _complet_port.get_queue_state(INFINITE);
				switch (key) {
				case 0:
					break;
				default:
					break;
				}
			}
		}

		inline void connect(library::socket& socket, uintptr_t key) noexcept {
			_complet_port.connect(socket, key);
		}
		inline void post(unsigned long transferred, uintptr_t key, OVERLAPPED* overlapped) noexcept {
			_complet_port.post_queue_state(transferred, key, overlapped);
		}
	};
}