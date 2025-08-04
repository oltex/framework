#pragma once
#include "library/socket.h"

namespace framework {
	class session final {
		unsigned long long _key;
		library::socket _socket;
	};
}