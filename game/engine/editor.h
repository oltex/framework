#pragma once
#include "../library/design-pattern/singleton.h"
#include "../library/window/window.h"
#include "../library/imgui/imgui.h"
#include "../library/imgui/imgui_impl_win32.h"
#include "../library/imgui/imgui_impl_dx11.h"
#include "graphic.h"
#include "outliner.h"

namespace engine {
	class editor final : public design_pattern::singleton<editor, design_pattern::member_static<editor>> {
		friend class design_pattern::singleton<editor, design_pattern::member_static<editor>>;
	public:
		inline static auto constructor(window::window& window, graphic& graphic) noexcept -> editor& {
			_instance = new editor(window, graphic);
			return *_instance;
		}
	private:
		inline explicit editor(window::window& window, graphic& graphic) noexcept {
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			//io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
			//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

			ImGui::StyleColorsDark();
			//ImGuiStyle& style = ImGui::GetStyle();
			//if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			//	style.WindowRounding = 0.0f;
			//	style.Colors[ImGuiCol_WindowBg].w = 1.0f;
			//}

			ImGui_ImplWin32_Init(window.data());
			ImGui_ImplDX11_Init(&graphic.get_device(), &graphic.get_context());
		};
		inline explicit editor(editor const& rhs) noexcept = delete;
		inline auto operator=(editor const& rhs) noexcept -> editor & = delete;
		inline explicit editor(editor&& rhs) noexcept = delete;
		inline auto operator=(editor&& rhs) noexcept -> editor & = delete;
		inline ~editor(void) noexcept {
			ImGui_ImplDX11_Shutdown();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
		};
	public:
		inline void update(void) noexcept {
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			_outliner.update();
		}
		inline void render(void) noexcept {
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}
	private:
		outliner _outliner;
	};
}