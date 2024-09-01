#pragma once
#include "../library/design-pattern/singleton.h"

namespace engine {
	class pipeline_manager final : public design_pattern::singleton<pipeline_manager> {
		friend class design_pattern::singleton<pipeline_manager>;
	private:
	protected:
		inline explicit pipeline_manager(void) noexcept = default;
		inline ~pipeline_manager(void) noexcept = default;
		inline explicit pipeline_manager(pipeline_manager const& rhs) noexcept = delete;
		inline auto operator=(pipeline_manager const& rhs) noexcept -> pipeline_manager & = delete;
		inline explicit pipeline_manager(pipeline_manager&& rhs) noexcept = delete;
		inline auto operator=(pipeline_manager&& rhs) noexcept -> pipeline_manager & = delete;
	public:
		inline void render(void) noexcept {

		}
	};
}