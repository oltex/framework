#pragma once

namespace engine {
	class object final {
		friend class object_manager;
	public:
		inline explicit object(unsigned int const type) noexcept
			: _type(type) {
		};
		inline virtual auto clone(void) noexcept -> object* = 0;
		inline explicit object(object const& rhs) noexcept
			: _type(rhs._type) {
		}
		inline auto operator=(object const& rhs) noexcept -> object & = delete;
		inline explicit object(object&& rhs) noexcept = delete;
		inline auto operator=(object&& rhs) noexcept -> object & = delete;
		inline virtual ~object(void) noexcept = default;
	public:
		inline virtual void Initialize(void) noexcept {
		}
		inline virtual void update(void) noexcept {
		}
	public:
		inline void set_active(bool const enable) noexcept {
			_active = enable;
			_Initialize = enable;
		}
		inline void set_destory(void) noexcept {
		}
		inline auto get_type(void) const noexcept -> unsigned int {
			return _type;
		}
	private:
		unsigned int _type;
		bool _active;
		bool _Initialize;
		bool _destory;
	};
}