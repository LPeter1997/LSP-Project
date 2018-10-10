#include <cstdlib>
#include "rpc.hpp"

namespace lsp {

static json json_msg_base() {
	return json{
		{ "jsonrpc", "2.0" }
	};
}

// Response message

json response_msg::to_json() const {
	json res = json_msg_base();
	res["id"] = id();
	if (auto const& res_field = result(); res_field) {
		res["result"] = *res_field;
	}
	if (auto const& err_field = error(); err_field) {
		res["error"] = err_field->to_json();
	}
	return res;
}

// Request message

json request_msg::to_json() const {
	json res = json_msg_base();
	res["id"] = id();
	res["method"] = method();
	if (auto const& params_field = params(); params_field) {
		res["params"] = *params_field;
	}
	return res;
}

response_msg request_msg::response() const {
	auto res = response_msg();
	res.id() = id();
	return res;
}

bool request_msg::is_impl_dependent() const {
	auto const& m = method();
	return m[0] == '$' && m[1] == '/';
}

// Notification message

json notification_msg::to_json() const {
	json res = json_msg_base();
	res["method"] = method();
	if (auto const& params_field = params(); params_field) {
		res["params"] = *params_field;
	}
	return res;
}

bool notification_msg::is_impl_dependent() const {
	auto const& m = method();
	return m[0] == '$' && m[1] == '/';
}

// Generic message

json msg::to_json() const {
	switch (type()) {
	case request: return as_request().to_json();
	case response: return as_response().to_json();
	case notification: return as_notification().to_json();
	default: lsp_unreachable;
	}
}

// The message reader

struct msg_header {
	u32 content_length = 0;
	std::string content_type = "";
};

bool parse_msg_header_part(std::istream& in, msg_header& h) {
	if (in.peek() == '\r') {
		char c1 = in.get();
		char c2 = in.get();
		lsp_assert(c1 == '\r' && c2 == '\n');
		return false;
	}
	// We assume it's 'Content-'
	lsp_assert(in.peek() == 'C');
	in.ignore(8);
	if (in.peek() == 'L') {
		// We assume 'Content-Length: '
		in.ignore(8);
		in >> h.content_length;
	}
	else {
		// We assume 'Content-Type: '
		lsp_assert(in.peek() == 'T');
		in.ignore(6);
		h.content_type.clear();
		while (in.peek() != '\r') {
			h.content_type += in.get();
		}
	}

	// Assume good delimeters
	char c1 = in.get();
	char c2 = in.get();
	lsp_assert(c1 == '\r' && c2 == '\n');
	return true;
}

msg_header parse_msg_header(std::istream& in) {
	msg_header h;
	while (parse_msg_header_part(in, h));
	return h;
}

json parse_msg_content(std::istream& in, msg_header const& h) {
	lsp_assert(h.content_length > 0);
	char* cont = (char*)std::malloc((h.content_length + 1) * sizeof(char));
	in.read(cont, h.content_length);
	cont[h.content_length] = '\0';
	auto js = json::parse(cont);
	std::free(cont);
	{
		lsp_assert(js.find("jsonrpc") != js.end());
		lsp_assert(*js.find("jsonrpc") == "2.0");
	}
	return js;
}

response_err<json> json_to_response_err(json& js) {
	return response_err<json>(
		js["code"],
		js["message"],
		js["data"]
	);
}

msg json_to_msg(json& js) {
	auto id_it = js.find("id");
	auto method_it = js.find("method");

	if (id_it != js.end()) {
		// Request or response
		if (method_it != js.end()) {
			lsp_assert(method_it != js.end());
			auto params_it = js.find("params");
			// Request
			return request_msg(
				std::move(*id_it),
				std::move(method_it->get<std::string>()),
				params_it == js.end() ? opt<json>() : *std::move(params_it)
			);
		}
		else {
			// Response
			auto res_it = js.find("result");
			auto err_it = js.find("error");
			return response_msg(
				std::move(*id_it),
				res_it == js.end() ? opt<json>() : *std::move(res_it),
				err_it == js.end() ? opt<response_err<json>>() : json_to_response_err(*err_it)
			);
		}
	}
	else {
		// Notification
		lsp_assert(method_it != js.end());
		auto params_it = js.find("params");
		return notification_msg(
			std::move(method_it->get<std::string>()),
			params_it == js.end() ? opt<json>() : *std::move(params_it)
		);
	}
}

msg msg_reader::next() {
	auto h = parse_msg_header(*m_Stream);
	auto cont = parse_msg_content(*m_Stream, h);
	return json_to_msg(cont);
}

} /* namespace lsp */
