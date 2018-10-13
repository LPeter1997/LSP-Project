#include <initializer_list>
#include <utility>
#include "lsp.hpp"
#include "rpc.hpp"

namespace lsp {

template <typename>
struct json_factory;

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

template <typename T, typename Fn>
static auto sub(Fn fn) {
	return [=](json const& js) -> T {
		auto fact = json_factory<T>(js);
		fn(fact);
		return fact.get();
	};
}

template <typename T>
static auto& dyn_reg(json_factory<T>& fact) {
	fact.opt(T::dynamic_registration, "dynamicRegistration", pass<bool>);
	return fact;
}

template <typename T>
static auto sub_dyn_reg() {
	return sub<T>([](auto& fact) {
		dyn_reg(fact);
	});
}

template <typename Fn>
static auto array(Fn fn) {
	return [=](json const& js) {
		lsp_assert(js.is_array());

		using elem_type = decltype(fn(*js.begin()));
		auto l = vec<elem_type>();
		l.reserve(js.size());

		for (auto it = js.begin(); it != js.end(); ++it) {
			l.push_back(fn(*it));
		}

		return l;
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

static symbol_kind to_symbol_kind(json const& js) {
	lsp_assert(js.is_number());
	auto num = js.get<i32>();
	lsp_assert(num >= 1 && num <= 26);
	return (symbol_kind)num;
}

static workspace_client_capabilities to_workspace_client_capabilities(json const& js) {
	using workspace = workspace_client_capabilities;
	using workspace_edit = workspace::workspace_edit_t;
	using did_change_configuration = workspace::did_change_configuration_t;
	using did_change_watched_files = workspace::did_change_watched_files_t;
	using symbol = workspace::symbol_t;
	using execute_command = workspace::execute_command_t;

	return json_factory<workspace>(js)
		.opt(workspace::apply_edit, "applyEdit", pass<bool>)
		.opt(workspace::workspace_edit, "workspaceEdit", sub<workspace_edit>([](auto& fact) { fact
			.opt(workspace_edit::document_changes, "documentChanges", pass<bool>)
			.opt(workspace_edit::resource_operations, "resourceOperations", array(map<str, resource_operation_kind>({
				{ "create", resource_operation_kind::create },
				{ "rename", resource_operation_kind::rename },
				{ "delete", resource_operation_kind::delete_ },
			})))
			.opt(workspace_edit::failure_handling, "failureHandling", map<str, failure_handling_kind>({
				{ "abort", failure_handling_kind::abort },
				{ "transactional", failure_handling_kind::transactional },
				{ "textOnlyTransactional", failure_handling_kind::text_only_transactional },
				{ "undo", failure_handling_kind::undo },
			}));
		}))
		.opt(workspace::did_change_configuration, "didChangeConfiguration", sub_dyn_reg<did_change_configuration>())
		.opt(workspace::did_change_watched_files, "didChangeWatchedFiles", sub_dyn_reg<did_change_watched_files>())
		.opt(workspace::symbol, "symbol", sub<symbol>([](auto& fact) { dyn_reg(fact)
			.opt(symbol::symbol_kind, "symbolKind", sub<symbol::symbol_kind_t>([](auto& fact) { fact
				.opt(symbol::symbol_kind_t::value_set, "valueSet", array(to_symbol_kind));
			}));
		}))
		.opt(workspace::execute_command, "executeCommand", sub_dyn_reg<execute_command>())
		.opt(workspace::workspace_folders, "workspaceFolders", pass<bool>)
		.opt(workspace::configuration, "configuration", pass<bool>)
		.get();
}

static text_document_client_capabilities to_text_document_client_capabilities(json const& js) {
	text_document_client_capabilities td;
	// XXX(LPeter1997): Implement
	return td;
}

static client_capabilities to_client_capabilities(json const& js) {
	return json_factory<client_capabilities>(js)
		.opt(client_capabilities::workspace, "workspace", to_workspace_client_capabilities)
		.opt(client_capabilities::text_document, "textDocument", to_text_document_client_capabilities)
		.opt(client_capabilities::experimental, "experimental", pass<json>)
		.get();
}

static workspace_folder to_workspace_folder(json const& js) {
	return json_factory<workspace_folder>(js)
		.req(workspace_folder::uri, "uri", pass<str>)
		.req(workspace_folder::name, "name", pass<str>)
		.get();
}

static opt<vec<workspace_folder>> to_workspace_folder_list(json const& js) {
	if (js.is_null()) {
		return none;
	}
	auto res = vec<workspace_folder>();
	res.reserve(js.size());
	for (auto it = js.begin(); it != js.end(); ++it) {
		res.push_back(to_workspace_folder(*it));
	}
	return res;
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
			{ "verbose", initialize_params::trace_kind::verbose },
		}))
		.opt(initialize_params::workspace_folders, "workspaceFolders", to_workspace_folder_list)
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
