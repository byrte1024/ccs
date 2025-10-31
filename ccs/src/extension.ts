import * as vscode from 'vscode';
import { rules } from './rules';

export function activate(context: vscode.ExtensionContext) {
    const diagnosticsCollection = vscode.languages.createDiagnosticCollection('macro-checker');
    context.subscriptions.push(diagnosticsCollection);

    // Function to run all rules
    const runDiagnostics = (document: vscode.TextDocument) => {
        if (!document.fileName.endsWith('.c') && !document.fileName.endsWith('.h')) {
            return;
        }

        const diagnostics: vscode.Diagnostic[] = [];

        for (let line = 0; line < document.lineCount; line++) {
            for (const rule of rules) {
                diagnostics.push(...rule.check(document, line));
            }
        }

        diagnosticsCollection.set(document.uri, diagnostics);
    };

    // Run on open + save + change
    vscode.workspace.onDidOpenTextDocument(runDiagnostics, null, context.subscriptions);
    vscode.workspace.onDidSaveTextDocument(runDiagnostics, null, context.subscriptions);
    vscode.workspace.onDidChangeTextDocument(event => runDiagnostics(event.document), null, context.subscriptions);

    // Run initially for currently open file
    if (vscode.window.activeTextEditor) {
        runDiagnostics(vscode.window.activeTextEditor.document);
    }

    console.log('Macro Checker extension activated');
}

export function deactivate() {}
