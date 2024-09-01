#pragma once
#include "../library/design-pattern/singleton.h"

namespace client {
	class client final : public design_pattern::singleton<client> {
		friend class design_pattern::singleton<client>;
	private:
		inline explicit client(void) noexcept = default;
		inline ~client(void) noexcept = default;
		inline explicit client(client const& rhs) noexcept = delete;
		inline auto operator=(client const& rhs) noexcept -> client & = delete;
		inline explicit client(client&& rhs) noexcept = delete;
		inline auto operator=(client&& rhs) noexcept -> client & = delete;
	public:
		inline void initialize(void) noexcept {
		}
	};
}