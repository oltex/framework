#pragma once
#include "../library/design-pattern/singleton.h"
#include <chrono>
#include <thread>

namespace engine {
	class timer final : public design_pattern::singleton<timer> {
		friend class design_pattern::singleton<timer>;
	private:
		inline explicit timer(void) noexcept
			: _time(std::chrono::steady_clock::now()) {
		};
		inline explicit timer(timer const& rhs) noexcept = delete;
		inline auto operator=(timer const& rhs) noexcept -> timer & = delete;
		inline explicit timer(timer&& rhs) noexcept = delete;
		inline auto operator=(timer&& rhs) noexcept -> timer & = delete;
		inline ~timer(void) noexcept = default;
	public:
		inline void update(void) noexcept {
			auto time = std::chrono::steady_clock::now();
			auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>(time - _time);
				_time += _frame;
			if (delta < _frame)
				std::this_thread::sleep_for(std::chrono::nanoseconds(_frame - delta));
		}
	public:
		inline void set_frame(long long frame) noexcept {
			_frame = std::chrono::nanoseconds(long long(1000000000. / frame));
		}
		inline auto get_frame(void) const noexcept -> long long {
			return static_cast<long long>(1000000000. / _frame.count());
		}
	private:
		std::chrono::steady_clock::time_point _time;
		std::chrono::nanoseconds _frame;
	};
}