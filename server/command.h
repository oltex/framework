#pragma once
#include "library/design-pattern/singleton.h"
#include "library/data-strucutre/unordered_map.h"

#include <string>
#include <functional>

class command final : public design_pattern::singleton<command> {
private:
	friend class design_pattern::singleton<command>;
public:
	class parameter final {
	private:
		using size_type = unsigned int;
	public:
		template<typename... argument>
		inline explicit parameter(argument&&... arg) noexcept {
			(_argument.emplace_back(std::forward<argument>(arg)), ...);
		}
		inline explicit parameter(data_structure::vector<std::string> & argument) noexcept 
			: _argument(argument) {
		}
		inline explicit parameter(parameter const& rhs) noexcept = delete;
		inline explicit parameter(parameter&& rhs) noexcept = delete;
		inline auto operator=(parameter const& rhs) noexcept -> parameter & = delete;
		inline auto operator=(parameter&& rhs) noexcept -> parameter & = delete;
		inline ~parameter(void) noexcept = default;
	public:
		inline auto get_string(size_type index) noexcept -> std::string& {
			return _argument[index];
		}
		inline auto get_int(size_type index) noexcept -> int {
			return std::stoi(_argument[index]);
		}
		inline auto get_float(size_type index) noexcept -> float {
			return std::stof(_argument[index]);
		}
		inline auto get_bool(size_type index) noexcept -> bool {
			std::string& arg = _argument[index];
			if (arg == "true" || arg == "on" || arg == "1")
				return true;
			return false;
		}
	private:
		data_structure::vector<std::string> _argument;
	};
private:
	inline explicit command(void) noexcept = default;
	inline explicit command(command const& rhs) noexcept = delete;
	inline explicit command(command&& rhs) noexcept = delete;
	inline auto operator=(command const& rhs) noexcept -> command & = delete;
	inline auto operator=(command&& rhs) noexcept -> command & = delete;
	inline ~command(void) noexcept = default;
public:
	inline void add(std::string_view name, std::function<int(parameter*)> function) noexcept {
		_function.emplace(name.data(), function);
	}
	inline int execute(std::string name, parameter* par) noexcept {
		auto res = _function.find(name.data());
		return res->_second(par);
	}
	data_structure::unordered_map<std::string, std::function<int(parameter*)>> _function;

};