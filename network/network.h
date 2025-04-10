#pragma once
#include "library/system-component/inputoutput_completion_port.h"
#include "library/system-component/thread.h"
#include "library/system-component/socket.h"
#include "library/system-component/wait_on_address.h"
#include "library/system-component/time/multimedia.h"

#include "library/data-structure/serialize_buffer.h"
#include "library/data-structure/thread-local/memory_pool.h"
#include "library/data-structure/intrusive/shared_pointer.h"
#include "library/data-structure/intrusive/list.h"
#include "library/data-structure/lockfree/queue.h"
#include "library/data-structure/priority_queue.h"

#include "library/database/mysql.h"
#include "library/database/redis.h"

//#include "library/utility/command.h"
//#include "library/utility/performance_data_helper.h"
//#include "library/utility/logger.h"
//#include "library/utility/crash_dump.h"

#include "library/design-pattern/singleton.h"

#include <optional>
#include <iostream>
#include <intrin.h>
#include <functional>

namespace framework {
	class network : public library::design_pattern::singleton<network> {
	private:
		friend class library::design_pattern::singleton<network>;
		enum class type : unsigned char {
			close, destory_session, task, destory_group
		};
		class server {
		private:
			enum class type : unsigned long long {
				accept = 0x0000800000000000ULL
			};
			class listen final {
			public:
				struct accept {
					inline explicit accept(void) noexcept
						: _socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), _overlapped() {
					};
					inline explicit accept(accept const&) noexcept = delete;
					inline explicit accept(accept&&) noexcept = delete;
					inline auto operator=(accept const&) noexcept -> accept & = delete;
					inline auto operator=(accept&&) noexcept -> accept & = delete;
					inline ~accept(void) noexcept = default;

					library::system_component::socket _socket;
					unsigned char _buffer[30];
					library::system_component::overlapped _overlapped;
				};

				inline void initialize(std::string_view const ip, unsigned short const port, int send_buffer, int const backlog) noexcept {
					library::system_component::socket_address_ipv4 socket_address;
					socket_address.set_address(ip.data());
					socket_address.set_port(port);
					_socket.create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
					_socket.set_option_linger(1, 0);
					_socket.set_option_send_buffer(send_buffer);
					_socket.bind(socket_address);
					_socket.listen(backlog);
				}

				library::system_component::socket _socket;
				accept* _accept;
			};
			class session;
		public:
			inline explicit server(void) noexcept
				: _network(network::instance()) {
			};
			inline explicit server(server const&) noexcept = delete;
			inline explicit server(server&&) noexcept = delete;
			inline auto operator=(server const&) noexcept -> server & = delete;
			inline auto operator=(server&&) noexcept -> server & = delete;
			inline ~server(void) noexcept = default;
		public:
			inline void accept(std::string_view const ip, unsigned short const port, int send_buffer, int const backlog) noexcept {
				_listen.initialize(ip, port, send_buffer, backlog);

				_network._complation_port.connect(_listen._socket, static_cast<ULONG_PTR>(type::accept) + reinterpret_cast<ULONG_PTR>(this));

				_listen._accept = reinterpret_cast<listen::accept*>(malloc(sizeof(listen::accept) * 16));
				listen::accept* accept_ = _listen._accept;
				for (unsigned short index = 0; index < 16; ++index, ++accept_) {
					::new(reinterpret_cast<void*>(accept_)) listen::accept();
					_listen._socket.accept_ex(accept_->_socket, accept_->_buffer, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, accept_->_overlapped);
				}
			}
			inline void connect(void) noexcept {

			}
		private:
			inline void worker(bool result, DWORD transferred, ULONG_PTR key, OVERLAPPED* _overlapped) noexcept {
				switch (static_cast<type>(0xFFFF800000000000ULL & key)) {
				case type::accept: {
				} break;
				default:
					break;
				}
			}

			//inline virtual bool accept(system_component::socket_address_ipv4& socket_address) noexcept = 0;
			network& _network;
			listen _listen;

		};
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
			class queue final : protected library::data_structure::lockfree::queue<task*> {
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

		inline explicit network(void) noexcept {
			library::system_component::time::multimedia::end_period(1);
			library::system_component::wsa_start_up();
			library::system_component::socket socket_(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			socket_.wsa_io_control_acccept_ex();
		};
		inline explicit network(network const&) noexcept = delete;
		inline explicit network(network&&) noexcept = delete;
		inline auto operator=(network const&) noexcept -> network & = delete;
		inline auto operator=(network&&) noexcept -> network & = delete;
		inline ~network(void) noexcept {
			library::system_component::wsa_clean_up();
			library::system_component::time::multimedia::end_period(1);
		};
	public:
		inline void start(unsigned long const concurrent_thread, unsigned short worker_thread) noexcept {
			_complation_port.create(concurrent_thread);
			for (unsigned short index = 0; index < worker_thread; ++index)
				_worker_thread.emplace_back(&network::worker, 0, this);

			_scheduler.initialize();
			_scheduler._thread.begin(&network::schedule, 0, this);
		}
		inline void stop(void) noexcept {

		}
	private:
		inline void worker(void) noexcept {
			auto [result, transferred, key, overlapped] = _complation_port.get_queue_state(INFINITE);
			switch (static_cast<type>(key)) {
				using enum type;
			case task:
				break;
			default:
				break;
				//reinterpret_cast<server*>(0x00007FFFFFFFFFFFULL & key)->worker(result, transferred, key, overlapped);
			}
		}
		inline void schedule(void) noexcept {
			scheduler::ready ready;
			unsigned long wait_time = INFINITE;
			while (0 == _scheduler._active || 0 != _scheduler._size) {
				if (_scheduler.wait(wait_time))
					while (!_scheduler._task_queue.empty())
						ready.push(&_scheduler._task_queue.pop());
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