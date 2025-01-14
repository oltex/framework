#pragma once
#include "command.h"
#include "library/utility/parser.h"

void command_default(void) noexcept {
	auto& _command = command::instance();

	_command.add("include", [](command::parameter* param) noexcept -> int {
		std::string path = param->get_string(0);
		utility::parser parser(std::wstring(path.begin(), path.end()));

		auto& command_ = command::instance();
		for (auto& iter : parser) {
			command::parameter param(iter.second);
			command_.execute(iter.first, &param);
		}
		return 0;
		});
}