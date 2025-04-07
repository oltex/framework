#pragma once
#include "scheduler.h"

class scheduler::task {
	inline explicit task(void) noexcept
		: _time(system_component::time::multimedia::get_time()) {
	};
	inline explicit task(task const&) noexcept = delete;
	inline explicit task(task&&) noexcept = delete;
	inline auto operator=(task const&) noexcept -> task & = delete;
	inline auto operator=(task&&) noexcept -> task & = delete;
	inline ~task(void) noexcept = default;

	inline virtual bool excute(void) noexcept = 0;

	unsigned long _time;
};