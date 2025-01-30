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
		inline void wait(volatile long& address, volatile long _compare, unsigned long milli_second) noexcept {
			WaitOnAddress(&address, (void*)&_compare, sizeof(long), milli_second);
		}
		inline void wake_single(volatile long& address) noexcept {
			WakeByAddressSingle((void*)&address);
		}
		inline void wake_all(volatile long& address) noexcept {
			WakeByAddressAll((void*)&address);
		}
	};
}