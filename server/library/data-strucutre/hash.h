#pragma once

//_INLINE_VAR constexpr size_t _FNV_offset_basis = 2166136261U;
//_INLINE_VAR constexpr size_t _FNV_prime = 16777619U;

namespace data_structure {
	template<class key_type>
	struct hash final {
		using size_type = unsigned int;
		inline auto operator()(key_type const& key) const noexcept -> size_type {
			size_type value = static_cast<size_type>(14695981039346656037ULL);
			unsigned char const* const byte = reinterpret_cast<unsigned char const*>(&key);
			for (size_type index = 0; index < sizeof(key_type); ++index) {
				value ^= static_cast<size_type>(byte[index]);
				value *= static_cast<size_type>(1099511628211ULL);
			}
			return value;
		}
	};
}