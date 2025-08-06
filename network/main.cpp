#include "iocp.h"
#include "server.h"

int main(void) noexcept {

	auto& iocp = framework::iocp::instance();
	iocp.create(4, 1);

	framework::server server;
	server.accept("127.0.0.1", 6000);

	system("pause");
	iocp.close();
	return 0;
}