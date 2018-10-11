#include "lsp.hpp"
#include "rpc.hpp"

namespace lsp {

template <typename T>
static opt<T> get_nullable_as(json& js, char const* key) {
	lsp_assert(js.find(key) != js.end());
	auto& val = js[key];
	if (val.is_null()) {
		return none;
	}
	return val.get<T>();
}

template <typename T>
static opt<T> get_nullable_as_opt(json& js, char const* key) {
	auto it = js.find(key);
	if (it == js.end()) {
		return none;
	}
	if (it->is_null()) {
		return none;
	}
	return it->get<T>();
}

template <typename T>
static opt<json> get_opt(json& js, char const* key) {
	auto it = js.find(key);
	if (it == js.end()) {
		return none;
	}
	return *it;
}

static initialize_params to_initialize_params(json& js) {
	initialize_params ip;
	ip.process_id() = get_nullable_as<i32>(js, "processId");
	ip.root_path() = get_nullable_as_opt<str>(js, "rootPath");
	ip.root_uri() = get_nullable_as<str>(js, "rootUri");
	ip.initialization_options() = get_opt<json>(js, "initializationOptions");
	// XXX(LPeter1997): Get client capabilities
	if (auto trace_it = js.find("trace"); trace_it != js.end()) {
		char fst = trace_it->get<str>()[0];
		lsp_assert(fst == 'v' || fst == 'm' || fst == 'o');
		ip.trace() =
			fst == 'v'
				? initialize_params::trace_kind::verbose
				: fst == 'm'
					? initialize_params::trace_kind::messages
					: initialize_params::trace_kind::off
			;
	}
	// XXX(LPeter1997): Get workspace folders
	return ip;
}

void msg_handler::handle(msg& m) {
	if (!m_Initialized) {
		switch (m.type()) {
		case msg::request: {
			auto& req = m.as_request();
			if (req.method() == "initialize") {
				lsp_assert(req.params());
				m_Server->init(to_initialize_params(*req.params()));
				m_Initialized = true; // XXX(LPeter1997): Only if success?
			}
			else {
				// XXX(LPeter1997): Send error if not init
				lsp_unimplemented;
			}
		} break;

		case msg::notification: {
			// XXX(LPeter1997): Drop if not exit, terminate on exit
			lsp_unimplemented;
		} break;

		default: lsp_unreachable;
		}
		return;
	}
	else {
		lsp_unimplemented;
	}
}

} /* namespace lsp */
