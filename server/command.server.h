#pragma once
#include "command.h"
#include "server.h"

void command_server(void) noexcept {
	auto& _command = command::instance();

	_command.add("start_server", [](command::parameter* param) noexcept -> int {
		auto& server_ = server::instance();

		server_.start();

		return 0;
		});


	_command.add("start_server", [](command::parameter* param) noexcept -> int {
		auto& server_ = server::instance();

		server_.start();

		return 0;
		});


	_command.add("start_server", [](command::parameter* param) noexcept -> int {
		auto& server_ = server::instance();

		server_.start();

		return 0;
		});


	_command.add("start_server", [](command::parameter* param) noexcept -> int {
		auto& server_ = server::instance();

		server_.start();

		return 0;
		});


	_command.add("start_server", [](command::parameter* param) noexcept -> int {
		auto& server_ = server::instance();

		server_.start();

		return 0;
		});


}