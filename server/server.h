#pragma once
#include "library/system-component/network/window_socket_api.h"
#include "library/system-component/input_output/completion_port.h"
#include "library/system-component/multi/thread.h"
#include "library/system-component/network/socket.h"

#include "library/data-strucutre/vector.h"
#include "library/data-strucutre/serialize_buffer.h"
#include "library/data-strucutre/thread-local/memory_pool.h"
#include "library/data-strucutre/intrusive/shared_pointer.h"
#include "library/data-strucutre/lockfree/queue.h"
#include "library/data-strucutre/unordered_map.h"

#include "library/design-pattern/singleton.h"

#include "library/utility/parser.h"

#include <optional>
#include <iostream>
#include <intrin.h>
#include <functional>

template<typename type>
concept string_size = std::_Is_any_of_v<type, unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long>;

class server final : public design_pattern::singleton<server> {
private:
	friend class design_pattern::singleton<server>;
	using size_type = unsigned int;
	using byte = unsigned char;
private:
	struct header final {
		unsigned short _size;
	};
	class buffer final : public data_structure::intrusive::shared_pointer_hook<0>, public data_structure::serialize_buffer {
	public:
		inline explicit buffer(void) noexcept = delete;
		inline explicit buffer(buffer const& rhs) noexcept = delete;
		inline explicit buffer(buffer&& rhs) noexcept = delete;
		inline auto operator=(buffer const& rhs) noexcept -> buffer & = delete;
		inline auto operator=(buffer&& rhs) noexcept -> buffer & = delete;
		inline ~buffer(void) noexcept = delete;
	public:
		inline void initialize(void) noexcept {
			_front = 0;
			_rear = 0;
		}
		friend inline static void destructor(buffer* rhs) {
			auto& memory_pool = data_structure::_thread_local::memory_pool<buffer>::instance();
			memory_pool.deallocate(*rhs);
		}
	};
	using message = data_structure::intrusive::shared_pointer<buffer, 0>;
	class view final {
	private:
		using size_type = unsigned int;
	public:
		inline explicit view(void) noexcept
			: _front(0), _rear(0) {
		}
		inline explicit view(message message_) noexcept
			: _message(message_), _front(0), _rear(0) {
		}
		inline explicit view(message message_, size_type front, size_type rear) noexcept
			: _message(message_), _front(front), _rear(rear) {
		}
		inline view(view const& rhs) noexcept
			: _message(rhs._message), _front(rhs._front), _rear(rhs._rear) {
		}
		inline auto operator=(view const& rhs) noexcept -> view&;
		inline auto operator=(view&& rhs) noexcept -> view&;
		inline ~view(void) noexcept = default;
	public:
		template<typename type>
			requires std::is_arithmetic_v<type>
		inline auto operator>>(type& value) noexcept -> view& {
			value = reinterpret_cast<type&>(_message->data()[_front]);
			_front += sizeof(type);
			return *this;
		}
		inline void peek(byte* const buffer, size_type const length) noexcept {
			memcpy(buffer, _message->data() + _front, length);
		}
		template<string_size type>
		inline void peek(std::string& value) noexcept {
			type length;
			peek(reinterpret_cast<byte*>(&length), sizeof(type));
			value.assign(reinterpret_cast<char*>(_message->data() + _front + sizeof(type)), length);
		}
		template<string_size type>
		inline void peek(std::wstring& value) noexcept {
			type length;
			peek(reinterpret_cast<byte*>(&length), sizeof(type));
			value.assign(reinterpret_cast<wchar_t*>(_message->data() + _front + sizeof(type)), length);
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
		inline auto data(void) noexcept -> message& {
			return _message;
		}
		inline void set(buffer* buffer_, size_type front, size_type rear) noexcept {
			_front = front;
			_rear = rear;
			_message.set(buffer_);
		}
		inline auto reset(void) noexcept {
			_message.reset();
		}
	private:
		size_type _front, _rear;
		message _message;
	};

	class send_queue final : protected data_structure::lockfree::queue<buffer*> {
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
		inline void push(message message_) noexcept {
			auto buf = message_.get();
			emplace(buf);
			message_.reset();
		}
		inline auto pop(void) noexcept -> message {
			unsigned long long head = _head;
			node* address = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & head);
			unsigned long long next = address->_next;

			if (_nullptr == next)
				__debugbreak();
			for (;;) {
				unsigned long long tail = _tail;
				node* tail_address = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & tail);
				if (reinterpret_cast<unsigned long long>(tail_address) == reinterpret_cast<unsigned long long>(address)) {
					unsigned long long tail_next = tail_address->_next;
					if (_nullptr != tail_next)
						_InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_tail), tail_next + (0xFFFF800000000000ULL & tail) + 0x0000800000000000ULL, tail);
				}
				else
					break;
			}

			message result;
			result.set(reinterpret_cast<node*>(next)->_value);
			_head = next;
			_memory_pool.deallocate(*address);
			return result;
		}
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
	class receive_queue final {
	private:
		struct receive_data {
			unsigned long long _key;
			size_type _front, _rear;
			buffer* _buffer;
		};
	public:
		inline void push(unsigned long long key, view view_) noexcept {
			receive_data receive_data_;
			receive_data_._buffer = &*view_.data();
			receive_data_._front = view_.front();
			receive_data_._rear = view_.rear();
			receive_data_._key = key;
			_queue.emplace(receive_data_);
			_InterlockedIncrement(&_size);
			view_.reset();
		}
		inline auto pop(void) noexcept -> data_structure::pair<unsigned long long, view> {
			auto receive_data_ = _queue.pop();
			data_structure::pair<unsigned long long, view> result;
			if (receive_data_) {
				result._first = receive_data_->_key;
				result._second.set(receive_data_->_buffer, receive_data_->_front, receive_data_->_rear);
				_InterlockedDecrement(&_size);
			}
			return result;
		}
	private:
		data_structure::lockfree::queue<receive_data> _queue;
	public:
		volatile long _size;
	};
	struct session final {
	private:
		inline static unsigned long long _static_id = 0x10000;
	public:
		inline explicit session(void) noexcept {
			auto& memory_pool = data_structure::_thread_local::memory_pool<buffer>::instance();
			buffer* receive_buffer = &memory_pool.allocate();
			receive_buffer->initialize();
			_receive_message = message(receive_buffer);
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
			_InterlockedExchange(&_send_flag, 0);
			_InterlockedExchange(&_cancel_flag, 0);
			_InterlockedIncrement(&_io_count);
			_InterlockedAnd((long*)&_io_count, 0x7FFFFFFF);
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
				WSABUF wsa_buffer{ 8192 - _receive_message->rear(),  reinterpret_cast<char*>(_receive_message->data() + _receive_message->rear()) };
				unsigned long flag = 0;
				_recv_overlapped.clear();
				int result = _socket.wsa_receive(&wsa_buffer, 1, &flag, _recv_overlapped);
				if (result != SOCKET_ERROR || GetLastError() == WSA_IO_PENDING) {
					if (result == SOCKET_ERROR && 1 == _cancel_flag)
						_socket.cancel_io_ex();
					return true;
				}
			}
			return false;
		}
		inline bool send(void) noexcept {
			if (1 == _cancel_flag) {
				return false;
			}

			while (!_send_queue.empty() && 0 == _InterlockedExchange(&_send_flag, 1)) {
				if (_send_queue.empty())
					_InterlockedExchange(&_send_flag, 0);
				else {
					WSABUF wsa_buffer[512];
					_send_size = 0;
					for (auto& iter : _send_queue) {
						wsa_buffer[_send_size].buf = reinterpret_cast<char*>(iter->data());
						wsa_buffer[_send_size].len = iter->rear();
						_send_size++;
					}
					_send_overlapped.clear();
					int result = _socket.wsa_send(wsa_buffer, _send_size, 0, _send_overlapped);
					if (result != SOCKET_ERROR || GetLastError() == WSA_IO_PENDING) {
						if (1 == _cancel_flag)
							_socket.cancel_io_ex();
						return true;
					}
					return false;
				}
			}
			return false;
		}
		inline void cancel(void) noexcept {
			_InterlockedExchange(&_cancel_flag, 1);
			_socket.cancel_io_ex();
		}
	public:
		inline void create_receive(void) noexcept {
			buffer* receive_buffer = &data_structure::_thread_local::memory_pool<buffer>::instance().allocate();

			receive_buffer->initialize();
			memcpy(receive_buffer->data(), _receive_message->data() + _receive_message->front(), _receive_message->size());
			receive_buffer->move_rear(_receive_message->size());
			_receive_message = message(receive_buffer);
		}
		inline void finish_send(void) noexcept {
			for (size_type index = 0; index < _send_size; ++index)
				message message_ = _send_queue.pop();
			_InterlockedExchange(&_send_flag, 0);
		}
	public:
		unsigned long long _key;
		system_component::network::socket _socket;
		message _receive_message;
		send_queue _send_queue;
		system_component::input_output::overlapped _recv_overlapped;
		system_component::input_output::overlapped _send_overlapped;
		volatile unsigned int _io_count; // release_flag
		volatile unsigned int _send_flag;
		volatile unsigned int _send_size;
		volatile unsigned int _cancel_flag;
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
			for (size_type index = 0; index < _size; ++index)
				_array[index]._value.~session();
			free(_array);
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
private:
	class command final {
	public:
		class parameter final {
		public:
			template<typename... argument>
			inline explicit parameter(argument&&... arg) noexcept {
				(_argument.emplace_back(std::forward<argument>(arg)), ...);
			}
			inline explicit parameter(data_structure::vector<std::string>& argument) noexcept
				: _argument(argument) {
			}
			inline explicit parameter(parameter const& rhs) noexcept = delete;
			inline explicit parameter(parameter&& rhs) noexcept = delete;
			inline auto operator=(parameter const& rhs) noexcept -> parameter & = delete;
			inline auto operator=(parameter&& rhs) noexcept -> parameter & = delete;
			inline ~parameter(void) noexcept = default;
		public:
			inline auto get_string(size_type index) noexcept -> std::string& {
				return _argument[index];
			}
			inline auto get_int(size_type index) noexcept -> int {
				return std::stoi(_argument[index]);
			}
			inline auto get_float(size_type index) noexcept -> float {
				return std::stof(_argument[index]);
			}
			inline auto get_bool(size_type index) noexcept -> bool {
				std::string& arg = _argument[index];
				if (arg == "true" || arg == "on" || arg == "1")
					return true;
				return false;
			}
		private:
			data_structure::vector<std::string> _argument;
		};
	public:
		inline explicit command(void) noexcept {
			add("include", [&](command::parameter* param) noexcept -> int {
				std::string path = param->get_string(1);
				utility::parser parser(std::wstring(path.begin(), path.end()));

				for (auto& iter : parser) {
					command::parameter param(iter);
					execute(iter[0], &param);
				}
				return 0;
				});
		};
		inline explicit command(command const& rhs) noexcept = delete;
		inline explicit command(command&& rhs) noexcept = delete;
		inline auto operator=(command const& rhs) noexcept -> command & = delete;
		inline auto operator=(command&& rhs) noexcept -> command & = delete;
		inline ~command(void) noexcept = default;
	public:
		inline void add(std::string_view name, std::function<int(parameter*)> function) noexcept {
			_function.emplace(name.data(), function);
		}
		inline int execute(std::string name, parameter* par) noexcept {
			auto res = _function.find(name.data());
			if (res == _function.end())
				return 0;
			return res->_second(par);
		}
	private:
		data_structure::unordered_map<std::string, std::function<int(parameter*)>> _function;
	};
	class monitor final {
	private:
		//std::function<void(void)>
	};
private:
	inline explicit server(void) noexcept {
		system_component::network::window_socket_api::start_up();

		_command.add("iocp_concurrent", [&](command::parameter* param) noexcept -> int {
			_concurrent_thread_count = param->get_int(1);
			return 0;
			});
		_command.add("iocp_worker", [&](command::parameter* param) noexcept -> int {
			_worker_thread_count = param->get_int(1);
			return 0;
			});
		_command.add("session_max", [&](command::parameter* param) noexcept -> int {
			_session_array_max = param->get_int(1);
			return 0;
			});
		_command.add("tcp_ip", [&](command::parameter* param) noexcept -> int {
			_listen_socket_ip = param->get_string(1);
			return 0;
			});
		_command.add("tcp_port", [&](command::parameter* param) noexcept -> int {
			_listen_socket_port = param->get_int(1);
			return 0;
			});
		_command.add("tcp_backlog", [&](command::parameter* param) noexcept -> int {
			_listen_socket_backlog = param->get_int(1);
			return 0;
			});
		_command.add("server_start", [&](command::parameter* param) noexcept -> int {
			start();
			return 0;
			});
		_command.add("server_stop", [&](command::parameter* param) noexcept -> int {
			stop();
			return 0;
			});

		 command::parameter param("include", "server.cfg");
		_command.execute("include", &param);
	};
	inline explicit server(server const& rhs) noexcept = delete;
	inline explicit server(server&& rhs) noexcept = delete;
	inline auto operator=(server const& rhs) noexcept -> server & = delete;
	inline auto operator=(server&& rhs) noexcept -> server & = delete;
	inline ~server(void) noexcept {
		system_component::network::window_socket_api::clean_up();
	};
public:
	inline void start(void) noexcept {
		_complation_port.create(_concurrent_thread_count);
		_accept_thread.begin(&server::accept, CREATE_SUSPENDED, this);
		for (size_type index = 0; index < _worker_thread_count; ++index)
			_worker_thread.emplace_back(&server::worker, 0, this);

		_session_array.initialize(_session_array_max);

		system_component::network::socket_address_ipv4 socket_address;
		socket_address.set_address(_listen_socket_ip.c_str());
		socket_address.set_port(_listen_socket_port);
		_listen_socket.create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		_listen_socket.set_linger(1, 0);
		_listen_socket.set_send_buffer(0);
		_listen_socket.bind(socket_address);
		_listen_socket.listen(_listen_socket_backlog);

		_accept_thread.resume();
	}
	inline void stop(void) noexcept {
		_listen_socket.close();
		_accept_thread.wait_for_single(INFINITE);
		//for (auto& iter : _session) {
		//	iter._value._socket.close();
		//	_session.release(iter._value);
		//}

		//HANDLE handle[128];
		//for (unsigned int index = 0; index < _thread.size(); ++index) {
		//	_completion.post_queue_state(0, 0, nullptr);
		//	handle[index] = _thread[index].data();
		//}
		//system_component::multi::thread::wait_for_multiple(_thread, true, INFINITE);
		//_thread.clear();

		//_completion.close();
	}
	inline void accept(void) noexcept {
		for (;;) {
			auto [socket, socket_address] = _listen_socket.accept();
			if (INVALID_SOCKET == socket.data())
				break;
			if (false == on_accept_socket(socket_address))
				socket.close();
			else {
				session& session_ = *_session_array.acquire();
				session_.initialize(std::move(socket));

				_complation_port.connect(session_._socket, reinterpret_cast<ULONG_PTR>(&session_));
				on_create_session(session_._key);

				if (!session_.receive() && session_.release())
					_session_array.release(&session_);
			}
		}
	}
	inline void worker(void) noexcept {
		for (;;) {
			auto [result, transferred, key, overlapped] = _complation_port.get_queue_state(INFINITE);
			session& session_ = *reinterpret_cast<session*>(key);

			if (0 != transferred) {
				if (&session_._recv_overlapped.data() == overlapped) {
					session_._receive_message->move_rear(transferred);

					for (;;) {
						if (sizeof(header) > session_._receive_message->size())
							break;
						header header_;
						session_._receive_message->peek(reinterpret_cast<unsigned char*>(&header_), sizeof(header));
						if (sizeof(header) + header_._size > session_._receive_message->size())
							break;
						session_._receive_message->pop(sizeof(header));

						view view_(session_._receive_message, session_._receive_message->front(), session_._receive_message->front() + header_._size);
						session_._receive_message->pop(header_._size);
						on_receive(session_._key, view_);
					}

					session_.create_receive();
					if (session_.receive())
						continue;
				}
				else {
					session_.finish_send();
					if (session_.send())
						continue;
				}
			}
			if (session_.release())
				_session_array.release(&session_);
		}
	}
public:
	inline virtual bool on_accept_socket(system_component::network::socket_address_ipv4& socket_address) noexcept {
		return true;
	}
	inline virtual void on_create_session(unsigned long long key) noexcept {
		message message_ = make_message();

		header header_{ 8 };
		message_->push(reinterpret_cast<unsigned char*>(&header_), sizeof(header));
		*message_ << 0x7fffffffffffffff;
		do_send(key, message_);
	}
	inline virtual void on_receive(unsigned long long key, view& view_) {
		unsigned long long value;
		view_ >> value;
		message message_ = make_message();

		header header_{ 8 };
		message_->push(reinterpret_cast<unsigned char*>(&header_), sizeof(header));
		*message_ << value;

		//if (rand() % 2 == 0)
		//	do_destroy(key);

		do_send(key, message_);
	}
	inline virtual void on_destroy_session(unsigned long long session_id) noexcept {

	}
	inline void do_send(unsigned long long key, message& message_) noexcept {
		session& session_ = _session_array[key];
		if (session_.acquire(key)) {
			session_._send_queue.push(message_);
			if (session_.send())
				return;
		}
		if (session_.release())
			_session_array.release(&session_);
	}
	inline void do_destroy(unsigned long long key) noexcept {
		session& session_ = _session_array[key];
		if (session_.acquire(key))
			session_.cancel();
		if (session_.release())
			_session_array.release(&session_);
	}
public:
	inline static auto make_message(void) noexcept -> message {
		auto& memory_pool = data_structure::_thread_local::memory_pool<buffer>::instance();
		message message_(&memory_pool.allocate());
		message_->initialize();
		return message_;
	}
private:
	system_component::input_output::completion_port _complation_port;
	system_component::network::socket _listen_socket;
	system_component::multi::thread _accept_thread;
	data_structure::vector<system_component::multi::thread> _worker_thread;
	session_array _session_array;
public:
	size_type _concurrent_thread_count = 0;
	size_type _worker_thread_count = 0;
	size_type _session_array_max = 0;
	std::string _listen_socket_ip;
	size_type _listen_socket_port = 0;
	size_type _listen_socket_backlog = 0;

	command _command;
	monitor _monitor;

	//function(unsigned long long key, view& view_);
};