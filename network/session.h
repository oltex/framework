#pragma once
#include "library/lockfree/free_list.h"
#include "library/socket.h"
#include "library/interlock.h"

#include "library/vector.h"
#include "message.h"

namespace framework {

	struct accept;
	struct listen final {
		library::socket _socket;

		inline explicit listen(void) noexcept = default;
		inline explicit listen(listen const&) noexcept = delete;
		inline explicit listen(listen&&) noexcept = delete;
		inline auto operator=(listen const&) noexcept -> listen & = delete;
		inline auto operator=(listen&&) noexcept -> listen & = delete;
		inline ~listen(void) noexcept = default;

		inline void initialize(char const* const ip, unsigned short port, int backlog) noexcept {
			_socket.create(AF_INET, SOCK_STREAM, IPPROTO_TCP, WSA_FLAG_OVERLAPPED);
			_socket.set_option_linger(1, 0);
			_socket.set_option_send_buffer(0);
			library::socket_address_ipv4 sockaddr;
			sockaddr.set_address(ip);
			sockaddr.set_port(port);
			_socket.bind(sockaddr);
			_socket.listen(backlog);
		}
		inline void accept(framework::accept& accept) noexcept;
	};
	struct accept {
		library::socket _socket;
		library::overlap _overlap;
		char _buffer[64];

		inline explicit accept(void) noexcept = default;
		inline explicit accept(accept const&) noexcept = delete;
		inline explicit accept(accept&&) noexcept = delete;
		inline auto operator=(accept const&) noexcept -> accept & = delete;
		inline auto operator=(accept&&) noexcept -> accept & = delete;
		inline ~accept(void) noexcept = default;

		inline void initialize(void) noexcept {
			_socket.create(AF_INET, SOCK_STREAM, IPPROTO_TCP, WSA_FLAG_OVERLAPPED);
			_overlap.clear();
		}
		inline void finalize(void) noexcept {
			_socket.close();
		}

		inline static auto recover(OVERLAPPED* overlapped) noexcept -> accept* {
			return reinterpret_cast<accept*>(reinterpret_cast<unsigned char*>(overlapped) - offsetof(library::overlap, _overlapped) - offsetof(accept, _overlap));
		}
		inline void inherit(listen& listen) const noexcept {
			_socket.set_option_update_accept_context(listen._socket);
		}
		inline auto address(void) noexcept {
			return library::socket::get_accept_ex_socket_address(_buffer);
		}
	};
	inline void listen::accept(framework::accept& accept) noexcept {
		_socket.accept_ex(accept._socket, accept._buffer, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, accept._overlap);
	}
	struct accept_array final {
		using size_type = unsigned int;
		using iterator = library::template vector<accept>::iterator;
		library::vector<accept> _vector;
	public:
		inline explicit accept_array(void) noexcept = default;
		inline explicit accept_array(accept_array const&) noexcept = delete;
		inline explicit accept_array(accept_array&&) noexcept = delete;
		inline auto operator=(accept_array const&) noexcept -> accept_array & = delete;
		inline auto operator=(accept_array&&) noexcept -> accept_array & = delete;
		inline ~accept_array(void) noexcept = default;

		inline void initialize(size_type capacity) noexcept {
			_vector.reserve(capacity);
			for (auto index = 0; index < 1; ++index) {
				auto& accept_ = _vector.emplace_back();
				accept_.initialize();
			}
		}

		inline auto begin(void) const noexcept -> iterator {
			return _vector.begin();
		}
		inline auto end(void) const noexcept -> iterator {
			return _vector.end();
		}
	};

	struct session final {
		inline static unsigned long long _id = 0x10000;
		using size_type = unsigned int;
		unsigned long long _key;
		unsigned long _io_count; // release_flag // recv_flag // io count
		library::socket _socket;
		//message _message;

	public:
		inline session(size_type& index) noexcept
			: _key(index++), _io_count(0x80000000) {
			//auto& memory_pool = library::_thread_local::pool<message>::instance();
			//message_pointer receive_message(&memory_pool.allocate());
			//_receive_message = receive_message;
		};
		inline explicit session(session const&) noexcept = delete;
		inline explicit session(session&&) noexcept = delete;
		inline auto operator=(session const&) noexcept -> session & = delete;
		inline auto operator=(session&&) noexcept -> session & = delete;
		inline ~session(void) noexcept = default;

		inline void initialize(library::socket&& socket, unsigned long long timeout_duration) noexcept {
			_key = 0xffff & _key | library::interlock_exchange_add(_id, 0x10000);
			_socket = std::move(socket);
			//_timeout_currnet = GetTickCount64();
			//_timeout_duration = timeout_duration;
			//_send_flag = 0;
			//_cancel_flag = 0;
			_InterlockedIncrement(&_io_count);
			_InterlockedAnd((long*)&_io_count, 0x7FFFFFFF);
		}
		//inline void receive(void) noexcept {
		//	if (0 == _cancel_flag) {
		//		WSABUF wsa_buffer{ message::capacity() - _receive_message->rear(),  reinterpret_cast<char*>(_receive_message->data() + _receive_message->rear()) };
		//		unsigned long flag = 0;
		//		_recv_overlapped.clear();
		//		_InterlockedIncrement(&_io_count);
		//		if (SOCKET_ERROR == _socket.wsa_receive(&wsa_buffer, 1, &flag, _recv_overlapped)) {
		//			if (WSA_IO_PENDING == GetLastError()) {
		//				if (1 == _cancel_flag)
		//					_socket.cancel_io_ex();
		//			}
		//			else {
		//				_InterlockedDecrement(&_io_count);
		//				_cancel_flag = 1;
		//			}
		//		}
		//	}
		//}

		//inline bool release(void) noexcept {
		//	if (0 == _InterlockedDecrement(&_io_count) && 0 == _InterlockedCompareExchange(&_io_count, 0x80000000, 0)) {
		//		_receive_message->clear();
		//		while (!_send_queue.empty())
		//			_send_queue.pop();
		//		while (!_receive_queue.empty())
		//			_receive_queue.pop();
		//		_socket.close();
		//		return true;
		//	}
		//	return false;
		//}
	};
	class session_array final {
		using size_type = unsigned int;
		size_type _size;
		library::lockfree::free_list<session> _free_list;
	public:
		inline explicit session_array(void) noexcept = default;
		inline explicit session_array(session_array const&) noexcept = delete;
		inline explicit session_array(session_array&&) noexcept = delete;
		inline auto operator=(session_array const&) noexcept -> session_array & = delete;
		inline auto operator=(session_array&&) noexcept -> session_array & = delete;
		inline ~session_array(void) noexcept = default;

		inline void initialize(size_type capacity) noexcept {
			size_type index = 0;
			_free_list.reserve(capacity, index);
			_size = 0;
		}
		inline void finalize(void) noexcept {
			_free_list.clear();
		}

		inline auto allocate(void) noexcept -> session* {
			auto result = _free_list.allocate();
			if (nullptr != result)
				library::interlock_increment(_size);
			return result;
		}
		inline void deallocate(session* value) noexcept {
			_free_list.deallocate(value);
			library::interlock_decrement(_size);
		}

		inline auto operator[](unsigned long long const key) noexcept -> session& {
			return _free_list[key & 0xffff];
		}
	};
}