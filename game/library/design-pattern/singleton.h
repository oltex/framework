#pragma once
#include <type_traits>

namespace design_pattern {
	template<typename type>
	class local_static {
	};
	template<typename type>
	class member_static {
	protected:
		inline static type* _instance;
	};

	template<typename type, typename trait = local_static<type>>
	class singleton : public trait {
	protected:
		inline explicit singleton(void) noexcept = default;
		inline ~singleton(void) noexcept = default;
	private:
		inline explicit singleton(singleton const& rhs) noexcept = delete;
		inline auto operator=(singleton const& rhs) noexcept -> singleton & = delete;
		inline explicit singleton(singleton&& rhs) noexcept = delete;
		inline auto operator=(singleton&& rhs) noexcept -> singleton & = delete;
	public:
		inline static auto instance(void) noexcept -> type& requires std::is_same<local_static<type>, trait>::value {
			static type _instance;
			return _instance;
		}
		inline static auto instance(void) noexcept -> type& requires std::is_same<member_static<type>, trait>::value {
			return *trait::_instance;
		}
		inline static void destructor(void) noexcept requires std::is_same<member_static<type>, trait>::value {
			delete trait::_instance;
		}
	};
}