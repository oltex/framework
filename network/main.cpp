#include "iocp.h"
#include "server.h"

int main(void) noexcept {
	framework::iocp iocp;

	iocp.start(4, 16);

	system("pause");
	iocp.stop();
	return 0;
}