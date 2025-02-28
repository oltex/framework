#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include "server.h"

class g : public server::scheduler::group {
	// 持失切
	// 社瑚切

	bool on_enter_session(unsigned long long key) noexcept override
	{
		return false;
	}
	bool on_receive_session(unsigned long long key, server::session::view& view_) noexcept override
	{
		return false;
	}
	bool on_leave_session(unsigned long long key) noexcept override
	{
		return false;
	}
	int on_update() noexcept override
	{
		printf("Test");
		return 1000;
	}
	int a = 10;
};

int main(void) noexcept {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	server _server;// = server::instance();
	{
		command::parameter param("include", "server.cfg");
		command::instance().execute("include", &param);
	}
	system("pause");
	unsigned long long key1 = _server.do_create_group<g>();
	unsigned long long key2 = _server.do_create_group<g>();
	unsigned long long key3 = _server.do_create_group<g>();
	unsigned long long key4 = _server.do_create_group<g>();
	unsigned long long key5 = _server.do_create_group<g>();
	unsigned long long key6 = _server.do_create_group<g>();
	_server.do_destroy_group(key1);
	_server.do_destroy_group(key3);
	_server.do_destroy_group(key2);
	_server.do_destroy_group(key4);
	_server.do_destroy_group(key5);
	_server.do_destroy_group(key6);
	_server.do_destroy_group(key3);
	_server.do_destroy_group(key4);
	_server.do_destroy_group(key4);
	system("pause");
	unsigned long long key7 = _server.do_create_group<g>();
	unsigned long long key8 = _server.do_create_group<g>();
	unsigned long long key9 = _server.do_create_group<g>();
	unsigned long long key10 = _server.do_create_group<g>();
	unsigned long long key11 = _server.do_create_group<g>();
	unsigned long long key12 = _server.do_create_group<g>();
	_server.do_destroy_group(key1);
	_server.do_destroy_group(key3);
	_server.do_destroy_group(key2);
	_server.do_destroy_group(key4);
	_server.do_destroy_group(key5);
	_server.do_destroy_group(key6);
	_server.do_destroy_group(key3);
	_server.do_destroy_group(key4);
	_server.do_destroy_group(key4);
	_server.do_destroy_group(key7);
	_server.do_destroy_group(key3);
	_server.do_destroy_group(key8);
	_server.do_destroy_group(key4);
	_server.do_destroy_group(key9);
	_server.do_destroy_group(key11);
	_server.do_destroy_group(key10);
	_server.do_destroy_group(key12);
	_server.do_destroy_group(key4);
	system("pause");
	{
		command::parameter param("server_stop");
		command::instance().execute("server_stop", &param);
	}
	//Sleep(INFINITE);
}