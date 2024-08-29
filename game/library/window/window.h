#pragma once
#include "device_context.h"
#include <Windows.h>

namespace window {
	class window final {
	public:
		inline explicit window(HWND const hwnd) noexcept
			: _hwnd(hwnd) {
		}
		inline explicit window(window const& rhs) noexcept = delete;
		inline auto operator=(window const& rhs) noexcept -> window & = delete;
		inline explicit window(window&& rhs) noexcept
			: _hwnd(rhs._hwnd) {
			rhs._hwnd = nullptr;
		}
		inline auto operator=(window&& rhs) noexcept -> window& {
			_hwnd = rhs._hwnd;
			rhs._hwnd = nullptr;
			return *this;
		}
		inline ~window(void) noexcept {
			destroy();
		};
	public:
		inline auto get_device_context(void) const noexcept -> device_context {
			HDC hdc = GetDC(_hwnd);
			return device_context(hdc);
		}
		inline void release_device_context(device_context& dc) const noexcept {
			ReleaseDC(_hwnd, dc.data());
		}
		inline auto begin_paint(void) const noexcept -> device_context {
			PAINTSTRUCT paintstruct;
			HDC hdc = BeginPaint(_hwnd, &paintstruct);
			return device_context(hdc, paintstruct);
		}
		inline void end_paint(device_context& dc) const noexcept {
			EndPaint(_hwnd, &dc.get_paint_struct());
		}
	public:
		inline void screen_to_client(LPPOINT const lpPoint) const noexcept {
			ScreenToClient(_hwnd, lpPoint);
		}
		inline auto show(int const nCmdShow) const noexcept -> BOOL {
			return ShowWindow(_hwnd, nCmdShow);
		}
		inline auto move(int const x, int const y, int const width, int const height) const noexcept -> BOOL {
			return MoveWindow(_hwnd, x, y, width, height, false);
		}
		inline auto update(void) const noexcept -> BOOL {
			return UpdateWindow(_hwnd);
		}
		inline auto close(void) const noexcept -> BOOL {
			return CloseWindow(_hwnd);
		}
		inline auto destroy(void) const noexcept -> BOOL {
			return DestroyWindow(_hwnd);
		}
		inline auto get_client_rect(void) const noexcept -> RECT {
			RECT rect;
			GetClientRect(_hwnd, &rect);
			return rect;
		}
		inline auto get_window_rect(void) const noexcept -> RECT {
			RECT rect;
			GetWindowRect(_hwnd, &rect);
			return rect;
		}
		inline auto set_window_text(LPCWSTR const lpString) const noexcept -> BOOL {
			return SetWindowTextW(_hwnd, lpString);
		}
		inline auto invalidate_rect(RECT const* lpRect, BOOL const bErase) const noexcept -> BOOL {
			return InvalidateRect(_hwnd, lpRect, bErase);
		}
	public:
		inline auto data(void) const noexcept -> HWND {
			return _hwnd;
		}
	protected:
		HWND _hwnd;
	};
}