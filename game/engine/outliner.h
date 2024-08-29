#pragma once
#include "../library/imgui/imgui.h"
//#include "../library/imgui/imgui_impl_win32.h"
//#include "../library/imgui/imgui_impl_dx11.h"

namespace engine {
	class outliner final {
	public:
		inline void update(void) noexcept {
			ImGui::Begin("outliner", &_open, ImGuiWindowFlags_None);
			ImGui::End();
		}
	private:
		bool _open = true;
	};
}