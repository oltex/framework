#pragma once

namespace detail {
	template <typename _type>
	struct remove_const {
		using type = _type;
	};
	template <typename _type>
	struct remove_const<_type const> {
		using type = _type;
	};

	template <typename _type>
	struct remove_volatile {
		using type = _type;
	};
	template <typename _type>
	struct remove_volatile<volatile _type> {
		using type = _type;
	};

	template <typename _type>
	struct remove_reference {
		using type = _type;
	};
	template <typename _type>
	struct remove_reference<_type&> {
		using type = _type;
	};
	template <typename _type>
	struct remove_reference<_type&&> {
		using type = _type;
	};

	template <typename _type>
	struct remove_pointer {
		using type = _type;
	};
	template <typename _type>
	struct remove_pointer<_type*> {
		using type = _type;
	};
	template <typename _type>
	struct remove_pointer<_type* const> {
		using type = _type;
	};
	template <typename _type>
	struct remove_pointer<_type* volatile> {
		using type = _type;
	};
	template <typename _type>
	struct remove_pointer<_type* const volatile> {
		using type = _type;
	};
}

namespace library {
	template <typename type>
	using remove_const = typename detail::remove_const<type>::type;
	template <typename type>
	using remove_volatile = typename detail::remove_volatile<type>::type;
	template <typename type>
	using remove_reference = typename detail::remove_reference<type>::type;
	template <typename type>
	using remove_pointer = typename detail::remove_pointer<type>::type;

	template <typename type>
	using remove_cp = typename detail::remove_const<typename detail::remove_pointer<type>::type>::type;

	template <typename, typename>
	inline constexpr bool same_type = false;
	template <typename type>
	inline constexpr bool same_type<type, type> = true;

	template <typename type, typename... rest>
	inline constexpr bool any_of_type = (same_type<type, rest> || ...);
}