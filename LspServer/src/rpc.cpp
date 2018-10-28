#include "rpc.hpp"

namespace lsp {
namespace rpc {

static json pass(json&& js) {
	js["jsonrpc"] = "2.0";
	return js;
}

json request::to_json() const {
	return pass({
		{ "id", m_ID },
		{ "method", method() },
		{ "params", params() },
	});
}

json response::to_json() const {
	auto res = pass({
		{ "id", m_ID },
		{ "result", result() },
	});
	if (auto const& err = error()) {
		res["error"] = err->to_json();
	}
	return res;
}

json notification::to_json() const {
	return pass({
		{ "method", method() },
		{ "params", params() },
	});
}

json message::to_json() const {
	if (is_request()) return as_request().to_json();
	if (is_response()) return as_response().to_json();
	if (is_notification()) return as_notification().to_json();
	lsp_unreachable;
}

message message::parse(char const* msg) {
	auto js = json::parse(msg);

	auto id_it = js.find("id");
	auto method_it = js.find("method");

	if (id_it == js.end()) {
		// Must be a notification
		lsp_assert(method_it != js.end());
		auto params_it = js.find("params");
		return notification(
			std::move(method_it->get<std::string>()),
			(params_it == js.end()) ? json{} : std::move(*params_it)
		);
	}
	else {
		// Request or Response
		if (method_it == js.end()) {
			// Response
			std::optional<response_error<json>> err = std::nullopt;
			auto err_it = js.find("error");
			if (err_it != js.end()) {
				json const& err_js = *err_it;
				auto data_it = err_js.find("data");
				if (data_it == err_js.end()) {
					err = response_error<json>(
						std::move(err_js["code"].get<i32>()),
						std::move(err_js["message"].get<std::string>())
					);
				}
				else {
					err = response_error<json>(
						std::move(err_js["code"].get<i32>()),
						std::move(err_js["message"].get<std::string>()),
						std::move(*data_it)
					);
				}
			}
			return response(
				std::move(*id_it),
				std::move(js["result"]),
				std::move(err)
			);
		}
		else {
			// Request
			return request(
				std::move(*id_it),
				std::move(method_it->get<std::string>()),
				std::move(js["params"])
			);
		}
	}
}

} /* namespace rpc */
} /* namespace lsp */
