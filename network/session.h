#pragma once
#include "library/lockfree/free_array.h"
#include "library/socket.h"
#include "library/interlock.h"
#include "message.h"

namespace framework {
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

	class session_array final : public library::lockfree::free_array<session> {
		using base = library::lockfree::free_array<session>;
		using size_type = library::lockfree::free_array<session>::size_type;
		size_type _size;
	public:
		inline explicit session_array(size_type capacity, size_type index = 0) noexcept
			: _size(0), base(capacity, index) {
		};
		inline explicit session_array(session_array const&) noexcept = delete;
		inline explicit session_array(session_array&&) noexcept = delete;
		inline auto operator=(session_array const&) noexcept -> session_array & = delete;
		inline auto operator=(session_array&&) noexcept -> session_array & = delete;
		inline ~session_array(void) noexcept = default;

		inline auto allocate(void) noexcept -> session* {
			auto result = base::allocate();
			if (nullptr != result)
				library::interlock_increment(_size);
			return result;
		}
		inline void deallocate(session* value) noexcept {
			base::deallocate(value);
			library::interlock_decrement(_size);
		}

		inline auto operator[](unsigned long long const key) noexcept -> session& {
			return base::operator[](key & 0xffff);
		}
	};
}