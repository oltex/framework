#pragma once
#include "library/system-component/socket.h"
#include "library/system-component/thread.h"

#include "library/data-structure/serialize_buffer.h"
#include "library/data-structure/intrusive/shared_pointer.h"
#include "library/data-structure/thread-local/memory_pool.h"

class network_client {
private:
	class message final : public data_structure::intrusive::shared_pointer_hook<0>, public data_structure::serialize_buffer<> {
	public:
		inline explicit message(void) noexcept = delete;
		inline explicit message(message const&) noexcept = delete;
		inline explicit message(message&&) noexcept = delete;
		inline auto operator=(message const&) noexcept -> message & = delete;
		inline auto operator=(message&&) noexcept -> message & = delete;
		inline ~message(void) noexcept = delete;

		inline void destructor(void) noexcept {
			auto& memory_pool = data_structure::_thread_local::memory_pool<message>::instance();
			memory_pool.deallocate(*this);
		}
	};
	using message_pointer = data_structure::intrusive::shared_pointer<message, 0>;
public:
	inline explicit network_client(void) noexcept = default;
	inline explicit network_client(network_client const&) noexcept = delete;
	inline explicit network_client(network_client&&) noexcept = delete;
	inline auto operator=(network_client const&) noexcept -> network_client & = delete;
	inline auto operator=(network_client&&) noexcept -> network_client & = delete;
	inline ~network_client(void) noexcept = default;

	inline void connect(void) noexcept {

	}


private:
	system_component::socket _socket;
	system_component::thread _receive_thread;
	message_pointer _receive_message_ptr;

	unsigned char _header_code;
	unsigned char _header_fixed_key;
};