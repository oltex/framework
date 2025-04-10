#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include "service.h"

int main(void) noexcept {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	framework::network::server& _service = framework::network::server::instance();

	_service.start(0, 16);
	//server_.accept("127.0.0.1", 6000, 0, 65535);
	system("pause");
	_service.stop();
	//Sleep(INFINITE);
}