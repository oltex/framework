#pragma once
#include "library/system-component/socket.h"
#include "library/system-component/thread.h"

#include "library/data-structure/serialize_buffer.h"
#include "library/data-structure/intrusive/shared_pointer.h"
#include "library/data-structure/thread-local/memory_pool.h"

class network_client {
protected:
	using size_type = unsigned int;
#pragma pack(push, 1)
	struct header final {
		inline explicit header(void) noexcept = default;
		inline explicit header(header const&) noexcept = delete;
		inline explicit header(header&&) noexcept = delete;
		inline auto operator=(header const&) noexcept -> header & = delete;
		inline auto operator=(header&&) noexcept -> header & = delete;
		inline ~header(void) noexcept = default;
		unsigned char _code;
		unsigned short _length;
		unsigned char _random_key;
		unsigned char _check_sum;
	};
#pragma pack(pop)
	class message final : public data_structure::intrusive::shared_pointer_hook<0>, public data_structure::serialize_buffer<> {
	public:
		inline explicit message(void) noexcept = delete;
		inline explicit message(message const&) noexcept = delete;
		inline explicit message(message&&) noexcept = delete;
		inline auto operator=(message const&) noexcept -> message & = delete;
		inline auto operator=(message&&) noexcept -> message & = delete;
		inline ~message(void) noexcept = delete;

		inline void destructor(void) noexcept {
			auto& memory_pool = data_structure::_thread_local::memory_pool<message>::instance();
			memory_pool.deallocate(*this);
		}
	};
	using message_pointer = data_structure::intrusive::shared_pointer<message, 0>;
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
public:
	inline explicit network_client(void) noexcept {
		auto& memory_pool = data_structure::_thread_local::memory_pool<message>::instance();
		message_pointer receive_message(&memory_pool.allocate());
		receive_message->clear();
		_receive_message = receive_message;
	};
	inline explicit network_client(network_client const&) noexcept = delete;
	inline explicit network_client(network_client&&) noexcept = delete;
	inline auto operator=(network_client const&) noexcept -> network_client & = delete;
	inline auto operator=(network_client&&) noexcept -> network_client & = delete;
	inline ~network_client(void) noexcept = default;
private:
	inline void receive(void) noexcept {
		for (;;) {
			int result = _socket.receive(reinterpret_cast<char*>(_receive_message->data() + _receive_message->rear()), message::capacity() - _receive_message->rear(), 0);
			_receive_message->move_rear(result);
			for (;;) {
				if (sizeof(header) > _receive_message->size())
					break;
				header header_;
				_receive_message->peek(reinterpret_cast<unsigned char*>(&header_), sizeof(header));
				if (header_._code != _header_code) {
					break;
				}
				if (header_._length > 256) { //юс╫ц
					break;
				}
				if (sizeof(header) + header_._length > _receive_message->size())
					break;
				_receive_message->pop(sizeof(header));

				auto& memory_pool = data_structure::_thread_local::memory_pool<view>::instance();
				view_pointer view_ptr(&memory_pool.allocate(_receive_message, _receive_message->front() - 1, _receive_message->front() + header_._length));
				_receive_message->pop(header_._length);

				//----------------------------------------------------------
				unsigned char p = 0;
				unsigned char e = 0;
				unsigned char temp = 0;
				size_type index = 0;
				unsigned char check_sum = 0;

				for (auto& iter : *view_ptr) {
					temp = iter ^ (e + _header_fixed_key + index + 1);
					e = iter;
					iter = temp ^ (p + header_._random_key + index + 1);
					p = temp;
					check_sum += iter;
					index++;
				}

				unsigned char check_sum_;
				(*view_ptr) >> check_sum_;
				check_sum -= check_sum_;
				if (check_sum != check_sum_) {
					break;
				}
				//----------------------------------------------------------

				if (false == on_receive(view_ptr)) {
					break;
				}
			}

			if (_receive_message->size() != _receive_message->capacity()) {
				auto& memory_pool = data_structure::_thread_local::memory_pool<message>::instance();
				message_pointer receive_message(&memory_pool.allocate());
				receive_message->clear();
				if (0 < _receive_message->size()) {
					memcpy(receive_message->data(), _receive_message->data() + _receive_message->front(), _receive_message->size());
					receive_message->move_rear(_receive_message->size());
				}
				_receive_message = receive_message;
			}
			else
				break;
		}
	}
public:
	inline void do_connect(char const* const address, unsigned short port) noexcept {
		_socket.create(AF_INET, SOCK_STREAM, 0);
		_socket.set_linger(1, 0);
		system_component::socket_address_ipv4 socket_address;
		socket_address.set_address("127.0.0.1");
		socket_address.set_port(port);
		_socket.connect(socket_address);
		_thread.begin(&network_client::receive, 0, this);
	}
	inline void do_disconnect(void) noexcept {
		_socket.close();
		_thread.wait_for_single(INFINITE);
		_thread.close();
	}
	inline virtual bool on_receive(view_pointer& view_ptr) noexcept = 0;
	inline virtual void on_disconnect(void) noexcept = 0;
	inline void do_send_session(message_pointer& message_ptr) noexcept {
		header* header_ = reinterpret_cast<header*>(message_ptr->data());
		if (0 == header_->_code) {
			header_->_code = _header_code;
			header_->_length = message_ptr->size() - 5;
			auto random_key = header_->_random_key = rand() % 256;
			auto header_fixed_key = _header_fixed_key;
			unsigned char check_sum = 0;

			auto end = message_ptr->end();
			for (auto iter = message_ptr->begin() + sizeof(header); iter != end; ++iter) {
				check_sum += *iter;
			}
			header_->_check_sum = check_sum;

			unsigned char p = 0;
			unsigned char e = 0;
			size_type index = 0;
			for (auto iter = message_ptr->begin() + sizeof(header) - 1; iter != end; ++iter) {
				p = *iter ^ (p + random_key + index + 1);
				e = p ^ (e + header_fixed_key + index + 1);
				*iter = e;
				index++;
			}
		}
		_socket.send(reinterpret_cast<char*>(message_ptr->data()), message_ptr->rear(), 0);
	}
	inline static auto create_message(void) noexcept -> message_pointer {
		auto& memory_pool = data_structure::_thread_local::memory_pool<message>::instance();
		message_pointer message_ptr(&memory_pool.allocate());
		message_ptr->clear();

		header header_{};
		message_ptr->push(reinterpret_cast<unsigned char*>(&header_), sizeof(header));
		return message_ptr;
	}
private:
	system_component::socket _socket;
	system_component::thread _thread;
	message_pointer _receive_message;

	unsigned char _header_code;
	unsigned char _header_fixed_key;
};