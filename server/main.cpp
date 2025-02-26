#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include "server.h"

class g : public server::scheduler::group {
	// 持失切
	// 社瑚切
	bool on_receive_session(unsigned long long key, server::session::view& view_) noexcept override
	{
		return false;
	}
	int on_update() noexcept override
	{
		printf("Test");
		return 1000;
	}
	//void destructor() noexcept override {
	//	auto& memory_pool = data_structure::_thread_local::memory_pool<g, 1024, false>::instance();
	//	memory_pool.deallocate(*this);
	//}
};

int main(void) noexcept {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	server _server;// = server::instance();
	{
		command::parameter param("include", "server.cfg");
		command::instance().execute("include", &param);
	}
	unsigned long long key = _server.do_create_group<g>();
	system("pause");
	_server.do_destroy_group(key);
	system("pause");
	_server.do_destroy_group(key);
	{
		command::parameter param("server_stop");
		command::instance().execute("server_stop", &param);
	}
	//Sleep(INFINITE);
}