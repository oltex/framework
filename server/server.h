#pragma once
#include "library/system-component/network/window_socket_api.h"
#include "library/system-component/input_output/completion_port.h"
#include "library/system-component/multi/thread.h"
#include "library/system-component/network/socket.h"

#include "library/data-strucutre/vector.h"
#include "library/data-strucutre/serialize_buffer.h"
#include "library/data-strucutre/thread-local/object_pool.h"
#include "library/data-strucutre/intrusive/shared_pointer.h"
#include "library/data-strucutre/lockfree/queue.h"
#include <optional>
#include <iostream>

template<typename type>
concept string_size = std::_Is_any_of_v<type, unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long>;

class server final {
private:
	using size_type = unsigned int;
	using byte = unsigned char;

	struct header final {
		unsigned short _size;
	};


	class buffer final : public data_structure::intrusive::shared_pointer_hook<0>, public data_structure::serialize_buffer {
	public:
		inline explicit buffer(void) noexcept = default;
		inline explicit buffer(buffer const& rhs) noexcept = default;
		inline explicit buffer(buffer&& rhs) noexcept = default;
		inline auto operator=(buffer const& rhs) noexcept -> buffer&;
		inline auto operator=(buffer&& rhs) noexcept -> buffer&;
		inline ~buffer(void) noexcept = default;
	public:
		inline void initialize(void) noexcept {
			_front = 0;
			_rear = 0;
		}
		friend inline static void destructor(buffer* rhs) {
			auto& object_pool = data_structure::_thread_local::object_pool<buffer>::instance();
			object_pool.release(*rhs);
		}
	};
	class receive_view final {
	private:
		using size_type = unsigned int;
	public:
		inline explicit receive_view(void) noexcept
			: _front(0), _rear(0) {
		}
		inline explicit receive_view(data_structure::intrusive::shared_pointer<buffer, 0> buffer_) noexcept
			: _buffer(buffer_), _front(0), _rear(0) {
		}
		inline explicit receive_view(data_structure::intrusive::shared_pointer<buffer, 0> buffer_, size_type front, size_type rear) noexcept
			: _buffer(buffer_), _front(front), _rear(rear) {
		}
		inline explicit receive_view(receive_view const& rhs) noexcept
			: _buffer(rhs._buffer), _front(rhs._front), _rear(rhs._rear) {
		}
		inline auto operator=(receive_view const& rhs) noexcept -> receive_view&;
		inline auto operator=(receive_view&& rhs) noexcept -> receive_view&;
		inline ~receive_view(void) noexcept = default;
	public:
		template<typename type>
		inline auto operator>>(type& value) noexcept -> receive_view& requires std::is_arithmetic_v<type> {
			value = reinterpret_cast<type&>(_buffer->data()[_front]);
			_front += sizeof(type);
			return *this;
		}
		inline void peek(byte* const buffer, size_type const length) noexcept {
			memcpy(buffer, _buffer->data() + _front, length);
		}
		template<string_size type>
		inline void peek(std::string& value) noexcept {
			type length;
			peek(reinterpret_cast<byte*>(&length), sizeof(type));
			value.assign(reinterpret_cast<char*>(_buffer->data() + _front + sizeof(type)), length);
		}
		template<string_size type>
		inline void peek(std::wstring& value) noexcept {
			type length;
			peek(reinterpret_cast<byte*>(&length), sizeof(type));
			value.assign(reinterpret_cast<wchar_t*>(_buffer->data() + _front + sizeof(type)), length);
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
		//inline void assign(receive_buffer* receive_buffer_, size_type const front, size_type const rear) noexcept {
		//	_receive_buffer = receive_buffer_;
		//	_front = front;
		//	_rear = rear;
		//}
		//inline auto data(void) noexcept -> receive_buffer* {
		//	return _receive_buffer;
		//}
	private:
		size_type _front, _rear;
		data_structure::intrusive::shared_pointer<buffer, 0> _buffer;
	};
	class send_queue final : public data_structure::lockfree::queue<buffer*> {
	public:
		inline explicit send_queue(void) noexcept = default;
		inline explicit send_queue(send_queue const& rhs) noexcept = default;
		inline explicit send_queue(send_queue&& rhs) noexcept = default;
		inline auto operator=(send_queue const& rhs) noexcept -> send_queue&;
		inline auto operator=(send_queue&& rhs) noexcept -> send_queue&;
		inline ~send_queue(void) noexcept = default;
	private:
		class iterator final {
		public:
			inline explicit iterator(node* node_) noexcept
				: _node(node_) {
			}
			inline iterator(iterator const& rhs) noexcept
				: _node(rhs._node) {
			}
		public:
			inline auto operator*(void) const noexcept -> buffer*& {
				return _node->_value;
			}
			inline auto operator++(void) noexcept -> iterator& {
				_node = reinterpret_cast<node*>(_node->_next);
				return *this;
			}
			inline bool operator==(iterator const& rhs) const noexcept {
				return _node == rhs._node;
			}
		private:
			node* _node;
		};
	public:
		inline auto begin(void) noexcept -> iterator {
			return iterator(reinterpret_cast<node*>(reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & _head)->_next));
		}
		inline auto end(void) noexcept -> iterator {
			return iterator(reinterpret_cast<node*>(_nullptr));
		}
		inline auto empty(void) const noexcept {
			unsigned long long head = _head;
			node* address = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & head);
			unsigned long long next = address->_next;
			if (_nullptr == next || 0 == next)
				return true;
			return false;
		}
	};

	struct session final {
	private:
		inline static unsigned long long _static_id = 0x10000;
	public:
		inline explicit session(void) noexcept {
			auto& object_pool = data_structure::_thread_local::object_pool<buffer>::instance();
			buffer* receive_buffer_;
			if (object_pool.empty())
				receive_buffer_ = &object_pool.allocate();
			else
				receive_buffer_ = &object_pool.acquire();
			_receive_buffer = receive_buffer_;
		};
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

			_InterlockedExchange(&_io_count, 1);
			_InterlockedExchange(&_send_flag, 0);
		}
		inline void finalize(void) noexcept {
			_receive_buffer->clear();
			while (!_send_queue.empty()) {
				auto result = _send_queue.pop();
				if (!result)
					__debugbreak();
				data_structure::intrusive::shared_pointer<buffer, 0> data;
				data.set(*result);
			}
			_socket.close();
			//_key._id = -1;
		}
	public:
		inline void create_receive(void) noexcept {
			auto& object_pool = data_structure::_thread_local::object_pool<buffer>::instance();
			buffer* receive_buffer_;
			if (object_pool.empty())
				receive_buffer_ = &object_pool.allocate();
			else
				receive_buffer_ = &object_pool.acquire();
			receive_buffer_->initialize();

			memcpy(receive_buffer_->data(), _receive_buffer->data() + _receive_buffer->front(), _receive_buffer->size());
			receive_buffer_->move_rear(_receive_buffer->size());
			_receive_buffer = receive_buffer_;
		}
		inline bool excute_receive(void) noexcept {
			WSABUF buffer{ 8192 - _receive_buffer->rear(),  reinterpret_cast<char*>(_receive_buffer->data() + _receive_buffer->rear()) };
			unsigned long flag = 0;
			_recv_overlapped.clear();
			int result = _socket.wsa_receive(&buffer, 1, &flag, _recv_overlapped);
			if (!(SOCKET_ERROR == result && WSA_IO_PENDING != GetLastError()))
				return true;
			return false;
		}

		inline void append_send(data_structure::intrusive::shared_pointer<buffer, 0>& message) noexcept {
			auto data = message.get();
			data->add_reference();
			_send_queue.emplace(data);
		}
		inline bool ready_send(void) noexcept {
			while (!_send_queue.empty() && 0 == InterlockedExchange(&_send_flag, 1)) {
				if (!_send_queue.empty())
					return true;
				InterlockedExchange(&_send_flag, 0);
			}
			return false;
		}
		inline bool excute_send(void) noexcept {
			WSABUF wsa_buffer[512];
			_send_size = 0;
			for (auto& iter : _send_queue) {
				wsa_buffer[_send_size].buf = reinterpret_cast<char*>(iter->data());
				wsa_buffer[_send_size].len = iter->rear();
				_send_size++;
			}
			_send_overlapped.clear();
			int result = _socket.wsa_send(wsa_buffer, _send_size, 0, _send_overlapped);
			if (!(SOCKET_ERROR == result && WSA_IO_PENDING != GetLastError()))
				return true;
			//InterlockedExchange(&_send_flag, 0);
			return false;
		}
		inline void finish_send(void) noexcept {
			for (size_type index = 0; index < _send_size; ++index) {
				auto result = _send_queue.pop();
				if (!result)
					__debugbreak();
				data_structure::intrusive::shared_pointer<buffer, 0> data;
				data.set(*result);
			}
			InterlockedExchange(&_send_flag, 0);
		}
		inline auto io_count_decrement(void) noexcept {
			return _InterlockedDecrement(&_io_count);
		}
	public:
		unsigned long long _key;
		system_component::network::socket _socket;
		data_structure::intrusive::shared_pointer<buffer, 0> _receive_buffer;
		send_queue _send_queue;
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
		inline auto operator[](unsigned long long const key) noexcept -> session& {
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
		socket_address.set_port(6000);
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

				_complation_port.connect(session_->_socket, reinterpret_cast<ULONG_PTR>(session_));
				on_create_session(session_->_key);

				if (false == session_->excute_receive())
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
					session_._receive_buffer->move_rear(transferred);

					for (;;) {
						if (sizeof(header) > session_._receive_buffer->size())
							break;
						header header_;
						session_._receive_buffer->peek(reinterpret_cast<unsigned char*>(&header_), sizeof(header));
						if (sizeof(header) + header_._size > session_._receive_buffer->size())
							break;
						session_._receive_buffer->pop(sizeof(header));

						receive_view receive_view_(session_._receive_buffer, session_._receive_buffer->front(), session_._receive_buffer->front() + header_._size);
						session_._receive_buffer->pop(header_._size);
						on_receive(session_._key, receive_view_);
					}

					session_.create_receive();
					if (session_.excute_receive())
						continue;
				}
				else {
					session_.finish_send();
					if (session_.ready_send() && session_.excute_send())
						continue;
				}
			}
			if (0 == _InterlockedDecrement(&session_._io_count)) {
				session_.finalize();
				_session_array.release(&session_);
			}
		}
	}
public:
	inline virtual bool on_accept_socket(system_component::network::socket_address_ipv4& socket_address) noexcept {
		return true;
	}
	inline virtual void on_create_session(unsigned long long key) noexcept {
		auto& object_pool = data_structure::_thread_local::object_pool<buffer>::instance();
		data_structure::intrusive::shared_pointer<buffer, 0> message;
		if (object_pool.empty())
			message = &object_pool.allocate();
		else
			message = &object_pool.acquire();
		message->initialize();

		header header_{ 8 };
		message->push(reinterpret_cast<unsigned char*>(&header_), sizeof(header));
		*message << 0x7fffffffffffffff;
		do_send(key, message);
	}
	inline virtual void on_receive(unsigned long long key, receive_view& receive_view_) {
		unsigned long long value;
		receive_view_ >> value;

		auto& object_pool = data_structure::_thread_local::object_pool<buffer>::instance();
		data_structure::intrusive::shared_pointer<buffer, 0> message;
		if (object_pool.empty())
			message = &object_pool.allocate();
		else
			message = &object_pool.acquire();
		message->initialize();

		header header_{ 8 };
		message->push(reinterpret_cast<unsigned char*>(&header_), sizeof(header));
		*message << value;
		do_send(key, message);
	}
	inline virtual void on_destroy_session(unsigned long long session_id) noexcept {

	}
public:
	inline void do_send(unsigned long long key, data_structure::intrusive::shared_pointer<buffer, 0>& message) noexcept {
		session& session_ = _session_array[key];
		session_.append_send(message);
		if (session_.ready_send()) {
			InterlockedIncrement(&session_._io_count);
			if (session_.excute_send())
				return;
			InterlockedDecrement(&session_._io_count);
		}
	}
	inline void do_destroy(unsigned long long session_id) noexcept {

	}
private:
	system_component::input_output::completion_port _complation_port;
	data_structure::vector<system_component::multi::thread> _thread;
	system_component::network::socket _listen_socket;

	session_array _session_array;
};