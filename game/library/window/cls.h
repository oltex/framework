#pragma once
#include "instance.h"
#include "icon.h"
#include "cursor.h"
#include <Windows.h>

namespace window {
	class cls final {
	public:
		inline explicit cls(void) noexcept {
			initialize();
		}
		inline ~cls(void) noexcept = default;
	public:
		inline void initialize(void) noexcept {
			memset(&_wcex, 0, sizeof(WNDCLASSEX));
			_wcex.cbSize = sizeof(WNDCLASSEX);
			_wcex.lpfnWndProc = DefWindowProcW;
			_wcex.hbrBackground = 0;
		}
		inline void register_(void) const noexcept {
			RegisterClassExW(&_wcex);
		}
		inline void unregister(void) const noexcept {
			UnregisterClassW(_wcex.lpszClassName, _wcex.hInstance);
		}
	public:
		inline void set_style(UINT style) noexcept {
			_wcex.style = style;
		}
		inline void set_procedure(WNDPROC const lpfnWndProc) noexcept {
			_wcex.lpfnWndProc = lpfnWndProc;
		}
		inline void set_class_extra(int const cbClsExtra) noexcept {
			_wcex.cbClsExtra = cbClsExtra;
		}
		inline void set_window_extra(int const cbWndExtra) noexcept {
			_wcex.cbWndExtra = cbWndExtra;
		}
		inline void set_instance(instance const& instance) noexcept {
			_wcex.hInstance = instance.data();
		}
		inline void set_icon(icon const& icon) noexcept {
			_wcex.hIcon = icon.data();
		}
		inline void set_cursor(cursor const& cursor) noexcept {
			_wcex.hCursor = cursor.data();
		}
		inline void set_background(HBRUSH const hbrBackground) noexcept {
			_wcex.hbrBackground = hbrBackground;
		}
		inline void set_menu_name(LPCWSTR const lpszMenuName) noexcept {
			_wcex.lpszMenuName = lpszMenuName;
		}
		inline void set_class_name(LPCWSTR const lpszClassName) noexcept {
			_wcex.lpszClassName = lpszClassName;
		};
		inline void set_icon_small(icon const& icon) noexcept {
			_wcex.hIconSm = icon.data();
		}
	private:
		WNDCLASSEXW _wcex;
	};
}