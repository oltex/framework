#pragma once
#pragma comment(lib, "Synchronization.lib")
#include <Windows.h>

namespace system_component::multi {
	class wait_on_address final {
	public:
		inline explicit wait_on_address(void) noexcept = default;
		inline explicit wait_on_address(wait_on_address const& rhs) noexcept = delete;
		inline explicit wait_on_address(wait_on_address&& rhs) noexcept = delete;
		inline auto operator=(wait_on_address const& rhs) noexcept -> wait_on_address & = delete;
		inline auto operator=(wait_on_address&& rhs) noexcept -> wait_on_address & = delete;
		inline ~wait_on_address(void) noexcept = default;
	public:
		inline bool wait(void* address, void* _compare, size_t size, unsigned long milli_second) noexcept {
			return WaitOnAddress(address, _compare, size, milli_second);
		}
		inline void wake_single(void* address) noexcept {
			WakeByAddressSingle(address);
		}
		inline void wake_all(void* address) noexcept {
			WakeByAddressAll(address);
		}
	};
}