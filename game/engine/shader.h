#pragma once
#include "../library/design-pattern/singleton.h"
#include "graphic.h"
#include "vertex.h"
#include "pixel.h"
#include "layout.h"
#include <unordered_map>
#include <string>

namespace engine {
	class shader final : public design_pattern::singleton<shader, design_pattern::member_static<shader>> {
		friend class design_pattern::singleton<shader, design_pattern::member_static<shader>>;
	public:
		inline static auto constructor(graphic& graphic) noexcept -> shader& {
			_instance = new shader(graphic);
			return *_instance;
		}
	private:
		inline explicit shader(graphic& graphic) noexcept {
			_vertex.emplace("test", vertex(graphic, L"vertex.cso", "main", 0, 0));
		}
		inline explicit shader(shader const& rhs) noexcept = delete;
		inline auto operator=(shader const& rhs) noexcept -> shader & = delete;
		inline explicit shader(shader&& rhs) noexcept = delete;
		inline auto operator=(shader&& rhs) noexcept -> shader & = delete;
	private:
		std::unordered_map<std::string, vertex> _vertex;
		std::unordered_map<std::string, pixel> _pixel;
		std::unordered_map<std::string, layout> _layout;
		//std::unordered_map<std::string,
	};
}