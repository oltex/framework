#pragma once
#include "library/data-structure/intrusive/shared_pointer.h"
#include "library/data-structure/thread-local/memory_pool.h"
#include "message.h"

class view final : public data_structure::intrusive::shared_pointer_hook<0> {
private:
	using size_type = unsigned int;
public:
	inline explicit view(void) noexcept
		: _front(0), _rear(0), _fail(false) {
	}
	inline explicit view(message_pointer message_, size_type front, size_type rear) noexcept
		: _message(message_), _front(front), _rear(rear), _fail(false) {
	}
	inline view(view const& rhs) noexcept
		: _message(rhs._message), _front(rhs._front), _rear(rhs._rear), _fail(rhs._fail) {
	}
	inline auto operator=(view const& rhs) noexcept -> view&;
	inline auto operator=(view&& rhs) noexcept -> view&;
	inline ~view(void) noexcept = default;

	template<typename type>
		requires std::is_arithmetic_v<type>
	inline auto operator<<(type const& value) noexcept -> view& {
		reinterpret_cast<type&>(_message->data()[_rear]) = value;
		_rear += sizeof(type);
		return *this;
	}
	inline void push(byte* const buffer, size_type const length) noexcept {
		memcpy(_message->data() + _rear, buffer, length);
		_rear += length;
	}
	template<typename type>
		requires std::is_arithmetic_v<type>
	inline auto operator>>(type& value) noexcept -> view& {
		if (sizeof(type) + _front > _rear) {
			_fail = true;
			return *this;
		}
		value = reinterpret_cast<type&>(_message->data()[_front]);
		_front += sizeof(type);
		return *this;
	}
	inline void peek(byte* const buffer, size_type const length) noexcept {
		if (length + _front > _rear) {
			_fail = true;
			return;
		}
		memcpy(buffer, _message->data() + _front, length);
	}
	inline void pop(size_type const length) noexcept {
		_front += length;
	}
	inline auto front(void) const noexcept -> size_type {
		return _front;
	}
	inline auto rear(void) const noexcept -> size_type {
		return _rear;
	}
	inline void move_front(size_type const length) noexcept {
		_front += length;
	}
	inline void move_rear(size_type const length) noexcept {
		_rear += length;
	}
	inline auto size(void) const noexcept -> size_type {
		return _rear - _front;
	}
	inline auto begin(void) noexcept {
		return _message->data() + _front;
	}
	inline auto end(void) noexcept {
		return _message->data() + _rear;
	}
	inline auto data(void) noexcept -> message_pointer& {
		return _message;
	}
	inline void set(message* message_, size_type front, size_type rear) noexcept {
		_front = front;
		_rear = rear;
		_fail = false;
		_message.set(message_);
	}
	inline auto reset(void) noexcept {
		_message.reset();
	}
	inline operator bool(void) const noexcept {
		return !_fail;
	}
	inline auto fail(void) const noexcept {
		return _fail;
	}

	inline void destructor(void) noexcept {
		auto& memory_pool = data_structure::_thread_local::memory_pool<view>::instance();
		memory_pool.deallocate(*this);
	}
private:
	size_type _front, _rear;
	bool _fail = false;
	message_pointer _message;
};
using view_pointer = data_structure::intrusive::shared_pointer<view, 0>;