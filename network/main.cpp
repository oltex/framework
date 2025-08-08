#include "iocp.h"
#include "server.h"

int main(void) noexcept {
	auto& iocp = framework::iocp::instance();
	iocp.create(4, 1);
	//-------------------------
	library::wsa_start_up();
	library::socket::wsa_io_control_acccept_ex();
	library::socket::wsa_io_control_get_accept_ex_sockaddr();

	framework::server server;
	server.start();
	server.accept("127.0.0.1", 6000);
	system("pause");
	server.reject();
	//-------------------------
	system("pause");
	//-------------------------


	library::wsa_clean_up();
	//-------------------------
	iocp.close();
	return 0;
}