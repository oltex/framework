#pragma once

namespace engine {
	class pipeline {
	public:
		inline explicit pipeline(void) noexcept = default;
		inline virtual ~pipeline(void) noexcept = default;
	public:
		inline virtual void render(void) noexcept = 0;
	};
}