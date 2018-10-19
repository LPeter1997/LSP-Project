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
	lsp_assert(num >= 1 && num <= 26); // XXX(LPeter1997): Magic number!
	return (symbol_kind)num;
}

static markup_kind to_markup_kind(json const& js) {
	return map<str, markup_kind>({
		{ "plaintext", markup_kind::plaintext },
		{ "markdown", markup_kind::markdown },
	})(js);
}

static completion_item_kind to_completion_item_kind(json const& js) {
	lsp_assert(js.is_number());
	auto num = js.get<i32>();
	lsp_assert(num >= 1 && num <= 25); // XXX(LPeter1997): Magic number!
	return (completion_item_kind)num;
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
	using document = text_document_client_capabilities;
	using synchronization = document::synchronization_t;
	using completion = document::completion_t;
	using completion_item = completion::completion_item_t;
	using completion_item_kind = completion::completion_item_kind_t;
	using hover = document::hover_t;
	using signature_help = document::signature_help_t;
	using signature_information = signature_help::signature_information_t;
	using references = document::references_t;
	using document_highlight = document::document_highlight_t;
	using document_symbol = document::document_symbol_t;
	using symbol_kind = document_symbol::symbol_kind_t;
	using formatting = document::formatting_t;
	using range_formatting = document::range_formatting_t;
	using on_type_formatting = document::on_type_formatting_t;
	using definition = document::definition_t;
	using type_definition = document::type_definition_t;
	using implementation = document::implementation_t;
	using code_action = document::code_action_t;
	using code_action_literal_support = code_action::code_action_literal_support_t;
	using code_action_kind = code_action_literal_support::code_action_kind_t;
	using code_lens = document::code_lens_t;
	using document_link = document::document_link_t;
	using color_provider = document::color_provider_t;
	using rename = document::rename_t;
	using publish_diagnostics = document::publish_diagnostics_t;
	using folding_range = document::folding_range_t;

	return json_factory<document>(js)
		.opt(document::synchronization, "synchronization", sub<synchronization>([](auto& fact) { dyn_reg(fact)
			.opt(synchronization::will_save, "willSave", pass<bool>)
			.opt(synchronization::will_save_wait_until, "willSaveWaitUntil", pass<bool>)
			.opt(synchronization::did_save, "didSave", pass<bool>);
		}))
		.opt(document::completion, "completion", sub<completion>([](auto& fact) { dyn_reg(fact)
			.opt(completion::completion_item, "completionItem", sub<completion_item>([](auto& fact) { fact
				.opt(completion_item::snippet_support, "snippetSupport", pass<bool>)
				.opt(completion_item::commit_character_support, "commitCharactersSupport", pass<bool>)
				.opt(completion_item::documentation_format, "documentationFormat", array(to_markup_kind))
				.opt(completion_item::deprecated_support, "deprecatedSupport", pass<bool>)
				.opt(completion_item::preselect_support, "preselectSupport", pass<bool>);
			}))
			.opt(completion::completion_item_kind, "completionItemKind", sub<completion_item_kind>([](auto& fact) { fact
				.opt(completion_item_kind::value_set, "valueSet", array(to_completion_item_kind));
			}))
			.opt(completion::context_support, "contextSupport", pass<bool>);
		}))
		.opt(document::hover, "hover", sub<hover>([](auto& fact) { dyn_reg(fact)
			.opt(hover::content_format, "contentFormat", array(to_markup_kind));
		}))
		.opt(document::signature_help, "signatureHelp", sub<signature_help>([](auto& fact) { dyn_reg(fact)
			.opt(signature_help::signature_information, "signatureInformation", sub<signature_information>([](auto& fact) { fact
				.opt(signature_information::documentation_format, "documentationFormat", array(to_markup_kind));
			}));
		}))
		.opt(document::references, "references", sub_dyn_reg<references>())
		.opt(document::document_highlight, "documentHighlight", sub_dyn_reg<document_highlight>())
		.opt(document::document_symbol, "documentSymbol", sub<document_symbol>([](auto& fact) { dyn_reg(fact)
			.opt(document_symbol::symbol_kind, "symbolKind", sub<symbol_kind>([](auto& fact) { fact
				.opt(symbol_kind::value_set, "valueSet", array(to_symbol_kind));
			}))
			.opt(document_symbol::hierarchical_document_symbol_support, "hierarchicalDocumentSymbolSupport", pass<bool>);
		}))
		.opt(document::formatting, "formatting", sub_dyn_reg<formatting>())
		.opt(document::range_formatting, "rangeFormatting", sub_dyn_reg<range_formatting>())
		.opt(document::on_type_formatting, "onTypeFormatting", sub_dyn_reg<on_type_formatting>())
		.opt(document::definition, "definition", sub_dyn_reg<definition>())
		.opt(document::type_definition, "typeDefinition", sub_dyn_reg<type_definition>())
		.opt(document::implementation, "implementation", sub_dyn_reg<implementation>())
		.opt(document::code_action, "codeAction", sub<code_action>([](auto& fact) { dyn_reg(fact)
			.opt(code_action::code_action_literal_support, "codeActionLiteralSupport", sub<code_action_literal_support>([](auto& fact) { fact
				.req(code_action_literal_support::code_action_kind, "codeActionKind", sub<code_action_kind>([](auto& fact) { fact
					.req(code_action_kind::value_set, "valueSet", array(pass<str>)); // XXX(LPeter1997): It's a '.' separated list, we should process it
				}));
			}));
		}))
		.opt(document::code_lens, "codeLens", sub_dyn_reg<code_lens>())
		.opt(document::document_link, "documentLink", sub_dyn_reg<document_link>())
		.opt(document::color_provider, "colorProvider", sub_dyn_reg<color_provider>())
		.opt(document::rename, "rename", sub<rename>([](auto& fact) { dyn_reg(fact)
			.opt(rename::prepare_support, "prepareSupport", pass<bool>);
		}))
		.opt(document::publish_diagnostics, "publishDiagnostics", sub<publish_diagnostics>([](auto& fact) { fact
			.opt(publish_diagnostics::related_information, "relatedInformation", pass<bool>);
		}))
		.opt(document::folding_range, "foldingRange", sub<folding_range>([](auto& fact) { dyn_reg(fact)
			.opt(folding_range::range_limit, "rangeLimit", pass<i32>)
			.opt(folding_range::line_folding_only, "lineFoldingOnly", pass<bool>);
		}))
		.get();
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

////////////////////////////////////////////////////////////////////////////////

void cat_jsons_impl(json& js) { }

template <typename First, typename... Rest>
void cat_jsons_impl(json& js, First const& f, Rest const&... r) {
	for (json::const_iterator it = f.begin(); it != f.end(); ++it) {
		js[it.key()] = it.value();
	}
	cat_jsons_impl(js, r...);
}

template <typename... Jsons>
json cat_jsons(Jsons const&... jsons) {
	json res;
	cat_jsons_impl(res, jsons...);
	return res;
}

json save_options::to_json() const {
	return json{
		{ "includeText", include_text() },
	};
}

json text_document_sync_options::to_json() const {
	auto res = json{
		{ "openClose", open_close() },
		{ "change", i32(change()) },
		{ "willSave", will_save() },
		{ "willSaveWaitUntil", will_save_wait_until() },
	};
	if (auto const& s = save()) {
		res["save"] = s->to_json();
	}
	return res;
}

json server_capabilities::workspace_t::workspace_folders_t::to_json() const {
	auto res = json{
		{ "supported", supported() },
	};
	auto const& cn = change_notifications();
	if (std::holds_alternative<bool>(cn)) {
		res["changeNotifications"] = std::get<bool>(cn);
	}
	else {
		res["changeNotifications"] = std::get<str>(cn);
	}
	return res;
}

json server_capabilities::workspace_t::to_json() const {
	auto res = json{};
	if (auto const& wf = workspace_folders()) {
		res["workspaceFolders"] = wf->to_json();
	}
	return res;
}

json completion_options::to_json() const {
	auto res = json{
		{ "resolveProvider", resolve_provider() },
	};
	if (auto const& tc = trigger_characters()) {
		json l;
		for (auto const& c : *tc) {
			l.push_back(c);
		}
		res["triggerCharacters"] = l;
	}
	return res;
}

json signature_help_options::to_json() const {
	json res;
	if (auto const& tc = trigger_characters()) {
		for (auto const& c : *tc) {
			res.push_back(c);
		}
	}
	return res;
}

json document_filter::to_json() const {
	json res;
	if (auto const& l = language()) {
		res["language"] = *l;
	}
	if (auto const& s = scheme()) {
		res["scheme"] = *s;
	}
	if (auto const& p = pattern()) {
		res["pattern"] = *p;
	}
	return res;
}

json text_document_registration_options::to_json() const {
	json res;
	if (auto const& ds = document_selector()) {
		json l;
		for (auto const& f : *ds) {
			l.push_back(f.to_json());
		}
		res["documentSelector"] = l;
	}
	return res;
}

json static_registration_options::to_json() const {
	json res;
	if (auto const& i = id()) {
		res["id"] = *i;
	}
	return res;
}

json code_action_options::to_json() const {
	json res;
	if (auto const& ck = code_action_kinds()) {
		json l;
		for (auto const& k : *ck) {
			l.push_back(k);
		}
		res["codeActionKinds"] = l;
	}
	return res;
}

json code_lens_options::to_json() const {
	return json{
		{ "resolveProvider", resolve_provider() },
	};
}

json document_on_type_formatting_options::to_json() const {
	auto res = json{
		{ "firstTriggerCharacter", first_trigger_character() },
	};
	if (auto const& mt = more_trigger_characters()) {
		json l;
		for (auto const& c : *mt) {
			l.push_back(c);
		}
		res["moreTriggerCharacter"] = l;
	}
	return res;
}

json rename_options::to_json() const {
	return json{
		{ "prepareProvider", prepare_provider() },
	};
}

json document_link_options::to_json() const {
	return json{
		{ "resolveProvider", resolve_provider() },
	};
}

json color_provider_options::to_json() const {
	return json{};
}

json folding_range_provider_options::to_json() const {
	return json{};
}

json execute_command_options::to_json() const {
	json l;
	auto const& cmds = commands();
	for (auto const& c : cmds) {
		l.push_back(c);
	}
	return json{
		{ "commands", l },
	};
}

json server_capabilities::to_json() const {
	auto res = json{
		{ "hoverProvider", hover_provider() },
		{ "definitionProvider", definition_provider() },
		{ "referencesProvider", references_provider() },
		{ "documentHighlightProvider", document_highlight_provider() },
		{ "documentSymbolProvider", document_symbol_provider() },
		{ "workspaceSymbolProvider", workspace_symbol_provider() },
		{ "documentFormattingProvider", document_formatting_provider() },
		{ "documentRangeFormattingProvider", document_range_formatting_provider() },
	};

	auto const& ts = text_document_sync();
	if (std::holds_alternative<text_document_sync_options>(ts)) {
		res["textDocumentSync"] = std::get<text_document_sync_options>(ts).to_json();
	}
	else {
		res["textDocumentSync"] = i32(std::get<text_document_sync_kind>(ts));
	}

	if (auto const& cp = completion_provider()) {
		res["completionProvider"] = cp->to_json();
	}
	if (auto const& sp = signature_help_provider()) {
		res["signatureHelpProvider"] = sp->to_json();
	}

	auto const& tp = type_definition_provider();
	if (std::holds_alternative<bool>(tp)) {
		res["typeDefinitionProvider"] = std::get<bool>(tp);
	}
	else {
		auto const& tup = std::get<prod<text_document_registration_options, static_registration_options>>(tp);
		res["typeDefinitionProvider"] = cat_jsons(
			std::get<0>(tup).to_json(),
			std::get<1>(tup).to_json()
		);
	}

	auto const& ip = implementation_provider();
	if (std::holds_alternative<bool>(ip)) {
		res["implementationProvider"] = std::get<bool>(ip);
	}
	else {
		auto const& tup = std::get<prod<text_document_registration_options, static_registration_options>>(ip);
		res["implementationProvider"] = cat_jsons(
			std::get<0>(tup).to_json(),
			std::get<1>(tup).to_json()
		);
	}

	auto const& cp = code_action_provider();
	if (std::holds_alternative<bool>(cp)) {
		res["codeActionProvider"] = std::get<bool>(cp);
	}
	else {
		auto const& t = std::get<code_action_options>(cp);
		res["codeActionProvider"] = t.to_json();
	}

	if (auto const& cp = code_lens_provider()) {
		res["codeLensProvider"] = cp->to_json();
	}

	if (auto const& dp = document_on_type_formatting_provider()) {
		res["documentOnTypeFormattingProvider"] = dp->to_json();
	}

	auto const& rp = rename_provider();
	if (std::holds_alternative<bool>(rp)) {
		res["renameProvider"] = std::get<bool>(rp);
	}
	else {
		res["renameProvider"] = std::get<rename_options>(rp).to_json();
	}

	if (auto const& dp = document_link_provider()) {
		res["documentLinkProvider"] = dp->to_json();
	}

	{
	auto const& cp = color_provider();
	if (std::holds_alternative<bool>(cp)) {
		res["colorProvider"] = std::get<bool>(cp);
	}
	else if (std::holds_alternative<color_provider_options>(cp)) {
		res["colorProvider"] = std::get<color_provider_options>(cp).to_json();
	}
	else {
		auto const& tup = std::get<prod<color_provider_options, text_document_registration_options, static_registration_options>>(cp);
		res["colorProvider"] = cat_jsons(
			std::get<0>(tup).to_json(),
			std::get<1>(tup).to_json(),
			std::get<2>(tup).to_json()
		);
	}
	}

	auto const& fp = folding_range_provider();
	if (std::holds_alternative<bool>(fp)) {
		res["foldingRangeProvider"] = std::get<bool>(fp);
	}
	else if (std::holds_alternative<folding_range_provider_options>(fp)) {
		res["foldingRangeProvider"] = std::get<folding_range_provider_options>(fp).to_json();
	}
	else {
		auto const& tup = std::get<prod<folding_range_provider_options, text_document_registration_options, static_registration_options>>(fp);
		res["foldingRangeProvider"] = cat_jsons(
			std::get<0>(tup).to_json(),
			std::get<1>(tup).to_json(),
			std::get<2>(tup).to_json()
		);
	}

	if (auto const& ep = execute_command_provider()) {
		res["executeCommandProvider"] = ep->to_json();
	}

	if (auto const& ws = workspace()) {
		res["workspace"] = ws->to_json();
	}

	if (auto const& ex = experimental()) {
		res["experimental"] = *ex;
	}

	return res;
}

json initialize_result::to_json() const {
	return json{
		{ "capabilities", capabilities().to_json() },
	};
}

////////////////////////////////////////////////////////////////////////////////

void msg_handler::handle(msg& m) {
	if (!m_Initialized) {
		switch (m.type()) {
		case msg::request: {
			auto& req = m.as_request();
			if (req.method() == "initialize") {
				lsp_assert(req.params());
				// XXX(LPeter1997): If the process_id is null, exit
				auto reply_res = m_Server->init(to_initialize_params(*req.params()));
				m_Initialized = true; // XXX(LPeter1997): Only if success?

				// We reply
				auto reply = req.response();
				reply.result() = reply_res.to_json();
				*m_Ostream << reply.to_json().dump();
				std::cerr << "Replied to init with: " << reply.to_json().dump(4) << std::endl;
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
