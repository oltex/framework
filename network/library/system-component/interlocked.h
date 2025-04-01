#pragma once
#include <Windows.h>

namespace system_component::interlocked {
	inline auto increment(long& addend) noexcept -> long {
		return _InterlockedIncrement(&addend);
	}
	inline auto increment(unsigned int& addend) noexcept -> unsigned int {
		return _InterlockedIncrement(&addend);
	}
	inline auto increment(unsigned long& addend) noexcept -> unsigned long {
		return _InterlockedIncrement(&addend);
	}
	inline auto increment(unsigned long long& addend) noexcept -> unsigned long long {
		return _InterlockedIncrement(&addend);
	}

	inline auto decrement(long& addend) noexcept -> long {
		return _InterlockedDecrement(&addend);
	}
	inline auto decrement(unsigned int& addend) noexcept -> unsigned int {
		return _InterlockedDecrement(&addend);
	}
	inline auto decrement(unsigned long& addend) noexcept -> unsigned long {
		return _InterlockedDecrement(&addend);
	}
	inline auto decrement(unsigned long long& addend) noexcept -> unsigned long long {
		return _InterlockedDecrement(&addend);
	}

	inline auto exchange(long& target, long value) noexcept -> long {
		return _InterlockedExchange(&target, value);
	}
	inline auto exchange(unsigned int& target, unsigned int value) noexcept -> unsigned int {
		return _InterlockedExchange(&target, value);
	}
	inline auto exchange(unsigned long& target, unsigned long value) noexcept -> unsigned long {
		return _InterlockedExchange(&target, value);
	}
	inline auto exchange(unsigned long long& target, unsigned long long value) noexcept -> unsigned long long {
		return _InterlockedExchange(&target, value);
	}

	//inline auto compare_exchange(void) noexcept {
	//	_InterlockedCompareExchange();
	//}
}