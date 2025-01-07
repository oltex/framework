#pragma once
#pragma comment(lib,"ws2_32.lib")
#include <WinSock2.h>
#include <intrin.h>

namespace system_component::network::window_socket_api {
	class initialize final {
	public:
		inline explicit initialize(void) noexcept {
			WSAData wsadata;
			if (0 != WSAStartup(0x0202, &wsadata))
				__debugbreak();
		}
		inline explicit initialize(initialize const&) noexcept = delete;
		inline explicit initialize(initialize&&) noexcept = delete;
		inline auto operator=(initialize const&) noexcept -> initialize & = delete;
		inline auto operator=(initialize&&) noexcept -> initialize & = delete;
		inline ~initialize(void) noexcept {
			if (SOCKET_ERROR == WSACleanup())
				__debugbreak();
		}
	};

	inline static void start_up(void) noexcept {
		WSAData wsadata;
		if (0 != WSAStartup(0x0202, &wsadata))
			__debugbreak();
	};
	inline static void clean_up(void) noexcept {
		if (SOCKET_ERROR == WSACleanup())
			__debugbreak();
	};
}