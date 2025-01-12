#pragma once
#include "../input_output/overlapped.h"
#include <Windows.h>
#include <concepts>

namespace system_component::kernel {
	class object {
	public:
		inline explicit object(void) noexcept
			: _handle(INVALID_HANDLE_VALUE) {
		};
		inline explicit object(HANDLE const handle) noexcept
			: _handle(handle) {
		};
		inline explicit object(object const& rhs) noexcept = delete;
		inline explicit object(object&& rhs) noexcept
			: _handle(rhs._handle) {
			rhs._handle = INVALID_HANDLE_VALUE;
		};
		inline auto operator=(object const& rhs) noexcept -> object & = delete;
		inline auto operator=(object&& rhs) noexcept -> object& {
			CloseHandle(_handle);
			_handle = rhs._handle;
			rhs._handle = INVALID_HANDLE_VALUE;
			return *this;
		};
		inline virtual ~object(void) noexcept {
			CloseHandle(_handle);
		};
	public:
		inline void close(void) noexcept {
			CloseHandle(_handle);
			_handle = INVALID_HANDLE_VALUE;
		}
		inline void cancel_io(void) const noexcept {
			CancelIo(_handle);
		}
		inline void cancel_io_ex(void) const noexcept {
			CancelIoEx(_handle, nullptr);
		}
		inline void cancel_io_ex(input_output::overlapped overlapped) const noexcept {
			CancelIoEx(_handle, &overlapped.data());
		}
		inline auto wait_for_single(unsigned long const milli_second) noexcept -> unsigned long {
			return WaitForSingleObject(_handle, milli_second);
		}
		inline auto wait_for_single_ex(unsigned long const milli_second, bool alertable) noexcept -> unsigned long {
			return WaitForSingleObjectEx(_handle, milli_second, alertable);
		}
		inline auto data(void) noexcept -> HANDLE& {
			return _handle;
		}
	public:
		inline static auto wait_for_multiple(unsigned long const count, HANDLE* handle, bool const wait_all, unsigned long const milli_second) noexcept -> unsigned long {
			return WaitForMultipleObjects(count, handle, wait_all, milli_second);
		}
		template<std::derived_from<object>... object_>
		inline static auto wait_for_multiple(bool const wait_all, unsigned long const milli_second, object_&... object) noexcept -> unsigned long {
			HANDLE handle[] = { object.data()... };
			return WaitForMultipleObjects(sizeof...(object_), handle, wait_all, milli_second);
		}
		inline static auto wait_for_multiple_ex(unsigned long const count, HANDLE* handle, bool const wait_all, unsigned long const milli_second, bool alertable) noexcept {
			return WaitForMultipleObjectsEx(count, handle, wait_all, milli_second, alertable);
		}
	protected:
		HANDLE _handle;
	};
}