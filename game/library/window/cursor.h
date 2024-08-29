#pragma once
#include <Windows.h>

namespace window {
	class cursor final {
	public:
		inline auto destroy(void) noexcept -> BOOL {
			return DestroyCursor(_hcursor);
		}
		inline void get(void) noexcept {
			_hcursor = GetCursor();
		}
		inline void load(HINSTANCE const hInst, LPCWSTR const name, UINT const type, int const cx, int const cy, UINT const fuLoad) noexcept {
			_hcursor = static_cast<HCURSOR>(LoadImageW(hInst, name, type, cx, cy, fuLoad));
		};
		static inline auto clip(RECT const rect) noexcept -> BOOL {
			return ClipCursor(&rect);
		}
		static inline auto set_pos(int const x, int const y) -> BOOL {
			return SetCursorPos(x, y);
		}
		static inline auto show(BOOL const bShow) noexcept -> int {
			return ShowCursor(bShow);
		};
	public:
		inline auto data(void) const noexcept -> HCURSOR {
			return _hcursor;
		}
	private:
		HCURSOR _hcursor = nullptr;
	};
}