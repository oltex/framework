#pragma once
#include "library/system-component/socket.h"

class server {
private:
	enum class type : unsigned long long {
		accept = 0x0000800000000000ULL
	};
	class listen final {
	public:
		struct accept {
			inline explicit accept(void) noexcept
				: _socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), _overlapped() {
			};
			inline explicit accept(accept const&) noexcept = delete;
			inline explicit accept(accept&&) noexcept = delete;
			inline auto operator=(accept const&) noexcept -> accept & = delete;
			inline auto operator=(accept&&) noexcept -> accept & = delete;
			inline ~accept(void) noexcept = default;

			system_component::socket _socket;
			unsigned char _buffer[30];
			system_component::overlapped _overlapped;
		};

		inline void initialize(std::string_view const ip, unsigned short const port, int send_buffer, int const backlog) noexcept {
			system_component::socket_address_ipv4 socket_address;
			socket_address.set_address(ip.data());
			socket_address.set_port(port);
			_socket.create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			_socket.set_option_linger(1, 0);
			_socket.set_option_send_buffer(send_buffer);
			_socket.bind(socket_address);
			_socket.listen(backlog);
		}

		system_component::socket _socket;
		accept* _accept;
	};
	class session;
public:
	inline explicit server(void) noexcept
		: _network(network::instance()) {
	};
	inline explicit server(server const&) noexcept = delete;
	inline explicit server(server&&) noexcept = delete;
	inline auto operator=(server const&) noexcept -> server & = delete;
	inline auto operator=(server&&) noexcept -> server & = delete;
	inline ~server(void) noexcept {
	};
public:
	inline void accept(std::string_view const ip, unsigned short const port, int send_buffer, int const backlog) noexcept {
		_listen.initialize(ip, port, send_buffer, backlog);

		_network._complation_port.connect(_listen._socket, static_cast<ULONG_PTR>(type::accept) + reinterpret_cast<ULONG_PTR>(this));

		_listen._accept = reinterpret_cast<listen::accept*>(malloc(sizeof(listen::accept) * 16));
		listen::accept* accept_ = _listen._accept;
		for (unsigned short index = 0; index < 16; ++index, ++accept_) {
			::new(reinterpret_cast<void*>(accept_)) listen::accept();
			_listen._socket.accept_ex(accept_->_socket, accept_->_buffer, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, accept_->_overlapped);
		}
	}
	inline void connect(void) noexcept {

	}
private:
	inline void worker(bool result, DWORD transferred, ULONG_PTR key, OVERLAPPED* _overlapped) noexcept {
		switch (static_cast<type>(0xFFFF800000000000ULL & key)) {
		case type::accept: {
		} break;
		default:
			break;
		}
	}

	//inline virtual bool accept(system_component::socket_address_ipv4& socket_address) noexcept = 0;
	network& _network;
	listen _listen;

};