#pragma once
#include "iocp.h"
#include "session.h"

namespace framework {
	class server final : iocp::object {
		enum class task {
			accept = 0, socket
		};
		iocp& _iocp;
		session_array _session_array;
		listen _listen;
		accept_array _accept_array;
	public:
		inline explicit server(void) noexcept
			: _iocp(framework::iocp::instance()) {
		}

		inline void start(void) noexcept {
			_session_array.initialize(100);
		}
		inline void stop(void) noexcept {
			_session_array.finalize();
		}

		inline void accept(char const* const ip, unsigned short port) noexcept {
			_listen.initialize(ip, port, 65535);
			_iocp.connect(*this, _listen._socket, static_cast<uintptr_t>(task::accept));

			_accept_array.initialize(10);
			for (auto& iter : _accept_array)
				_listen.accept(iter);
		}
		inline void reject(void) {
			_listen._socket.close();
		}
		inline void connect(void) noexcept {

		}

	private:
		inline void worker(bool result, unsigned long transferred, uintptr_t key, OVERLAPPED* overlapped) noexcept override {
			switch (static_cast<task>(key)) {
			case task::accept: {
				auto& accept = *accept::recover(overlapped);
				accept.inherit(_listen);
				if (false == result) {
					accept.finalize();
					break;
				}
				auto address = accept.address();
				if (false == on_accept_socket(address._second))
					accept.finalize();
				else {
					auto& session = *_session_array.allocate();
					session.initialize(std::move(accept._socket), 400000);
					_iocp.connect(*this, session._socket, static_cast<uintptr_t>(task::socket));
					on_create_session(session._key);

					//session.receive();
					//if (session_->release()) {
					//	on_destroy_session(session_->_key);
					//	_session_array.release(*session_);
					//}
				}

				accept.initialize();
				_listen.accept(accept);
			} break;
			default:
				break;
			}
		};

		inline virtual bool on_accept_socket(library::socket_address_ipv4& socket_address) noexcept {
			return true;
		}
		inline virtual void on_create_session(unsigned long long key) noexcept {
			//message_pointer message_ = server::create_message();
			//*message_ << 0x7fffffffffffffff;
			//do_send_session(key, message_);
		}
	};

}