#pragma once
#include <chrono>
#include <thread>

class timer final {
public:
	inline explicit timer(void) noexcept
		: _time(std::chrono::steady_clock::now()) {
	}
	inline ~timer(void) noexcept = default;
public:
	inline void update(std::chrono::steady_clock::time_point time) noexcept {
		_delta = std::chrono::duration_cast<std::chrono::nanoseconds>(time - _time);
		if (0 != _frame.count())
			_time += _frame;
		else
			_time = time;
	}
public:
	inline void set_frame(std::chrono::nanoseconds const frame) noexcept {
		_frame = frame;
	}
	inline auto get_frame(void) const noexcept -> std::chrono::nanoseconds {
		return _frame;
	}
	inline auto get_delta(void) const noexcept -> std::chrono::nanoseconds {
		return _delta;
	}
private:
	std::chrono::steady_clock::time_point _time;
	std::chrono::nanoseconds _frame;
	std::chrono::nanoseconds _delta;
};