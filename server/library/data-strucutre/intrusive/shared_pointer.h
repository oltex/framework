#pragma once
#include <malloc.h>
#include <utility>
#include <type_traits>

namespace data_structure::intrusive {
	template<size_t index>
	class shared_pointer_hook {
	private:
		using size_type = unsigned int;
		template<typename type, size_t>
		friend class shared_pointer;
		struct reference final {
		public:
			size_type _use;
			size_type _weak;
		};
	public:
		//inline explicit shared_pointer_hook(void) noexcept = delete;
		//inline explicit shared_pointer_hook(shared_pointer_hook const& rhs) noexcept = delete;
		//inline explicit shared_pointer_hook(shared_pointer_hook&& rhs) noexcept = delete;
		//inline auto operator=(shared_pointer_hook const& rhs) noexcept -> shared_pointer_hook & = delete;
		//inline auto operator=(shared_pointer_hook&& rhs) noexcept -> shared_pointer_hook & = delete;
		//inline ~shared_pointer_hook(void) noexcept = delete;
	public:
		inline auto add_reference(void) noexcept -> size_type {
			return _InterlockedIncrement(&_reference._use);
		}
		inline auto release(void) noexcept -> size_type {
			return _InterlockedDecrement(&_reference._use);
		}
	private:
		reference _reference;
	};

	template<typename type, size_t index>
	class shared_pointer final {
	private:
		using size_type = unsigned int;
		using hook = shared_pointer_hook<index>;
		static_assert(std::is_base_of<hook, type>::value);
	public:
		inline constexpr explicit shared_pointer(void) noexcept
			: _pointer(nullptr) {
		}
		inline constexpr shared_pointer(nullptr_t) noexcept
			: _pointer(nullptr) {
		};
		inline explicit shared_pointer(type* value) noexcept {
			_pointer = static_cast<hook*>(value);
			_pointer->_reference._use = 1;
			_pointer->_reference._weak = 0;
		}
		inline shared_pointer(shared_pointer const& rhs) noexcept
			: _pointer(rhs._pointer) {
			if (nullptr != _pointer)
				++_pointer->_reference._use;
		};
		inline explicit shared_pointer(shared_pointer&& rhs) noexcept
			: _pointer(rhs._pointer) {
			rhs._pointer = nullptr;
		};
		inline auto operator=(shared_pointer const& rhs) noexcept -> shared_pointer& {
			shared_pointer(rhs).swap(*this);
			return *this;
		}
		inline auto operator=(shared_pointer&& rhs) noexcept -> shared_pointer& {
			shared_pointer(std::move(rhs)).swap(*this);
			return *this;
		};
		inline ~shared_pointer(void) noexcept {
			if (nullptr != _pointer && 0 == --_pointer->_reference._use)
				destructor(static_cast<type*>(_pointer));
		}
	public:
		inline auto operator*(void) noexcept -> type& {
			return static_cast<type&>(*_pointer);
		}
		inline auto operator->(void) noexcept -> type* {
			return static_cast<type*>(_pointer);
		}
	public:
		inline void swap(shared_pointer& rhs) noexcept {
			auto temp = _pointer;
			_pointer = rhs._pointer;
			rhs._pointer = temp;
		}
		inline auto use_count(void) const noexcept -> size_type {
			return _pointer->_reference._use;
		}
		inline auto get(void) const noexcept -> type* {
			return static_cast<type*>(_pointer);
		}
		inline void set(type* value) noexcept {
			_pointer = static_cast<hook*>(value);
		}
		inline void reset(void) noexcept {
			_pointer = nullptr;
		}
		template <class type, size_t index>
		friend inline bool operator==(shared_pointer<type, index> const& rhs, nullptr_t) noexcept {
			return rhs._pointer == nullptr;
		}
	private:
		hook* _pointer;
	};
}