#include "parserSP26.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

using namespace std;

map<string, bool> defVar;
map<string, bool> defConst;

namespace Parser {
	bool pushed_back = false;
	LexItem pushed_token;

	static LexItem GetNextToken(istream& in, int& line) {
		if (pushed_back) {
			pushed_back = false;
			return pushed_token;
		}
		return getNextToken(in, line);
	}

	static void PushBackToken(LexItem& t) {
		if (pushed_back) {
			abort();
		}
		pushed_back = true;
		pushed_token = t;
	}
}

static int error_count = 0;
static int last_error_line = 0;
static string progName;

void ParseError(int line, string msg);

static bool IsTypeToken(Token tt)
{
	return tt == INTEGER || tt == REAL || tt == BOOLEAN || tt == CHAR || tt == STRING;
}

static bool IsRelOp(Token tt)
{
	return tt == EQ || tt == LTHAN || tt == GTHAN;
}

static bool IsSimpleStmtStart(Token tt)
{
	return tt == IDENT || tt == READLN || tt == WRITELN || tt == WRITE;
}

static bool IsStructuredStmtStart(Token tt)
{
	return tt == IF || tt == BEGIN;
}

static bool SeenInCurrentList(const vector<string>& names, const string& name)
{
	return find(names.begin(), names.end(), name) != names.end();
}

static void PrintDeclaredNames(const map<string, bool>& table)
{
	bool first = true;
	for (const auto& entry : table) {
		if (!first) {
			cout << ", ";
		}
		cout << entry.first;
		first = false;
	}
	cout << endl;
}

static int CurrentErrorLine(int fallback)
{
	return last_error_line != 0 ? last_error_line : fallback;
}

static bool ReportLexicalError(const LexItem& tok)
{
	if (tok != ERR) {
		return false;
	}

	++error_count;
	last_error_line = tok.GetLinenum();
	cout << tok.GetLinenum() << ": Unrecognized Input Pattern." << endl;
	cout << "(" << tok.GetLexeme() << ")" << endl;
	return true;
}

static bool ParseIdentifierListInternal(istream& in, int& line, vector<string>& ids)
{
	LexItem tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok != IDENT) {
		return false;
	}

	while (true) {
		string name = tok.GetLexeme();
		if (defConst.count(name) != 0) {
			ParseError(tok.GetLinenum(), "Illegal use of a constant name as a variable: " + name);
			return false;
		}
		if (defVar.count(name) != 0 || SeenInCurrentList(ids, name)) {
			ParseError(tok.GetLinenum(), "Variable Redefinition: " + name);
			return false;
		}
		ids.push_back(name);

		tok = Parser::GetNextToken(in, line);
		if (ReportLexicalError(tok)) {
			return false;
		}

		if (tok == COMMA) {
			tok = Parser::GetNextToken(in, line);
			if (ReportLexicalError(tok)) {
				return false;
			}
			if (tok != IDENT) {
				return false;
			}
			continue;
		}

		if (tok == IDENT) {
			ParseError(tok.GetLinenum(), "Missing comma in declaration statement.");
			return false;
		}

		Parser::PushBackToken(tok);
		return true;
	}
}

static bool ParseVariableInternal(istream& in, int& line, string& name)
{
	LexItem tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok != IDENT) {
		return false;
	}

	name = tok.GetLexeme();
	if (defConst.count(name) != 0) {
		ParseError(tok.GetLinenum(), "Illegal use of a constant name as a variable: " + name);
		return false;
	}
	if (defVar.count(name) == 0) {
		ParseError(tok.GetLinenum(), "Undeclared Variable: " + name);
		return false;
	}

	return true;
}

static bool ParseVarListInternal(istream& in, int& line, vector<string>& vars)
{
	string name;
	if (!ParseVariableInternal(in, line, name)) {
		return false;
	}
	vars.push_back(name);

	while (true) {
		LexItem tok = Parser::GetNextToken(in, line);
		if (ReportLexicalError(tok)) {
			return false;
		}
		if (tok != COMMA) {
			Parser::PushBackToken(tok);
			return true;
		}

		if (!ParseVariableInternal(in, line, name)) {
			return false;
		}
		vars.push_back(name);
	}
}

static bool ParseStmtInternal(istream& in, int& line, bool reportStmtError)
{
	LexItem tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}

	if (tok == ELSE) {
		ParseError(tok.GetLinenum(), "Illegal Else-clause.");
		return false;
	}

	Parser::PushBackToken(tok);
	if (IsSimpleStmtStart(tok.GetToken())) {
		if (!SimpleStmt(in, line)) {
			if (reportStmtError) {
				ParseError(CurrentErrorLine(line), "Syntactic error in the statement.");
			}
			return false;
		}
		return true;
	}

	if (IsStructuredStmtStart(tok.GetToken())) {
		if (!StructuredStmt(in, line)) {
			if (reportStmtError) {
				ParseError(CurrentErrorLine(line), "Syntactic error in the statement.");
			}
			return false;
		}
		return true;
	}

	return false;
}

int ErrCount()
{
	return error_count;
}

void ParseError(int line, string msg)
{
	++error_count;
	last_error_line = line;
	cout << line << ": " << msg << endl;
}

bool Prog(istream& in, int& line)
{
	defVar.clear();
	defConst.clear();
	progName.clear();
	last_error_line = 0;
	Parser::pushed_back = false;

	LexItem tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok != PROGRAM) {
		ParseError(tok.GetLinenum(), "Missing PROGRAM keyword.");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok != IDENT) {
		ParseError(tok.GetLinenum(), "Missing Program Name.");
		return false;
	}
	progName = tok.GetLexeme();

	tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok != SEMICOL) {
		ParseError(tok.GetLinenum(), "Missing semicolon after Program Name.");
		return false;
	}

	if (!Block(in, line)) {
		ParseError(CurrentErrorLine(line), "Incorrect Program Body.");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok != DOT) {
		ParseError(tok.GetLinenum(), "Missing period at end of program.");
		return false;
	}

	cout << "Program Name: " << progName << endl;
	cout << "Declared Variables:" << endl;
	PrintDeclaredNames(defVar);
	cout << endl;
	cout << "Defined Constants:" << endl;
	PrintDeclaredNames(defConst);
	cout << endl;
	cout << "DONE" << endl;

	return true;
}

bool Block(istream& in, int& line)
{
	LexItem tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}

	if (tok == CONST || tok == VAR) {
		Parser::PushBackToken(tok);
		if (!DeclPart(in, line)) {
			return false;
		}
	}
	else {
		Parser::PushBackToken(tok);
	}

	if (!CompStmt(in, line)) {
		ParseError(CurrentErrorLine(line), "Incorrect Program Body.");
		return false;
	}

	return true;
}

bool DeclPart(istream& in, int& line)
{
	LexItem tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}

	if (tok == CONST) {
		Parser::PushBackToken(tok);
		if (!ConstPart(in, line)) {
			ParseError(CurrentErrorLine(line), "Incorrect Constant Definition Part.");
			return false;
		}

		tok = Parser::GetNextToken(in, line);
		if (ReportLexicalError(tok)) {
			return false;
		}
		if (tok == VAR) {
			Parser::PushBackToken(tok);
			if (!VarPart(in, line)) {
				ParseError(CurrentErrorLine(line), "Incorrect Declaration Part.");
				return false;
			}
		}
		else {
			Parser::PushBackToken(tok);
		}
		return true;
	}

	if (tok == VAR) {
		Parser::PushBackToken(tok);
		if (!VarPart(in, line)) {
			ParseError(CurrentErrorLine(line), "Incorrect Declaration Part.");
			return false;
		}
		return true;
	}

	Parser::PushBackToken(tok);
	return true;
}

bool ConstPart(istream& in, int& line)
{
	LexItem tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok != CONST) {
		Parser::PushBackToken(tok);
		return true;
	}

	if (!ConstDef(in, line)) {
		ParseError(CurrentErrorLine(line), "Syntactic error in Constants Definitions Part.");
		return false;
	}

	while (true) {
		tok = Parser::GetNextToken(in, line);
		if (ReportLexicalError(tok)) {
			return false;
		}
		if (tok != SEMICOL) {
			ParseError(tok.GetLinenum(), "Syntactic error in Constants Definitions Part.");
			return false;
		}

		tok = Parser::GetNextToken(in, line);
		if (ReportLexicalError(tok)) {
			return false;
		}
		if (tok != IDENT) {
			Parser::PushBackToken(tok);
			return true;
		}

		Parser::PushBackToken(tok);
		if (!ConstDef(in, line)) {
			ParseError(CurrentErrorLine(line), "Syntactic error in Constants Definitions Part.");
			return false;
		}
	}
}

bool ConstDef(istream& in, int& line)
{
	LexItem tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok != IDENT) {
		return false;
	}

	string name = tok.GetLexeme();
	if (defConst.count(name) != 0 || defVar.count(name) != 0) {
		ParseError(tok.GetLinenum(), "Constant Redefinition: " + name);
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok != EQ) {
		ParseError(tok.GetLinenum(), "Incorrect constant definition syntax.");
		return false;
	}

	if (!Expr(in, line)) {
		return false;
	}

	defConst[name] = true;
	return true;
}

bool VarPart(istream& in, int& line)
{
	LexItem tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok != VAR) {
		Parser::PushBackToken(tok);
		return true;
	}

	if (!DeclStmt(in, line)) {
		ParseError(CurrentErrorLine(line), "Syntactic error in Declaration Block.");
		return false;
	}

	while (true) {
		tok = Parser::GetNextToken(in, line);
		if (ReportLexicalError(tok)) {
			return false;
		}
		if (tok != SEMICOL) {
			ParseError(tok.GetLinenum(), "Syntactic error in Declaration Block.");
			return false;
		}

		tok = Parser::GetNextToken(in, line);
		if (ReportLexicalError(tok)) {
			return false;
		}
		if (tok != IDENT) {
			Parser::PushBackToken(tok);
			return true;
		}

		Parser::PushBackToken(tok);
		if (!DeclStmt(in, line)) {
			ParseError(CurrentErrorLine(line), "Syntactic error in Declaration Block.");
			return false;
		}
	}
}

bool IdentList(istream& in, int& line)
{
	vector<string> ids;
	return ParseIdentifierListInternal(in, line, ids);
}

bool DeclStmt(istream& in, int& line)
{
	vector<string> ids;
	if (!ParseIdentifierListInternal(in, line, ids)) {
		ParseError(CurrentErrorLine(line), "Incorrect identifiers list in Declaration Statement.");
		return false;
	}

	LexItem tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok != COLON) {
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (!IsTypeToken(tok.GetToken())) {
		ParseError(tok.GetLinenum(), "Incorrect Declaration Type: " + tok.GetLexeme());
		return false;
	}

	bool initialized = false;
	tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok == ASSOP) {
		if (!Expr(in, line)) {
			return false;
		}
		initialized = true;
	}
	else {
		Parser::PushBackToken(tok);
	}

	for (const string& id : ids) {
		defVar[id] = initialized;
	}

	return true;
}

bool Stmt(istream& in, int& line)
{
	return ParseStmtInternal(in, line, true);
}

bool SimpleStmt(istream& in, int& line)
{
	LexItem tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	Parser::PushBackToken(tok);

	bool status = false;
	if (tok == IDENT) {
		status = AssignStmt(in, line);
	}
	else if (tok == READLN) {
		status = ReadLnStmt(in, line);
	}
	else if (tok == WRITELN) {
		status = WriteLnStmt(in, line);
	}
	else if (tok == WRITE) {
		status = WriteStmt(in, line);
	}

	if (!status) {
		ParseError(CurrentErrorLine(line), "Incorrect Simple Statement.");
		return false;
	}

	return true;
}

bool StructuredStmt(istream& in, int& line)
{
	LexItem tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	Parser::PushBackToken(tok);

	bool status = false;
	if (tok == IF) {
		status = IfStmt(in, line);
	}
	else if (tok == BEGIN) {
		status = CompStmt(in, line);
	}

	if (!status) {
		ParseError(CurrentErrorLine(line), "Incorrect Structured Statement.");
		return false;
	}

	return true;
}

bool CompStmt(istream& in, int& line)
{
	LexItem tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok != BEGIN) {
		return false;
	}

	LexItem stmtTok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(stmtTok)) {
		return false;
	}
	Parser::PushBackToken(stmtTok);
	int stmtLine = stmtTok.GetLinenum();

	if (!Stmt(in, line)) {
		return false;
	}

	while (true) {
		tok = Parser::GetNextToken(in, line);
		if (ReportLexicalError(tok)) {
			return false;
		}

		if (tok == END) {
			return true;
		}

		if (tok == SEMICOL) {
			LexItem nextTok = Parser::GetNextToken(in, line);
			if (ReportLexicalError(nextTok)) {
				return false;
			}

			if (nextTok == END) {
				return true;
			}

			if (nextTok == ELSE) {
				ParseError(nextTok.GetLinenum(), "Illegal Else-clause.");
				return false;
			}

			if (nextTok == DONE || nextTok == DOT) {
				ParseError(nextTok.GetLinenum(), "Missing end of compound statement.");
				return false;
			}

			stmtLine = nextTok.GetLinenum();
			Parser::PushBackToken(nextTok);
			if (!Stmt(in, line)) {
				return false;
			}
			continue;
		}

		if (tok == DONE || tok == DOT) {
			ParseError(tok.GetLinenum(), "Missing end of compound statement.");
			return false;
		}

		ParseError(stmtLine, "Missing semicolon.");
		return false;
	}
}

bool WriteLnStmt(istream& in, int& line)
{
	LexItem tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok != WRITELN) {
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok != LPAREN) {
		ParseError(tok.GetLinenum(), "Missing Left Parenthesis in WriteLn statement.");
		return false;
	}

	if (!ExprList(in, line)) {
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok != RPAREN) {
		ParseError(tok.GetLinenum(), "Missing right parenthesis after expression.");
		return false;
	}

	return true;
}

bool WriteStmt(istream& in, int& line)
{
	LexItem tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok != WRITE) {
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok != LPAREN) {
		ParseError(tok.GetLinenum(), "Missing Left Parenthesis in Write statement.");
		return false;
	}

	if (!ExprList(in, line)) {
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok != RPAREN) {
		ParseError(tok.GetLinenum(), "Missing right parenthesis after expression.");
		return false;
	}

	return true;
}

bool ReadLnStmt(istream& in, int& line)
{
	LexItem tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok != READLN) {
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok != LPAREN) {
		ParseError(tok.GetLinenum(), "Missing Left Parenthesis in ReadLn statement.");
		return false;
	}

	vector<string> vars;
	if (!ParseVarListInternal(in, line, vars)) {
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok != RPAREN) {
		ParseError(tok.GetLinenum(), "Missing right parenthesis after expression.");
		return false;
	}

	for (const string& name : vars) {
		defVar[name] = true;
	}

	return true;
}

bool IfStmt(istream& in, int& line)
{
	LexItem tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok != IF) {
		return false;
	}

	if (!Expr(in, line)) {
		ParseError(CurrentErrorLine(line), "Missing if statement Logic Expression.");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok != THEN) {
		ParseError(tok.GetLinenum(), "If Statement Syntax Error.");
		return false;
	}

	if (!ParseStmtInternal(in, line, false)) {
		ParseError(CurrentErrorLine(line), "Missing Statement for If-Then-Part.");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok == ELSE) {
		if (!ParseStmtInternal(in, line, false)) {
			ParseError(CurrentErrorLine(line), "Missing Statement for If-Else-Part.");
			return false;
		}
	}
	else {
		Parser::PushBackToken(tok);
	}

	return true;
}

bool AssignStmt(istream& in, int& line)
{
	string name;
	if (!ParseVariableInternal(in, line, name)) {
		ParseError(CurrentErrorLine(line), "Missing Left-Hand Side Variable in Assignment statement.");
		return false;
	}

	LexItem tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (tok != ASSOP) {
		ParseError(tok.GetLinenum(), "Missing Assignment Operator.");
		return false;
	}

	if (!Expr(in, line)) {
		ParseError(CurrentErrorLine(line), "Missing Expression in Assignment Statement.");
		return false;
	}

	defVar[name] = true;
	return true;
}

bool Variable(istream& in, int& line)
{
	string name;
	return ParseVariableInternal(in, line, name);
}

bool ExprList(istream& in, int& line)
{
	if (!Expr(in, line)) {
		return false;
	}

	while (true) {
		LexItem tok = Parser::GetNextToken(in, line);
		if (ReportLexicalError(tok)) {
			return false;
		}
		if (tok != COMMA) {
			Parser::PushBackToken(tok);
			return true;
		}

		if (!Expr(in, line)) {
			return false;
		}
	}
}

bool VarList(istream& in, int& line)
{
	vector<string> vars;
	return ParseVarListInternal(in, line, vars);
}

bool Expr(istream& in, int& line)
{
	if (!SimpleExpr(in, line)) {
		return false;
	}

	LexItem tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}
	if (!IsRelOp(tok.GetToken())) {
		Parser::PushBackToken(tok);
		return true;
	}

	if (!SimpleExpr(in, line)) {
		return false;
	}

	LexItem nextTok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(nextTok)) {
		return false;
	}
	if (IsRelOp(nextTok.GetToken())) {
		ParseError(nextTok.GetLinenum(), "Illegal Relational Expression.");
		return false;
	}

	Parser::PushBackToken(nextTok);
	return true;
}

bool SimpleExpr(istream& in, int& line)
{
	if (!Term(in, line)) {
		return false;
	}

	while (true) {
		LexItem tok = Parser::GetNextToken(in, line);
		if (ReportLexicalError(tok)) {
			return false;
		}

		if (tok != PLUS && tok != MINUS && tok != OR) {
			Parser::PushBackToken(tok);
			return true;
		}

		if (!Term(in, line)) {
			ParseError(line, "Missing operand after operator.");
			return false;
		}
	}
}

bool Term(istream& in, int& line)
{
	if (!SFactor(in, line)) {
		return false;
	}

	while (true) {
		LexItem tok = Parser::GetNextToken(in, line);
		if (ReportLexicalError(tok)) {
			return false;
		}

		if (tok != MULT && tok != DIV && tok != IDIV && tok != MOD && tok != AND) {
			Parser::PushBackToken(tok);
			return true;
		}

		if (!SFactor(in, line)) {
			ParseError(line, "Missing operand after operator.");
			return false;
		}
	}
}

bool SFactor(istream& in, int& line)
{
	LexItem tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}

	int sign = 0;
	if (tok == PLUS || tok == MINUS || tok == NOT) {
		sign = static_cast<int>(tok.GetToken());
	}
	else {
		Parser::PushBackToken(tok);
	}

	return Factor(in, line, sign);
}

bool Factor(istream& in, int& line, int sign)
{
	LexItem tok = Parser::GetNextToken(in, line);
	if (ReportLexicalError(tok)) {
		return false;
	}

	switch (tok.GetToken()) {
		case IDENT:
			if (defVar.count(tok.GetLexeme()) == 0 && defConst.count(tok.GetLexeme()) == 0) {
				ParseError(tok.GetLinenum(), "Undeclared Variable: " + tok.GetLexeme());
				return false;
			}
			return true;

		case ICONST:
		case RCONST:
		case SCONST:
		case BCONST:
		case CCONST:
			return true;

		case LPAREN:
			if (!Expr(in, line)) {
				ParseError(CurrentErrorLine(line), "Missing expression after Left Parenthesis.");
				return false;
			}

			tok = Parser::GetNextToken(in, line);
			if (ReportLexicalError(tok)) {
				return false;
			}
			if (tok != RPAREN) {
				ParseError(tok.GetLinenum(), "Missing right parenthesis after expression.");
				return false;
			}
			return true;

		default:
			(void)sign;
			return false;
	}
}
