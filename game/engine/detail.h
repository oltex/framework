#pragma once
#include "../library/imgui/imgui.h"

namespace engine {
	class detail final {
	public:
		inline explicit detail(void)noexcept = default;
		inline explicit detail(detail const& rhs) noexcept = delete;
		inline auto operator=(detail const& rhs) noexcept -> detail & = delete;
		inline explicit detail(detail&& rhs) noexcept = delete;
		inline auto operator=(detail&& rhs) noexcept -> detail & = delete;
		inline ~detail(void) noexcept = default;
	public:
		inline void update(void) noexcept {
			ImGui::Begin("detail", &_open, ImGuiWindowFlags_None);
			ImGui::End();
		}
	private:
		bool _open = true;
	};
}