#pragma once
#include "network.h"
#include "server.h"

inline void network::worker(void) noexcept {
	auto [result, transferred, key, overlapped] = _complation_port.get_queue_state(INFINITE);
	switch (static_cast<type>(key)) {
		using enum type;
	case task:
		break;
	default:
		reinterpret_cast<server*>(0x00007FFFFFFFFFFFULL & key)->worker(result, transferred, key, overlapped);
	}
}