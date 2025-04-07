#pragma once
#include "library/system-component/inputoutput_completion_port.h"
#include "library/system-component/interlocked.h"
#include "library/system-component/socket.h"
#include "library/system-component/thread.h"
#include "library/system-component/time/multimedia.h"

#include "library/data-structure/intrusive/shared_pointer.h"
#include "library/data-structure/lockfree/queue.h"
#include "library/data-structure/thread-local/memory_pool.h"
#include "library/data-structure/serialize_buffer.h"
#include "library/data-structure/vector.h"

#include "string_view"

class network {
	class listen final {
	public:
		struct accept {
			system_component::socket _socket;
			unsigned char _buffer[30];
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

			//for (unsigned short index = 0; index < 16; ++index)
			//	_socket.accept_ex()
		}

		system_component::socket _socket;
		accept* _accept;
	};
private:
	enum class post_queue_state : unsigned char {
		accept, close_worker, destory_session, excute_task, destory_group
	};
public:
	inline explicit network(void) noexcept {
		system_component::time::multimedia::end_period(1);
		system_component::wsa_start_up();
		system_component::socket socket_(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		socket_.wsa_io_control_acccept_ex();
	};
	inline explicit network(network const&) noexcept = delete;
	inline explicit network(network&&) noexcept = delete;
	inline auto operator=(network const&) noexcept -> network & = delete;
	inline auto operator=(network&&) noexcept -> network & = delete;
	inline ~network(void) noexcept {
		system_component::wsa_clean_up();
		system_component::time::multimedia::end_period(1);
	};

	inline void start(void) noexcept {
		_complation_port.create(0);
		for (unsigned short index = 0; index < 16; ++index)
			_worker_thread.emplace_back(&network::worker, 0, this);
	}
	inline void stop(void) noexcept {

	}
	inline void accept(void) noexcept {
		_complation_port.connect(_listen._socket, )
	

	}
	inline void connect(void) noexcept {

	}
	inline void worker(void) noexcept {
		auto [result, transferred, key, overlapped] = _complation_port.get_queue_state(INFINITE);
		switch (static_cast<post_queue_state>(key)) {
		}
	}
private:
	system_component::inputoutput_completion_port _complation_port;
	data_structure::vector<system_component::thread> _worker_thread;
	listen _listen;
};