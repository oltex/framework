#pragma once
#include "component.h"
#include <unordered_map>
#include <string>

namespace engine {
	class object final {
		friend class object_manager;
	public:
		inline explicit object(unsigned int const type) noexcept {
		};
		inline explicit object(object const& rhs) noexcept {
		}
		inline auto operator=(object const& rhs) noexcept -> object & = delete;
		inline explicit object(object&& rhs) noexcept = delete;
		inline auto operator=(object&& rhs) noexcept -> object & = delete;
		inline ~object(void) noexcept = default;
	public:
		inline void set_active(bool const enable) noexcept {
			_active = enable;
		}
		inline void set_destory(bool const enable) noexcept {
			_destory = enable;
		}
	private:
		bool _active = false;
		bool _destory = false;
		std::unordered_map<std::string, component*> _component;
	};
}