#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "parserInterpSP26.h"

using namespace std;

namespace {

int errorCount = 0;
bool pushedBack = false;
LexItem pushedToken;

string programName;
map<string, Token> symTable;
map<string, Value> symValue;
map<string, bool> symInitialized;
map<string, bool> symConst;

bool execFlag = true;
vector<Value> exprVals;

string LowerStr(const string& src)
{
    string out = src;
    transform(out.begin(), out.end(), out.begin(),
        [](unsigned char ch) { return static_cast<char>(tolower(ch)); });
    return out;
}

void ResetInterpreterState()
{
    errorCount = 0;
    pushedBack = false;
    programName.clear();
    symTable.clear();
    symValue.clear();
    symInitialized.clear();
    symConst.clear();
    execFlag = true;
    exprVals.clear();
}

void ParseError(int line, const string& msg)
{
    errorCount++;
    cout << line << ": " << msg << endl;
}

LexItem GetNextToken(istream& in, int& line)
{
    if (pushedBack) {
        pushedBack = false;
        return pushedToken;
    }
    return getNextToken(in, line);
}

void PushBackToken(const LexItem& tok)
{
    if (pushedBack) {
        abort();
    }
    pushedBack = true;
    pushedToken = tok;
}

Value DefaultValue(Token t)
{
    switch (t) {
        case INTEGER:
            return Value(0);
        case REAL:
            return Value(0.0);
        case BOOLEAN:
            return Value(false);
        case CHAR:
            return Value(char(0));
        case STRING:
            return Value(string(""));
        default:
            return Value();
    }
}

Token ValueTypeToToken(const Value& val)
{
    if (val.IsInt()) {
        return INTEGER;
    }
    if (val.IsReal()) {
        return REAL;
    }
    if (val.IsBool()) {
        return BOOLEAN;
    }
    if (val.IsChar()) {
        return CHAR;
    }
    if (val.IsString()) {
        return STRING;
    }
    return ERR;
}

bool IsProgramName(const string& name)
{
    return !programName.empty() && LowerStr(name) == programName;
}

bool IsDeclared(const string& name)
{
    return symTable.find(LowerStr(name)) != symTable.end();
}

bool IsCompatibleAssignment(Token lhsType, const Value& rhs)
{
    switch (lhsType) {
        case INTEGER:
            return rhs.IsInt();
        case REAL:
            return rhs.IsInt() || rhs.IsReal();
        case STRING:
            return rhs.IsString() || rhs.IsChar();
        case CHAR:
            return rhs.IsChar();
        case BOOLEAN:
            return rhs.IsBool();
        default:
            return false;
    }
}

Value CoerceAssignmentValue(Token lhsType, const Value& rhs)
{
    switch (lhsType) {
        case INTEGER:
            return Value(rhs.GetInt());
        case REAL:
            if (rhs.IsInt()) {
                return Value(static_cast<double>(rhs.GetInt()));
            }
            return Value(rhs.GetReal());
        case STRING:
            if (rhs.IsChar()) {
                string temp(1, rhs.GetChar());
                return Value(temp);
            }
            return Value(rhs.GetString());
        case CHAR:
            return Value(rhs.GetChar());
        case BOOLEAN:
            return Value(rhs.GetBool());
        default:
            return Value();
    }
}

void SetNumericPlaceholder(const Value& lhs, const Value& rhs, Value& result, Token op)
{
    if (lhs.IsInt() && rhs.IsInt() && op != DIV) {
        result = Value(0);
    }
    else {
        result = Value(0.0);
    }
}

bool ApplyUnary(Token op, const Value& operand, int line, Value& result)
{
    switch (op) {
        case PLUS:
            if (operand.IsInt()) {
                result = execFlag ? Value(operand.GetInt()) : Value(0);
                return true;
            }
            if (operand.IsReal()) {
                result = execFlag ? Value(operand.GetReal()) : Value(0.0);
                return true;
            }
            ParseError(line, "Illegal Operand Type for Sign/NOT Operator.");
            return false;

        case MINUS:
            if (operand.IsInt()) {
                result = execFlag ? -operand : Value(0);
                return true;
            }
            if (operand.IsReal()) {
                result = execFlag ? -operand : Value(0.0);
                return true;
            }
            if (operand.IsString() || operand.IsChar()) {
                result = execFlag ? operand.Trim() : Value(string(""));
                return true;
            }
            ParseError(line, "Illegal Operand Type for Sign/NOT Operator.");
            return false;

        case NOT:
            if (!operand.IsBool()) {
                ParseError(line, "Illegal operand type for NOT operator.");
                return false;
            }
            result = execFlag ? !operand : Value(false);
            return true;

        default:
            result = operand;
            return true;
    }
}

bool ApplyBinary(Token op, const Value& lhs, const Value& rhs, int line, Value& result)
{
    switch (op) {
        case PLUS:
            if ((lhs.IsString() || lhs.IsChar()) && (rhs.IsString() || rhs.IsChar())) {
                result = execFlag ? (lhs + rhs) : Value(string(""));
                return true;
            }
            if ((lhs.IsInt() || lhs.IsReal()) && (rhs.IsInt() || rhs.IsReal())) {
                if (execFlag) {
                    result = lhs + rhs;
                }
                else {
                    SetNumericPlaceholder(lhs, rhs, result, op);
                }
                return true;
            }
            ParseError(line, "Illegal operand types for an arithmetic or logical operator.");
            return false;

        case MINUS:
            if ((lhs.IsInt() || lhs.IsReal()) && (rhs.IsInt() || rhs.IsReal())) {
                if (execFlag) {
                    result = lhs - rhs;
                }
                else {
                    SetNumericPlaceholder(lhs, rhs, result, op);
                }
                return true;
            }
            ParseError(line, "Illegal operand types for an arithmetic or logical operator.");
            return false;

        case MULT:
            if ((lhs.IsInt() || lhs.IsReal()) && (rhs.IsInt() || rhs.IsReal())) {
                if (execFlag) {
                    result = lhs * rhs;
                }
                else {
                    SetNumericPlaceholder(lhs, rhs, result, op);
                }
                return true;
            }
            ParseError(line, "Illegal operand types for an arithmetic or logical operator.");
            return false;

        case DIV:
            if (!(lhs.IsInt() || lhs.IsReal()) || !(rhs.IsInt() || rhs.IsReal())) {
                ParseError(line, "Illegal operand types for an arithmetic or logical operator.");
                return false;
            }
            if (execFlag) {
                if ((rhs.IsInt() && rhs.GetInt() == 0) || (rhs.IsReal() && rhs.GetReal() == 0.0)) {
                    ParseError(line, "Run-Time Error-Illegal division by Zero");
                    return false;
                }
                result = lhs / rhs;
            }
            else {
                result = Value(0.0);
            }
            return true;

        case IDIV:
            if (!lhs.IsInt() || !rhs.IsInt()) {
                ParseError(line, "Invalid non-integer operands for the DIV operator.");
                return false;
            }
            if (execFlag) {
                if (rhs.GetInt() == 0) {
                    ParseError(line, "Run-Time Error-Illegal division by Zero");
                    return false;
                }
                result = lhs.idiv(rhs);
            }
            else {
                result = Value(0);
            }
            return true;

        case MOD:
            if (!lhs.IsInt() || !rhs.IsInt()) {
                ParseError(line, "Invalid non-integer operands for the MOD operator.");
                return false;
            }
            if (execFlag) {
                if (rhs.GetInt() == 0) {
                    ParseError(line, "Run-Time Error-Illegal division by Zero");
                    return false;
                }
                result = lhs % rhs;
            }
            else {
                result = Value(0);
            }
            return true;

        case AND:
        case OR:
            if (!lhs.IsBool() || !rhs.IsBool()) {
                ParseError(line, "Illegal operand types for an arithmetic or logical operator.");
                return false;
            }
            if (execFlag) {
                result = (op == AND) ? (lhs && rhs) : (lhs || rhs);
            }
            else {
                result = Value(false);
            }
            return true;

        case EQ:
            if ((lhs.IsInt() || lhs.IsReal()) && (rhs.IsInt() || rhs.IsReal())) {
                result = execFlag ? (lhs == rhs) : Value(false);
                return true;
            }
            if ((lhs.IsString() || lhs.IsChar()) && (rhs.IsString() || rhs.IsChar())) {
                result = execFlag ? (lhs == rhs) : Value(false);
                return true;
            }
            if (lhs.IsBool() && rhs.IsBool()) {
                result = execFlag ? (lhs == rhs) : Value(false);
                return true;
            }
            ParseError(line, "Run-Time Error-Illegal Mixed Type Operands");
            return false;

        case LTHAN:
        case GTHAN:
            if ((lhs.IsInt() || lhs.IsReal()) && (rhs.IsInt() || rhs.IsReal())) {
                if (execFlag) {
                    result = (op == LTHAN) ? (lhs < rhs) : (lhs > rhs);
                }
                else {
                    result = Value(false);
                }
                return true;
            }
            if ((lhs.IsString() || lhs.IsChar()) && (rhs.IsString() || rhs.IsChar())) {
                if (execFlag) {
                    result = (op == LTHAN) ? (lhs < rhs) : (lhs > rhs);
                }
                else {
                    result = Value(false);
                }
                return true;
            }
            ParseError(line, "Run-Time Error-Illegal Mixed Type Operands");
            return false;

        default:
            return false;
    }
}

bool GetStoredValue(const string& name, int line, Value& retVal)
{
    string key = LowerStr(name);
    if (!IsDeclared(key)) {
        ParseError(line, "Undeclared Variable: " + name);
        return false;
    }

    if (symConst[key]) {
        retVal = symValue[key];
        return true;
    }

    if (!symInitialized[key]) {
        if (execFlag) {
            ParseError(line, "Using a variable before being assinged: " + name);
            return false;
        }
        retVal = DefaultValue(symTable[key]);
        return true;
    }

    retVal = symValue[key];
    return true;
}

bool ReadIntegerToken(const string& text, size_t& pos, int& outVal)
{
    while (pos < text.size() && isspace(static_cast<unsigned char>(text[pos]))) {
        pos++;
    }
    if (pos >= text.size()) {
        return false;
    }

    size_t start = pos;
    if (text[pos] == '+' || text[pos] == '-') {
        pos++;
    }
    if (pos >= text.size() || !isdigit(static_cast<unsigned char>(text[pos]))) {
        return false;
    }
    while (pos < text.size() && !isspace(static_cast<unsigned char>(text[pos]))) {
        pos++;
    }

    string token = text.substr(start, pos - start);
    for (size_t i = (token[0] == '+' || token[0] == '-') ? 1 : 0; i < token.size(); i++) {
        if (!isdigit(static_cast<unsigned char>(token[i]))) {
            return false;
        }
    }

    try {
        outVal = stoi(token);
    }
    catch (...) {
        return false;
    }
    return true;
}

bool ReadRealToken(const string& text, size_t& pos, double& outVal)
{
    while (pos < text.size() && isspace(static_cast<unsigned char>(text[pos]))) {
        pos++;
    }
    if (pos >= text.size()) {
        return false;
    }

    size_t start = pos;
    if (text[pos] == '+' || text[pos] == '-') {
        pos++;
    }
    bool seenDigit = false;
    while (pos < text.size() && isdigit(static_cast<unsigned char>(text[pos]))) {
        seenDigit = true;
        pos++;
    }
    if (pos < text.size() && text[pos] == '.') {
        pos++;
        while (pos < text.size() && isdigit(static_cast<unsigned char>(text[pos]))) {
            seenDigit = true;
            pos++;
        }
    }
    if (!seenDigit) {
        return false;
    }
    if (pos < text.size() && (text[pos] == 'e' || text[pos] == 'E')) {
        pos++;
        if (pos < text.size() && (text[pos] == '+' || text[pos] == '-')) {
            pos++;
        }
        bool expDigit = false;
        while (pos < text.size() && isdigit(static_cast<unsigned char>(text[pos]))) {
            expDigit = true;
            pos++;
        }
        if (!expDigit) {
            return false;
        }
    }
    if (pos < text.size() && !isspace(static_cast<unsigned char>(text[pos]))) {
        return false;
    }

    string token = text.substr(start, pos - start);
    try {
        outVal = stod(token);
    }
    catch (...) {
        return false;
    }
    return true;
}

bool ReadBooleanToken(const string& text, size_t& pos, bool& outVal)
{
    while (pos < text.size() && isspace(static_cast<unsigned char>(text[pos]))) {
        pos++;
    }
    if (pos >= text.size()) {
        return false;
    }

    size_t start = pos;
    while (pos < text.size() && !isspace(static_cast<unsigned char>(text[pos]))) {
        pos++;
    }

    string token = LowerStr(text.substr(start, pos - start));
    if (token == "true") {
        outVal = true;
        return true;
    }
    if (token == "false") {
        outVal = false;
        return true;
    }
    return false;
}

string BuildOutputText(const vector<Value>& values)
{
    ostringstream out;
    for (size_t i = 0; i < values.size(); i++) {
        out << values[i];
    }
    return out.str();
}

bool ParseType(istream& in, int& line, Token& typeTok)
{
    LexItem tok = GetNextToken(in, line);
    switch (tok.GetToken()) {
        case INTEGER:
        case REAL:
        case BOOLEAN:
        case CHAR:
        case STRING:
            typeTok = tok.GetToken();
            return true;
        default:
            ParseError(line, "Incorrect Data Type in Declaration.");
            return false;
    }
}

} // namespace

int ErrCount()
{
    return errorCount;
}

bool Prog(istream& in, int& line)
{
    ResetInterpreterState();

    LexItem tok = GetNextToken(in, line);
    if (tok != PROGRAM) {
        ParseError(line, "Missing PROGRAM keyword.");
        return false;
    }

    tok = GetNextToken(in, line);
    if (tok != IDENT) {
        ParseError(line, "Missing Program Name.");
        return false;
    }

    programName = LowerStr(tok.GetLexeme());

    tok = GetNextToken(in, line);
    if (tok != SEMICOL) {
        ParseError(line, "Missing semicolon after Program Name.");
        return false;
    }

    if (!Block(in, line)) {
        ParseError(line, "Incorrect Program Body.");
        return false;
    }

    tok = GetNextToken(in, line);
    if (tok != DOT) {
        ParseError(line, "Missing period at end of Program.");
        return false;
    }

    tok = GetNextToken(in, line);
    if (tok != DONE) {
        ParseError(line, "Extraneous input after Program End.");
        return false;
    }

    cout << "\n\nDONE" << endl;
    return true;
}

bool Block(istream& in, int& line)
{
    LexItem tok = GetNextToken(in, line);
    if (tok == CONST || tok == VAR) {
        PushBackToken(tok);
        if (!DeclPart(in, line)) {
            ParseError(line, "Incorrect Declaration Part.");
            return false;
        }
    }
    else {
        PushBackToken(tok);
    }

    if (!CompStmt(in, line)) {
        ParseError(line, "Incorrect Program Body.");
        return false;
    }

    return true;
}

bool DeclPart(istream& in, int& line)
{
    LexItem tok = GetNextToken(in, line);
    if (tok == CONST) {
        PushBackToken(tok);
        if (!ConstPart(in, line)) {
            return false;
        }
        tok = GetNextToken(in, line);
        if (tok == VAR) {
            PushBackToken(tok);
            if (!VarPart(in, line)) {
                return false;
            }
        }
        else {
            PushBackToken(tok);
        }
        return true;
    }

    if (tok == VAR) {
        PushBackToken(tok);
        return VarPart(in, line);
    }

    PushBackToken(tok);
    return true;
}

bool ConstPart(istream& in, int& line)
{
    LexItem tok = GetNextToken(in, line);
    if (tok != CONST) {
        ParseError(line, "Missing CONST keyword.");
        return false;
    }

    if (!ConstDef(in, line)) {
        ParseError(line, "Syntactic error in Declaration Block.");
        return false;
    }

    tok = GetNextToken(in, line);
    while (tok == SEMICOL) {
        LexItem next = GetNextToken(in, line);
        if (next != IDENT) {
            PushBackToken(next);
            return true;
        }
        PushBackToken(next);
        if (!ConstDef(in, line)) {
            ParseError(line, "Syntactic error in Declaration Block.");
            return false;
        }
        tok = GetNextToken(in, line);
    }

    ParseError(line, "Missing semicolon in Const Declaration.");
    return false;
}

bool ConstDef(istream& in, int& line)
{
    LexItem idtok = GetNextToken(in, line);
    if (idtok != IDENT) {
        ParseError(line, "Missing Identifier in Const Declaration.");
        return false;
    }

    string key = LowerStr(idtok.GetLexeme());
    if (IsProgramName(key) || IsDeclared(key)) {
        ParseError(line, "Redefinition of Identifier: " + idtok.GetLexeme());
        return false;
    }

    LexItem tok = GetNextToken(in, line);
    if (tok != EQ) {
        ParseError(line, "Missing equal sign in Const Definition.");
        return false;
    }

    Value val;
    if (!Expr(in, line, val)) {
        ParseError(line, "Missing Expression in Const Definition.");
        return false;
    }

    symTable[key] = ValueTypeToToken(val);
    symValue[key] = val;
    symInitialized[key] = true;
    symConst[key] = true;
    return true;
}

bool VarPart(istream& in, int& line)
{
    LexItem tok = GetNextToken(in, line);
    if (tok != VAR) {
        ParseError(line, "Missing VAR keyword.");
        return false;
    }

    if (!DeclStmt(in, line)) {
        ParseError(line, "Syntactic error in Declaration Block.");
        return false;
    }

    tok = GetNextToken(in, line);
    while (tok == SEMICOL) {
        LexItem next = GetNextToken(in, line);
        if (next != IDENT) {
            PushBackToken(next);
            return true;
        }
        PushBackToken(next);
        if (!DeclStmt(in, line)) {
            ParseError(line, "Syntactic error in Declaration Block.");
            return false;
        }
        tok = GetNextToken(in, line);
    }

    ParseError(line, "Missing semicolon in Variable Declaration.");
    return false;
}

bool IdentList(istream& in, int& line, vector<string>& idList)
{
    idList.clear();

    LexItem tok = GetNextToken(in, line);
    if (tok != IDENT) {
        ParseError(line, "Missing Identifier.");
        return false;
    }

    string key = LowerStr(tok.GetLexeme());
    if (IsProgramName(key) || IsDeclared(key)) {
        ParseError(line, "Variable Redefinition: " + tok.GetLexeme());
        return false;
    }
    idList.push_back(key);

    tok = GetNextToken(in, line);
    while (tok == COMMA) {
        tok = GetNextToken(in, line);
        if (tok != IDENT) {
            ParseError(line, "Missing Identifier after comma.");
            return false;
        }
        key = LowerStr(tok.GetLexeme());
        if (IsProgramName(key) || IsDeclared(key) ||
            find(idList.begin(), idList.end(), key) != idList.end()) {
            ParseError(line, "Variable Redefinition: " + tok.GetLexeme());
            return false;
        }
        idList.push_back(key);
        tok = GetNextToken(in, line);
    }

    PushBackToken(tok);
    return true;
}

bool DeclStmt(istream& in, int& line)
{
    vector<string> idList;
    if (!IdentList(in, line, idList)) {
        return false;
    }

    LexItem tok = GetNextToken(in, line);
    if (tok != COLON) {
        ParseError(line, "Missing colon in Declaration Statement.");
        return false;
    }

    Token typeTok = ERR;
    if (!ParseType(in, line, typeTok)) {
        return false;
    }

    for (size_t i = 0; i < idList.size(); i++) {
        symTable[idList[i]] = typeTok;
        symConst[idList[i]] = false;
        symInitialized[idList[i]] = false;
    }

    tok = GetNextToken(in, line);
    if (tok == ASSOP) {
        Value val;
        if (!Expr(in, line, val)) {
            ParseError(line, "Incorrect initialization expression.");
            return false;
        }

        if (!IsCompatibleAssignment(typeTok, val)) {
            ParseError(line, "Illegal expression type for the assigned variable.");
            return false;
        }

        Value stored = CoerceAssignmentValue(typeTok, val);
        for (size_t i = 0; i < idList.size(); i++) {
            symValue[idList[i]] = stored;
            symInitialized[idList[i]] = true;
        }
        return true;
    }

    PushBackToken(tok);
    return true;
}

bool Stmt(istream& in, int& line)
{
    LexItem tok = GetNextToken(in, line);
    PushBackToken(tok);

    if (tok == IDENT || tok == READLN || tok == WRITE || tok == WRITELN) {
        if (!SimpleStmt(in, line)) {
            ParseError(line, "Syntactic error in the statement.");
            return false;
        }
        return true;
    }

    if (tok == IF || tok == BEGIN) {
        if (!StructuredStmt(in, line)) {
            ParseError(line, "Syntactic error in the statement.");
            return false;
        }
        return true;
    }

    ParseError(line, "Invalid Statement.");
    return false;
}

bool SimpleStmt(istream& in, int& line)
{
    LexItem tok = GetNextToken(in, line);
    PushBackToken(tok);

    if (tok == IDENT) {
        if (!AssignStmt(in, line)) {
            ParseError(line, "Incorrect Simple Statement.");
            return false;
        }
        return true;
    }

    if (tok == READLN) {
        if (!ReadLnStmt(in, line)) {
            ParseError(line, "Incorrect Simple Statement.");
            return false;
        }
        return true;
    }

    if (tok == WRITE) {
        if (!WriteStmt(in, line)) {
            ParseError(line, "Incorrect Simple Statement.");
            return false;
        }
        return true;
    }

    if (tok == WRITELN) {
        if (!WriteLnStmt(in, line)) {
            ParseError(line, "Incorrect Simple Statement.");
            return false;
        }
        return true;
    }

    ParseError(line, "Invalid Simple Statement.");
    return false;
}

bool StructuredStmt(istream& in, int& line)
{
    LexItem tok = GetNextToken(in, line);
    PushBackToken(tok);

    if (tok == IF) {
        int errLine = tok.GetLinenum();
        if (!IfStmt(in, line)) {
            ParseError(errLine, "Incorrect Structured Statement.");
            return false;
        }
        return true;
    }

    if (tok == BEGIN) {
        int errLine = tok.GetLinenum();
        if (!CompStmt(in, line)) {
            ParseError(errLine, "Incorrect Structured Statement.");
            return false;
        }
        return true;
    }

    ParseError(line, "Invalid Structured Statement.");
    return false;
}

bool CompStmt(istream& in, int& line)
{
    LexItem tok = GetNextToken(in, line);
    if (tok != BEGIN) {
        ParseError(line, "Missing BEGIN in Compound Statement.");
        return false;
    }

    if (!Stmt(in, line)) {
        return false;
    }

    tok = GetNextToken(in, line);
    while (tok == SEMICOL) {
        LexItem next = GetNextToken(in, line);
        if (next == END) {
            return true;
        }
        PushBackToken(next);
        if (!Stmt(in, line)) {
            return false;
        }
        tok = GetNextToken(in, line);
    }

    if (tok != END) {
        ParseError(line, "Missing END in Compound Statement.");
        return false;
    }

    return true;
}

bool WriteLnStmt(istream& in, int& line)
{
    LexItem tok = GetNextToken(in, line);
    if (tok != WRITELN) {
        ParseError(line, "Missing WRITELN keyword.");
        return false;
    }

    tok = GetNextToken(in, line);
    if (tok != LPAREN) {
        ParseError(line, "Missing Left Parenthesis for WriteLn statement.");
        return false;
    }

    if (!ExprList(in, line)) {
        ParseError(line, "Missing expression list for WriteLn statement.");
        return false;
    }

    tok = GetNextToken(in, line);
    if (tok != RPAREN) {
        ParseError(line, "Missing Right Parenthesis for WriteLn statement.");
        return false;
    }

    if (execFlag) {
        string out = BuildOutputText(exprVals);
        while (!out.empty() && out.back() == ' ') {
            out.pop_back();
        }
        cout << out << endl;
    }

    return true;
}

bool WriteStmt(istream& in, int& line)
{
    LexItem tok = GetNextToken(in, line);
    if (tok != WRITE) {
        ParseError(line, "Missing WRITE keyword.");
        return false;
    }

    tok = GetNextToken(in, line);
    if (tok != LPAREN) {
        ParseError(line, "Missing Left Parenthesis for Write statement.");
        return false;
    }

    if (!ExprList(in, line)) {
        ParseError(line, "Missing expression list for Write statement.");
        return false;
    }

    tok = GetNextToken(in, line);
    if (tok != RPAREN) {
        ParseError(line, "Missing Right Parenthesis for Write statement.");
        return false;
    }

    if (execFlag) {
        cout << BuildOutputText(exprVals);
    }

    return true;
}

bool Variable(istream& in, int& line, LexItem& idtok)
{
    idtok = GetNextToken(in, line);
    if (idtok != IDENT) {
        ParseError(line, "Missing Left-Hand Side Variable in Assignment statement.");
        return false;
    }

    string name = idtok.GetLexeme();
    string key = LowerStr(name);
    if (IsProgramName(key)) {
        ParseError(line, "Illegal use of program name as a variable: " + name);
        return false;
    }
    if (!IsDeclared(key)) {
        ParseError(line, "Undeclared Variable: " + name);
        return false;
    }
    if (symConst[key]) {
        ParseError(line, "Illegal use of constant name as a variable: " + name);
        return false;
    }

    return true;
}

bool VariableList(istream& in, int& line, vector<string>& varList)
{
    varList.clear();

    LexItem idtok;
    if (!Variable(in, line, idtok)) {
        return false;
    }
    varList.push_back(LowerStr(idtok.GetLexeme()));

    LexItem tok = GetNextToken(in, line);
    while (tok == COMMA) {
        if (!Variable(in, line, idtok)) {
            return false;
        }
        varList.push_back(LowerStr(idtok.GetLexeme()));
        tok = GetNextToken(in, line);
    }

    PushBackToken(tok);
    return true;
}

bool ReadLnStmt(istream& in, int& line)
{
    LexItem tok = GetNextToken(in, line);
    if (tok != READLN) {
        ParseError(line, "Missing READLN keyword.");
        return false;
    }

    tok = GetNextToken(in, line);
    if (tok != LPAREN) {
        ParseError(line, "Missing Left Parenthesis for ReadLn statement.");
        return false;
    }

    vector<string> vars;
    if (!VariableList(in, line, vars)) {
        ParseError(line, "Missing variable list for ReadLn statement.");
        return false;
    }

    tok = GetNextToken(in, line);
    if (tok != RPAREN) {
        ParseError(line, "Missing Right Parenthesis for ReadLn statement.");
        return false;
    }

    if (!execFlag) {
        return true;
    }

    string inputLine;
    if (!getline(cin, inputLine)) {
        ParseError(line, "Run-Time Error-Read failure.");
        return false;
    }

    size_t pos = 0;
    for (size_t i = 0; i < vars.size(); i++) {
        Token type = symTable[vars[i]];
        if (type == INTEGER) {
            int ival = 0;
            if (!ReadIntegerToken(inputLine, pos, ival)) {
                ParseError(line, "Run-Time Error-Illegal Integer Input.");
                return false;
            }
            symValue[vars[i]] = Value(ival);
            symInitialized[vars[i]] = true;
        }
        else if (type == REAL) {
            double rval = 0.0;
            if (!ReadRealToken(inputLine, pos, rval)) {
                ParseError(line, "Run-Time Error-Illegal Real Input.");
                return false;
            }
            symValue[vars[i]] = Value(rval);
            symInitialized[vars[i]] = true;
        }
        else if (type == BOOLEAN) {
            bool bval = false;
            if (!ReadBooleanToken(inputLine, pos, bval)) {
                ParseError(line, "Run-Time Error-Illegal Boolean Input.");
                return false;
            }
            symValue[vars[i]] = Value(bval);
            symInitialized[vars[i]] = true;
        }
        else if (type == CHAR) {
            if (pos >= inputLine.size()) {
                ParseError(line, "Run-Time Error-Illegal Character Input.");
                return false;
            }
            symValue[vars[i]] = Value(inputLine[pos]);
            symInitialized[vars[i]] = true;
            pos++;
        }
        else if (type == STRING) {
            symValue[vars[i]] = Value(inputLine.substr(pos));
            symInitialized[vars[i]] = true;
            pos = inputLine.size();
        }
    }

    return true;
}

bool AssignStmt(istream& in, int& line)
{
    LexItem idtok;
    if (!Variable(in, line, idtok)) {
        ParseError(line, "Missing Left-Hand Side Variable in Assignment statement.");
        return false;
    }

    LexItem tok = GetNextToken(in, line);
    if (tok != ASSOP) {
        ParseError(line, "Missing Assignment Operator.");
        return false;
    }

    Value val;
    if (!Expr(in, line, val)) {
        ParseError(line, "Missing Expression in Assignment Statement.");
        return false;
    }

    string key = LowerStr(idtok.GetLexeme());
    Token lhsType = symTable[key];
    if (!IsCompatibleAssignment(lhsType, val)) {
        ParseError(line, "Illegal expression type for the assigned variable.");
        return false;
    }

    if (execFlag) {
        symValue[key] = CoerceAssignmentValue(lhsType, val);
        symInitialized[key] = true;
    }

    return true;
}

bool IfStmt(istream& in, int& line)
{
    LexItem tok = GetNextToken(in, line);
    if (tok != IF) {
        ParseError(line, "Missing IF keyword.");
        return false;
    }

    int ifLine = tok.GetLinenum();
    Value cond;
    if (!Expr(in, line, cond)) {
        ParseError(line, "Missing if statement Logic Expression.");
        return false;
    }

    tok = GetNextToken(in, line);
    if (tok != THEN) {
        ParseError(line, "Missing THEN keyword.");
        return false;
    }

    if (!cond.IsBool()) {
        ParseError(ifLine, "Run-Time Error-Illegal Type for If statement condition.");
        bool parentExec = execFlag;
        execFlag = false;
        (void)Stmt(in, line);
        execFlag = parentExec;
        return false;
    }

    bool parentExec = execFlag;
    bool condVal = parentExec ? cond.GetBool() : false;

    execFlag = parentExec && condVal;
    if (!Stmt(in, line)) {
        execFlag = parentExec;
        return false;
    }
    execFlag = parentExec;

    tok = GetNextToken(in, line);
    if (tok == ELSE) {
        execFlag = parentExec && !condVal;
        if (!Stmt(in, line)) {
            execFlag = parentExec;
            return false;
        }
        execFlag = parentExec;
    }
    else {
        PushBackToken(tok);
    }

    return true;
}

bool ExprList(istream& in, int& line)
{
    exprVals.clear();

    Value val;
    if (!Expr(in, line, val)) {
        ParseError(line, "Missing Expression.");
        return false;
    }
    exprVals.push_back(val);

    LexItem tok = GetNextToken(in, line);
    while (tok == COMMA) {
        if (!Expr(in, line, val)) {
            ParseError(line, "Missing Expression.");
            return false;
        }
        exprVals.push_back(val);
        tok = GetNextToken(in, line);
    }

    PushBackToken(tok);
    return true;
}

bool Expr(istream& in, int& line, Value& retVal)
{
    if (!SimpleExpr(in, line, retVal)) {
        return false;
    }

    LexItem tok = GetNextToken(in, line);
    if (tok == EQ || tok == LTHAN || tok == GTHAN) {
        Value rhs;
        if (!SimpleExpr(in, line, rhs)) {
            ParseError(line, "Missing operand after operator.");
            return false;
        }
        return ApplyBinary(tok.GetToken(), retVal, rhs, line, retVal);
    }

    PushBackToken(tok);
    return true;
}

bool SimpleExpr(istream& in, int& line, Value& retVal)
{
    if (!Term(in, line, retVal)) {
        return false;
    }

    LexItem tok = GetNextToken(in, line);
    while (tok == PLUS || tok == MINUS || tok == OR) {
        Value rhs;
        if (!Term(in, line, rhs)) {
            ParseError(line, "Missing operand after operator.");
            return false;
        }
        if (!ApplyBinary(tok.GetToken(), retVal, rhs, line, retVal)) {
            return false;
        }
        tok = GetNextToken(in, line);
    }

    PushBackToken(tok);
    return true;
}

bool Term(istream& in, int& line, Value& retVal)
{
    if (!SFactor(in, line, retVal)) {
        return false;
    }

    LexItem tok = GetNextToken(in, line);
    while (tok == MULT || tok == DIV || tok == IDIV || tok == MOD || tok == AND) {
        Value rhs;
        if (!SFactor(in, line, rhs)) {
            ParseError(line, "Missing operand after operator.");
            return false;
        }
        if (!ApplyBinary(tok.GetToken(), retVal, rhs, line, retVal)) {
            return false;
        }
        tok = GetNextToken(in, line);
    }

    PushBackToken(tok);
    return true;
}

bool SFactor(istream& in, int& line, Value& retVal)
{
    LexItem tok = GetNextToken(in, line);
    int sign = 0;

    if (tok == PLUS) {
        sign = PLUS;
    }
    else if (tok == MINUS) {
        sign = MINUS;
    }
    else if (tok == NOT) {
        sign = NOT;
    }
    else {
        PushBackToken(tok);
    }

    return Factor(in, line, sign, retVal);
}

bool Factor(istream& in, int& line, int sign, Value& retVal)
{
    LexItem tok = GetNextToken(in, line);

    switch (tok.GetToken()) {
        case IDENT: {
            string name = tok.GetLexeme();
            string key = LowerStr(name);
            if (IsProgramName(key)) {
                ParseError(line, "Illegal use of program name as a variable: " + name);
                return false;
            }
            if (!GetStoredValue(name, line, retVal)) {
                return false;
            }
            break;
        }

        case ICONST:
            retVal = Value(stoi(tok.GetLexeme()));
            break;

        case RCONST:
            retVal = Value(stod(tok.GetLexeme()));
            break;

        case SCONST:
            retVal = Value(tok.GetLexeme());
            break;

        case BCONST:
            retVal = Value(LowerStr(tok.GetLexeme()) == "true");
            break;

        case CCONST:
            retVal = Value(tok.GetLexeme()[0]);
            break;

        case LPAREN:
            if (!Expr(in, line, retVal)) {
                ParseError(line, "Missing expression after Left Parenthesis.");
                return false;
            }
            tok = GetNextToken(in, line);
            if (tok != RPAREN) {
                ParseError(line, "Missing Right Parenthesis.");
                return false;
            }
            break;

        default:
            ParseError(line, "Missing Factor.");
            return false;
    }

    if (sign != 0) {
        if (!ApplyUnary(static_cast<Token>(sign), retVal, line, retVal)) {
            return false;
        }
    }

    return true;
}
