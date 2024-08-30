#pragma once
#include "../library/design-pattern/singleton.h"
#include "../library/window/instance.h"
#include "../library/window/window.h"

#include "graphic.h"
#include "shader.h"
#include "editor.h"

#include "object_manager.h"
#include "component_manager.h"
#include "timer_manager.h"
#include "input_manager.h"
#include "sound_manager.h"

namespace engine {
	class engine final : public design_pattern::singleton<engine, design_pattern::member_static<engine>> {
		friend class design_pattern::singleton<engine, design_pattern::member_static<engine>>;
	public:
		inline static auto constructor(window::instance& instance, window::window& window) noexcept -> engine& {
			_instance = new engine(instance, window);
			atexit(destructor);
			return *_instance;
		}
	private:
		inline explicit engine(window::instance& instance, window::window& window) noexcept
			: _graphic(graphic::constructor(window)),
			_shader(shader::constructor(_graphic)),
			_editor(editor::constructor(window, _graphic)),
			_object_manager(object_manager::instance()),
			_component_manager(component_manager::instance()),
			_timer_manager(timer_manager::instance()),
			_input_manager(input_manager::constructor(instance, window)),
			_sound_manager(sound_manager::instance()) {
		};
		inline explicit engine(engine const& rhs) noexcept = delete;
		inline auto operator=(engine const& rhs) noexcept -> engine & = delete;
		inline explicit engine(engine&& rhs) noexcept = delete;
		inline auto operator=(engine&& rhs) noexcept -> engine & = delete;
		inline ~engine(void) noexcept {
			_editor.destructor();
			_shader.destructor();
			_graphic.destructor();
		};
	public:
		inline void initialize(void) noexcept {
		}
		inline void run(void) const noexcept {
			_timer_manager.set_frame(50);

			MSG msg;
			for (;;) {
				if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
					if (WM_QUIT == msg.message)
						break;
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else {
					//update
					_input_manager.update();
					_sound_manager.update();
					_editor.update();

					//render
					_graphic.begin_render();
					_editor.render();
					_graphic.end_render();

					_timer_manager.update();

				}
			}
		};
	private:
		graphic& _graphic;
		shader& _shader;
		editor& _editor;

		object_manager& _object_manager;
		component_manager& _component_manager;
		timer_manager& _timer_manager;
		input_manager& _input_manager;
		sound_manager& _sound_manager;
	};
}