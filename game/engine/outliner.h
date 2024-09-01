#pragma once
#include "../library/imgui/imgui.h"

namespace engine {
	class outliner final {
	public:
		inline explicit outliner(void)noexcept = default;
		inline explicit outliner(outliner const& rhs) noexcept = delete;
		inline auto operator=(outliner const& rhs) noexcept -> outliner & = delete;
		inline explicit outliner(outliner&& rhs) noexcept = delete;
		inline auto operator=(outliner&& rhs) noexcept -> outliner & = delete;
		inline ~outliner(void) noexcept = default;
	public:
		inline void update(void) noexcept {
			ImGui::Begin("outliner", &_open, ImGuiWindowFlags_None);
			ImGui::End();
		}
	private:
		bool _open = true;
	};
}