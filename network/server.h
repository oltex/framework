#pragma once
#include "iocp.h"

namespace framework {
	class server final : iocp::object {
		inline void start(void) noexcept {

		}
		inline void worker(bool result, unsigned long transferred, uintptr_t key, OVERLAPPED* overlapped) noexcept override {

		};
	};
}