#pragma once
#include "library/system-component/network/window_socket_api.h"
#include "library/system-component/input-output/completion_port.h"
#include "library/system-component/thread.h"
#include "library/system-component/network/socket.h"
#include "library/system-component/multi/wait_on_address.h"

#include "library/data-strucutre/serialize_buffer.h"
#include "library/data-strucutre/thread-local/memory_pool.h"
#include "library/data-strucutre/intrusive/shared_pointer.h"
#include "library/data-strucutre/lockfree/queue.h"
#include "library/data-strucutre/priority_queue.h"

#include "library/database/mysql.h"
//#include "library/database/redis.h"

#include "library/utility/command.h"
#include "library/utility/performance_data_helper.h"
#include "library/utility/logger.h"
#include "library/utility/crash_dump.h"

#include "library/design-pattern/singleton.h"

#include <optional>
#include <iostream>
#include <intrin.h>
#include <functional>

template<typename type>
concept string_size = std::_Is_any_of_v<type, unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long>;

class server final/* : public design_pattern::singleton<server>*/ {
	//friend class design_pattern::singleton<server>;
private:
	using size_type = unsigned int;
	using byte = unsigned char;
	enum class post_queue_state {
		close_worker, destory_session, excute_task
	};
	class scheduler final {
	public:
		class task final : public data_structure::intrusive::shared_pointer_hook<0> {
		public:
			template <typename function, typename... argument>
			inline explicit task(function&& func, argument&&... arg) noexcept
				: _time(0), _function(std::bind(std::forward<function>(func), std::forward<argument>(arg)...)) {
			};
			inline explicit task(task const&) noexcept = delete;
			inline explicit task(task&&) noexcept = delete;
			inline auto operator=(task const&) noexcept -> task & = delete;
			inline auto operator=(task&&) noexcept -> task & = delete;
			inline ~task(void) noexcept = default;

			friend inline static void destructor(task* rhs) {
				auto& memory_pool = data_structure::_thread_local::memory_pool<task>::instance();
				memory_pool.deallocate(*rhs);
			}
		public:
			unsigned long long _time;
			std::function<int(void)> _function;
		};
	private:
		inline static auto less(task* const& source, task* const& destination) noexcept -> std::strong_ordering {
			return source->_time <=> destination->_time;
		}
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

			inline void push(task* task_) noexcept {
				base::emplace(task_);
				_InterlockedIncrement(&_size);
			}
			inline auto pop(void) noexcept -> task* {
				unsigned long long head = _head;
				node* address = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & head);
				unsigned long long next = address->_next;

				if (0x10000 > (0x00007FFFFFFFFFFFULL & next))
					__debugbreak();
				unsigned long long tail = _tail;
				if (tail == head)
					_InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_tail), next, tail);

				task* result = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & next)->_value;
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
		public:
			size_type _size = 0;
		};
		using ready_queue = data_structure::priority_queue<task*, less>;
	public:
#pragma warning(suppress: 26495)
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
			_active = -1;
			_wait_on_address.wake_single(&_task_queue._size);
			_thread.wait_for_single(INFINITE);
			_thread.close();
		}
		template <typename function, typename... argument>
		inline void regist_task(function&& func, argument&&... arg) noexcept {
			if (0 == _active) {
				_InterlockedIncrement(&_size);
				if (0 == _active) {
					auto& memory_pool = data_structure::_thread_local::memory_pool<task>::instance();
					task* task_(&memory_pool.allocate(std::forward<function>(func), std::forward<argument>(arg)...));
					_task_queue.push(task_);
					_wait_on_address.wake_single(&_task_queue._size);
				}
				else {
					_InterlockedDecrement(&_size);
				}
			}
		}
	public:
		size_type _active;
		system_component::thread _thread;
		system_component::multi::wait_on_address _wait_on_address;
		task_queue _task_queue;
		ready_queue _ready_queue;
		size_type _size;
	};
	class session final {
	public:
		struct header final {
			inline explicit header(void) noexcept = default;
			inline explicit header(header const&) noexcept = delete;
			inline explicit header(header&&) noexcept = delete;
			inline auto operator=(header const&) noexcept -> header & = delete;
			inline auto operator=(header&&) noexcept -> header & = delete;
			inline ~header(void) noexcept = default;
			unsigned short _size;
		};
		class message final : public data_structure::intrusive::shared_pointer_hook<0>, public data_structure::serialize_buffer<> {
		public:
			inline explicit message(void) noexcept = delete;
			inline explicit message(message const&) noexcept = delete;
			inline explicit message(message&&) noexcept = delete;
			inline auto operator=(message const&) noexcept -> message & = delete;
			inline auto operator=(message&&) noexcept -> message & = delete;
			inline ~message(void) noexcept = delete;

			friend inline static void destructor(message* rhs) {
				auto& memory_pool = data_structure::_thread_local::memory_pool<message>::instance();
				memory_pool.deallocate(*rhs);
			}
		};
		using message_pointer = data_structure::intrusive::shared_pointer<message, 0>;
		class view final {
		private:
			using size_type = unsigned int;
		public:
			inline explicit view(void) noexcept
				: _front(0), _rear(0), _fail(false) {
			}
			inline explicit view(message_pointer message_) noexcept
				: _message(message_), _front(0), _rear(0), _fail(false) {
			}
			inline explicit view(message_pointer message_, size_type front, size_type rear) noexcept
				: _message(message_), _front(front), _rear(rear), _fail(false) {
			}
			inline view(view const& rhs) noexcept
				: _message(rhs._message), _front(rhs._front), _rear(rhs._rear), _fail(rhs._fail) {
			}
			inline auto operator=(view const& rhs) noexcept -> view&;
			inline auto operator=(view&& rhs) noexcept -> view&;
			inline ~view(void) noexcept = default;

			template<typename type>
				requires std::is_arithmetic_v<type>
			inline auto operator>>(type& value) noexcept -> view& {
				if (sizeof(type) + _front > _rear) {
					_fail = true;
					return *this;
				}
				value = reinterpret_cast<type&>(_message->data()[_front]);
				_front += sizeof(type);
				return *this;
			}
			inline void peek(byte* const buffer, size_type const length) noexcept {
				if (length + _front > _rear) {
					_fail = true;
					return;
				}
				memcpy(buffer, _message->data() + _front, length);
			}
			inline void pop(size_type const length) noexcept {
				_front += length;
			}
			inline auto front(void) const noexcept -> size_type {
				return _front;
			}
			inline auto rear(void) const noexcept -> size_type {
				return _rear;
			}
			inline void move_front(size_type const length) noexcept {
				_front += length;
			}
			inline void move_rear(size_type const length) noexcept {
				_rear += length;
			}
			inline auto size(void) const noexcept -> size_type {
				return _rear - _front;
			}
			inline auto begin(void) noexcept {
				return _message->data() + _front;
			}
			inline auto end(void) noexcept {
				return _message->data() + _rear;
			}
			inline auto data(void) noexcept -> message_pointer& {
				return _message;
			}
			inline void set(message* message_, size_type front, size_type rear) noexcept {
				_front = front;
				_rear = rear;
				_fail = false;
				_message.set(message_);
			}
			inline auto reset(void) noexcept {
				_message.reset();
			}
			inline operator bool(void) const noexcept {
				return !_fail;
			}
			inline auto fail(void) const noexcept {
				return _fail;
			}
		private:
			size_type _front, _rear;
			bool _fail = false;
			message_pointer _message;
		};
		class send_queue final : protected data_structure::lockfree::queue<message*> {
		private:
			using base = data_structure::lockfree::queue<message*>;
			class iterator final {
			public:
				inline explicit iterator(void) noexcept = default;
				inline explicit iterator(node* node_) noexcept
					: _node(node_) {
				}
				inline explicit iterator(iterator const&) noexcept = delete;
				inline explicit iterator(iterator&&) noexcept = delete;
				inline auto operator=(iterator const&) noexcept -> iterator & = delete;
				inline auto operator=(iterator&&) noexcept -> iterator & = delete;
				inline ~iterator(void) noexcept = default;

				inline auto operator*(void) const noexcept -> message*& {
					return _node->_value;
				}
				inline auto operator++(void) noexcept -> iterator& {
					_node = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & _node->_next);
					return *this;
				}
				inline bool operator==(iterator const& rhs) const noexcept {
					return _node == rhs._node;
				}
			private:
				node* _node;
			};
		public:
			inline explicit send_queue(void) noexcept = default;
			inline explicit send_queue(send_queue const&) noexcept = delete;
			inline explicit send_queue(send_queue&&) noexcept = delete;
			inline auto operator=(send_queue const&) noexcept -> send_queue & = delete;
			inline auto operator=(send_queue&&) noexcept -> send_queue & = delete;
			inline ~send_queue(void) noexcept = default;

			inline void push(message_pointer message_ptr) noexcept {
				base::emplace(message_ptr.get());
				message_ptr.reset();
			}
			inline auto pop(void) noexcept -> message_pointer {
				unsigned long long head = _head;
				node* address = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & head);
				unsigned long long next = address->_next;

				if (0x10000 > (0x00007FFFFFFFFFFFULL & next))
					__debugbreak();
				unsigned long long tail = _tail;
				if (tail == head)
					_InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_tail), next, tail);

				message_pointer result;
				result.set(reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & next)->_value);
				_head = next;
				_memory_pool::instance().deallocate(*address);
				return result;
			}
			inline auto begin(void) noexcept -> iterator {
				return iterator(reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & _head)->_next));
			}
			inline auto end(void) noexcept -> iterator {
				return iterator(reinterpret_cast<node*>(_nullptr));
			}
			inline auto empty(void) const noexcept {
				unsigned long long head = _head;
				node* address = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & head);
				unsigned long long next = address->_next;
				if (_nullptr == (0x00007FFFFFFFFFFFULL & next))
					return true;
				return false;
			}
		};
		inline static unsigned long long _static_id = 0x10000;
	public:
#pragma warning(suppress: 26495)
		inline explicit session(size_type const index) noexcept
			: _key(index) {
			auto& memory_pool = data_structure::_thread_local::memory_pool<message>::instance();
			message_pointer receive_message(&memory_pool.allocate());
			receive_message->clear();
			_receive_message = receive_message;
		};
		inline explicit session(session const&) noexcept = delete;
		inline explicit session(session&&) noexcept = delete;
		inline auto operator=(session const&) noexcept -> session & = delete;
		inline auto operator=(session&&) noexcept -> session & = delete;
		inline ~session(void) noexcept = default;

		inline void initialize(system_component::network::socket&& socket, unsigned long long timeout_duration) noexcept {
			_key = 0xffff & _key | _static_id;
			_static_id += 0x10000;
			_socket = std::move(socket);
			_timeout_currnet = GetTickCount64();
			_timeout_duration = timeout_duration;
			_InterlockedExchange(&_send_flag, 0);
			_InterlockedExchange(&_cancel_flag, 0);
			_InterlockedIncrement(&_io_count);
			_InterlockedAnd((long*)&_io_count, 0x7FFFFFFF);
		}
		inline auto acquire(void) noexcept -> bool {
			auto io_count = _InterlockedIncrement(&_io_count);
			if (0x80000000 & io_count)
				return false;
			return true;
		}
		inline auto acquire(unsigned long long key) noexcept -> bool {
			auto io_count = _InterlockedIncrement(&_io_count);
			if ((0x80000000 & io_count) || _key != key)
				return false;
			return true;
		}
		inline bool release(void) noexcept {
			if (0 == _InterlockedDecrement(&_io_count) && 0 == _InterlockedCompareExchange(&_io_count, 0x80000000, 0)) {
				_receive_message->clear();
				while (!_send_queue.empty())
					_send_queue.pop();
				_socket.close();
				return true;
			}
			return false;
		}
		inline bool receive(void) noexcept {
			if (0 == _cancel_flag) {
				WSABUF wsa_buffer{ message::capacity() - _receive_message->rear(),  reinterpret_cast<char*>(_receive_message->data() + _receive_message->rear()) };
				unsigned long flag = 0;
				_recv_overlapped.clear();
				int result = _socket.wsa_receive(&wsa_buffer, 1, &flag, _recv_overlapped);
				if (result != SOCKET_ERROR)
					return true;
				else if (GetLastError() == WSA_IO_PENDING) {
					if (1 == _cancel_flag)
						_socket.cancel_io_ex();
					return true;
				}
			}
			return false;
		}
		inline bool send(void) noexcept {
			if (0 == _cancel_flag) {
				while (!_send_queue.empty() && 0 == _InterlockedExchange(&_send_flag, 1)) {
					if (_send_queue.empty())
						_InterlockedExchange(&_send_flag, 0);
					else {
						WSABUF wsa_buffer[512];
						_send_size = 0;
						for (auto& iter : _send_queue) {
							if (512 <= _send_size) {
								cancel();
								return false;
							}
							wsa_buffer[_send_size].buf = reinterpret_cast<char*>(iter->data());
							wsa_buffer[_send_size].len = iter->rear();
							_send_size++;
						}
						_send_overlapped.clear();
						int result = _socket.wsa_send(wsa_buffer, _send_size, 0, _send_overlapped);
						if (result != SOCKET_ERROR)
							return true;
						else if (GetLastError() == WSA_IO_PENDING) {
							if (1 == _cancel_flag)
								_socket.cancel_io_ex();
							return true;
						}
						return false;
					}
				}
			}
			return false;
		}
		inline void cancel(void) noexcept {
			_InterlockedExchange(&_cancel_flag, 1);
			_socket.cancel_io_ex();
		}
		inline bool ready_receive(void) noexcept {
			if (_receive_message->size() != _receive_message->capacity()) {
				auto& memory_pool = data_structure::_thread_local::memory_pool<message>::instance();
				message_pointer receive_message(&memory_pool.allocate());
				receive_message->clear();
				if (0 < _receive_message->size()) {
					memcpy(receive_message->data(), _receive_message->data() + _receive_message->front(), _receive_message->size());
					receive_message->move_rear(_receive_message->size());
				}
				_receive_message = receive_message;
				return true;
			}
			cancel();
			log_message("server", utility::logger::level::info, L"session(%llu) close / reason : receive buffer full")
				return false;
		}
		inline void finish_send(void) noexcept {
			for (size_type index = 0; index < _send_size; ++index)
				_send_queue.pop();
			_InterlockedExchange(&_send_flag, 0);
		}
	public:
		unsigned long long _key;
		system_component::network::socket _socket;
		message_pointer _receive_message;
		send_queue _send_queue;
		system_component::input_output::overlapped _recv_overlapped;
		system_component::input_output::overlapped _send_overlapped;
		volatile unsigned int _io_count = 0x80000000; // release_flag
		volatile unsigned int _send_flag;
		volatile unsigned int _send_size;
		volatile unsigned int _cancel_flag;
		unsigned long long _timeout_currnet;
		unsigned long long _timeout_duration;
	};
	class session_array final {
	private:
		struct node final {
			inline explicit node(void) noexcept = delete;
			inline explicit node(node const&) noexcept = delete;
			inline explicit node(node&&) noexcept = delete;
			inline auto operator=(node const&) noexcept -> node & = delete;
			inline auto operator=(node&&) noexcept -> node & = delete;
			inline ~node(void) noexcept = delete;
			node* _next;
			session _value;
		};
		using iterator = node*;
	public:
#pragma warning(suppress: 26495)
		inline explicit session_array(void) noexcept = default;
		inline explicit session_array(session_array const&) noexcept = delete;
		inline explicit session_array(session_array&&) noexcept = delete;
		inline auto operator=(session_array const&) noexcept -> session_array & = delete;
		inline auto operator=(session_array&&) noexcept -> session_array & = delete;
		inline ~session_array(void) noexcept = default;

		inline void initialize(size_type const capacity) noexcept {
			_size = 0;
			_capacity = capacity;
			_array = reinterpret_cast<node*>(malloc(sizeof(node) * capacity));
			node* current = _array;
			node* next = current + 1;
			for (size_type index = 0; index < capacity - 1; ++index, current = next++) {
				current->_next = next;
				::new(reinterpret_cast<void*>(&current->_value)) session(index);
			}
#pragma warning(suppress: 6011)
			current->_next = nullptr;
			::new(reinterpret_cast<void*>(&current->_value)) session(capacity - 1);

			_head = reinterpret_cast<unsigned long long>(_array);
		}
		inline void finalize(void) noexcept {
			for (size_type index = 0; index < _capacity; ++index)
				_array[index]._value.~session();
			free(_array);
		}
		inline auto acquire(void) noexcept -> session* {
			for (;;) {
				unsigned long long head = _head;
				node* current = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & head);
				if (nullptr == current)
					return nullptr;
				unsigned long long next = reinterpret_cast<unsigned long long>(current->_next) + (0xFFFF800000000000ULL & head) + 0x0000800000000000ULL;
				if (head == _InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_head), next, head)) {
					_InterlockedIncrement(&_size);
					return &current->_value;
				}
			}
		}
		inline void release(session* value) noexcept {
			node* current = reinterpret_cast<node*>(reinterpret_cast<uintptr_t*>(value) - 1);
			for (;;) {
				unsigned long long head = _head;
				current->_next = reinterpret_cast<node*>(head & 0x00007FFFFFFFFFFFULL);
				unsigned long long next = reinterpret_cast<unsigned long long>(current) + (head & 0xFFFF800000000000ULL);
				if (head == _InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_head), next, head))
					break;
			}
			_InterlockedDecrement(&_size);
		}
		inline auto begin(void) const noexcept -> iterator {
			return _array;
		}
		inline auto end(void) const noexcept -> iterator {
			return _array + _capacity;
		}
		inline auto operator[](unsigned long long const key) noexcept -> session& {
			return _array[key & 0xffff]._value;
		}
	public:
		unsigned long long _head;
		node* _array;
		size_type _size;
		size_type _capacity;
	};
public:
#pragma warning(suppress: 26495)
	inline explicit server(void) noexcept {
		utility::crash_dump();
		database::mysql::initialize();
		system_component::network::window_socket_api::start_up();
		utility::logger::instance().create("server", L"server.log");

		auto& command_ = command::instance();
		command_.add("log_output", [&](command::parameter* param) noexcept -> int {
			unsigned char output = 0;
			for (size_type index = 1; index < param->size(); ++index) {
				if ("file" == param->get_string(index))
					output |= utility::logger::output::file;
				else if ("console" == param->get_string(index))
					output |= utility::logger::output::console;
			}
			utility::logger::instance().set_output(output);
			return 0;
			});
		command_.add("log_level", [&](command::parameter* param) noexcept -> int {
			if ("trace" == param->get_string(1))
				utility::logger::instance().set_level(utility::logger::level::trace);
			else if ("debug" == param->get_string(1))
				utility::logger::instance().set_level(utility::logger::level::debug);
			else if ("info" == param->get_string(1))
				utility::logger::instance().set_level(utility::logger::level::info);
			else if ("warning" == param->get_string(1))
				utility::logger::instance().set_level(utility::logger::level::warning);
			else if ("error" == param->get_string(1))
				utility::logger::instance().set_level(utility::logger::level::error);
			else if ("fatal" == param->get_string(1))
				utility::logger::instance().set_level(utility::logger::level::fatal);
			return 0;
			});

		command_.add("iocp_concurrent", [&](command::parameter* param) noexcept -> int {
			_concurrent_thread_count = param->get_int(1);
			return 0;
			});
		command_.add("iocp_worker", [&](command::parameter* param) noexcept -> int {
			_worker_thread_count = param->get_int(1);
			return 0;
			});
		command_.add("session_max", [&](command::parameter* param) noexcept -> int {
			_session_array_max = param->get_int(1);
			return 0;
			});
		command_.add("send_mode", [&](command::parameter* param) noexcept -> int {
			auto& send_mode = param->get_string(1);
			if ("fast" == send_mode)
				_send_mode = false;
			else if ("direct" == send_mode)
				_send_mode = true;
			return 0;
			});
		command_.add("send_frame", [&](command::parameter* param) noexcept -> int {
			_send_frame = param->get_int(1);
			return 0;
			});
		command_.add("timeout_duration", [&](command::parameter* param) noexcept -> int {
			_timeout_duration = param->get_int(1);
			return 0;
			});
		command_.add("timeout_frame", [&](command::parameter* param) noexcept -> int {
			_timeout_frame = param->get_int(1);
			return 0;
			});
		command_.add("tcp_ip", [&](command::parameter* param) noexcept -> int {
			_listen_socket_ip = param->get_string(1);
			return 0;
			});
		command_.add("tcp_port", [&](command::parameter* param) noexcept -> int {
			_listen_socket_port = param->get_int(1);
			return 0;
			});
		command_.add("tcp_backlog", [&](command::parameter* param) noexcept -> int {
			_listen_socket_backlog = param->get_int(1);
			return 0;
			});
		command_.add("server_start", [&](command::parameter* param) noexcept -> int {
			this->start();
			return 0;
			});
		command_.add("server_stop", [&](command::parameter* param) noexcept -> int {
			this->stop();
			return 0;
			});
	};
	inline explicit server(server const&) noexcept = delete;
	inline explicit server(server&&) noexcept = delete;
	inline auto operator=(server const&) noexcept -> server & = delete;
	inline auto operator=(server&&) noexcept -> server & = delete;
	inline ~server(void) noexcept {
		system_component::network::window_socket_api::clean_up();
		database::mysql::end();
	};

	inline void start(void) noexcept {
		_complation_port.create(_concurrent_thread_count);
		for (size_type index = 0; index < _worker_thread_count; ++index)
			_worker_thread.emplace_back(&server::worker, 0, this);
		_scheduler.initialize();
		_scheduler._thread.begin(&server::schedule, 0, this);

		_session_array.initialize(_session_array_max);
		if (0 != _send_frame)
			_scheduler.regist_task(&server::send, this);
		if (0 != _timeout_duration)
			_scheduler.regist_task(&server::timeout, this);
		auto& query = utility::performance_data_helper::query::instance();
		_processor_total_time = query.add_counter(L"\\Processor(_Total)\\% Processor Time");
		_processor_user_time = query.add_counter(L"\\Processor(_Total)\\% User Time");
		_processor_kernel_time = query.add_counter(L"\\Processor(_Total)\\% Privileged Time");
		_process_total_time = query.add_counter(L"\\Process(server)\\% Processor Time");
		_process_user_time = query.add_counter(L"\\Process(server)\\% User Time");
		_process_kernel_time = query.add_counter(L"\\Process(server)\\% Privileged Time");
		_memory_available_byte = query.add_counter(L"\\Memory\\Available Bytes");
		_memory_pool_nonpaged_byte = query.add_counter(L"\\Memory\\Pool Nonpaged Bytes");
		_process_private_byte = query.add_counter(L"\\Process(server)\\Private Bytes");
		_process_pool_nonpaged_byte = query.add_counter(L"\\Process(server)\\Pool Nonpaged Bytes");
		_tcpv4_segments_received_sec = query.add_counter(L"\\TCPv4\\Segments Received/sec");
		_tcpv4_segments_sent_sec = query.add_counter(L"\\TCPv4\\Segments Sent/sec");
		_tcpv4_segments_retransmitted_sec = query.add_counter(L"\\TCPv4\\Segments Retransmitted/sec");
		_scheduler.regist_task(&server::monit, this);

		on_start();

		system_component::network::socket_address_ipv4 socket_address;
		socket_address.set_address(_listen_socket_ip.c_str());
		socket_address.set_port(_listen_socket_port);
		_listen_socket.create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		_listen_socket.set_linger(1, 0);
		if (true == _send_mode)
			_listen_socket.set_send_buffer(0);
		_listen_socket.bind(socket_address);
		_listen_socket.listen(_listen_socket_backlog);
		_accept_thread.begin(&server::accept, 0, this);
	}
	inline void stop(void) noexcept {
		_listen_socket.close();
		_accept_thread.wait_for_single(INFINITE);
		_accept_thread.close();
		for (auto& iter : _session_array) {
			if (iter._value.acquire())
				iter._value.cancel();
			if (iter._value.release()) {
				on_destroy_session(iter._value._key);
				_session_array.release(&iter._value);
			}
		}
		while (_session_array._size != 0) {
		}

		on_stop();

		_scheduler.finalize();
		_session_array.finalize();

		for (size_type index = 0; index < _worker_thread.size(); ++index)
			_complation_port.post_queue_state(0, 0, nullptr);
		HANDLE handle[128];
		for (unsigned int index = 0; index < _worker_thread.size(); ++index)
			handle[index] = _worker_thread[index].data();
		system_component::handle::wait_for_multiple(_worker_thread.size(), handle, true, INFINITE);
		_worker_thread.clear();
		_complation_port.close();
	}
private:
	inline void accept(void) noexcept {
		for (;;) {
			auto [socket, socket_address] = _listen_socket.accept();
			if (INVALID_SOCKET == socket.data())
				break;
			++_accept_total_count;
			++_accept_tps;
			if (false == on_accept_socket(socket_address))
				socket.close();
			else {
				session& session_ = *_session_array.acquire();
				session_.initialize(std::move(socket), _timeout_duration);

				_complation_port.connect(session_._socket, reinterpret_cast<ULONG_PTR>(&session_));
				on_create_session(session_._key);

				if (!session_.receive() && session_.release()) {
					on_destroy_session(session_._key);
					_session_array.release(&session_);
				}
			}
		}
	}
	inline void worker(void) noexcept {
		for (;;) {
			auto [result, transferred, key, overlapped] = _complation_port.get_queue_state(INFINITE);
			switch (static_cast<post_queue_state>(key)) {
				using enum post_queue_state;
			case close_worker:
				return;
			case destory_session: {
				session& session_ = *reinterpret_cast<session*>(overlapped);
				on_destroy_session(session_._key);
				_session_array.release(&session_);
			} break;
			case excute_task: {
				scheduler::task& task = *reinterpret_cast<scheduler::task*>(overlapped);
				int time;
				do
					time = task._function();
				while (time == 0);

				if (-1 == time) {
					_InterlockedDecrement(&_scheduler._size);
					auto& memory_pool = data_structure::_thread_local::memory_pool<scheduler::task>::instance();
					memory_pool.deallocate(task);
				}
				else {
					task._time = time + GetTickCount64();
					_scheduler._task_queue.push(&task);
					_scheduler._wait_on_address.wake_single(&_scheduler._task_queue._size);
				}
			} break;
			default: {
				session& session_ = *reinterpret_cast<session*>(key);
				if (0 != transferred) {
					if (&session_._recv_overlapped.data() == overlapped) {
						session_._receive_message->move_rear(transferred);
						session_._timeout_currnet = GetTickCount64();

						for (;;) {
							if (sizeof(session::header) > session_._receive_message->size())
								break;
							session::header header_;
							session_._receive_message->peek(reinterpret_cast<unsigned char*>(&header_), sizeof(session::header));
							if (sizeof(session::header) + header_._size > session_._receive_message->size())
								break;
							session_._receive_message->pop(sizeof(session::header));

							session::view view_(session_._receive_message, session_._receive_message->front(), session_._receive_message->front() + header_._size);
							session_._receive_message->pop(header_._size);
							if (false == on_receive_session(session_._key, view_)) {
								session_.cancel();
								break;
							}
							_InterlockedIncrement(&_receive_tps);
						}
						if (session_.ready_receive() && session_.receive())
							continue;
					}
					else {
						_interlockedadd((volatile long*)&_send_tps, session_._send_size);
						session_.finish_send();
						if (0 == _send_frame && session_.send())
							continue;
					}
				}
				if (session_.release()) {
					on_destroy_session(session_._key);
					_session_array.release(&session_);
				}
			} break;
			}
		}
	}
	inline void schedule(void) {
		unsigned long wait_time = INFINITE;
		while (0 == _scheduler._active || 0 != _scheduler._size) {
			bool result = _scheduler._wait_on_address.wait(&_scheduler._task_queue._size, &_scheduler._active, sizeof(size_type), wait_time);
			if (result) {
				while (!_scheduler._task_queue.empty())
					_scheduler._ready_queue.push(_scheduler._task_queue.pop());
			}
			wait_time = INFINITE;
			unsigned long long time = GetTickCount64();
			while (!_scheduler._ready_queue.empty()) {
				auto task_ = _scheduler._ready_queue.top();
				if (time >= task_->_time) {
					_scheduler._ready_queue.pop();
					_complation_port.post_queue_state(0, static_cast<uintptr_t>(post_queue_state::excute_task), reinterpret_cast<OVERLAPPED*>(task_));
				}
				else {
					wait_time = static_cast<unsigned long>(task_->_time - time);
					break;
				}
			}
		}
	}
	inline int send(void) noexcept {
		for (auto& iter : _session_array) {
			if (iter._value.acquire()) {
				if (iter._value.send())
					continue;
			}
			if (iter._value.release()) {
				on_destroy_session(iter._value._key);
				_session_array.release(&iter._value);
			}
		}
		if (-1 == _scheduler._active)
			return -1;
		return _send_frame;
	}
	inline int timeout(void) noexcept {
		for (auto& iter : _session_array) {
			if (iter._value.acquire()) {
				if (iter._value._timeout_currnet + iter._value._timeout_duration < GetTickCount64()) {
					iter._value.cancel();
					++_timeout_total_count;
				}
			}
			if (iter._value.release()) {
				on_destroy_session(iter._value._key);
				_session_array.release(&iter._value);
			}
		}
		if (-1 == _scheduler._active)
			return -1;
		return _timeout_frame;
	}
	inline int monit(void) noexcept {
		system("cls");
		auto& query = utility::performance_data_helper::query::instance();
		query.collect_query_data();

		SYSTEM_INFO info;
		GetSystemInfo(&info);
		printf("--------------------------------------\n"\
			"[ System Monitor ]\n"\
			"CPU Usage\n"\
			" System  - Total  :   %f %%\n"\
			"           User   :   %f %%\n"\
			"           Kernel :   %f %%\n"\
			" Process - Total  :   %f %%\n"\
			"           User   :   %f %%\n"\
			"           Kernel :   %f %%\n"\
			"Memory Usage\n"\
			" System  - Available :   %f GB\n"\
			"           Non-Paged :   %f MB\n"\
			" Process - Private   :   %f MB\n"\
			"           Non-Paged :   %f MB\n"\
			"Network Usage\n"\
			" Receive        :   %f\n"\
			" Send           :   %f\n"\
			" Retransmission :   %f\n",
			_processor_total_time.get_formatted_value(PDH_FMT_DOUBLE).doubleValue,
			_processor_user_time.get_formatted_value(PDH_FMT_DOUBLE).doubleValue,
			_processor_kernel_time.get_formatted_value(PDH_FMT_DOUBLE).doubleValue,
			_process_total_time.get_formatted_value(PDH_FMT_DOUBLE/* | PDH_FMT_NOCAP100*/).doubleValue/* / info.dwNumberOfProcessors*/,
			_process_user_time.get_formatted_value(PDH_FMT_DOUBLE/* | PDH_FMT_NOCAP100*/).doubleValue/* / info.dwNumberOfProcessors*/,
			_process_kernel_time.get_formatted_value(PDH_FMT_DOUBLE/* | PDH_FMT_NOCAP100*/).doubleValue/* / info.dwNumberOfProcessors*/,
			_memory_available_byte.get_formatted_value(PDH_FMT_DOUBLE).doubleValue / 0x40000000,
			_memory_pool_nonpaged_byte.get_formatted_value(PDH_FMT_DOUBLE).doubleValue / 0x100000,
			_process_private_byte.get_formatted_value(PDH_FMT_DOUBLE).doubleValue / 0x100000,
			_process_pool_nonpaged_byte.get_formatted_value(PDH_FMT_DOUBLE).doubleValue / 0x100000,
			_tcpv4_segments_received_sec.get_formatted_value(PDH_FMT_DOUBLE).doubleValue,
			_tcpv4_segments_sent_sec.get_formatted_value(PDH_FMT_DOUBLE).doubleValue,
			_tcpv4_segments_retransmitted_sec.get_formatted_value(PDH_FMT_DOUBLE).doubleValue);

		auto& memory_pool = data_structure::_thread_local::memory_pool<session::message>::instance();
		printf("--------------------------------------\n"\
			"[ Server Monitor ]\n"\
			"Connect\n"\
			" Accept Total  :   %llu\n"\
			" Timeout Total :   %llu\n"\
			" Session Count :   %u\n"\
			"Traffic\n"\
			" Accept  :   %u TPS\n"\
			" Receive :   %u TPS\n"\
			" Send    :   %u TPS\n"\
			"Resource Usage\n"\
			" Message  - Pool Count :   %u\n"\
			"            Use Count  :   %u\n",
			_accept_total_count, _timeout_total_count, _session_array._size, _accept_tps, _receive_tps, _send_tps,
			memory_pool._stack._capacity, memory_pool._use_count);
		_accept_tps = 0;
		_receive_tps = 0;
		_send_tps = 0;

		on_monit();

		if (-1 == _scheduler._active)
			return -1;
		return 1000;
	}
protected:
	inline virtual void on_start(void) noexcept {};
	inline virtual void on_stop(void) noexcept {};
	inline virtual bool on_accept_socket(system_component::network::socket_address_ipv4& socket_address) noexcept {
		return true;
	}
	inline virtual void on_create_session(unsigned long long key) noexcept {
		//session::message_pointer message_ = make_message();
		//*message_ << 0x7fffffffffffffff;
		//do_send_session(key, message_);
	}
	inline virtual bool on_receive_session(unsigned long long key, session::view& view_) noexcept {
		unsigned long long value;
		view_ >> value;
		session::message_pointer message_ = make_message();
		*message_ << value;
		do_set_timeout(key, 3000);
		do_send_session(key, message_);
		return true;
	}
	inline virtual void on_destroy_session(unsigned long long key) noexcept {
	}
	inline virtual void on_monit(void) noexcept {
	}
	inline void do_send_session(unsigned long long key, session::message_pointer& message_ptr) noexcept {
		session& session_ = _session_array[key];
		if (session_.acquire(key)) {
			session_._send_queue.push(message_ptr);
			if (0 == _send_frame && session_.send())
				return;
		}
		if (session_.release())
			_complation_port.post_queue_state(0, static_cast<uintptr_t>(post_queue_state::destory_session), reinterpret_cast<OVERLAPPED*>(&session_));
	}
	inline void do_destroy_session(unsigned long long key) noexcept {
		session& session_ = _session_array[key];
		if (session_.acquire(key))
			session_.cancel();
		if (session_.release())
			_complation_port.post_queue_state(0, static_cast<uintptr_t>(post_queue_state::destory_session), reinterpret_cast<OVERLAPPED*>(&session_));
	}
	inline void do_set_timeout(unsigned long long key, unsigned long long duration) noexcept {
		session& session_ = _session_array[key];
		if (session_.acquire(key))
			session_._timeout_duration = duration;
		if (session_.release())
			_complation_port.post_queue_state(0, static_cast<uintptr_t>(post_queue_state::destory_session), reinterpret_cast<OVERLAPPED*>(&session_));
	}
	inline static auto make_message(void) noexcept -> session::message_pointer {
		auto& memory_pool = data_structure::_thread_local::memory_pool<session::message>::instance();
		session::message_pointer message_(&memory_pool.allocate());
		message_->clear();

		session::header header_;
		header_._size = 8;
		message_->push(reinterpret_cast<unsigned char*>(&header_), sizeof(session::header));
		return message_;
	}
	template <typename function, typename... argument>
	inline void register_task(function&& func, argument&&... arg) noexcept {
		_scheduler.regist_task(std::forward<function>(func), std::forward<argument>(arg)...);
	}
private:
	system_component::input_output::completion_port _complation_port;
	data_structure::vector<system_component::thread> _worker_thread;
	scheduler _scheduler;
	session_array _session_array;
	system_component::network::socket _listen_socket;
	system_component::thread _accept_thread;
public:
	size_type _concurrent_thread_count;
	size_type _worker_thread_count;
	size_type _session_array_max;
	bool _send_mode;
	size_type _send_frame;
	unsigned long long _timeout_duration;
	size_type _timeout_frame;
	std::string _listen_socket_ip;
	size_type _listen_socket_port;
	size_type _listen_socket_backlog;
	unsigned char _header_fixed_key;

	utility::performance_data_helper::query::counter _processor_total_time;
	utility::performance_data_helper::query::counter _processor_user_time;
	utility::performance_data_helper::query::counter _processor_kernel_time;
	utility::performance_data_helper::query::counter _process_total_time;
	utility::performance_data_helper::query::counter _process_user_time;
	utility::performance_data_helper::query::counter _process_kernel_time;
	utility::performance_data_helper::query::counter _memory_available_byte;
	utility::performance_data_helper::query::counter _memory_pool_nonpaged_byte;
	utility::performance_data_helper::query::counter _process_private_byte;
	utility::performance_data_helper::query::counter _process_pool_nonpaged_byte;
	utility::performance_data_helper::query::counter _tcpv4_segments_received_sec;
	utility::performance_data_helper::query::counter _tcpv4_segments_sent_sec;
	utility::performance_data_helper::query::counter _tcpv4_segments_retransmitted_sec;
	unsigned long long _accept_total_count = 0;
	unsigned long long _timeout_total_count = 0;
	size_type _accept_tps = 0;
	size_type _receive_tps = 0;
	size_type _send_tps = 0;
};