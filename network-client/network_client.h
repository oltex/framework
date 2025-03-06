#pragma once
#include "library/system-component/socket.h"
#include "library/system-component/thread.h"

class network_client {


	system_component::socket _socket;
	system_component::thread _receive_thread;

	unsigned char _header_code;
	unsigned char _header_fixed_key;
};