#include <memory>
#include "jwrap.hpp"
#include "lsp.hpp"


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


namespace lsp {

void connection::global_init() {
	platform_init();
}

void connection::write(rpc::message const& msg) {
	auto content = msg.to_json().dump();
	out()
		<< "Content-Length: "
		<< content.length()
		<< "\r\n\r\n"
		<< content
		<< std::flush;
}

bool connection::read_message_header_part(connection::message_header& h) {
	if (in().peek() == '\r') {
		char c1 = in().get();
		char c2 = in().get();
		lsp_assert(c1 == '\r' && c2 == '\n');
		return false;
	}
	// We assume it's 'Content-'
	lsp_assert(in().peek() == 'C');
	in().ignore(8);
	if (in().peek() == 'L') {
		// We assume 'Content-Length: '
		in().ignore(8);
		in() >> h.content_length;
	}
	else {
		// We assume 'Content-Type: '
		lsp_assert(in().peek() == 'T');
		in().ignore(6);
		h.content_type.clear();
		while (in().peek() != '\r') {
			h.content_type += in().get();
		}
	}

	// Assume good delimeters
	char c1 = in().get();
	char c2 = in().get();
	lsp_assert(c1 == '\r' && c2 == '\n');
	return true;
}

connection::message_header connection::read_message_header() {
	message_header h;
	while (read_message_header_part(h));
	return h;
}

rpc::message connection::read() {
	auto h = read_message_header();
	lsp_assert(h.content_length > 0);
	auto content = std::make_unique<char[]>(h.content_length);
	in().read(content.get(), h.content_length);
	return rpc::message::parse(content.get());
}

void langserver_handler::next() {
	auto msg = m_Connection.read();
	if (msg.is_request()) {
		auto const& req = msg.as_request();

		if (req.method() == "initialize") {
			if (m_Initialized) {
				// XXX(LPeter1997): Send error
				lsp_unimplemented;
			}
			auto init_params = initialize_params::from_json(req.params());
			// XXX(LPeter1997): If parent process is null, exit
			// XXX(LPeter1997): Handle init error?
			auto init_result = m_Langserver->on_initialize(init_params);
			auto response = req.reply(init_result.to_json());
			m_Connection.write(response);
			m_Initialized = true;
		}
		else {
			lsp_unimplemented;
		}
	}
	else {
		lsp_unimplemented;
	}
}

void start_langserver(langserver& ls, std::istream& in, std::ostream& out) {
	auto h = langserver_handler(in, out, ls);
	while (true) {
		h.next();
	}
}

template <typename T>
T ident(T&& val) {
	return val;
}

template <typename T>
T def(std::optional<T>&& opt, T&& dv) {
	if (opt) {
		return *std::move(opt);
	}
	else {
		return dv;
	}
}

template <typename U = json>
std::optional<U> null_to_opt(json&& val) {
	if (val.is_null()) {
		return std::nullopt;
	}
	else {
		return val.template get<U>();
	}
}

template <typename Fn>
auto list(Fn fn) {
	return [=](auto&& js) {
		using element_type = decltype(fn(*js.begin()));

		lsp_assert(js.is_array());
		std::vector<element_type> vec;
		vec.reserve(js.size());

		for (auto it = js.begin(); it != js.end(); ++it) {
			vec.push_back(fn(*it));
		}

		return vec;
	};
}

#define dyn_reg() \
dynamic_registration(def(jw.opt<bool>("dynamicRegistration"), false))

// ResourceOperationKind

resource_operation_kind to_resource_operation_kind(std::string const& str) {
	if (str == "create") return resource_operation_kind::create;
	if (str == "rename") return resource_operation_kind::rename;

	lsp_assert(str == "delete");
	return resource_operation_kind::delete_;
}

// FailureHandlingKind

failure_handling_kind to_failure_handling_kind(std::string const& str) {
	if (str == "abort") return failure_handling_kind::abort;
	if (str == "transactional") return failure_handling_kind::transactional;
	if (str == "textOnlyTransactional") return failure_handling_kind::text_only_transactional;

	lsp_assert(str == "undo");
	return failure_handling_kind::undo;
}

// SymbolKind

symbol_kind to_symbol_kind(i32 num) {
	lsp_assert(num >= 1 && num <= 26);

	return symbol_kind(num);
}

// MarkupKind

markup_kind to_markup_kind(std::string const& str) {
	if (str == "plaintext") return markup_kind::plaintext;

	lsp_assert(str == "markdown");
	return markup_kind::markdown;
}

// CompletionItemKind

completion_item_kind to_completion_item_kind(i32 num) {
	lsp_assert(num >= 1 && num <= 25);

	return completion_item_kind(num);
}

// WorkspaceFolder

workspace_folder workspace_folder::from_json(json const& js) {
	return workspace_folder()
		.uri(js["uri"])
		.name(js["name"])
		;
}

// InitializeParams

initialize_params::trace_t to_trace(std::string const& str) {
	if (str == "off") return initialize_params::trace_t::off;
	if (str == "messages") return initialize_params::trace_t::messages;

	lsp_assert(str == "verbose");
	return initialize_params::trace_t::verbose;
}

initialize_params initialize_params::from_json(json const& js) {
	auto jw = jwrap(js);
	return initialize_params()
		.process_id(null_to_opt<i32>(jw.get("processId")))
		.root_path(jw.opt("rootPath") | null_to_opt<std::string>)
		.root_uri(null_to_opt<std::string>(jw.get("rootUri")))
		.initialization_options(def(jw.opt("initializationOptions"), (json)nullptr))
		.capabilities(client_capabilities::from_json(jw.get("capabilities")))
		.trace(def(jw.opt("trace") | to_trace, trace_t::off))
		.workspace_folders(jw.opt("workspaceFolders") | null_to_opt<> | list(workspace_folder::from_json))
		;
}

std::optional<std::string> const& initialize_params::root() const {
	if (auto const& u = root_uri()) {
		return u;
	}
	else {
		return root_path();
	}
}

// ClientCapabilities

client_capabilities client_capabilities::from_json(json const& js) {
	auto jw = jwrap(js);
	return client_capabilities()
		.workspace(jw.opt("workspace") | workspace_client_capabilities::from_json)
		.text_document(jw.opt("textDocument") | text_document_client_capabilities::from_json)
		.experimental(jw.opt("experimental"))
		;
}

// WorkspaceClientCapabilities

workspace_client_capabilities workspace_client_capabilities::from_json(json const& js) {
	auto jw = jwrap(js);
	return workspace_client_capabilities()
		// XXX(LPeter1997): Implement
		.apply_edit(def(jw.opt<bool>("applyEdit"), false))
		.workspace_edit(jw.opt("workspaceEdit") | workspace_edit_t::from_json)
		.did_change_configuration(jw.opt("didChangeConfiguration") | did_change_configuration_t::from_json)
		.did_change_watched_files(jw.opt("didChangeWatchedFiles") | did_change_watched_files_t::from_json)
		.symbol(jw.opt("symbol") | symbol_t::from_json)
		.execute_command(jw.opt("executeCommand") | execute_command_t::from_json)
		.workspace_folders(def(jw.opt<bool>("workspaceFolders"), false))
		.configuration(def(jw.opt<bool>("configuration"), false))
		;
}

workspace_client_capabilities::workspace_edit_t
workspace_client_capabilities::workspace_edit_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return workspace_edit_t()
		.document_changes(def(jw.opt<bool>("documentChanges"), false))
		.resource_operations(def(jw.opt("resourceOperations") | list(to_resource_operation_kind), std::vector<resource_operation_kind>()))
		.failure_handling(def(jw.opt("failureHandling") | to_failure_handling_kind, failure_handling_kind::abort))
		;
}

workspace_client_capabilities::did_change_configuration_t
workspace_client_capabilities::did_change_configuration_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return did_change_configuration_t()
		.dyn_reg()
		;
}

workspace_client_capabilities::did_change_watched_files_t
workspace_client_capabilities::did_change_watched_files_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return did_change_watched_files_t()
		.dyn_reg()
		;
}

workspace_client_capabilities::symbol_t workspace_client_capabilities::symbol_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return symbol_t()
		.dyn_reg()
		.symbol_kind(jw.opt("symbolKind") | symbol_kind_t::from_json)
		;
}

std::vector<symbol_kind> default_symbol_kinds() {
	std::vector<symbol_kind> res;
	res.reserve(i32(symbol_kind::array) - i32(symbol_kind::file));
	for (i32 i = i32(symbol_kind::file); i != i32(symbol_kind::array); ++i) {
		res.push_back(symbol_kind(i));
	}
	return res;
}

workspace_client_capabilities::symbol_t::symbol_kind_t
workspace_client_capabilities::symbol_t::symbol_kind_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return symbol_kind_t()
		.value_set(def(jw.opt("valueSet") | list(to_symbol_kind), default_symbol_kinds()))
		;
}

workspace_client_capabilities::execute_command_t workspace_client_capabilities::execute_command_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return execute_command_t()
		.dyn_reg()
		;
}

// TextDocumentClientCapabilities

text_document_client_capabilities text_document_client_capabilities::from_json(json const& js) {
	auto jw = jwrap(js);
	return text_document_client_capabilities()
		// XXX(LPeter1997): Implement
		.synchronization(jw.opt("synchronization") | synchronization_t::from_json)
		.completion(jw.opt("completion") | completion_t::from_json)
		.hover(jw.opt("hover") | hover_t::from_json)
		.signature_help(jw.opt("signatureHelp") | signature_help_t::from_json)
		.references(jw.opt("references") | references_t::from_json)
		.document_highlight(jw.opt("documentHighlight") | document_highlight_t::from_json)
		.document_symbol(jw.opt("documentSymbol") | document_symbol_t::from_json)
		.formatting(jw.opt("formatting") | formatting_t::from_json)
		.range_formatting(jw.opt("rangeFormatting") | range_formatting_t::from_json)
		.on_type_formatting(jw.opt("onTypeFormatting") | on_type_formatting_t::from_json)
		.definition(jw.opt("definition") | definition_t::from_json)
		.type_definition(jw.opt("typeDefinition") | type_definition_t::from_json)
		.implementation(jw.opt("implementation") | implementation_t::from_json)
		.code_action(jw.opt("codeAction") | code_action_t::from_json)
		.code_lens(jw.opt("codeLens") | code_lens_t::from_json)
		.document_link(jw.opt("documentLink") | document_link_t::from_json)
		.color_provider(jw.opt("colorProvider") | color_provider_t::from_json)
		.rename(jw.opt("rename") | rename_t::from_json)
		.publish_diagnostics(jw.opt("publishDiagnostics") | publish_diagnostics_t::from_json)
		.folding_range(jw.opt("foldingRange") | folding_range_t::from_json)
		;
}

text_document_client_capabilities::synchronization_t
text_document_client_capabilities::synchronization_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return synchronization_t()
		.dyn_reg()
		.will_save(def(jw.opt<bool>("willSave"), false))
		.will_save_wait_until(def(jw.opt<bool>("willSaveWaitUntil"), false))
		.did_save(def(jw.opt<bool>("didSave"), false))
		;
}

text_document_client_capabilities::completion_t
text_document_client_capabilities::completion_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return completion_t()
		.dyn_reg()
		.completion_item(jw.opt("completionItem") | completion_item_t::from_json)
		.completion_item_kind(jw.opt("completionItemKind") | completion_item_kind_t::from_json)
		.context_support(def(jw.opt<bool>("contextSupport"), false))
		;
}

text_document_client_capabilities::completion_t::completion_item_t
text_document_client_capabilities::completion_t::completion_item_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return completion_item_t()
		.snippet_support(def(jw.opt<bool>("snippetSupport"), false))
		.commit_characters_support(def(jw.opt<bool>("commitCharactersSupport"), false))
		.documentation_format(def(jw.opt("documentationFormat") | list(to_markup_kind), std::vector<markup_kind>()))
		.deprecated_support(def(jw.opt<bool>("deprecatedSupport"), false))
		.preselect_support(def(jw.opt<bool>("preselectSupport"), false))
		;
}

std::vector<completion_item_kind> default_completion_item_kinds() {
	std::vector<completion_item_kind> res;
	res.reserve(i32(completion_item_kind::reference) - i32(completion_item_kind::text));
	for (i32 i = i32(completion_item_kind::text); i != i32(completion_item_kind::reference); ++i) {
		res.push_back(completion_item_kind(i));
	}
	return res;
}

text_document_client_capabilities::completion_t::completion_item_kind_t
text_document_client_capabilities::completion_t::completion_item_kind_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return completion_item_kind_t()
		.value_set(def(jw.opt("valueSet") | list(to_completion_item_kind), default_completion_item_kinds()))
		;
}

text_document_client_capabilities::hover_t text_document_client_capabilities::hover_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return hover_t()
		.dyn_reg()
		.content_format(def(jw.opt("contentFormat") | list(to_markup_kind), std::vector<markup_kind>()))
		;
}

text_document_client_capabilities::signature_help_t
text_document_client_capabilities::signature_help_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return signature_help_t()
		.dyn_reg()
		.signature_information(jw.opt("signatureInformation") | signature_information_t::from_json)
		;
}

text_document_client_capabilities::signature_help_t::signature_information_t
text_document_client_capabilities::signature_help_t::signature_information_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return signature_information_t()
		.documentation_format(def(jw.opt("documentationFormat") | list(to_markup_kind), std::vector<markup_kind>()))
		;
}

text_document_client_capabilities::references_t
text_document_client_capabilities::references_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return references_t()
		.dyn_reg()
		;
}

text_document_client_capabilities::document_highlight_t
text_document_client_capabilities::document_highlight_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return document_highlight_t()
		.dyn_reg()
		;
}

text_document_client_capabilities::document_symbol_t text_document_client_capabilities::document_symbol_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return document_symbol_t()
		.dyn_reg()
		.symbol_kind(jw.opt("symbolKind") | symbol_kind_t::from_json)
		.hierarchical_document_symbol_support(def(jw.opt<bool>("hierarchicalDocumentSymbolSupport"), false))
		;
}

text_document_client_capabilities::document_symbol_t::symbol_kind_t
text_document_client_capabilities::document_symbol_t::symbol_kind_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return symbol_kind_t()
		.value_set(def(jw.opt("valueSet") | list(to_symbol_kind), default_symbol_kinds()))
		;
}

text_document_client_capabilities::formatting_t
text_document_client_capabilities::formatting_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return formatting_t()
		.dyn_reg()
		;
}

text_document_client_capabilities::range_formatting_t
text_document_client_capabilities::range_formatting_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return range_formatting_t()
		.dyn_reg()
		;
}

text_document_client_capabilities::on_type_formatting_t
text_document_client_capabilities::on_type_formatting_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return on_type_formatting_t()
		.dyn_reg()
		;
}

text_document_client_capabilities::definition_t
text_document_client_capabilities::definition_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return definition_t()
		.dyn_reg()
		;
}

text_document_client_capabilities::type_definition_t
text_document_client_capabilities::type_definition_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return type_definition_t()
		.dyn_reg()
		;
}

text_document_client_capabilities::implementation_t
text_document_client_capabilities::implementation_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return implementation_t()
		.dyn_reg()
		;
}

text_document_client_capabilities::code_action_t
text_document_client_capabilities::code_action_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return code_action_t()
		.dyn_reg()
		.code_action_literal_support(jw.opt("codeActionLiteralSupport") | code_action_literal_support_t::from_json)
		;
}

text_document_client_capabilities::code_action_t::code_action_literal_support_t
text_document_client_capabilities::code_action_t::code_action_literal_support_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return code_action_literal_support_t()
		.code_action_kind(code_action_kind_t::from_json(jw.get("codeActionKind")))
		;
}

text_document_client_capabilities::code_action_t::code_action_literal_support_t::code_action_kind_t
text_document_client_capabilities::code_action_t::code_action_literal_support_t::code_action_kind_t::
from_json(json const& js) {
	auto jw = jwrap(js);
	return code_action_kind_t()
		.value_set(list(ident<std::string>)(jw.get("valueSet")))
		;
}

text_document_client_capabilities::code_lens_t
text_document_client_capabilities::code_lens_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return code_lens_t()
		.dyn_reg()
		;
}

text_document_client_capabilities::document_link_t
text_document_client_capabilities::document_link_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return document_link_t()
		.dyn_reg()
		;
}

text_document_client_capabilities::color_provider_t
text_document_client_capabilities::color_provider_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return color_provider_t()
		.dyn_reg()
		;
}

text_document_client_capabilities::rename_t
text_document_client_capabilities::rename_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return rename_t()
		.dyn_reg()
		.prepare_support(def(jw.opt<bool>("prepareSupport"), false))
		;
}

text_document_client_capabilities::publish_diagnostics_t
text_document_client_capabilities::publish_diagnostics_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return publish_diagnostics_t()
		.related_information(def(jw.opt<bool>("relatedInformation"), false))
		;
}

text_document_client_capabilities::folding_range_t
text_document_client_capabilities::folding_range_t::from_json(json const& js) {
	auto jw = jwrap(js);
	return folding_range_t()
		.dyn_reg()
		.range_limit(jw.opt<i32>("rangeLimit"))
		.line_folding_only(def(jw.opt<bool>("lineFoldingOnly"), true))
		;
}

} /* namespace lsp */
