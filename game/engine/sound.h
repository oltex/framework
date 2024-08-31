#pragma once
#ifdef _DEBUG
#pragma comment(lib, "library/fmod/fmodL_vc.lib") 
#else
#pragma comment(lib, "library/fmod/fmod_vc.lib")
#endif
#include "../library/design-pattern/singleton.h"
#include "../library/fmod/fmod.hpp"

namespace engine {
	class sound final : public design_pattern::singleton<sound> {
		friend class design_pattern::singleton<sound>;
	private:
		inline explicit sound(void) noexcept {
			FMOD::System_Create(&_system);
			_system->init(FMOD_MAX_CHANNEL_WIDTH, FMOD_INIT_NORMAL, nullptr);
			_system->set3DSettings(1.f, 0.1f, 1.f);
		};
		inline explicit sound(sound const& rhs) noexcept = delete;
		inline auto operator=(sound const& rhs) noexcept -> sound & = delete;
		inline explicit sound(sound&& rhs) noexcept = delete;
		inline auto operator=(sound&& rhs) noexcept -> sound & = delete;
		inline ~sound(void) {
			_system->close();
			_system->release();
		};
	public:
		inline void update(void) noexcept {
			_system->update();
		}
	private:
		FMOD::System* _system;
	};
}