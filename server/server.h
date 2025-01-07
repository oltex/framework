#pragma once
#include "library/system-component/network/window_socket_api.h"
#include "library/system-component/input_output/completion_port.h"
#include "library/system-component/multi/thread.h"
#include "library/system-component/network/socket.h"

#include "library/data-strucutre/vector.h"
#include "library/data-strucutre/reference_count.h"
#include "library/data-strucutre/thread-local/object_pool.h"
#include <optional>

template<typename type>
concept string_size = std::_Is_any_of_v<type, unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long>;

class server final {
private:
	using size_type = unsigned int;
	using byte = unsigned char;

	struct header final {
		unsigned short _size;
	};

	class receive_buffer final : public data_structure::reference_count {
	public:
		inline explicit receive_buffer(void) noexcept = default;
		inline explicit receive_buffer(receive_buffer const& rhs) noexcept = default;
		inline explicit receive_buffer(receive_buffer&& rhs) noexcept = default;
		inline auto operator=(receive_buffer const& rhs) noexcept -> receive_buffer&;
		inline auto operator=(receive_buffer&& rhs) noexcept -> receive_buffer&;
		inline ~receive_buffer(void) noexcept = default;
	public:
		inline auto data(void) noexcept -> byte* {
			return _array;
		}
	private:
		byte _array[4096];
	};
	class receive_view final {
	private:
		using size_type = unsigned int;
	public:
		inline explicit receive_view(void) noexcept
			: _receive_buffer(nullptr), _front(0), _rear(0) {
		}
		inline explicit receive_view(receive_buffer* receive_buffer) noexcept
			: _receive_buffer(receive_buffer), _front(0), _rear(0) {
		}
		inline explicit receive_view(receive_buffer* receive_buffer, size_type front, size_type rear) noexcept
			: _receive_buffer(receive_buffer), _front(front), _rear(rear) {
		}
		inline explicit receive_view(receive_view const& rhs) noexcept
			: _receive_buffer(rhs._receive_buffer), _front(rhs._front), _rear(rhs._rear) {
		}
		inline auto operator=(receive_view const& rhs) noexcept -> receive_view&;
		inline auto operator=(receive_view&& rhs) noexcept -> receive_view&;
		inline ~receive_view(void) noexcept = default;
	public:
		template<typename type>
		inline auto operator>>(type& value) noexcept -> receive_view& requires std::is_arithmetic_v<type> {
			value = reinterpret_cast<type&>(_receive_buffer->data()[_front]);
			_front += sizeof(type);
			return *this;
		}
		inline void peek(byte* const buffer, size_type const length) noexcept {
			memcpy(buffer, _receive_buffer->data() + _front, length);
		}
		template<string_size type>
		inline void peek(std::string& value) noexcept {
			type length;
			peek(reinterpret_cast<byte*>(&length), sizeof(type));
			value.assign(reinterpret_cast<char*>(_receive_buffer->data() + _front + sizeof(type)), length);
		}
		template<string_size type>
		inline void peek(std::wstring& value) noexcept {
			type length;
			peek(reinterpret_cast<byte*>(&length), sizeof(type));
			value.assign(reinterpret_cast<wchar_t*>(_receive_buffer->data() + _front + sizeof(type)), length);
		}
		inline void pop(size_type const length) noexcept {
			_front += length;
		}
		template<string_size type>
		inline void pop(std::string& value) noexcept {
			_front += sizeof(type) + sizeof(std::string::value_type) * value.size();
		}
		template<string_size type>
		inline void pop(std::wstring& value) noexcept {
			_front += sizeof(type) + sizeof(std::wstring::value_type) * value.size();
		}
	public:
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
		inline void assign(receive_buffer* receive_buffer_, size_type const front, size_type const rear) noexcept {
			_receive_buffer = receive_buffer_;
			_front = front;
			_rear = rear;
		}
		inline auto data(void) noexcept -> receive_buffer* {
			return _receive_buffer;
		}
	private:
		size_type _front, _rear;
		receive_buffer* _receive_buffer;
	};

	class send_queue final {

	};

	struct session final {
	private:
		inline static unsigned long long _static_id = 0x10000;
	public:
		inline explicit session(void) noexcept {
			auto& object_pool = data_structure::_thread_local::object_pool<receive_buffer>::instance();
			receive_buffer* receive_buffer_;
			if (object_pool.empty())
				receive_buffer_ = &object_pool.allocate();
			else
				receive_buffer_ = &object_pool.acquire();
			receive_buffer_->add_reference();
			_receive_view.assign(receive_buffer_, 0, 0);
		};
		inline explicit session(session const&) noexcept = delete;
		inline explicit session(session&&) noexcept = delete;
		inline auto operator=(session const&) noexcept -> session & = delete;
		inline auto operator=(session&&) noexcept -> session & = delete;
		inline ~session(void) noexcept {
			receive_buffer* receive_buffer_ = _receive_view.data();
			if (0 == receive_buffer_->release())
				data_structure::_thread_local::object_pool<receive_buffer>::instance().release(*receive_buffer_);
		};
	public:
		inline void initialize(system_component::network::socket&& socket) noexcept {
			_key = 0xffff & _key | _static_id;
			_static_id += 0x10000;
			_socket = std::move(socket);

		}
		inline void finalize(void) noexcept {
		}
	public:
		inline bool receive(void) noexcept {
			//auto& object_pool = data_structure::_thread_local::object_pool<receive_buffer>::instance();
			//receive_buffer* receive_buffer_ = &object_pool.acquire();
			//memcpy(receive_buffer_->data(), _receive_view.data()->data() + _receive_view.front(), _receive_view.size());
			//_receive_view.assign(receive_buffer_, 0, _receive_view.size());

			WSABUF buffer{ 4096 - _receive_view.rear(),  reinterpret_cast<char*>(_receive_view.data()->data() + _receive_view.rear()) };
			unsigned long flag = 0;
			_recv_overlapped.clear();
			int result = _socket.wsa_receive(&buffer, 1, &flag, _recv_overlapped);
			if (!(SOCKET_ERROR == result && WSA_IO_PENDING != GetLastError()))
				return true;
			return false;
		}
		inline auto io_count_decrement(void) noexcept {
			return _InterlockedDecrement(&_io_count);
		}
	public:
		unsigned long long _key;
		system_component::network::socket _socket;
		receive_view _receive_view;
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

				if (false == session_->receive())
					if (0 == session_->io_count_decrement()) {
						session_->finalize();
						_session_array.release(session_);
					}
			}
		}
	}
	inline void worker(void) noexcept {
		for (;;) {
			auto [result, transferred, key, overlapped] = _complation_port.get_queue_state(INFINITE);
			session& session_ = *reinterpret_cast<session*>(key);

			if (0 != transferred) {
				if (&session_._recv_overlapped.data() == overlapped) {
					session_._receive_view.move_rear(transferred);

					for (;;) {
						if (sizeof(header) > session_._receive_view.size())
							break;
						header header_;
						session_._receive_view.peek(reinterpret_cast<unsigned char*>(&header_), sizeof(header));
						if (sizeof(header) + header_._size > session_._receive_view.size())
							break;
						session_._receive_view.pop(sizeof(header));

						receive_view receive_view_(session_._receive_view.data(), session_._receive_view.front(), session_._receive_view.front() + header_._size);
						on_receive(session_._key, receive_view_);
						receive_view_.data()->release();
					}

					auto& object_pool = data_structure::_thread_local::object_pool<receive_buffer>::instance();
					receive_buffer* receive_buffer_;
					if (object_pool.empty())
						receive_buffer_ = &object_pool.allocate();
					else
						receive_buffer_ = &object_pool.acquire();
					receive_buffer_->add_reference();
					_receive_view.assign(receive_buffer_, 0, 0);

				}
				else {
				}
			}
			if (0 == _InterlockedDecrement(&session_._io_count)) {
				session_.finalize();
				_session_array.release(&session_);
			}
		}
	}

	//inline bool receive(session& session_) noexcept {
	//	WSABUF buffer{ 4096 - session_._receive_buffer->rear(),  reinterpret_cast<char*>(session_._receive_buffer->data() + session_._receive_buffer->rear()) };
	//	unsigned long flag = 0;
	//	session_._recv_overlapped.clear();
	//	int result = session_._socket.wsa_receive(&buffer, 1, &flag, session_._recv_overlapped);
	//	if (!(SOCKET_ERROR == result && WSA_IO_PENDING != GetLastError()))
	//		return true;
	//	return false;
	//}
public:
	inline virtual bool on_accept_socket(system_component::network::socket_address_ipv4& socket_address) noexcept {
		return true;
	}
	inline virtual void on_create_session(unsigned long long key) noexcept {
	}
	inline virtual void on_receive(unsigned long long key, receive_view& receive_view_) {

	}
private:
	system_component::input_output::completion_port _complation_port;
	data_structure::vector<system_component::multi::thread> _thread;
	system_component::network::socket _listen_socket;

	session_array _session_array;
};