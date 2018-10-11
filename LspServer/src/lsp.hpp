/**
 * lsp.hpp
 *
 * @author Peter Lenkefi
 * @date 2018-10-11
 * @description Structures for the actual LSP communication.
 */

#ifndef LSP_LSP_HPP
#define LSP_LSP_HPP

#include "common.hpp"

namespace lsp {

using uri_type = std::string;
using code_action_kind = std::string;

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

/**
 * WorkspaceClientCapabilities.
 */
struct workspace_client_capabilities {
	struct workspace_edit_t {

	private:
		opt<bool> m_DocumentChanges;
		opt<std::vector<resource_operation_kind>> m_ResourceOperations;
		opt<failure_handling_kind> m_FailureHandling;
	};

	struct did_change_configuration_t {

	private:
		opt<bool> m_DynamicRegistration;
	};

	struct did_change_watched_files_t {

	private:
		opt<bool> m_DynamicRegistration;
	};

	struct symbol_t {
		struct symbol_kind_t {

		private:
			opt<std::vector<symbol_kind>> m_ValueSet;
		};

	private:
		opt<bool> m_DynamicRegistration;
		opt<symbol_kind_t> m_SymbolKind;
	};

	struct execute_command_t {

	private:
		opt<bool> m_DynamicRegistration;
	};

private:
	opt<bool> m_ApplyEdit;
	opt<workspace_edit_t> m_WorkspaceEdit;
	opt<did_change_configuration_t> m_DidChangeConfiguration;
	opt<did_change_watched_files_t> m_DidChangeWatchedFiles;
	opt<symbol_t> m_Symbol;
	opt<execute_command_t> m_ExecuteCommand;
	opt<bool> m_WorkspaceFolders;
	opt<bool> m_Configuration;
};

/**
 * TextDocumentClientCapabilities.
 */
struct text_document_client_capabilities {
	struct synchronization_t {

	private:
		opt<bool> m_DynamicRegistration;
		opt<bool> m_WillSave;
		opt<bool> m_WillSaveWaitUntil;
		opt<bool> m_DidSave;
	};

	struct completion_t {
		struct completion_item_t {

		private:
			opt<bool> m_SnippetSupport;
			opt<bool> m_CommitCharactersSupport;
			opt<std::vector<markup_kind>> m_DocumentationFormat;
			opt<bool> m_DeprecatedSupport;
			opt<bool> m_PreselectSupport;
		};

		struct completion_item_kind_t {

		private:
			opt<std::vector<completion_item_kind>> m_ValueSet;
		};

	private:
		opt<bool> m_DynamicRegistration;
		opt<completion_item_t> m_CompletionItem;
		opt<completion_item_kind_t> m_CompletionItemKind;
		opt<bool> m_ContextSupport;
	};

	struct hover_t {

	private:
		opt<bool> m_DynamicRegistration;
		opt<std::vector<markup_kind>> m_ContentFormat;
	};

	struct signature_help_t {
		struct signature_information_t {

		private:
			opt<std::vector<markup_kind>> m_DocumentationFormat;
		};

	private:
		opt<bool> m_DynamicRegistration;
		opt<signature_information_t> m_SignatureInformation;
	};

	struct references_t {

	private:
		opt<bool> m_DynamicRegistration;
	};

	struct document_highlight_t {

	private:
		opt<bool> m_DynamicRegistration;
	};

	struct document_symbol_t {
		struct symbol_kind_t {

		private:
			opt<std::vector<symbol_kind>> m_ValueSet;
		};

	private:
		opt<bool> m_DynamicRegistration;
		opt<symbol_kind_t> m_SymbolKind;
		opt<bool> m_HierarchicalDocumentSymbolSupport;
	};

	struct formatting_t {

	private:
		opt<bool> m_DynamicRegistration;
	};

	struct range_formatting_t {

	private:
		opt<bool> m_DynamicRegistration;
	};

	struct on_type_formatting_t {

	private:
		opt<bool> m_DynamicRegistration;
	};

	struct definition_t {

	private:
		opt<bool> m_DynamicRegistration;
	};

	struct type_definition_t {

	private:
		opt<bool> m_DynamicRegistration;
	};

	struct implementation_t {

	private:
		opt<bool> m_DynamicRegistration;
	};

	struct code_action_t {
		struct code_action_literal_support_t {
			struct code_action_kind_t {

			private:
				std::vector<code_action_kind> m_ValueSet;
			};

		private:
			code_action_kind_t m_CodeActionKind;
		};

	private:
		opt<bool> m_DynamicRegistration;
		opt<code_action_literal_support_t> m_CodeActionLiteralSupport;
	};

	struct code_lens_t {

	private:
		opt<bool> m_DynamicRegistration;
	};

	struct document_link_t {

	private:
		opt<bool> m_DynamicRegistration;
	};

	struct color_provider_t {

	private:
		opt<bool> m_DynamicRegistration;
	};

	struct rename_t {

	private:
		opt<bool> m_DynamicRegistration;
		opt<bool> m_PrepareSupport;
	};

	struct publish_diagnostics_t {

	private:
		opt<bool> m_RelatedInformation;
	};

	struct folding_range_t {

	private:
		opt<bool> m_DynamicRegistration;
		opt<u32> m_RangeLimit;
		opt<bool> m_LineFoldingOnly;
	};

private:
	opt<synchronization_t> m_Synchronization;
	opt<completion_t> m_Completion;
	opt<hover_t> m_Hover;
	opt<signature_help_t> m_SignatureHelp;
	opt<references_t> m_References;
	opt<document_highlight_t> m_DocumentHighlight;
	opt<document_symbol_t> m_DocumentSymbol;
	opt<formatting_t> m_Formatting;
	opt<range_formatting_t> m_RangeFormatting;
	opt<on_type_formatting_t> m_OnTypeFormatting;
	opt<definition_t> m_Definition;
	opt<type_definition_t> m_TypeDefinition;
	opt<implementation_t> m_Implementation;
	opt<code_action_t> m_CodeAction;
	opt<code_lens_t> m_CodeLens;
	opt<document_link_t> m_DocumentLink;
	opt<color_provider_t> m_ColorProvider;
	opt<rename_t> m_Rename;
	opt<publish_diagnostics_t> m_PublishDiagnostics;
	opt<folding_range_t> m_FoldingRange;
};

/**
 * The ClientCapabilities structure used for initialization.
 */
struct client_capabilities {


private:
	opt<workspace_client_capabilities> m_Workspace;
	opt<text_document_client_capabilities> m_TextDocument;
	opt<json> m_Experimental;
};

/**
 * WorkspaceFolder.
 */
struct workspace_folder {

private:
	std::string m_URI;
	std::string m_Name;
};

/**
 * InitializeParams, the 'params' for the 'initialize' request.
 */
struct initialize_params {
	enum class trace {
		off, messages, verbose,
	};

private:
	opt<i32> m_ProcessID;
	opt<std::string> m_RootPath;
	opt<uri_type> m_RoorURI;
	opt<json> m_InitializationOptions;
	client_capabilities m_Capabilities;
	opt<trace> m_Trace;
	opt<std::vector<workspace_folder>> m_WorkspaceFolders;
};

}

#endif /* LSP_LSP_HPP */
