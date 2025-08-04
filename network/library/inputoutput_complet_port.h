#pragma once
#pragma comment(lib,"ws2_32.lib")
#include <WinSock2.h>
#include "socket.h"
#include "handle.h"
#include "tuple.h"

namespace library {
	class inputoutput_complet_port final : public handle {
	public:
		inline explicit inputoutput_complet_port(void) noexcept = default;
		inline explicit inputoutput_complet_port(unsigned long const concurrent_thread) noexcept
			: handle(CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, concurrent_thread)) {
		};
		inline explicit inputoutput_complet_port(inputoutput_complet_port const&) noexcept = delete;
		inline explicit inputoutput_complet_port(inputoutput_complet_port&& rhs) noexcept
			: handle(std::move(rhs)) {
		};
		inline auto operator=(inputoutput_complet_port const&) noexcept -> inputoutput_complet_port & = delete;
		inline auto operator=(inputoutput_complet_port&& rhs) noexcept -> inputoutput_complet_port& {
			handle::operator=(std::move(rhs));
			return *this;
		}
		inline virtual ~inputoutput_complet_port(void) noexcept override = default;

		inline void create(unsigned long const concurrent_thread) noexcept {
			_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, concurrent_thread);
		}
		inline void connect(library::socket& socket, ULONG_PTR const key) noexcept {
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket.data()), _handle, key, 0);
		}
		inline auto get_queue_state(unsigned long const milli_second) noexcept -> library::tuple<bool, DWORD, ULONG_PTR, OVERLAPPED*> {
			library::tuple<bool, DWORD, ULONG_PTR, OVERLAPPED*> result;
			result.get<0>() = GetQueuedCompletionStatus(_handle, &result.get<1>(), &result.get<2>(), &result.get<3>(), milli_second);
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