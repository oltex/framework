#pragma once
#include "../../template/template.h"
#include <Windows.h>

namespace library {
	//inline auto interlock_bit(void) noexcept {
	//}

	template<typename type>
		requires library::any_of_type<type, unsigned char, char, unsigned short, short, unsigned int, int, unsigned long, long, unsigned long long, long long>
	inline auto interlock_and(type& destine, library::type_identity<type> value) noexcept {
		if constexpr (1 == sizeof(type))
			return ::_InterlockedAnd8(reinterpret_cast<volatile char*>(&destine), static_cast<char>(value));
		else if constexpr (2 == sizeof(type))
			return ::_InterlockedAnd16(reinterpret_cast<volatile short*>(&destine), static_cast<short>(value));
		else if constexpr (4 == sizeof(type))
			return ::_InterlockedAnd(reinterpret_cast<volatile long*>(&destine), static_cast<long>(value));
		else
			return ::_InterlockedAnd64(reinterpret_cast<volatile long long*>(&destine), static_cast<long long>(value));
	}
	template<typename type>
		requires library::any_of_type<type, unsigned char, char, unsigned short, short, unsigned int, int, unsigned long, long, unsigned long long, long long>
	inline auto interlock_or(type& destine, library::type_identity<type> value) noexcept {
		if constexpr (1 == sizeof(type))
			return ::_InterlockedOr8(reinterpret_cast<volatile char*>(&destine), static_cast<char>(value));
		else if constexpr (2 == sizeof(type))
			return ::_InterlockedOr16(reinterpret_cast<volatile short*>(&destine), static_cast<short>(value));
		else if constexpr (4 == sizeof(type))
			return ::_InterlockedOr(reinterpret_cast<volatile long*>(&destine), static_cast<long>(value));
		else
			return ::_InterlockedOr64(reinterpret_cast<volatile long long*>(&destine), static_cast<long long>(value));
	}
	template<typename type>
		requires library::any_of_type<type, unsigned char, char, unsigned short, short, unsigned int, int, unsigned long, long, unsigned long long, long long>
	inline auto interlock_xor(type& destine, library::type_identity<type> value) noexcept {
		if constexpr (1 == sizeof(type))
			return ::_InterlockedXor8(reinterpret_cast<volatile char*>(&destine), static_cast<char>(value));
		else if constexpr (2 == sizeof(type))
			return ::_InterlockedXor16(reinterpret_cast<volatile short*>(&destine), static_cast<short>(value));
		else if constexpr (4 == sizeof(type))
			return ::_InterlockedXor(reinterpret_cast<volatile long*>(&destine), static_cast<long>(value));
		else
			return ::_InterlockedXor64(reinterpret_cast<volatile long long*>(&destine), static_cast<long long>(value));
	}

	template<typename type>
		requires library::any_of_type<type, unsigned short, short, unsigned int, int, unsigned long, long, unsigned long long, long long>
	inline auto interlock_increment(type& addend) noexcept -> type {
		if constexpr (2 == sizeof(type))
			return ::_InterlockedIncrement16(reinterpret_cast<volatile short*>(&addend));
		else if constexpr (4 == sizeof(type))
			return ::_InterlockedIncrement(reinterpret_cast<volatile long*>(&addend));
		else
			return ::_InterlockedIncrement64(reinterpret_cast<volatile long long*>(&addend));
	}
	template<typename type>
		requires library::any_of_type<type, unsigned short, short, unsigned int, int, unsigned long, long, unsigned long long, long long>
	inline auto interlock_decrement(type& addend) noexcept -> type {
		if constexpr (2 == sizeof(type))
			return ::_InterlockedDecrement16(reinterpret_cast<volatile short*>(&addend));
		else if constexpr (4 == sizeof(type))
			return ::_InterlockedDecrement(reinterpret_cast<volatile long*>(&addend));
		else
			return ::_InterlockedDecrement64(reinterpret_cast<volatile long long*>(&addend));
	}
	template<typename type>
		requires (library::any_of_type<type, unsigned char, char, unsigned short, short, unsigned int, int, unsigned long, long, unsigned long long, long long> || library::pointer_type<type>)
	inline auto interlock_exchange(type& target, library::type_identity<type> value) noexcept -> type {
		if constexpr (library::pointer_type<type>)
			return ::_InterlockedExchangePointer(reinterpret_cast<void* volatile*>(&target), static_cast<void*>(value));
		else if constexpr (1 == sizeof(type))
			return ::_InterlockedExchange8(reinterpret_cast<volatile char*>(&target), static_cast<char>(value));
		else if constexpr (2 == sizeof(type))
			return ::_InterlockedExchange16(reinterpret_cast<volatile short*>(&target), static_cast<short>(value));
		else if constexpr (4 == sizeof(type))
			return ::_InterlockedExchange(reinterpret_cast<volatile long*>(&target), static_cast<long>(value));
		else
			return ::_InterlockedExchange64(reinterpret_cast<volatile long long*>(&target), static_cast<long long>(value));
	}
	template<typename type>
		requires library::any_of_type<type, unsigned int, int, unsigned long, long, unsigned long long, long long>
	inline auto interlock_exchange_add(type& addend, library::type_identity<type> value) noexcept {
		if constexpr (4 == sizeof(type))
			return ::_InterlockedExchangeAdd(reinterpret_cast<long volatile*>(&addend), static_cast<long>(value));
		else
			return ::_InterlockedExchangeAdd64(reinterpret_cast<long long volatile*>(&addend), static_cast<long long>(value));
	}
	template<typename type>
		requires (library::any_of_type<type, unsigned short, short, unsigned int, int, unsigned long, long, unsigned long long, long long> || library::pointer_type<type>)
	inline auto interlock_compare_exhange(type& destine, library::type_identity<type> exchange, library::type_identity<type> compare) noexcept -> type {
		if constexpr (library::pointer_type<type>)
			return ::_InterlockedCompareExchangePointer(reinterpret_cast<void* volatile*>(&destine), static_cast<void*>(exchange), static_cast<void*>(compare));
		else if constexpr (2 == sizeof(type))
			return ::_InterlockedCompareExchange16(reinterpret_cast<short volatile*>(&destine), static_cast<short>(exchange), static_cast<short>(compare));
		else if constexpr (4 == sizeof(type))
			return ::_InterlockedCompareExchange(reinterpret_cast<long volatile*>(&destine), static_cast<long>(exchange), static_cast<long>(compare));
		else
			return ::_InterlockedCompareExchange64(reinterpret_cast<long long volatile*>(&destine), static_cast<long long>(exchange), static_cast<long long>(compare));
	}
}