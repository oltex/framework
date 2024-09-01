#pragma once
#include "../library/design-pattern/singleton.h"
#include "component.h"
#include "transform.h"
#include <unordered_map>

namespace engine {
	class component_manager final : public design_pattern::singleton<component_manager> {
		friend class design_pattern::singleton<component_manager>;
	private:
		inline explicit component_manager(void) noexcept {
		};
		inline explicit component_manager(component_manager const& rhs) noexcept = delete;
		inline auto operator=(component_manager const& rhs) noexcept -> component_manager & = delete;
		inline explicit component_manager(component_manager&& rhs) noexcept = delete;
		inline auto operator=(component_manager&& rhs) noexcept -> component_manager & = delete;
		inline ~component_manager(void) noexcept {
			//for (auto& iter : _prototype)
			//	delete iter._second;
		};
	public:
		inline void update(void) noexcept {
		}
	private:
		std::unordered_map<std::string, component* const> _prototype;
	};
}