#pragma once
#include "library/lockfree/free_array.h"
#include "library/socket.h"
#include "library/interlock.h"

#include "library/vector.h"
#include "message.h"

namespace framework {

	struct listen final {
		library::socket _socket;
	public:
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

		inline static auto recover(OVERLAPPED* overlapped) noexcept -> accept* {
			return reinterpret_cast<accept*>(reinterpret_cast<unsigned char*>(overlapped) - offsetof(library::overlap, _overlapped) - offsetof(accept, _overlap));
		}
	};

	struct accept_array final {
		using size_type = unsigned int;
		using iterator = library::template vector<accept>::iterator;
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
				accept_._socket.create(AF_INET, SOCK_STREAM, IPPROTO_TCP, WSA_FLAG_OVERLAPPED);
				accept_._overlap.clear();
			}
		}

		inline auto begin(void) const noexcept -> iterator {
			return _vector.begin();
		}
		inline auto end(void) const noexcept -> iterator {
			return _vector.end();
		}

		library::vector<accept> _vector;
	};


	class session final {
		using size_type = unsigned int;
		unsigned long long _key;
		unsigned long _io_count; // release_flag // recv_flag // io count
		library::socket _socket;

	public:
		inline session(size_type& index) noexcept
			: _key(index++), _io_count(0x80000000) {
			//auto& memory_pool = library::_thread_local::pool<message>::instance();
			//message_pointer receive_message(&memory_pool.allocate());
			//_receive_message = receive_message;
		};
	};

	class session_array final {
		using size_type = unsigned int;
		size_type _size;
		library::lockfree::free_array<session> _free_list;
	public:
		inline explicit session_array(void) noexcept = default;
		inline explicit session_array(session_array const&) noexcept = delete;
		inline explicit session_array(session_array&&) noexcept = delete;
		inline auto operator=(session_array const&) noexcept -> session_array & = delete;
		inline auto operator=(session_array&&) noexcept -> session_array & = delete;
		inline ~session_array(void) noexcept = default;

		inline void initialize(void) noexcept {
			_size = 0;
			//base(capacity, index)
		}
		inline void finalize(void) noexcept {
			_size = 0;
			//base(capacity, index)
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