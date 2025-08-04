#pragma once
#include <utility>
#include <type_traits>

namespace library {
	template <typename type_1, typename type_2>
	struct pair;
	template<typename... type>
	class tuple;
	template<>
	class tuple<> {
	};
	template<typename type, typename... rest>
	class tuple<type, rest...> : public tuple<rest...> {
		using size_type = unsigned int;
		friend class tuple;
		type _value;
	public:
		template <size_type index, typename tuple>
		struct element;
		template <size_type index, typename _this, typename... rest>
		struct element<index, tuple<_this, rest...>> : element<index - 1, tuple<rest...>> {
		};
		template <typename _this, typename... rest>
		struct element<0, tuple<_this, rest...>> {
			using type = _this;
			using tuple = tuple<_this, rest...>;
		};

		inline explicit constexpr tuple(void) noexcept = default;
		template<typename type_argument, typename... rest_argument>
		inline explicit constexpr tuple(type_argument&& type_arg, rest_argument&&... rest_arg) noexcept
			: tuple<rest...>(std::forward<rest_argument>(rest_arg)...), _value(std::forward<type_argument>(type_arg)) {
		}
		inline constexpr tuple(tuple&) noexcept = default;
		inline constexpr tuple(tuple&&) noexcept = default;
		inline constexpr auto operator=(tuple& rhs) noexcept -> tuple& {
			_value = rhs._value;
			static_cast<tuple<rest...>&>(*this) = static_cast<tuple<rest...>&>(rhs);
			//get_rest() = rhs.get_rest();
			return *this;
		}
		inline constexpr auto operator=(tuple&& rhs) noexcept -> tuple& {
			_value = std::move(rhs._value);
			static_cast<tuple<rest...>&>(*this) = std::move(static_cast<tuple<rest...>&>(rhs));
			//get_rest() = std::move(rhs.get_rest());
			return *this;
		}
		inline ~tuple(void) noexcept = default;

		template <class type_1, class type_2>
		inline auto operator=(pair<type_1, type_2>&& pair) noexcept -> tuple& {
			_value = std::forward<type_1>(pair._first);
			reinterpret_cast<tuple<rest...>&>(*this)._value = std::forward<type_2>(pair._second);
			return *this;
		}
		template <size_type index>
		inline auto get(void) noexcept -> element<index, tuple>::type& {
			return reinterpret_cast<element<index, tuple>::tuple&>(*this)._value;
		}
		template <size_type index>
		inline auto move(void) noexcept -> element<index, tuple>::type&& {
			return static_cast<element<index, tuple>::type&&>(reinterpret_cast<element<index, tuple>::tuple&>(*this)._value);
		}
		//inline auto get_rest(void) noexcept ->  tuple<rest...>& {
		//	return reinterpret_cast<tuple<rest...>&>(*this);
		//}
	};

	template <typename... type>
	inline constexpr auto tie(type&... arg) noexcept -> tuple<type&...> {
		return tuple<type&...>(arg...);
	}
	template <typename... type>
	inline constexpr auto forward_as_tuple(type&&... arg) noexcept -> tuple<type&&...> {
		return tuple<type&&...>(std::forward<type>(arg)...);
	}
}

template <class... _Types>
struct std::tuple_size<library::tuple<_Types...>> : integral_constant<size_t, sizeof...(_Types)> {
};
template <size_t _Index>
struct _MSVC_KNOWN_SEMANTICS std::tuple_element<_Index, library::tuple<>> {
	static_assert(_Always_false<integral_constant<size_t, _Index>>, "tuple index out of bounds");
};
template <class _This, class... _Rest>
struct _MSVC_KNOWN_SEMANTICS std::tuple_element<0, library::tuple<_This, _Rest...>> {
	using type = _This;
	using _Ttype = tuple<_This, _Rest...>;
};
template <size_t _Index, class _This, class... _Rest>
struct _MSVC_KNOWN_SEMANTICS std::tuple_element<_Index, library::tuple<_This, _Rest...>>
	: tuple_element<_Index - 1, tuple<_Rest...>> {
};