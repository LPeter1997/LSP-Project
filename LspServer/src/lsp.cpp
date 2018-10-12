#include <initializer_list>
#include <utility>
#include "lsp.hpp"
#include "rpc.hpp"

namespace lsp {

template <typename T>
static opt<T> nullable_to_opt(json const& js) {
	if (js.is_null()) {
		return none;
	}
	return js.get<T>();
}

template <typename T>
static T pass(json const& js) {
	return js.get<T>();
}

template <typename T, typename U>
static auto map(std::initializer_list<std::pair<T, U>> ps) {
	return [=](json const& js) {
		for (auto const& [from, to] : ps) {
			if (from == js.get<T>()) {
				return to;
			}
		}
		lsp_unreachable;
	};
}

template <typename TCons>
struct json_factory {
	explicit json_factory(json const& js)
		: m_JSON(js) {
	}

	template <typename MemTy, typename F>
	json_factory& req(MemTy&(TCons::* mem)(), char const* key, F func) {
		auto it = m_JSON.find(key);
		lsp_assert(it != m_JSON.end());
		(m_Value.*mem)() = func(*it);
		return *this;
	}

	template <typename MemTy, typename F>
	json_factory& opt(MemTy&(TCons::* mem)(), char const* key, F func) {
		auto it = m_JSON.find(key);
		if (it != m_JSON.end()) {
			(m_Value.*mem)() = func(*it);
		}
		return *this;
	}

	template <typename MemTy, typename F, typename D>
	json_factory& opt_def(MemTy&(TCons::* mem)(), char const* key, D def, F func) {
		auto it = m_JSON.find(key);
		if (it != m_JSON.end()) {
			(m_Value.*mem)() = func(*it);
		}
		else {
			(m_Value.*mem)() = def;
		}
		return *this;
	}

	TCons get() {
		return std::move(m_Value);
	}

private:
	TCons m_Value;
	json const& m_JSON;
};

static workspace_client_capabilities to_workspace_client_capabilities(json const& js) {
	workspace_client_capabilities wc;
	// XXX(LPeter1997): Implement
	return wc;
}

static text_document_client_capabilities to_text_document_client_capabilities(json& js) {
	text_document_client_capabilities td;
	// XXX(LPeter1997): Implement
	return td;
}

static client_capabilities to_client_capabilities(json const& js) {
	client_capabilities cc;
	// XXX(LPeter1997): Implement
	return cc;
}

static workspace_folder to_workspace_folder(json& js) {
	workspace_folder wf;
	// XXX(LPeter1997): Implement
	return wf;
}

static initialize_params to_initialize_params(json& js) {
	// XXX(LPeter1997): Implement workspace folders
	return json_factory<initialize_params>(js)
		.req(initialize_params::process_id, "processId", nullable_to_opt<i32>)
		.opt(initialize_params::root_path, "rootPath", nullable_to_opt<str>)
		.req(initialize_params::root_uri, "rootUri", nullable_to_opt<str>)
		.opt(initialize_params::initialization_options, "initializationOptions", pass<json>)
		.req(initialize_params::capabilities, "capabilities", to_client_capabilities)
		.opt_def(initialize_params::trace, "trace", initialize_params::trace_kind::off, map<str, initialize_params::trace_kind>({
			{ "off", initialize_params::trace_kind::off },
			{ "messages", initialize_params::trace_kind::messages },
			{ "verbose", initialize_params::trace_kind::verbose }
		}))
		.get();
}

void msg_handler::handle(msg& m) {
	if (!m_Initialized) {
		switch (m.type()) {
		case msg::request: {
			auto& req = m.as_request();
			if (req.method() == "initialize") {
				lsp_assert(req.params());
				// XXX(LPeter1997): If the process_id is null, exit
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
