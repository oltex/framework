#pragma once
#include "bitmap.h"
#include "object.h"
#include "brush.h"
#include <Windows.h>

namespace window {
	class device_context final {
	public:
		inline explicit device_context(HDC const hdc) noexcept
			: _hdc(hdc) {
		}
		inline explicit device_context(HDC const hdc, PAINTSTRUCT const& paint)
			: _hdc(hdc), _paint(paint) {
		}
		inline explicit device_context(device_context const& rhs) noexcept = delete;
		inline auto operator=(device_context const& rhs) noexcept -> device_context & = delete;
		inline explicit device_context(device_context&& rhs) noexcept
			: _hdc(rhs._hdc), _paint(rhs._paint) {
			rhs._hdc = nullptr;
			rhs._paint = PAINTSTRUCT{};
		};
		inline auto operator=(device_context&& rhs) noexcept -> device_context& {
			_hdc = rhs._hdc;
			rhs._hdc = nullptr;
			_paint = rhs._paint;
			rhs._paint = PAINTSTRUCT{};
		};
		inline ~device_context(void) noexcept {
			DeleteDC(_hdc);
		};
	public:
		inline auto create_compatible_device_context(void) const noexcept -> device_context {
			HDC hdc = CreateCompatibleDC(_hdc);
			return device_context(hdc);
		}
		inline auto create_compatible_bitmap(int const cx, int const cy) const noexcept {
			HBITMAP hbitmap = CreateCompatibleBitmap(_hdc, cx, cy);
			return bitmap(hbitmap);
		}
	public:
		inline void select_object(object const& object_) noexcept {
			SelectObject(_hdc, object_.data());
		}
		inline void fill_rect(RECT const* const rect, brush const& brush_) noexcept {
			FillRect(_hdc, rect, (HBRUSH)brush_.data());
		}
	public:
		inline void set_bk_mode(int const mode) const noexcept {
			SetBkMode(_hdc, mode);
		}
		inline void set_text_color(COLORREF const color) const noexcept {
			SetTextColor(_hdc, color);
		}
		inline void draw_text(LPCWSTR const lpchText, int const cchText, LPRECT const lprc, UINT const format) const noexcept {
			DrawTextW(_hdc, lpchText, cchText, lprc, format);
		}
		inline void text_out(int const x, int const y, LPCWSTR const lpString, int const c) const noexcept {
			TextOutW(_hdc, x, y, lpString, c);
		}
		inline void ellipse(int const left, int const top, int const right, int const bottom) const noexcept {
			Ellipse(_hdc, left, top, right, bottom);
		}
		inline void pat_blt(int const x, int const y, int const w, int const h, DWORD const rop) const noexcept {
			PatBlt(_hdc, x, y, w, h, rop);
		}
		inline void bit_blt(int const x, int const y, int const cx, int const cy, device_context const& dc, int const x1, int const y1, DWORD const rop) const noexcept {
			BitBlt(_hdc, x, y, cx, cy, dc._hdc, x1, y1, rop);
		}
		inline void move_to(int const x, int const y) const noexcept {
			MoveToEx(_hdc, x, y, nullptr);
		}
		inline void line_to(int const x, int const y) const noexcept {
			LineTo(_hdc, x, y);
		}
		inline void rectangle(int const left, int const top, int const right, int const bottom) const noexcept {
			Rectangle(_hdc, left, top, right, bottom);
		}
	public:
		inline auto data(void) const noexcept -> HDC {
			return _hdc;
		}
		inline auto get_paint_struct(void) noexcept -> PAINTSTRUCT& {
			return _paint;
		}
	private:
		HDC _hdc;
		PAINTSTRUCT _paint{};
	};
}