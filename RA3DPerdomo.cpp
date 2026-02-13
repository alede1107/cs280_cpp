#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <cctype>

using namespace std;

int main(int argc, char* argv[]) {


    if (argc < 2) {
        cerr << "NO SPECIFIED INPUT FILE NAME." << endl;
        return 1;
    }
    bool flagAll = false;
    bool flagT1 = false;
    bool flagT2 = false;
    bool flagT3 = false;

    for (int i = 2; i < argc; i++) {
        string arg = argv[i];

        if (arg == "-all") flagAll = true;
        else if (arg == "-t1") flagT1 = true;
        else if (arg == "-t2") flagT2 = true;
        else if (arg == "-t3") flagT3 = true;
    }

    string name = argv[1];

    ifstream file(name);

    if (!file) {
        cerr << "CANNOT OPEN THE FILE " << name << endl;
        return 1;
    }
    string word;

    int total = 0,
        t1 = 0, //_
        t2 = 0, //@
        t3 = 0; //#


    set<string> uniqueat;

    set<string> uniqueunder;

    set<string> uniquehash;

    while (file >> word) {
        total++;

        if (word.size() < 2) continue;          // need prefix + at least 1 more char

        char start = word[0];

        if (start != '_' && start != '@' && start != '#') continue;

        // validate that every char after the prefix is letter/digit/underscore
        bool valid = true;
        for (size_t i = 1; i < word.size(); i++) {
            unsigned char c = (unsigned char)word[i];
            if (!(isalnum(c) || c == '_')) {
                valid = false;
                break;
            }
        }
        if (!valid) continue;

        string sub = word.substr(1);
        //_
        if (start == '_') {
            t1++;
            uniqueunder.insert(sub);
        }


        //@
        else if (start == '@') {
            t2++;
            uniqueat.insert(sub);
        }
        //#
        else {
            t3++;
            uniquehash.insert(sub);
        }
    }

    if (total == 0) {
        std::cout << "The file is empty.\n";
        return 1;
    }
    if (flagAll == false && flagT1 == false && flagT2 == false && flagT3 == false) {
        std::cout << "Total number of words: " << total << endl;
    }
    if (flagAll == true) {
        cout << "Total number of words: " << total << endl
            << "Occurrences of Type1 Names (Starting with '_' character): "
            << t1 << endl
            << "Occurrences of Type2 Names (Starting with '@' character): "
            << t2 << endl
            << "Occurrences of Type3 Names (Starting with '#' character): "
            << t3 << endl;
    }
    if (flagT1 == true) {
        std::cout << "Count of Type1 Unique Names: " << uniqueunder.size() << endl;
    }
    if (flagT2 == true) {
        std::cout << "Count of Type2 Unique Names: " << uniqueat.size() << endl;
    }
    if (flagT3 == true) {
        std::cout << "Count of Type3 Unique Names: " << uniquehash.size() << endl;
    }


    return 0;
}