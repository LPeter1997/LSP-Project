/**
 * lsp.hpp
 *
 * @author Peter Lenkefi
 * @date 2018-10-11
 * @description Structures for the actual LSP communication.
 */

#ifndef LSP_LSP_HPP
#define LSP_LSP_HPP

#include <iostream>
#include "common.hpp"

namespace lsp {

using uri_type = str;
using code_action_kind = str;

enum class resource_operation_kind {
	create, rename, delete_,
};

enum class failure_handling_kind {
	abort, transactional, undo, text_only_transactional
};

enum class symbol_kind {
	file = 1,
	module = 2,
	namespace_ = 3,
	package = 4,
	class_ = 5,
	method = 6,
	property = 7,
	dield = 8,
	constructor = 9,
	enum_ = 10,
	interface = 11,
	function = 12,
	variable = 13,
	constant = 14,
	string = 15,
	number = 16,
	boolean = 17,
	array = 18,
	object = 19,
	key = 20,
	null = 21,
	enum_member = 22,
	struct_ = 23,
	event = 24,
	operator_ = 25,
	type_parameter = 26,
};

enum class markup_kind {
	plaintext, markdown,
};

enum class completion_item_kind {
	text = 1,
	method = 2,
	function = 3,
	constructor = 4,
	field = 5,
	variable = 6,
	class_ = 7,
	interface = 8,
	module = 9,
	property = 10,
	unit = 11,
	value = 12,
	enum_ = 13,
	keyword = 14,
	snippet = 15,
	color = 16,
	file = 17,
	reference = 18,
	folder = 19,
	enum_member = 20,
	constant = 21,
	struct_ = 22,
	event = 23,
	operator_ = 24,
	type_parameter = 25,
};

#define member(ty, name, access) 				\
public:											\
	ty& access() { return name; }				\
	ty const& access() const { return name; }	\
private:										\
	ty name

#define ctors(name)						\
name() = default;						\
name(name const&) = default;			\
name(name&&) = default;					\
name& operator=(name const&) = default;	\
name& operator=(name&&) = default

#define dyn_reg \
member(bool, m_DynamicRegistration, dynamic_registration) = false

/**
 * WorkspaceClientCapabilities.
 */
struct workspace_client_capabilities {
	struct workspace_edit_t {
		ctors(workspace_edit_t);

		member(bool, m_DocumentChanges, document_changes) = false;
		member(opt<vec<resource_operation_kind>>, m_ResourceOperations, resource_operations);
		member(opt<failure_handling_kind>, m_FailureHandling, failure_handling);
	};

	struct did_change_configuration_t {
		ctors(did_change_configuration_t);

		dyn_reg;
	};

	struct did_change_watched_files_t {
		ctors(did_change_watched_files_t);

		dyn_reg;
	};

	struct symbol_t {
		struct symbol_kind_t {
			ctors(symbol_kind_t);

			member(opt<vec<symbol_kind>>, m_ValueSet, value_set) = none;
		};

		ctors(symbol_t);

		dyn_reg;
		member(opt<symbol_kind_t>, m_SymbolKind, symbol_kind) = none;
	};

	struct execute_command_t {
		ctors(execute_command_t);

		dyn_reg;
	};

	ctors(workspace_client_capabilities);

	member(bool, m_ApplyEdit, apply_edit) = false;
	member(opt<workspace_edit_t>, m_WorkspaceEdit, workspace_edit) = none;
	member(opt<did_change_configuration_t>, m_DidChangeConfiguration, did_change_configuration) = none;
	member(opt<did_change_watched_files_t>, m_DidChangeWatchedFiles, did_change_watched_files) = none;
	member(opt<symbol_t>, m_Symbol, symbol) = none;
	member(opt<execute_command_t>, m_ExecuteCommand, execute_command) = none;
	member(bool, m_WorkspaceFolders, workspace_folders) = false;
	member(bool, m_Configuration, configuration) = false;
};

/**
 * TextDocumentClientCapabilities.
 */
struct text_document_client_capabilities {
	struct synchronization_t {
		ctors(synchronization_t);

		dyn_reg;
		member(bool, m_WillSave, will_save) = false;
		member(bool, m_WillSaveWaitUntil, will_save_wait_until) = false;
		member(bool, m_DidSave, did_save) = false;
	};

	struct completion_t {
		struct completion_item_t {
			ctors(completion_item_t);

			member(bool, m_SnippetSupport, snippet_support) = false;
			member(bool, m_CommitCharactersSupport, commit_character_support) = false;
			member(opt<vec<markup_kind>>, m_DocumentationFormat, documentation_format) = none;
			member(bool, m_DeprecatedSupport, deprecated_support) = false;
			member(bool, m_PreselectSupport, preselect_support) = false;
		};

		struct completion_item_kind_t {
			ctors(completion_item_kind_t);

			member(opt<vec<completion_item_kind>>, m_ValueSet, value_set) = none;
		};

		ctors(completion_t);

		dyn_reg;
		member(opt<completion_item_t>, m_CompletionItem, completion_item) = none;
		member(opt<completion_item_kind_t>, m_CompletionItemKind, completion_item_kind) = none;
		member(bool, m_ContextSupport, context_support) = false;
	};

	struct hover_t {
		ctors(hover_t);

		dyn_reg;
		member(opt<vec<markup_kind>>, m_ContentFormat, content_format) = none;
	};

	struct signature_help_t {
		struct signature_information_t {
			ctors(signature_information_t);

			member(opt<vec<markup_kind>>, m_DocumentationFormat, documentation_format) = none;
		};

		ctors(signature_help_t);

		dyn_reg;
		member(opt<signature_information_t>, m_SignatureInformation, signature_information) = none;
	};

	struct references_t {
		ctors(references_t);

		dyn_reg;
	};

	struct document_highlight_t {
		ctors(document_highlight_t);

		dyn_reg;
	};

	struct document_symbol_t {
		struct symbol_kind_t {
			ctors(symbol_kind_t);

			member(opt<vec<symbol_kind>>, m_ValueSet, value_set) = none;
		};

		ctors(document_symbol_t);

		dyn_reg;
		member(opt<symbol_kind_t>, m_SymbolKind, symbol_kind) = none;
		member(bool, m_HierarchicalDocumentSymbolSupport, hierarchical_document_symbol_support) = false;
	};

	struct formatting_t {
		ctors(formatting_t);

		dyn_reg;
	};

	struct range_formatting_t {
		ctors(range_formatting_t);

		dyn_reg;
	};

	struct on_type_formatting_t {
		ctors(on_type_formatting_t);

		dyn_reg;
	};

	struct definition_t {
		ctors(definition_t);

		dyn_reg;
	};

	struct type_definition_t {
		ctors(type_definition_t);

		dyn_reg;
	};

	struct implementation_t {
		ctors(implementation_t);

		dyn_reg;
	};

	struct code_action_t {
		struct code_action_literal_support_t {
			struct code_action_kind_t {
				ctors(code_action_kind_t);

				member(vec<code_action_kind>, m_ValueSet, value_set);
			};

			ctors(code_action_literal_support_t);

			member(code_action_kind_t, m_CodeActionKind, code_action_kind);
		};

		ctors(code_action_t);

		dyn_reg;
		member(opt<code_action_literal_support_t>, m_CodeActionLiteralSupport, code_action_literal_support) = none;
	};

	struct code_lens_t {
		ctors(code_lens_t);

		dyn_reg;
	};

	struct document_link_t {
		ctors(document_link_t);

		dyn_reg;
	};

	struct color_provider_t {
		ctors(color_provider_t);

		dyn_reg;
	};

	struct rename_t {
		ctors(rename_t);

		dyn_reg;
		member(bool, m_PrepareSupport, prepare_support) = false;
	};

	struct publish_diagnostics_t {
		ctors(publish_diagnostics_t);

		member(bool, m_RelatedInformation, related_information) = false;
	};

	struct folding_range_t {
		ctors(folding_range_t);

		dyn_reg;
		member(opt<u32>, m_RangeLimit, range_limit) = none;
		member(bool, m_LineFoldingOnly, line_folding_only) = true;
	};

	ctors(text_document_client_capabilities);

	member(opt<synchronization_t>, m_Synchronization, synchronization) = none;
	member(opt<completion_t>, m_Completion, completion) = none;
	member(opt<hover_t>, m_Hover, hover) = none;
	member(opt<signature_help_t>, m_SignatureHelp, signature_help) = none;
	member(opt<references_t>, m_References, references) = none;
	member(opt<document_highlight_t>, m_DocumentHighlight, document_highlight) = none;
	member(opt<document_symbol_t>, m_DocumentSymbol, document_symbol) = none;
	member(opt<formatting_t>, m_Formatting, formatting) = none;
	member(opt<range_formatting_t>, m_RangeFormatting, range_formatting) = none;
	member(opt<on_type_formatting_t>, m_OnTypeFormatting, on_type_formatting) = none;
	member(opt<definition_t>, m_Definition, definition) = none;
	member(opt<type_definition_t>, m_TypeDefinition, type_definition) = none;
	member(opt<implementation_t>, m_Implementation, implementation) = none;
	member(opt<code_action_t>, m_CodeAction, code_action) = none;
	member(opt<code_lens_t>, m_CodeLens, code_lens) = none;
	member(opt<document_link_t>, m_DocumentLink, document_link) = none;
	member(opt<color_provider_t>, m_ColorProvider, color_provider) = none;
	member(opt<rename_t>, m_Rename, rename) = none;
	member(opt<publish_diagnostics_t>, m_PublishDiagnostics, publish_diagnostics) = none;
	member(opt<folding_range_t>, m_FoldingRange, folding_range) = none;
};

/**
 * The ClientCapabilities structure used for initialization.
 */
struct client_capabilities {
	ctors(client_capabilities);

	member(workspace_client_capabilities, m_Workspace, workspace);
	member(text_document_client_capabilities, m_TextDocument, text_document);
	member(opt<json>, m_Experimental, experimental);
};

/**
 * WorkspaceFolder.
 */
struct workspace_folder {
	ctors(workspace_folder);

	member(str, m_URI, uri);
	member(str, m_Name, name);
};

/**
 * InitializeParams, the 'params' for the 'initialize' request.
 */
struct initialize_params {
	enum class trace_kind {
		off, messages, verbose,
	};

	ctors(initialize_params);

	auto const& root() const {
		if (auto const& res = root_uri(); res) {
			return res;
		}
		return root_path();
	}

	member(opt<i32>, m_ProcessID, process_id) = none;
	member(opt<str>, m_RootPath, root_path) = none;
	member(opt<uri_type>, m_RoorURI, root_uri) = none;
	member(opt<json>, m_InitializationOptions, initialization_options) = none;
	member(client_capabilities, m_Capabilities, capabilities);
	member(trace_kind, m_Trace, trace) = trace_kind::off;
	member(opt<vec<workspace_folder>>, m_WorkspaceFolders, workspace_folders) = none;
};

#undef dyn_reg
#undef member
#undef ctors

struct i_server {
	virtual void init(initialize_params const&) = 0;
};

struct msg;

struct msg_handler {
	explicit msg_handler(std::ostream& os, i_server& srvr)
		: m_Ostream(&os), m_Server(&srvr) {
	}

	void handle(msg& m);

private:
	std::ostream* m_Ostream;
	i_server* m_Server;
	bool m_Initialized = false;
};

} /* namespace lsp */

#endif /* LSP_LSP_HPP */
