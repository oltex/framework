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
			_vertex.emplace("test", vertex(graphic, L"vertex.cso"));

			D3D11_INPUT_ELEMENT_DESC desc[]{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			unsigned int size = static_cast<unsigned int>(sizeof(desc) / sizeof(D3D11_INPUT_ELEMENT_DESC));
			_layout.emplace("Test", layout(graphic, desc, size, L"vertex.cso"));
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