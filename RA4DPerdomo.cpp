#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <sstream>
#include <iomanip>
#include <cmath>

using namespace std;

int main(int argc, char* argv[]) {
    string filename = "";
    bool flagAll = false, flagInt = false, flagReal = false;

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-all")  flagAll = true;
        else if (arg == "-int")  flagInt = true;
        else if (arg == "-real") flagReal = true;
        else if (arg[0] == '-') { std::cout << "UNRECOGNIZED FLAG " << arg << endl; return 1; }
        else filename = arg;
    }

    if (filename.empty()) { 
        std::cout << "NO SPECIFIED INPUT FILE NAME." << endl; return 1; 
    }

    ifstream infile(filename);
    if (!infile.is_open()) { 
        std::cout << "CANNOT OPEN THE FILE " << filename << endl; 
        return 1; 
    }

    string line;
    if (!getline(infile, line)) { 
        std::cout << "File is empty." << endl; 
        return 1; 
    }

    map<string, int> allLiterals;
    map<string, int> intLiterals;
    map<string, double> floatLiterals;
    int totalLines = 0;

    do {
        totalLines++;
        size_t i = 0, len = line.size();

        while (i < len) {
            if (!isdigit(line[i])) {
                i++; 
                continue;
            }

            // --- Parse Numeral ---
            string numeral = "", fracStr = "", expStr = "";
            bool isFloat = false;

            while (i < len && isdigit(line[i])) {
                numeral += line[i++];
            }

            // --- Parse Fraction (optional): . Digit+ ---
            if (i < len && line[i] == '.') {
                size_t dotPos = i++;
                string fracDigits = "";
                while (i < len && isdigit(line[i])) {
                    fracDigits += line[i++];
                }

                if (fracDigits.empty()) {
                    i = dotPos; // un-consume dot, no fraction
                }
                else {
                    fracStr = "." + fracDigits;
                    isFloat = true;

                    if (i < len && line[i] == '.') { // second dot = invalid
                        i++;
                        std::cout << "Line " << totalLines << ": Invalid floating-point literal \""
                            << numeral + fracStr + "." << "\"" << endl;
                        continue;
                    }
                }
            }

            // --- Parse Exponent (optional): (E|e)(+|-)? Digit+ ---
            if (i < len && (line[i] == 'E' || line[i] == 'e')) {
                expStr += line[i++]; // consume E/e
                isFloat = true;

                if (i < len && (line[i] == '+' || line[i] == '-')) {
                    expStr += line[i++]; // consume first sign

                    if (i < len && (line[i] == '+' || line[i] == '-')) { // second sign = invalid
                        expStr += line[i++];
                        std::cout << "Line " << totalLines << ": Invalid exponent for a numeric literal: \""
                            << numeral + fracStr + expStr << "\"" << endl;
                        continue;
                    }
                }

                string expDigits = "";
                while (i < len && isdigit(line[i])) expDigits += line[i++];
                expStr += expDigits;

                if (i < len && (line[i] == 'E' || line[i] == 'e')) { // second E = invalid
                    expStr += line[i++];
                    std::cout << "Line " << totalLines << ": Invalid exponent for a numeric literal: \""
                        << numeral + fracStr + expStr << "\"" << endl;
                    continue;
                }

                if (!expDigits.empty() && i < len && (line[i] == '+' || line[i] == '-')) { // trailing sign = invalid
                    expStr += line[i++];
                    std::cout << "Line " << totalLines << ": Invalid exponent for a numeric literal: \""
                        << numeral + fracStr + expStr << "\"" << endl;
                    continue;
                }

                if (expDigits.empty()) { // no digits after E = invalid
                    std::cout << "Line " << totalLines << ": Invalid exponent for a numeric literal: \""
                        << numeral + fracStr + expStr << "\"" << endl;
                    continue;
                }
            }

            // --- Valid literal: store it ---
            string lit = numeral + fracStr + expStr;
            allLiterals[lit]++;
            if (isFloat) floatLiterals[lit] = stod(lit);
            else         intLiterals[lit] = stoll(lit);
        }

    } while (getline(infile, line));
    infile.close();

    std::cout << "Total Number of Lines: " << totalLines << endl;
    std::cout << "Number of Integer Literals: " << intLiterals.size() << endl;
    std::cout << "Number of Floating-Point Literals: " << floatLiterals.size() << endl;

    if (flagAll) {
        std::cout << "\nList of All Numeric Literals and their Number of Occurrences:" << endl;
        for (auto& p : allLiterals)
            std::cout << "\"" << p.first << "\": " << p.second << endl;
    }

    if (flagInt) {
        std::cout << "\nList of Integer Literals and their Values:" << endl;
        for (auto& p : intLiterals)
            std::cout << "\"" << p.first << "\": " << p.second << endl;
    }

    if (flagReal) {
        cout << "\nList of Floating-Point Literals and their Values:" << endl;
        for (auto& p : floatLiterals) {
            double val = p.second;
            cout << "\"" << p.first << "\": ";
            if (val == floor(val))
                cout << (long long)val << endl;
            else
                cout << p.first << endl;
        }
    }

    return 0;
}