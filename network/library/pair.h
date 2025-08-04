#pragma once
#include <utility>

namespace library {
	struct piecewise_construct {
		inline explicit piecewise_construct(void) noexcept = default;
	};
	constexpr piecewise_construct _piecewise_construct{};

	template<typename... type>
	class tuple;
	template <typename type_1, typename type_2>
	struct pair final {
		type_1 _first;
		type_2 _second;

		inline explicit constexpr pair(void) noexcept = default;
		template <typename type_1, typename type_2>
		inline explicit constexpr pair(type_1 first, type_2 second) noexcept
			: _first(first), _second(second) {
		}
		template <class... type_1, class... type_2>
		inline explicit constexpr pair(piecewise_construct, tuple<type_1...> first, tuple<type_2...> second) noexcept
			: pair(first, second, std::index_sequence_for<type_1...>{}, std::index_sequence_for<type_2...>{}) {
		}
		template <class tuple_1, class tuple_2, size_t... index_1, size_t... index_2>
		inline explicit constexpr pair(tuple_1& first, tuple_2& second, std::index_sequence<index_1...>, std::index_sequence<index_2...>)
			: _first(first.template move<index_1>()...), _second(second.template move<index_2>()...) {
		}
	};
}