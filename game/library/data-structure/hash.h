#pragma once

template<class key_type>
struct hash final {
    inline size_t operator()(key_type const& key) const noexcept {
        size_t val = 14695981039346656037ULL;
        unsigned char const* const byte = &reinterpret_cast<unsigned char const&>(key);
        for (size_t idx = 0; idx < sizeof(key_type); ++idx) {
            val ^= static_cast<size_t>(byte[idx]);
            val *= 1099511628211ULL;
        }
        return val;
    }
};