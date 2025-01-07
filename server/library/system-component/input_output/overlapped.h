#pragma once
#include <Windows.h>

namespace system_component::input_output {
	class overlapped final {
	public:
		inline explicit overlapped(void) noexcept = default;
		inline explicit overlapped(overlapped const& rhs) noexcept = delete;
		inline explicit overlapped(overlapped&& rhs) noexcept
			: _overlapped(rhs._overlapped) {
		};
		inline auto operator=(overlapped const& rhs) noexcept -> overlapped & = delete;
		inline auto operator=(overlapped&& rhs) noexcept -> overlapped& {
			_overlapped = rhs._overlapped;
			return *this;
		};
		inline ~overlapped(void) noexcept = default;
	public:
		inline auto get_result(HANDLE handle, unsigned long* byte, bool const wait) noexcept -> bool {
			return GetOverlappedResult(handle, &_overlapped, byte, wait);
		}
		inline bool has_completed(void) const noexcept {
			return HasOverlappedIoCompleted(&_overlapped);
		}
	public:
		inline void clear(void) noexcept {
			memset(&_overlapped, 0, sizeof(_OVERLAPPED));
		}
		inline auto data(void) noexcept -> _OVERLAPPED& {
			return _overlapped;
		}
	private:
		_OVERLAPPED _overlapped;
	};
}