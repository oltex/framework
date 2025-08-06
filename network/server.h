#pragma once
#include "iocp.h"
#include "session.h"

namespace framework {
	class server final : iocp::object {
		framework::iocp& _iocp;
		library::socket _listen;
		struct accept_socket {
			library::socket _socket;
			library::overlapped _overlapped;
			char _buffer[64];
		};
		library::vector<accept_socket> _accept;
	public:
		inline explicit server(void) noexcept
			: _iocp(framework::iocp::instance()) {
			library::wsa_start_up();

		}
		inline ~server(void) noexcept {
			library::wsa_clean_up();
		}

		inline void accept(char const* const ip, unsigned short port) noexcept {
			_listen.create(AF_INET, SOCK_STREAM, IPPROTO_TCP, WSA_FLAG_OVERLAPPED);
			_iocp.connect(*this, _listen, 3);
			_listen.set_option_linger(1, 0);
			_listen.set_option_send_buffer(0);

			library::socket_address_ipv4 sockaddr;
			sockaddr.set_address(ip);
			sockaddr.set_port(port);
			_listen.bind(sockaddr);
			_listen.listen(200);

			_accept.reserve(10);
			for (auto index = 0; index < 10; ++index) {
				auto& accept_ = _accept.emplace_back();
				accept_._socket.create(AF_INET, SOCK_STREAM, IPPROTO_TCP, WSA_FLAG_OVERLAPPED);
				accept_._overlapped.clear();
				_listen.accept_ex(accept_._socket, accept_._buffer, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, accept_._overlapped);
			}
		}
		inline void connect(void) noexcept {

		}
		inline void worker(bool result, unsigned long transferred, uintptr_t key, OVERLAPPED* overlapped) noexcept override {
			switch (key) {
			default:
				break;
			}
			auto current = reinterpret_cast<library::overlapped*>(reinterpret_cast<unsigned char*>(overlapped) - offsetof(library::overlapped, _overlapped));
			auto current2 = reinterpret_cast<accept_socket*>(reinterpret_cast<unsigned char*>(current) - offsetof(accept_socket, _overlapped));
			current2->_socket.get_accept_ex_socket_address(current2->_buffer);
			int a = 10;
		};
	};

}