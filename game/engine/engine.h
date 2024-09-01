#pragma once
#include "../library/design-pattern/singleton.h"
#include "../library/window/instance.h"
#include "../library/window/window.h"

#include "graphic.h"
#include "input.h"
#include "sound.h"
#include "timer.h"
#include "editor.h"

#include "pipeline_manager.h"
#include "object_manager.h"
#include "component_manager.h"

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
			_input(input::constructor(instance, window)),
			_sound(sound::instance()),
			_timer(timer::instance()),
			_editor(editor::constructor(window, _graphic)),
			_pipeline_manager(pipeline_manager::instance()),
			_object_manager(object_manager::instance()),
			_component_manager(component_manager::instance()) {
		};
		inline explicit engine(engine const& rhs) noexcept = delete;
		inline auto operator=(engine const& rhs) noexcept -> engine & = delete;
		inline explicit engine(engine&& rhs) noexcept = delete;
		inline auto operator=(engine&& rhs) noexcept -> engine & = delete;
		inline ~engine(void) noexcept {
			_editor.destructor();
			_graphic.destructor();
		};
	public:
		inline void initialize(void) noexcept {
		}
		inline void run(void) const noexcept {
			_timer.set_frame(50);

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
					_input.update();
					_sound.update();

					_component_manager.update();
					_editor.update();

					//render
					_pipeline_manager.render();
					_editor.render();

					_timer.update();
				}
			}
		};
	private:
		graphic& _graphic;
		input& _input;
		sound& _sound;
		timer& _timer;
		editor& _editor;

		pipeline_manager& _pipeline_manager;
		object_manager& _object_manager;
		component_manager& _component_manager;
	};
}