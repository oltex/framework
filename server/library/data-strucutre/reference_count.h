#pragma once

namespace data_structure {
	class reference_count {
	private:
		using size_type = unsigned int;
	public:
		inline explicit reference_count(void) noexcept = default;
		inline explicit reference_count(reference_count& rhs) noexcept = default;
		inline explicit reference_count(reference_count&& rhs) noexcept = default;
		inline auto operator=(reference_count const& rhs) noexcept -> reference_count & = default;
		inline auto operator=(reference_count&& rhs) noexcept -> reference_count & = default;
		inline ~reference_count(void) noexcept = default;
	public:
		inline auto add_reference(void) noexcept -> size_type {
			return _InterlockedIncrement(&_count);
		}
		inline auto release(void) noexcept -> size_type {
			return _InterlockedDecrement(&_count);
		}
	private:
		size_type _count = 0;
	};
}