#pragma once
#include "../kernel/object.h"
#include <process.h>
#include <Windows.h>
#include <tuple>
#include <type_traits>
#include <memory>

namespace system_component::multi {
	class thread final : public kernel::object {
	private:
		template <typename tuple, size_t... index>
		inline static unsigned int __stdcall invoke(void* arg) noexcept {
			const std::unique_ptr<tuple> value(reinterpret_cast<tuple*>(arg));
			tuple& tuple = *value.get();
			std::invoke(std::move(std::get<index>(tuple))...);
			return 0;
		}
		template <typename tuple, size_t... index>
		inline static constexpr auto make(std::index_sequence<index...>) noexcept {
			return &invoke<tuple, index...>;
		}
	public:
		inline explicit thread(void) noexcept = default;
		template <typename function, typename... argument>
		inline explicit thread(function&& func, unsigned int flag, argument&&... arg) noexcept {
			using tuple = std::tuple<std::decay_t<function>, std::decay_t<argument>...>;
			auto copy = std::make_unique<tuple>(std::forward<function>(func), std::forward<argument>(arg)...);
			constexpr auto proc = make<tuple>(std::make_index_sequence<1 + sizeof...(argument)>());
			_handle = (HANDLE)_beginthreadex(nullptr, 0, proc, copy.get(), flag, 0);

			if (_handle)
				copy.release();
			else
				__debugbreak();
		}
		inline explicit thread(thread const& rhs) noexcept = delete;
		inline explicit thread(thread&& rhs) noexcept
			: object(std::move(rhs)) {
		};
		inline auto operator=(thread const& rhs) noexcept -> thread & = delete;
		inline auto operator=(thread&& rhs) noexcept -> thread& {
			object::operator=(std::move(rhs));
			return *this;
		};
		inline virtual ~thread(void) noexcept override = default;
	public:
		template <typename function, typename... argument>
		inline void begin(function&& func, unsigned int flag, argument&&... arg) noexcept {
			using tuple = std::tuple<std::decay_t<function>, std::decay_t<argument>...>;
			auto copy = std::make_unique<tuple>(std::forward<function>(func), std::forward<argument>(arg)...);
			constexpr auto proc = make<tuple>(std::make_index_sequence<1 + sizeof...(argument)>());
			_handle = (HANDLE)_beginthreadex(nullptr, 0, proc, copy.get(), flag, 0);

			if (_handle)
				copy.release();
			else
				__debugbreak();
		}
		inline void join(unsigned long milli_second) noexcept {
			WaitForSingleObject(_handle, milli_second);
		}
		inline void detach(void) noexcept {
			CloseHandle(_handle);
		}
		inline unsigned long id(void) noexcept {
			GetThreadId(_handle);
		}
	public:
		inline void set_affinity_mask(DWORD_PTR mask) noexcept {
			SetThreadAffinityMask(_handle, mask);
		}
		inline void set_priority(int const priority) noexcept {
			SetThreadPriority(_handle, priority);
		}
		inline void suspend(void) noexcept {
			SuspendThread(_handle);
		}
		inline void resume(void) noexcept {
			ResumeThread(_handle);
		}
		inline void terminate(void) noexcept {
#pragma warning(suppress: 6258)
			TerminateThread(_handle, 0);
		}
		inline auto get_exit_code(void) noexcept -> unsigned long {
			unsigned long code;
			GetExitCodeThread(_handle, &code);
			return code;
		}
	public:
		inline static void switch_to(void) noexcept {
			SwitchToThread();
		}
		inline static auto current_id(void) noexcept {
			return GetCurrentThreadId();
		}
	};
}