#pragma once
#include <Windows.h>
#include "memory.h"

namespace library {
	class overlap final {
	public:
		inline explicit overlap(void) noexcept = default;
		inline explicit overlap(overlap const&) noexcept = delete;
		inline explicit overlap(overlap&&) noexcept = delete;
		inline auto operator=(overlap const&) noexcept -> overlap & = delete;
		inline auto operator=(overlap&&) noexcept -> overlap & = delete;
		inline ~overlap(void) noexcept = default;

		//inline void set_event(multi::event& event) noexcept {
		//	_overlapped.hEvent = event.data();
		//}
		inline auto get_result(HANDLE handle, unsigned long* byte, bool const wait) noexcept -> bool {
			return GetOverlappedResult(handle, &_overlapped, byte, wait);
		}
		inline auto has_completed(void) const noexcept -> bool {
			return HasOverlappedIoCompleted(&_overlapped);
		}
		inline void clear(void) noexcept {
			library::memory_set(&_overlapped, 0, sizeof(_OVERLAPPED));
		}
		inline auto data(void) noexcept -> _OVERLAPPED& {
			return _overlapped;
		}
	//private:
		_OVERLAPPED _overlapped;
	};
}