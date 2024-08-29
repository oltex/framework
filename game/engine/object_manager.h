#pragma once
#include "../library/design-pattern/singleton.h"
#include "object.h"
#include <string>
#include <list>
#include <unordered_map>

namespace engine {
	class object_manager final : public design_pattern::singleton<object_manager> {
		friend class design_pattern::singleton<object_manager>;
	private:
		inline explicit object_manager(void) noexcept = default;
		inline ~object_manager(void) noexcept {
		};
		inline explicit object_manager(object_manager const& rhs) noexcept = delete;
		inline auto operator=(object_manager const& rhs) noexcept -> object_manager & = delete;
		inline explicit object_manager(object_manager&& rhs) noexcept = delete;
		inline auto operator=(object_manager&& rhs) noexcept -> object_manager & = delete;
	public:
		inline void update(void) noexcept {
			//for (auto iter = _object.begin(); iter != _object.end();) {
			//	if (true == (*iter)->_destory) {

			//	}
			//	else {

			//	}
			//}
		}
		inline void fixed_update(void) noexcept {
		}
	public:
		inline void create(std::string const key, object* const prototype) noexcept {
			//_prototype.emplace(key, prototype);
		}
		inline auto clone(std::string const key, std::string const layer) noexcept -> object* {
			//object* prototype = (*_prototype.find(key)).second;
			//object* clone = prototype->clone();
			//_object[layer].emplace_back(clone);
			//return clone;
			return nullptr;
		}
	private:
		std::unordered_map<std::string, object* const> _prototype;
		std::unordered_map<std::string, std::list<object>> _clone;
	};
}