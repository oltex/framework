#pragma once
#pragma comment(lib,"ws2_32.lib")
#include <WinSock2.h>
#include "../network/socket.h"
#include "../kernel/object.h"

namespace system_component::input_output {
	class completion_port final : public kernel::object {
	public:
		inline explicit completion_port(void) noexcept = default;
		inline explicit completion_port(unsigned long const concurrent_thread) noexcept
			: object(CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, concurrent_thread)) {
		};
		inline explicit completion_port(completion_port const& rhs) noexcept = delete;
		inline explicit completion_port(completion_port&& rhs) noexcept
			: object(std::move(rhs)) {
		};
		inline auto operator=(completion_port const& rhs) noexcept -> completion_port & = delete;
		inline auto operator=(completion_port&& rhs) noexcept -> completion_port& {
			object::operator=(std::move(rhs));
			return *this;
		}
		inline virtual ~completion_port(void) noexcept override = default;
	public:
		inline void create(unsigned long const concurrent_thread) noexcept {
			_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, concurrent_thread);
		}
		inline void connect(network::socket& socket, ULONG_PTR key) noexcept {
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket.data()), _handle, key, 0);
		}
		struct get_queue_state_result final {
			bool _result;
			DWORD _transferred;
			ULONG_PTR _key;
			OVERLAPPED* _overlapped;
		};
		inline auto get_queue_state(unsigned long milli_second) noexcept -> get_queue_state_result {
			get_queue_state_result result;
			result._result = GetQueuedCompletionStatus(_handle, &result._transferred, &result._key, &result._overlapped, milli_second);
			return result;
		}
		//inline auto get_queue_state_ex(void) noexcept {
		//	GetQueuedCompletionStatusEx()
		//}
		inline void post_queue_state(unsigned long transferred, uintptr_t key, OVERLAPPED* overlapped) noexcept {
			PostQueuedCompletionStatus(_handle, transferred, key, overlapped);
		}
	};
}