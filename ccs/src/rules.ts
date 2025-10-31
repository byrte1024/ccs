import * as vscode from 'vscode';

export interface Rule {
    name: string;
    description: string;
    check(document: vscode.TextDocument, line: number): vscode.Diagnostic[];
}

// Helper function: check if a line is within INSPECTION_IMMUNITY block
function isWithinImmunity(document: vscode.TextDocument, lineNumber: number): boolean {
    let inBlock = false;
    for (let i = 0; i <= lineNumber; i++) {
        const lineText = document.lineAt(i).text;
        if (lineText.includes("#define INSPECTION_IMMUNITY")) {
            inBlock = true;
        } else if (lineText.includes("#endif INSPECTION_IMMUNITY")) {
            inBlock = false;
        }
    }
    return inBlock;
}

// Rule 1: IMPL_FUNCTION / IMPLOTHER_FUNCTION must be followed by IMPL_HEADER within 3 lines
export const implHeaderRule: Rule = {
    name: "impl-function-header-rule",
    description: "Ensure IMPL_FUNCTION or IMPLOTHER_FUNCTION is followed by IMPL_HEADER within 3 lines, ignoring INSPECTION_IMMUNITY blocks",
    check(document, lineNumber) {
        const diagnostics: vscode.Diagnostic[] = [];
        const lineText = document.lineAt(lineNumber).text;

        if (isWithinImmunity(document, lineNumber)) {
            return diagnostics; // skip lines within immunity
        }

        if (lineText.includes("IMPL_FUNCTION") || lineText.includes("IMPLOTHER_FUNCTION")) {
            let found = false;
            for (let i = 1; i <= 3 && lineNumber + i < document.lineCount; i++) {
                const nextLine = document.lineAt(lineNumber + i).text;
                if (nextLine.includes("IMPL_HEADER")) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                const diagnostic = new vscode.Diagnostic(
                    new vscode.Range(lineNumber, 0, lineNumber, lineText.length),
                    "Missing IMPL_HEADER within 3 lines of IMPL_FUNCTION or IMPLOTHER_FUNCTION",
                    vscode.DiagnosticSeverity.Error
                );
                diagnostics.push(diagnostic);
            }
        }

        return diagnostics;
    },
};

// Rule 2: Cross-reference rule
export const crossReferenceRule: Rule = {
    name: "impl-cross-reference-rule",
    description: "Ensure that IMPL_FUNCTION(X) has FUNFIND_IMPL(X) and IMPLOTHER_FUNCTION(X) has FUNFIND_IMPLOTHER(X) in the same file",
    check(document, lineNumber) {
        const diagnostics: vscode.Diagnostic[] = [];
        const lineText = document.lineAt(lineNumber).text;

        if (isWithinImmunity(document, lineNumber)) {
            return diagnostics; // skip lines within immunity
        }

        const implMatch = lineText.match(/IMPL_FUNCTION\(([^)]+)\)/);
        if (implMatch) {
            const x = implMatch[1].trim();
            let found = false;
            for (let i = 0; i < document.lineCount; i++) {
                if (document.lineAt(i).text.includes(`FUNFIND_IMPL(${x})`)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                diagnostics.push(new vscode.Diagnostic(
                    new vscode.Range(lineNumber, 0, lineNumber, lineText.length),
                    `IMPL_FUNCTION(${x}) requires FUNFIND_IMPL(${x}) in the same file`,
                    vscode.DiagnosticSeverity.Error
                ));
            }
        }

        const otherMatch = lineText.match(/IMPLOTHER_FUNCTION\(([^)]+)\)/);
        if (otherMatch) {
            const x = otherMatch[1].trim();
            let found = false;
            for (let i = 0; i < document.lineCount; i++) {
                if (document.lineAt(i).text.includes(`FUNFIND_IMPLOTHER(${x})`)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                diagnostics.push(new vscode.Diagnostic(
                    new vscode.Range(lineNumber, 0, lineNumber, lineText.length),
                    `IMPLOTHER_FUNCTION(${x}) requires FUNFIND_IMPLOTHER(${x}) in the same file`,
                    vscode.DiagnosticSeverity.Error
                ));
            }
        }

        return diagnostics;
    }
};

// Export all rules here
export const rules: Rule[] = [implHeaderRule, crossReferenceRule];
