#pragma once
#include "library/system-component/socket.h"
#include "library/system-component/overlapped.h"

//#include "libr"

class session final {
	inline explicit session(unsigned short const index) noexcept
		: _key(index), _io_count(0x80000000) {
		auto& memory_pool = data_structure::_thread_local::memory_pool<message>::instance();
		message_pointer receive_message(&memory_pool.allocate());
		receive_message->clear();
		_receive_message = receive_message;
	};
	inline explicit session(session const&) noexcept = delete;
	inline explicit session(session&&) noexcept = delete;
	inline auto operator=(session const&) noexcept -> session & = delete;
	inline auto operator=(session&&) noexcept -> session & = delete;
	inline ~session(void) noexcept = default;

	system_component::socket _socket;
	message_pointer _receive_message;
	queue _receive_queue;
	queue _send_queue;
	system_component::overlapped _recv_overlapped;
	system_component::overlapped _send_overlapped;
	unsigned long long _key;
	unsigned long _io_count; // release_flag : 1 group_io : ? : io_count : ?
	unsigned long _cancel_flag;
	unsigned long _receive_count; // release_flag : 2
	unsigned long _send_flag;
	unsigned long _send_size;
	unsigned long long _timeout_currnet;
	unsigned long long _timeout_duration;
	unsigned long long _group_key;
};
