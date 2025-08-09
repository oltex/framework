#pragma once
#include "iocp.h"
#include "session.h"

namespace framework {
	class server final : iocp::object {
		iocp& _iocp;
		session_array _session_array;

		listen _listen;
		accept_array _accept_array;

		enum class task {
			accept = 0
		};
	public:
		inline explicit server(void) noexcept
			: _iocp(framework::iocp::instance()) {
		}

		inline void start(void) noexcept {
			_session_array.initialize();
		}
		inline void stop(void) noexcept {
			_session_array.finalize();
		}

		inline void accept(char const* const ip, unsigned short port) noexcept {
			_listen.initialize(ip, port, 65535);
			_iocp.connect(*this, _listen._socket, static_cast<uintptr_t>(task::accept));

			_accept_array.initialize(10);
			for (auto& iter : _accept_array)
				_listen._socket.accept_ex(iter._socket, iter._buffer, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, iter._overlap);
		}
		inline void reject() {
			_listen._socket.close();
		}
		inline void connect(void) noexcept {

		}
		inline void worker(bool result, unsigned long transferred, uintptr_t key, OVERLAPPED* overlapped) noexcept override {
			switch (static_cast<task>(key)) {
			case task::accept: {
				auto accept_ = accept::recover(overlapped);

				auto result2 = library::socket::get_accept_ex_socket_address(accept_->_buffer);
				auto addr1 = result2._first.get_address();
				auto addr2 = result2._second.get_address();
				int a = 10;
				//if (INVALID_SOCKET == socket.data())
				//	break;

			} break;
			default:
				break;
			}
		};
	};

}