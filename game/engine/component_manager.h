#pragma once
#include "../library/design-pattern/singleton.h"
#include "unordered_map"
#include "component.h"

#include "transform.h"

namespace engine {
	class component_manager final : public design_pattern::singleton<component_manager> {
		friend class design_pattern::singleton<component_manager>;
	private:
		inline explicit component_manager(void) noexcept {

		};
		inline ~component_manager(void) noexcept {
			//for (auto& iter : _prototype)
			//	delete iter._second;
		};
		inline explicit component_manager(component_manager const& rhs) noexcept = delete;
		inline auto operator=(component_manager const& rhs) noexcept -> component_manager & = delete;
		inline explicit component_manager(component_manager&& rhs) noexcept = delete;
		inline auto operator=(component_manager&& rhs) noexcept -> component_manager & = delete;
	private:
		std::unordered_map<std::string, component* const> _prototype;
	};
}