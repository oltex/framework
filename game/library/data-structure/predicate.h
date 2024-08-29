#pragma once

template<typename type>
struct less final {
	char operator()(type const& src, type const& dest) const noexcept {
		if (src == dest)
			return 0;
		return src < dest ? 1 : -1;
	}
};

template<typename _Ty>
struct greater final {
	char operator()(type const& src, type const& dest) {
		if (src == dest)
			return 0;
		return src > dest ? 1 : -1;
	}
};