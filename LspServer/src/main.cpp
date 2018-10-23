#include <iostream>
#include "common.hpp"
#include "lsp.hpp"
#include "rpc.hpp"

#if defined(_WIN32) || defined(_WIN64)
#include <fcntl.h>
#include <io.h>

void platform_init() {
	auto res = _setmode(_fileno(stdin), _O_BINARY);
	lsp_assert(res != -1);
	res = _setmode(_fileno(stdout), _O_BINARY);
	lsp_assert(res != -1);
}
#else
void platform_init() { }
#endif

struct my_server : public lsp::i_server {
	lsp::initialize_result init(lsp::initialize_params const& p) override {
		return lsp::initialize_result().capabilities(lsp::server_capabilities()
			.text_document_sync(lsp::text_document_sync_options()
				.open_close(true)
				.change(lsp::text_document_sync_kind::full)
			)
			.completion_provider(lsp::completion_options()
				.resolve_provider(true)
			)
		);
	}
};

int main() {
	platform_init();
	auto reader = lsp::msg_reader(std::cin);
	auto srvr = my_server();
	auto handler = lsp::msg_handler(std::cout, srvr);
	for (;;) {
		auto msg = reader.next();
		std::cerr << "MSG: " << msg.to_json().dump(4) << std::endl;
		handler.handle(msg);
	}
	return 0;
}
