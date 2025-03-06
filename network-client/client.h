#pragma once
#include "network_client.h"

class client final : public network_client {
	inline virtual bool on_receive(view_pointer& view_ptr) noexcept override {
		return true;
	};
	inline virtual void on_disconnect(void) noexcept override {
	};
};