#pragma once
#include "library/lockfree/free_array.h"
#include "library/socket.h"
#include "message.h"

namespace framework {
	class session final {
		using size_type = unsigned int;
		unsigned long long _key;
		unsigned long _io_count; // release_flag // recv_flag // io count
		library::socket _socket;

		inline explicit session(size_type const index) noexcept
			: _key(index), _io_count(0x80000000) {
			auto& memory_pool = data_structure::_thread_local::memory_pool<message>::instance();
			message_pointer receive_message(&memory_pool.allocate());
			_receive_message = receive_message;
		};
	};

	class session_array final : public library::lockfree::free_array<session> {
		using base = library::lockfree::free_array<session>;
		using size_type = library::lockfree::free_array<session>::size_type;
		size_type _size;
	public:
		inline explicit session_array(size_type capacity) noexcept 
			: base(capacity, ) {

		};
		inline explicit session_array(session_array const&) noexcept = delete;
		inline explicit session_array(session_array&&) noexcept = delete;
		inline auto operator=(session_array const&) noexcept -> session_array & = delete;
		inline auto operator=(session_array&&) noexcept -> session_array & = delete;
		inline ~session_array(void) noexcept = default;

		inline auto operator[](unsigned long long const key) noexcept -> session& {
			return base::operator[](key & 0xffff);
		}
	};
}