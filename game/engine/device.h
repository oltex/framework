#pragma once
#include <GameInput.h>

namespace engine {
	class device {
	public:
		inline explicit device(IGameInputDevice* device_) noexcept
			: _device(device_){
			_device->AddRef();
		};
		inline explicit device(device const& rhs) = delete;
		inline auto operator=(device const& rhs) noexcept -> device & = delete;
		inline explicit device(device&& rhs) noexcept = delete;
		inline auto operator=(device&& rhs) noexcept -> device & = delete;
		inline ~device(void) noexcept {
			_device->Release();
		};
	protected:
		IGameInputDevice* _device;
	};
}