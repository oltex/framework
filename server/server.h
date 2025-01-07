#pragma once
#include "library/system-component/network/window_socket_api.h"
#include "library/system-component/input_output/completion_port.h"
#include "library/system-component/multi/thread.h"
#include "library/system-component/network/socket.h"

#include "library/data-strucutre/vector.h"
#include <optional>

class server final {
private:
	using size_type = unsigned int;
	struct session final {
	private:
		inline static unsigned long long _static_id = 0x10000;
	public:
		inline explicit session(void) noexcept = default;
		inline explicit session(session const&) noexcept = delete;
		inline explicit session(session&&) noexcept = delete;
		inline auto operator=(session const&) noexcept -> session & = delete;
		inline auto operator=(session&&) noexcept -> session & = delete;
		inline ~session(void) noexcept = default;
	public:
		inline void initialize(system_component::network::socket&& socket) noexcept {
			_key = 0xffff & _key | _static_id;
			_static_id += 0x10000;
			_socket = std::move(socket);

		}
		inline void finalize(void) noexcept {
		}
	public:
		unsigned long long _key;
		system_component::network::socket _socket;
		//receive_buffer* _receive_buffer;
		//data_structure::circular_queue<data_structure::serialize_buffer*> _send_queue;
		system_component::input_output::overlapped _recv_overlapped;
		system_component::input_output::overlapped _send_overlapped;
		volatile unsigned int _io_count;
		volatile unsigned int _send_flag;
		volatile unsigned int _send_size;
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
		inline explicit session_array(void) noexcept = default;
		inline explicit session_array(session_array const&) noexcept = delete;
		inline explicit session_array(session_array&&) noexcept = delete;
		inline auto operator=(session_array const&) noexcept -> session_array & = delete;
		inline auto operator=(session_array&&) noexcept -> session_array & = delete;
		inline ~session_array(void) noexcept = default;
	public:
		inline void initialize(size_type const size) noexcept {
			_size = size;
			_array = reinterpret_cast<node*>(malloc(sizeof(node) * size));
			node* current = _array;
			node* next = current + 1;
			for (size_type index = 0; index < size - 1; ++index, current = next++) {
				current->_next = next;
				::new(reinterpret_cast<void*>(&current->_value)) session();
				current->_value._key = index;
			}
			current->_next = nullptr;
			::new(reinterpret_cast<void*>(&current->_value)) session();
			current->_value._key = size - 1;

			_head = reinterpret_cast<unsigned long long>(_array);
		}
		inline void finalize(void) noexcept {
		}
	public:
		inline auto acquire(void) noexcept -> session* {
			for (;;) {
				unsigned long long head = _head;
				node* current = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & head);
				if (nullptr == current)
					return nullptr;
				unsigned long long next = reinterpret_cast<unsigned long long>(current->_next) + (0xFFFF800000000000ULL & head) + 0x0000800000000000ULL;
				if (head == _InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_head), next, head))
					return &current->_value;
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
		}
		inline auto begin(void) noexcept -> iterator {
			return _array;
		}
		inline auto end(void) noexcept -> iterator {
			return _array + _size;
		}
		inline auto operator[](size_type const key) noexcept -> session& {
			return _array[key & 0xffff]._value;
		}
	private:
		unsigned long long _head;
		node* _array;
		size_type _size;
	};
public:
	inline explicit server(void) noexcept {
		system_component::network::window_socket_api::start_up();
	};
	inline explicit server(server const& rhs) noexcept = delete;
	inline explicit server(server&& rhs) noexcept = delete;
	inline auto operator=(server const& rhs) noexcept -> server & = delete;
	inline auto operator=(server&& rhs) noexcept -> server & = delete;
	inline ~server(void) noexcept {
		system_component::network::window_socket_api::clean_up();
	};
public:
	inline void start(void) noexcept { // concurrent_thread, worker_thread, ip, port, backlog_queue,   session_max
		_complation_port.create(0);
		_thread.emplace_back(&server::accept, CREATE_SUSPENDED, this);
		for (size_type index = 0; index < 16; ++index)
			_thread.emplace_back(&server::worker, 0, this);

		_session_array.initialize(10000);

		system_component::network::socket_address_ipv4 socket_address;
		socket_address.set_address(INADDR_ANY);
		socket_address.set_port(20000);
		_listen_socket.create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		_listen_socket.set_linger(1, 0);
		_listen_socket.set_send_buffer(0);
		_listen_socket.bind(socket_address);
		_listen_socket.listen(65535);

		_thread[0].resume();
	}
	inline void stop(void) noexcept {
		//_listen_socket.close();
		//for (auto& iter : _session) {
		//	iter._value._socket.close();
		//	_session.release(iter._value);
		//}

		//HANDLE handle[128];
		//for (unsigned int index = 0; index < _thread.size(); ++index) {
		//	_completion.post_queue_state(0, 0, nullptr);
		//	handle[index] = _thread[index].data();
		//}
		system_component::multi::thread::wait_for_multiple(_thread, true, INFINITE);
		//_thread.clear();

		//_completion.close();
	}
public:
	inline void accept(void) noexcept {
		for (;;) {
			auto [socket, socket_address] = _listen_socket.accept();
			if (INVALID_SOCKET == socket.data())
				break;

			if (false == on_accept_socket(socket_address))
				socket.close();
			else {
				session* session_ = _session_array.acquire();
				session_->initialize(std::move(socket));

				_complation_port.connect(session_->_socket, reinterpret_cast<ULONG_PTR>(&session_));
				on_create_session(session_->_key);
			}
		}
	}
	inline void worker(void) noexcept {

	}

	inline bool receive(session& session_) noexcept {
		WSABUF buffer{ 4096 - session_._receive_buffer->rear(),  reinterpret_cast<char*>(session_._receive_buffer->data() + session_._receive_buffer->rear()) };
		unsigned long flag = 0;
		session_._recv_overlapped.clear();
		int result = session_._socket.wsa_receive(&buffer, 1, &flag, session_._recv_overlapped);
		if (!(SOCKET_ERROR == result && WSA_IO_PENDING != GetLastError()))
			return true;
		return false;
	}
public:
	inline virtual bool on_accept_socket(system_component::network::socket_address_ipv4& socket_address) noexcept {
		return true;
	}
	inline virtual bool on_create_session(unsigned long long key) noexcept {
		return true;
	}
private:
	system_component::input_output::completion_port _complation_port;
	data_structure::vector<system_component::multi::thread> _thread;
	system_component::network::socket _listen_socket;

	session_array _session_array;
};