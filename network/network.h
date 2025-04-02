#pragma once

#include "library/data-structure/intrusive/shared_pointer.h"
#include "library/data-structure/lockfree/queue.h"
#include "library/data-structure/thread-local/memory_pool.h"
#include "library/data-structure/serialize_buffer.h"

#include "library/system-component/socket.h"
#include "library/system-component/interlocked.h"

class network {
private:
	enum class post_queue_state : unsigned char {
		accept, close_worker, destory_session, excute_task, destory_group
	};
	class session final {
	public:
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
	private:
		class queue final : protected data_structure::lockfree::queue<view*> {
		private:
			using base = data_structure::lockfree::queue<view*>;
			class iterator final {
			public:
				inline explicit iterator(void) noexcept = default;
				inline explicit iterator(node* node_) noexcept
					: _node(node_) {
				}
				inline explicit iterator(iterator const&) noexcept = delete;
				inline explicit iterator(iterator&&) noexcept = delete;
				inline auto operator=(iterator const&) noexcept -> iterator & = delete;
				inline auto operator=(iterator&&) noexcept -> iterator & = delete;
				inline ~iterator(void) noexcept = default;

				inline auto operator*(void) const noexcept -> view*& {
					return _node->_value;
				}
				inline auto operator++(void) noexcept -> iterator& {
					_node = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & _node->_next);
					return *this;
				}
				inline bool operator==(iterator const& rhs) const noexcept {
					return _node == rhs._node;
				}
			private:
				node* _node;
			};
		public:
			inline explicit queue(void) noexcept = default;
			inline explicit queue(queue const&) noexcept = delete;
			inline explicit queue(queue&&) noexcept = delete;
			inline auto operator=(queue const&) noexcept -> queue & = delete;
			inline auto operator=(queue&&) noexcept -> queue & = delete;
			inline ~queue(void) noexcept = default;

			inline void push(view_pointer view_ptr) noexcept {
				_InterlockedIncrement(&_size);
				base::emplace(view_ptr.get());
				view_ptr.reset();
			}
			inline auto pop(void) noexcept -> view_pointer {
				unsigned long long head = _head;
				node* address = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & head);
				unsigned long long next = address->_next;

				if (0x10000 > (0x00007FFFFFFFFFFFULL & next))
					__debugbreak();
				unsigned long long tail = _tail;
				if (tail == head)
					_InterlockedCompareExchange(reinterpret_cast<unsigned long long volatile*>(&_tail), next, tail);

				view_pointer result;
				result.set(reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & next)->_value);
				_head = next;
				_memory_pool::instance().deallocate(*address);

				_InterlockedDecrement(&_size);
				return result;
			}
			inline auto begin(void) noexcept -> iterator {
				return iterator(reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & _head)->_next));
			}
			inline auto end(void) noexcept -> iterator {
				return iterator(reinterpret_cast<node*>(_nullptr));
			}
			inline auto empty(void) const noexcept {
				unsigned long long head = _head;
				node* address = reinterpret_cast<node*>(0x00007FFFFFFFFFFFULL & head);
				unsigned long long next = address->_next;
				if (_nullptr == (0x00007FFFFFFFFFFFULL & next))
					return true;
				return false;
			}
			inline auto size(void) const noexcept {
				return _size;
			}
		private:
			size_type _size = 0;
		};
		inline static unsigned long long _static_id = 0x10000;
	public:
		inline explicit session(unsigned long const index) noexcept
			: _key(index), _io_count(0x80000000),
			_receive_message(&data_structure::_thread_local::memory_pool<message>::instance().allocate()) {
			_receive_message->clear();
		};
		inline explicit session(session const&) noexcept = delete;
		inline explicit session(session&&) noexcept = delete;
		inline auto operator=(session const&) noexcept -> session & = delete;
		inline auto operator=(session&&) noexcept -> session & = delete;
		inline ~session(void) noexcept = default;

		inline void initialize(system_component::socket&& socket, unsigned long long timeout_duration) noexcept {
			_key = 0xffff & _key | _static_id;
			_static_id += 0x10000;
			_socket = std::move(socket);
			//_timeout_currnet = GetTickCount64();
			//_timeout_duration = timeout_duration;
			//_send_flag = 0;
			//_cancel_flag = 0;
			_InterlockedIncrement(&_io_count);
			//_InterlockedAnd((long*)&_receive_count, 0x3FFFFFFF);
			_InterlockedAnd((long*)&_io_count, 0x7FFFFFFF);
		}
		inline bool acquire(void) noexcept {
			if (0x80000000 & _InterlockedIncrement(&_io_count))
				return false;
			return true;
		}
		inline bool acquire(unsigned long long key) noexcept {
			if ((0x80000000 & _InterlockedIncrement(&_io_count)) || _key != key)
				return false;
			return true;
		}
		inline bool release(void) noexcept {
			if (0 == _InterlockedDecrement(&_io_count) && 0 == _InterlockedCompareExchange(&_io_count, 0x80000000, 0)) {
				_receive_message->clear();
				while (!_send_queue.empty())
					_send_queue.pop();
				while (!_receive_queue.empty())
					_receive_queue.pop();
				_socket.close();
				return true;
			}
			return false;
		}
		inline bool receive(void) noexcept {
			WSABUF wsa_buffer{ message::capacity() - _receive_message->rear(),  reinterpret_cast<char*>(_receive_message->data() + _receive_message->rear()) };
			unsigned long flag = 0;
			_recv_overlapped.clear();
			unsigned long long key = _key;
			if (SOCKET_ERROR == _socket.wsa_receive(&wsa_buffer, 1, &flag, _recv_overlapped)) {
				if (WSA_IO_PENDING == GetLastError()) {
					if (true == _cancel_flag) {
						if (acquire(key))
							_socket.cancel_io_ex();
						return false;
					}
					return true;
				}
				return false;
			}
			return true;
		}
		inline bool send(void) noexcept {
			while (!_send_queue.empty() && 0 == _InterlockedExchange(&_send_flag, 1)) {
				if (_send_queue.empty())
					_InterlockedExchange(&_send_flag, 0);
				else {
					WSABUF wsa_buffer[512];
					_send_size = 0;
					for (auto iter = _send_queue.begin(), end = _send_queue.end(); iter != end || 512 <= _send_size; ++iter, ++_send_size) {
						wsa_buffer[_send_size].buf = reinterpret_cast<char*>((*iter)->data()->data() + (*iter)->front());
						wsa_buffer[_send_size].len = (*iter)->size();
					}
					_send_overlapped.clear();
					unsigned long long key = _key;
					if (SOCKET_ERROR == _socket.wsa_send(wsa_buffer, _send_size, 0, _send_overlapped)) {
						if (GetLastError() == WSA_IO_PENDING) {
							if (true == _cancel_flag) {
								if (acquire(key))
									_socket.cancel_io_ex();
								return false;
							}
							return true;
						}
						return false;
					}
					return true;
				}
			}
			return false;
		}
		inline void cancel(void) noexcept {
			_cancel_flag = 1;
			_socket.cancel_io_ex();
		}

		
		unsigned long long _key;
		unsigned long _io_count;
		bool _cancel_flag;
		unsigned long _send_flag;
		unsigned long _send_size;

		system_component::socket _socket;
		message_pointer _receive_message;
		queue _receive_queue;
		queue _send_queue;
		system_component::overlapped _recv_overlapped;
		system_component::overlapped _send_overlapped;

	}
};