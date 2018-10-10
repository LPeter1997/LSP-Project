'use strict';
// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
import * as path from 'path';
import * as vscode from 'vscode';
import {
    LanguageClient,
    LanguageClientOptions,
    ServerOptions,
    TransportKind
} from 'vscode-languageclient';

let client: LanguageClient;

// this method is called when your extension is activated
// your extension is activated the very first time the command is executed
export function activate(context: vscode.ExtensionContext) {

    // The command to run the language server
    let command = context.asAbsolutePath(
        path.join('out', 'langserver_test.exe')
    );

    // Create the server runner options
    let serverOptions: ServerOptions = {
        command: command,
    };
    // Client options
    let clientOptions: LanguageClientOptions = {
        documentSelector:  [{ scheme: 'file', language: 'plaintext' }],
    };

    // Create the client instance
    client = new LanguageClient(
        'languageServerExample',
        'Language Server Example',
        serverOptions,
        clientOptions
    );

    // Start client (launches server too)
    client.start();
}

// this method is called when your extension is deactivated
export function deactivate(): Thenable<void> {
    if (!client) {
        return undefined;
    }
    return client.stop();
}