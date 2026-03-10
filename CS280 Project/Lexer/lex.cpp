#include <iostream>
#include <map>
#include "lex.h"

ostream& operator<<(ostream& out, const LexItem& tok) {
	/*
	prints a token to the screen

	TYPE					|	Output Format			| Example

	ICONST, RCONST, BCONST	|	TOKEN: (value)					| ICONST: (42)

	IDENT					|	IDENT: <name>					| IDENT: <myVar>

	SCONST					|	SCONST: 'text'					| SCONST: 'Hello'

	ERR						|	ERR: Error in line (N) message	| ERR: Error in line (3) Invalid character...

	Everything else			|	TOKEN: "lexeme"					| PLUS: "+"


	rules about tokens:

	identifiers:

	must start with a letter, followed by letters, digits, _, or $

	Integer Constants (ICONST)
	one or more digits

	Real Constants (RCONST)
	Digits, then a dot, then more digits. Optional exponent with E or e.

	String Constants (SCONST)
	Text between single quotes, one a single line
	ONLY '' not ""

	Boolean Constants (BCONST)
	keywords true and false (reserved words)



	*/
	map<Token, string> tokenNames = {
		{IF, "IF"},
		{ELSE, "ELSE"},
		{WRITELN, "WRITELN"},
		{WRITE, "WRITE"},
		{READLN, "READLN"},
		{INTEGER, "INTEGER"},
		{REAL, "REAL"},
		{BOOLEAN, "BOOLEAN"},
		{CHAR, "CHAR"},
		{STRING, "STRING"},
		{BEGIN, "BEGIN"},
		{END, "END"},
		{VAR, "VAR"},
		{CONST, "CONST"},
		{THEN, "THEN"},
		{PROGRAM, "PROGRAM"},
		{IDENT, "IDENT"},
		{TRUE, "TRUE"},
		{FALSE, "FALSE"},
		{ICONST, "ICONST"},
		{RCONST, "RCONST"},
		{SCONST, "SCONST"},
		{BCONST, "BCONST"},
		{CCONST, "CCONST"},
		{PLUS, "PLUS"},
		{MINUS, "MINUS"},
		{MULT, "MULT"},
		{DIV, "DIV"},
		{IDIV, "IDIV"},
		{MOD, "MOD"},
		{ASSOP, "ASSOP"},
		{EQ, "EQ"},
		{GTHAN, "GTHAN"},
		{LTHAN, "LTHAN"},
		{AND, "AND"},
		{OR, "OR"},
		{NOT, "NOT"},
		{COMMA, "COMMA"},
		{SEMICOL, "SEMICOL"},
		{LPAREN, "LPAREN"},
		{RPAREN, "RPAREN"},
		{LBRACE, "LBRACE"},
		{RBRACE, "RBRACE"},
		{DOT, "DOT"},
		{COLON, "COLON"},
		{ERR, "ERR"},
		{DONE, "DONE"},
		{THEN, "THEN"}
	};

	if (
		tok.GetToken() == ICONST ||
		tok.GetToken() == RCONST ||
		tok.GetToken() == BCONST
		) {
		out << tokenNames[tok.GetToken()] << ": (" << tok.GetLexeme() << ")\n";
	}
	else if (tok.GetToken() == IDENT) {
		out << "IDENT: <" << tok.GetLexeme() << ">\n";
	}
	else if (tok.GetToken() == SCONST) {
		out << "SCONST: " << tok.GetLexeme() << '\n';
	}
	else if (tok.GetToken() == ERR) {
		out << "ERR: Error in line (" << tok.GetLinenum() << ") " << tok.GetLexeme() << '\n';
	}
	else {
		out << tokenNames[tok.GetToken()] << ": \"" << tok.GetLexeme() << "\"\n";
	}
	return out;
}
LexItem id_or_kw(const string& lexeme, int linenum) {
	/*
	when you read a word, this function checks if it's a keyword/identifier
	if it matches keywork, return keyword token
	otherwise return an IDENT token
	*/
	string lower = lexeme;
	for (auto& c : lower) c = tolower(c);
	map<string, Token> id_kw = {
		{"and", AND},
		{"then", THEN},
		{"begin", BEGIN},
		{"boolean", BOOLEAN},
		{"char", CHAR},
		{"const", CONST},
		{"div", IDIV},
		{"else", ELSE},
		{"end", END},
		{"false", FALSE},
		{"if", IF},
		{"integer", INTEGER},
		{"mod", MOD},
		{"not", NOT},
		{"or", OR},
		{"program", PROGRAM},
		{"readln", READLN},
		{"real", REAL},
		{"string", STRING},
		{"true", TRUE},
		{"var", VAR},
		{"write", WRITE},
		{"writeln", WRITELN}

	};
	if (id_kw.count(lower) == 0) {
		return LexItem(IDENT, lexeme, linenum);
	}
	return LexItem(id_kw[lower], lexeme, linenum);
}
LexItem getNextToken(istream& in, int& linenum) {
	/*
	holds
	token type -> PLUS, IDENT, ICONST
	lexeme -> actual type: "+", "x", "42"
	line number where it was found


	RULES
	skip whitespace (spaces, tabs, newlines -> newlines still count for linenum)
	skip comments: {...} and {*...*}, still count for linenums
	return ERR for invalid input.
	return DONE when the file ends
	once ERR or DONE is returned, don't call getNextToken again

	*/
	char ch;
	if (!in.get(ch)) {
		return LexItem(DONE, "", linenum);
	}

	string lexeme;
	//use in.get instead of cin>> because it reads white space
	while (isspace(ch)) {
		if (ch == '\n')
			linenum++;
		if (!in.get(ch)) {
			return LexItem(DONE, "", linenum);
		}
	}

	//checking if it's an identifier/keyword
	if (isalpha(ch)) {

		lexeme += ch;
		while (in.get(ch)) {
			if (isalnum(ch) ||
				ch == '_' ||
				ch == '$') {
				lexeme += ch;
			}
			else {
				in.putback(ch);
				break;
			}
		}
		return id_or_kw(lexeme, linenum);
	}
	//checking if it's an integer or real
	if (isdigit(ch)) {
		int real = 0;
		lexeme += ch;
		while (in.get(ch)) {
			if (isdigit(ch)) {
				lexeme += ch;
			}
			//real
			else if (ch == '.') {
				real++;
				char next = in.peek();
				if (isdigit(next)) {
					lexeme += ch;
					while (in.get(ch)) {

						if (isdigit(ch)) {
							lexeme += ch;
						}
						//check if it has a second decimal
						else if (ch == '.') {
							lexeme += ch;
							in.get(ch);
							lexeme += ch;
							return LexItem(ERR, "Invalid floating-point constant \"" + lexeme + "\"", linenum);
						}


						//check if it has e/E
						else if (ch == 'e' || ch == 'E') {
							lexeme += ch;
							char nexte = in.peek();
							if (isdigit(nexte)) {
								in.get(ch);
								lexeme += ch;
								while (in.get(ch)) {
									if (isdigit(ch)) {
										lexeme += ch;
									}
									else if (ch == 'E' || ch == 'e') {
										lexeme += ch;
										return LexItem(ERR, "Invalid exponent for floating-point constant \"" + lexeme + "\"", linenum);
									}
									else {
										in.putback(ch);
										break;
									}
								}
							}
							else if (nexte == '+' || nexte == '-') {
								in.get(ch);
								lexeme += ch;

								while (in.get(ch)) {
									if (isdigit(ch)) {
										lexeme += ch;
									}
									else if (ch == '+' || ch == '-') {
										lexeme += ch;

										return LexItem(ERR, "Invalid exponent for floating-point constant \"" + lexeme + "\"", linenum);
									}
									else if (ch == 'E' || ch == 'e') {
										lexeme += ch;
										return LexItem(ERR, "Invalid exponent for floating-point constant \"" + lexeme + "\"", linenum);
									}
									else {
										in.putback(ch);
										break;
									}
								}
							}
							else if (nexte == 'E' || nexte == 'e') {
								in.get(ch);
								lexeme += ch;
								return LexItem(ERR, "Invalid exponent for a floating-point constant \"" + lexeme + "\"", linenum);
							}
							else {
								in.putback(ch);
								LexItem res(ERR, lexeme, linenum);
								return res;
							}
						}

						else {
							in.putback(ch);
							break;
						}
					}
				}
				else if (isspace(next)) {
					in.putback(ch);
					return LexItem(ICONST, lexeme, linenum);
				}
				else {
					in.putback(ch);
					LexItem res(ERR, lexeme, linenum);
					return res;
				}
			}

			else {
				in.putback(ch);
				break;
			}
		}

		if (real == 0) {
			LexItem res(ICONST, lexeme, linenum);
			return res;
		}
		else if (real == 1) {
			LexItem res(RCONST, lexeme, linenum);
			return res;
		}
		else {
			LexItem res(ERR, lexeme, linenum);
			return res;
		}
	}
	//check if it's a string
	if (ch == '\'') {
		bool closed = false;
		lexeme += ch;
		while (in.get(ch)) {
			if (ch == '\n') {
				return LexItem(ERR, "New line is not allowed within string literal \"" + lexeme + "\"", linenum);
			}

			else if (ch == '\'') {
				lexeme += ch;
				closed = true;
				break;
			}
			else {
				lexeme += ch;
			}

		}
		if (!closed) {
			return LexItem(ERR, lexeme, linenum);
		}
		return LexItem(SCONST, lexeme, linenum);
	}

	//check {} comments
	if (ch == '{') {
		bool closed = false;
		lexeme += ch;
		while (in.get(ch)) {
			if (ch == '}') {
				lexeme += ch;
				closed = true;
				break;
			}
			else if (ch == '{') {
				return LexItem(ERR, lexeme, linenum);
			}
			else if (ch == '\n') {
				linenum++;
			}
			else {
				lexeme += ch;
			}

		}
		if (!closed) {
			return  LexItem(ERR, lexeme, linenum);
		}
		return getNextToken(in, linenum);
	}
	//checking (**) comments
	if (ch == '(') {
		bool closed = false;
		lexeme += ch;
		//it's a comment
		if (in.peek() == '*') {
			in.get(ch);
			lexeme += ch;
			while (in.get(ch)) {
				if (ch == '*' && in.peek() == ')') {
					lexeme += ch;
					in.get(ch);
					lexeme += ch;
					closed = true;
					break;
				}
				else if (ch == '\n') {
					linenum++;
				}
				else if (ch == '(' && in.peek() == '*') {
					return LexItem(ERR, "Invalid nesting of comments \"(*(*\"", linenum);
				}
				else lexeme += ch;
			}
			if (!closed) {
				return LexItem(ERR, "Missing closing symbol(s) for a comment \"(*\"", linenum);
			}
			return getNextToken(in, linenum);
		}
		else return LexItem(LPAREN, "(", linenum);
	}
	if (ch == '+') return LexItem(PLUS, "+", linenum);
	if (ch == '-') return LexItem(MINUS, "-", linenum);
	if (ch == '*') return LexItem(MULT, "*", linenum);
	if (ch == '/') return LexItem(DIV, "/", linenum);
	if (ch == '=') return LexItem(EQ, "=", linenum);
	if (ch == '<') return LexItem(LTHAN, "<", linenum);
	if (ch == '>') return LexItem(GTHAN, ">", linenum);
	if (ch == ',') return LexItem(COMMA, ",", linenum);
	if (ch == ';') return LexItem(SEMICOL, ";", linenum);
	if (ch == ')') return LexItem(RPAREN, ")", linenum);
	if (ch == '.') return LexItem(DOT, ".", linenum);
	if (ch == '}') return LexItem(RBRACE, "}", linenum);
	if (ch == ':') {
		if (in.peek() == '=') {
			in.get(ch);
			return LexItem(ASSOP, ":=", linenum);
		}
		else return LexItem(COLON, ":", linenum);
	}
	else return LexItem(ERR, "Invalid character for starting a token \"" + string(1, ch) + "\"", linenum);
}
