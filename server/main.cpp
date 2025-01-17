#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include "server.h"

int main(void) noexcept {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	server& _server = server::instance();

	{
		server::command::parameter param("include", "server.cfg");
		_server._command.execute("include", &param);
	}
	Sleep(5000);
	{
		server::command::parameter param("server_stop");
		_server._command.execute("server_stop", &param);
	}
}