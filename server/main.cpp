#include "command.server.h"
#include "command.default.h"

int main(void) noexcept {
	command_default();
	command_server();

	command& _command = command::instance();
	command::parameter param("server.cfg");
	_command.execute("include", &param);

	Sleep(INFINITE);
}