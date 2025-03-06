#include "client.h"

int main(void) noexcept {
	client _client;
	_client.do_connect("127.0.0.1", 6000);
	system("pause");
	return 0;
}