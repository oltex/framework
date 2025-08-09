#pragma once
#include "library/intrusive/share_pointer.h"
#include "library/serialize_buffer.h"

namespace framework {
	class message : public library::intrusive::share_pointer_hook<0>, public library::serialize_buffer<false, 128> {
	public:
		inline explicit message(void) noexcept = delete;
		inline explicit message(message const&) noexcept = delete;
		inline explicit message(message&&) noexcept = delete;
		inline auto operator=(message const&) noexcept -> message & = delete;
		inline auto operator=(message&&) noexcept -> message & = delete;
		inline ~message(void) noexcept = delete;

		template<size_t index>
		inline void destructor(void) noexcept;
		template<>
		inline void destructor<1>(void) noexcept {
			//auto& memory_pool = data_structure::_thread_local::memory_pool<message>::instance();
			//memory_pool.deallocate(*this);
			int a = 10;
		}
	};

	//using message_pointer = data_structure::intrusive::shared_pointer<message, 0>;
	//class view final : public data_structure::intrusive::shared_pointer_hook<0> {

}