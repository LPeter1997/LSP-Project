#include <iostream>
#include "common.hpp"
#include "rpc.hpp"

#if defined(_WIN32) || defined(_WIN64)
#include <fcntl.h>
#include <io.h>

void platform_init() {
	auto res = _setmode(_fileno(stdin), _O_BINARY);
	lsp_assert(res != -1);
}
#else
void platform_init() { }
#endif

void process_message(lsp::msg& msg) {
	switch (msg.type()) {
	case lsp::msg::request: {
		auto& req = msg.as_request();
		std::cerr << "Got a request[" << req.id().dump() << "]: '" << req.method() << "'" << std::endl;
	} return;

	case lsp::msg::response: {
		auto& resp = msg.as_response();
		std::cerr << "Got a response[" << resp.id().dump() << "]" << std::endl;
	} return;

	case lsp::msg::notification: {
		auto& notif = msg.as_notification();
		std::cerr << "Got a notification: '" << notif.method() << "'" << std::endl;
	} return;

	default: lsp_unreachable;
	}
}

void start() {
	platform_init();
	auto reader = lsp::msg_reader(std::cin);
	for (;;) {
		auto msg = reader.next();
		process_message(msg);
	}
}

int main() {
	start();
	return 0;
}
