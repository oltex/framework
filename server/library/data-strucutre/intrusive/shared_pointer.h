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
		using node = shared_pointer_hook<index>;
		static_assert(std::is_base_of<node, type>::value);
	public:
		inline constexpr explicit shared_pointer(void) noexcept
			: _node(nullptr) {
		}
		inline constexpr shared_pointer(nullptr_t) noexcept
			: _node(nullptr) {
		};
		inline explicit shared_pointer(type* value) noexcept {
			_node = static_cast<node*>(value);
			_InterlockedExchange(&_node->_reference._use, 1);
			_node->_reference._weak = 0;
		}
		inline shared_pointer(shared_pointer const& rhs) noexcept
			: _node(rhs._node) {
			if (nullptr != _node)
				_InterlockedIncrement(&_node->_reference._use);
		};
		inline explicit shared_pointer(shared_pointer&& rhs) noexcept
			: _node(rhs._node) {
			rhs._node = nullptr;
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
			if (nullptr != _node && 0 == _InterlockedDecrement(&_node->_reference._use))
				destructor(static_cast<type*>(_node));
		}
	public:
		inline auto operator*(void) noexcept -> type& {
			return static_cast<type&>(*_node);
		}
		inline auto operator->(void) noexcept -> type* {
			return static_cast<type*>(_node);
		}
	public:
		inline void swap(shared_pointer& rhs) noexcept {
			auto temp = _node;
			_node = rhs._node;
			rhs._node = temp;
		}
		inline auto use_count(void) const noexcept -> size_type {
			return _node->_reference._use;
		}
		inline auto get(void) const noexcept -> type* {
			return static_cast<type*>(_node);
		}
		inline auto set(type* value) noexcept {
			_node = static_cast<node*>(value);
		}
		inline auto reset(void) noexcept {
			_node = nullptr;
		}
	private:
		node* _node;
	};

	template <class type, size_t index>
	inline bool operator==(const shared_pointer<type, index>& value, nullptr_t) noexcept {
		return value.get() == nullptr;
	}
}