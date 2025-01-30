#pragma once
#include <compare>

namespace predicate {
	template<typename type>
	struct less final {
		inline auto operator()(type const& source, type const& destination) const noexcept {
			return source <=> destination;
		}
	};

	template<typename type>
	struct greater final {
		inline auto operator()(type const& source, type const& destination) const noexcept {
			return destination <=> source;
		}
	};
}