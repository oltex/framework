#pragma once
#include <Windows.h>

namespace window {
	class icon final {
	public:
		inline auto destroy(void) noexcept -> BOOL {
			return DestroyIcon(_hicon);
		}
		inline auto draw(HDC const dc, int x, int y) noexcept {
			DrawIcon(dc, x, y, _hicon);
		}
		inline void load(HINSTANCE const hInst, LPCWSTR const name, UINT const type, int const cx, int const cy, UINT const fuLoad) noexcept {
			_hicon = static_cast<HICON>(LoadImageW(hInst, name, type, cx, cy, fuLoad));
		};
	public:
		inline auto data(void) const noexcept -> HICON {
			return _hicon;
		}
	private:
		HICON _hicon = nullptr;
	};
}