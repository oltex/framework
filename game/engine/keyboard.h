#pragma once
#include "device.h"
#include <GameInput.h>
#include <vector>
#include <iostream>

namespace engine {
	class keyboard final : public device {
	public:
		inline explicit keyboard(IGameInputDevice* device_) noexcept
			: device(device_) {
			auto info = device_->GetDeviceInfo();
			_state.resize(info->keyboardInfo->maxSimultaneousKeys);
			_previous_state.resize(info->keyboardInfo->maxSimultaneousKeys);
		};
	public:
		inline void update(IGameInput& input) noexcept {
			IGameInputReading* reading;
			IGameInputReading* previous_reading;
			input.GetCurrentReading(GameInputKindKeyboard, _device, &reading);
			_state.resize(reading->GetKeyCount());
			reading->GetKeyState(reading->GetKeyCount(), _state.data());
			input.GetPreviousReading(reading, GameInputKindKeyboard, _device, &previous_reading);
			reading->Release();
		}
	public:
		inline bool key_press(unsigned char const key) noexcept {
			for (auto& iter : _state)
				//iter.codePoint
				//iter.isDeadKey
				//iter.scanCode
				//iter.virtualKey
				if (iter.virtualKey == key) {
					int a = 10;
					return true;
				}
			return false;
		}
	private:
		std::vector<GameInputKeyState> _state;
		std::vector<GameInputKeyState> _previous_state;
	};
}

//switch (static_cast<bool>(_keyboard_state[key])) {
//case true:
//	if (false == _keyboard_down[key])
//		_keyboard_down[key] = _keyboard_down_frame[key] = true;
//	break;
//case false:
//	if (true == _keyboard_down[key])
//		_keyboard_down[key] = false;
//	break;
//}
//return _keyboard_down_frame[key];