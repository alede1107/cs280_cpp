#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <sstream>
#include <iomanip>
#include <cmath>

#include "lex.h"
using namespace std;

/*
OBJECTIVE OF MAIN.CPP

calls GetNextToken until it gets DONE or ERR

Must support the following command-line flags:

./lexer

[-all] -> print every token as it's recognized
[-ids] -> print all unique identifiers and keywords (sorted, with occurence counts)
[-nums] -> print all unique integer and real constants (sorted numerically, with counts)
[-str] -> print all unique constants (sorted alphabetically, with counts)

filename

FLAG RULES:

flags may appear in any order and may repeat
unrecognized flag: print UNRECOGNIZED FLAG {arg in question} and stop
more than one filename: print ONLY ONE FILE NAME IS ALLOWED. and stop
no filename: print NO SPECIFIED INPUT FILE. and stop
File can't be opened: print CANNOT OPEN THE FILE {arg in question} and stop.

When reaching DONE, always print this summary:
    Lines: L
    Total Tokens: M
    Identifiers & Keywords: N
    Numbers: O
    Booleans: P
    Strings: Q

Then output any flag-based lists in this order:
-ids: print KEYWORDS: header, then each keyword with count in parens, comma-separated. then IDENTIFIERS: header with same
-num: print INTEGERS: then REALS: with values in numeric order and counts.
-str: print STRINGS: then each string in single quotes, alphabetically

if a category has no items, skip printing that category entirely


Handling ERR

if getNextToken returns ERR, print the error using the operator<< format and stop. Do not print a summary
*/

int main(int argc, char* argv[]) {
    string filename = "";
    bool flagAll = false, flagNum = false, flagStr = false, flagIds = false;

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-all")  flagAll = true;
        else if (arg == "-num")  flagNum = true;
        else if (arg == "-str") flagStr = true;
        else if (arg == "-ids") flagIds = true;
        else if (arg[0] == '-') { std::cout << "UNRECOGNIZED FLAG {" << arg << "}" << endl; return 1; }
        else {
            if (!filename.empty()) {
                cout << "ONLY ONE FILE NAME IS ALLOWED." << endl;
                return 1;
            }
            filename = arg;

        }
    }

    if (filename.empty()) {
        std::cout << "NO SPECIFIED INPUT FILE." << endl; return 1;
    }

    ifstream infile(filename);
    if (!infile.is_open()) {
        std::cout << "CANNOT OPEN THE FILE " << filename << endl;
        return 1;
    }


    int totalTokens = 0, totalLines = 1, totalIds = 0,
        totalNums = 0, totalBools = 0, totalStrings = 0;

    map<string, int> identifiers;
    map<string, int> keywords;
    map<string, int> strings;
    map<string, int> integers;
    map<string, int> reals;
    set<Token> keywordTokens = {
        IF, ELSE, WRITELN, WRITE, READLN, INTEGER, REAL, BOOLEAN, CHAR,
        STRING, BEGIN, END, VAR, CONST, THEN, PROGRAM, AND, OR,
        NOT, IDIV, MOD
    };

    int linenum = 1;
    LexItem token(PLUS, "", 0);

    while (token != DONE && token != ERR) {
        token = getNextToken(infile, linenum);
        if (token == ERR) {
            cout << token;
            return 1;
        }
        if (token != DONE && token != ERR) {
            if (flagAll) {
                cout << token;
            }
            totalTokens++;
            if (token == IDENT) {
                identifiers[token.GetLexeme()]++;
                totalIds++;
            }
            else if (keywordTokens.count(token.GetToken()) > 0) {
                string lower = token.GetLexeme();
                for (auto& c : lower) c = tolower(c);
                keywords[lower]++;
                totalIds++;
            }
            else if (token == ICONST) {
                integers[token.GetLexeme()]++;
                totalNums++;
            }
            else if (token == RCONST) {
                reals[token.GetLexeme()]++;
                totalNums++;
            }
            else if (token == SCONST) {
                strings[token.GetLexeme()]++;
                totalStrings++;
            }
            else if (token == TRUE || token == FALSE) {
                totalBools++;
            }
        }
    }
    totalLines = linenum - 1;
    if (totalTokens == 0) {
        cout << "Empty File." << endl;
        return 0;
    }
    cout << endl;
    cout << "Lines: " << totalLines << endl;
    cout << "Total Tokens: " << totalTokens << endl;
    cout << "Identifiers & Keywords: " << totalIds << endl;
    cout << "Numbers: " << totalNums << endl;
    cout << "Booleans: " << totalBools << endl;
    cout << "Strings: " << totalStrings << endl;


    if (flagIds) {
        if (!identifiers.empty()) {
            bool first = true;
            cout << "IDENTIFIERS:" << endl;
            for (auto& pair : identifiers) {
                if (!first) cout << ", ";
                cout << pair.first << " (" << pair.second << ")";
                first = false;
            }
            cout << endl;
        }
        else cout << endl;
        if (!keywords.empty()) {
            bool first = true;
            cout << "KEYWORDS:" << endl;
            for (auto& pair : keywords) {
                if (!first) cout << ", ";
                cout << pair.first << " (" << pair.second << ")";
                first = false;
            }
            cout << endl;
        }
    }
    if (flagNum) {
        if (!integers.empty()) {
            bool first = true;
            cout << "INTEGERS:" << endl;
            for (auto& pair : integers) {
                if (!first) cout << ", ";
                cout << pair.first << " (" << pair.second << ")";
                first = false;
            }
            cout << endl;
        }
        if (!reals.empty()) {
            bool first = true;
            cout << "REALS:" << endl;
            for (auto& pair : reals) {
                if (!first) cout << ", ";
                cout << pair.first << " (" << pair.second << ")";
                first = false;
            }
            cout << endl;
        }
    }
    if (flagStr) {
        if (!strings.empty()) {
            bool first = true;
            cout << "STRINGS:" << endl;
            for (auto& pair : strings) {
                if (!first) cout << ", ";
                cout << pair.first << " (" << pair.second << ")";
                first = false;
            }
            cout << endl;
        }
    }


    return 0;
}