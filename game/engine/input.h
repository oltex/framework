#pragma once
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#include "../library/design-pattern/singleton.h"
#include "../library/window/instance.h"
#include "../library/window/window.h"
#include "keyboard.h"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <GameInput.h>

namespace engine {
	class input final : public design_pattern::singleton<input, design_pattern::member_static<input>> {
		friend class design_pattern::singleton<input, design_pattern::member_static<input>>;
	public:
		enum class move : unsigned char {
			x, y, z
		};
		enum class button : unsigned char {
			left, right, wheel
		};
	public:
		inline static auto constructor(window::instance& instance, window::window& window) noexcept -> input& {
			_instance = new input(instance, window);
			atexit(destructor);
			return *_instance;
		}
	private:
		inline explicit input(window::instance& instance, window::window& window) noexcept {
			//GameInputCreate(&_input);
			//GameInputCallbackToken _token;
			//_input->RegisterDeviceCallback(nullptr, GameInputKindKeyboard, GameInputDeviceAnyStatus, GameInputAsyncEnumeration, (void*)this, keyboard_callback, nullptr);


			//auto hinstance = instance.data();
			//auto hwnd = window.data();
			//DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>(&_input), nullptr);

			//_input->CreateDevice(GUID_SysMouse, &_mouse, nullptr);
			//_mouse->SetDataFormat(&c_dfDIMouse);
			//_mouse->SetCooperativeLevel(hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
			//_mouse->Acquire();

			//_input->CreateDevice(GUID_SysKeyboard, &_keyboard, nullptr);
			//_keyboard->SetDataFormat(&c_dfDIKeyboard);
			//_keyboard->SetCooperativeLevel(hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
			//_keyboard->Acquire();

			//memset(_mouse_up, 1, 3);
			//memset(_keyboard_up, 1, 256);
		};
		inline explicit input(input const& rhs) noexcept = delete;
		inline auto operator=(input const& rhs) noexcept -> input & = delete;
		inline explicit input(input&& rhs) noexcept = delete;
		inline auto operator=(input&& rhs) noexcept -> input & = delete;
		inline ~input(void) noexcept {
			//_input->Release();
			//_keyboard->Unacquire();
			//_keyboard->Release();
			//_mouse->Unacquire();
			//_mouse->Release();
			//_input->Release();
		};
	public:
		inline void update(void) noexcept {
			//for (auto& iter : _keyboard)
			//	iter.update(*_input);

			//_reading->Release();
			//_input->GetCurrentReading(GameInputKindKeyboard, _keyboard, &_keyboard_reading);
			//_keyboard_reading->GetDevice(&_keyboard);
			//HRESULT hr = _input->GetPreviousReading(_keyboard_reading, GameInputKindKeyboard, _keyboard, &_reading2);
			//_mouse->GetDeviceState(sizeof(_DIMOUSESTATE), &_mouse_state);
			//_keyboard->GetDeviceState(256, _keyboard_state);
			//memset(_mouse_up_frame, 0, 3);
			//memset(_mouse_down_frame, 0, 3);
			//memset(_keyboard_up_frame, 0, 256);
			//memset(_keyboard_down_frame, 0, 256);
			//_keyboard_reading->Release();
		}
	public:
		inline static void __stdcall keyboard_callback(
			_In_ GameInputCallbackToken callback_token, _In_ void* context,
			_In_ IGameInputDevice* device, _In_ uint64_t timestamp,
			_In_ GameInputDeviceStatus current_status, _In_ GameInputDeviceStatus previous_status) noexcept {
			auto input_mgr = reinterpret_cast<input*>(context);
			input_mgr->_keyboard.emplace_back(device);
		}
		//inline long mouse_move(move const move) const noexcept {
		//	return *(&_mouse_state.lX + static_cast<unsigned char>(move));
		//}
		//inline bool mouse_press(button const btn) const noexcept {
		//	return static_cast<bool>(_mouse_state.rgbButtons[static_cast<unsigned char>(btn)]);
		//}
		//inline bool mouse_up(button const btn) noexcept {
		//	switch (static_cast<bool>(_mouse_state.rgbButtons[static_cast<unsigned char>(btn)])) {
		//	case false:
		//		if (false == _mouse_up[static_cast<unsigned char>(btn)])
		//			_mouse_up[static_cast<unsigned char>(btn)] = _mouse_up_frame[static_cast<unsigned char>(btn)] = true;
		//		break;
		//	case true:
		//		if (true == _mouse_up[static_cast<unsigned char>(btn)])
		//			_mouse_up[static_cast<unsigned char>(btn)] = false;
		//		break;
		//	}
		//	return _mouse_up_frame[static_cast<unsigned char>(btn)];
		//}
		//inline bool mouse_down(button const btn) noexcept {
		//	switch (static_cast<bool>(_mouse_state.rgbButtons[static_cast<unsigned char>(btn)])) {
		//	case true:
		//		if (false == _mouse_down[static_cast<unsigned char>(btn)])
		//			_mouse_down[static_cast<unsigned char>(btn)] = _mouse_down_frame[static_cast<unsigned char>(btn)] = true;
		//		break;
		//	case false:
		//		if (true == _mouse_down[static_cast<unsigned char>(btn)])
		//			_mouse_down[static_cast<unsigned char>(btn)] = false;
		//		break;
		//	}
		//	return _mouse_down_frame[static_cast<unsigned char>(btn)];
		//}
		inline bool key_press(unsigned char const key) noexcept {
			for (auto& iter : _keyboard)
				if (true == iter.key_press(key))
					return true;
			return false;
		}
		//inline bool key_up(unsigned char const key) noexcept {
		//	switch (static_cast<bool>(_keyboard_state[key])) {
		//	case false:
		//		if (false == _keyboard_up[key])
		//			_keyboard_up[key] = _keyboard_up_frame[key] = true;
		//		break;
		//	case true:
		//		if (true == _keyboard_up[key])
		//			_keyboard_up[key] = false;
		//		break;
		//	}
		//	return _keyboard_up_frame[key];
		//}
		//inline bool key_down(unsigned char const key) noexcept {
		//	for (auto& iter : _keyboard)
		//		if (true == iter.key_down(key))
		//			return true;
		//	return false;
		//	//switch (static_cast<bool>(_keyboard_state[key])) {
		//	//case true:
		//	//	if (false == _keyboard_down[key])
		//	//		_keyboard_down[key] = _keyboard_down_frame[key] = true;
		//	//	break;
		//	//case false:
		//	//	if (true == _keyboard_down[key])
		//	//		_keyboard_down[key] = false;
		//	//	break;
		//	//}
		//	//return _keyboard_down_frame[key];
		//}
	private:
		//IDirectInput8W* _input;
		//IDirectInputDevice8W* _mouse;
		//IDirectInputDevice8W* _keyboard;

		//_DIMOUSESTATE _mouse_state;
		//unsigned char _keyboard_state[256];

		//bool _mouse_up[3];
		//bool _mouse_up_frame[3];
		//bool _mouse_down[3]{};
		//bool _mouse_down_frame[3];
		//bool _keyboard_up[256];
		//bool _keyboard_up_frame[256];
		//bool _keyboard_down[256]{};
		//bool _keyboard_down_frame[256];

		IGameInput* _input;
		std::list<keyboard> _keyboard;
	};
}

