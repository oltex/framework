#include "iocp.h"
#include "server.h"

int main(void) noexcept {
	library::wsa_start_up();

	auto& iocp = framework::iocp::instance();
	iocp.create(4, 1);

	framework::server server;
	server.accept("127.0.0.1", 6000);

	system("pause");
	iocp.close();


	library::wsa_clean_up();
	return 0;
}