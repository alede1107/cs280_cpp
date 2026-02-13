#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cctype>
using namespace std;

int main() {

	ifstream file;
	string name;
	cout << "Enter the name of a file to read from: \n";


	cin >> name;

	file.open(name.c_str());

	//if file can't be opened
	if (!file) {
		cout << endl; // <-- blank line expected by case1
		cerr << "File cannot be opened: " << name << endl;

		exit(1);
	}

	string line;

	int tlines = 0, vlines = 0, ilines = 0,
		commlines = 0, cd = 0, dir = 0,
		copy = 0, del = 0;

	//loops through each line in file
	while (getline(file, line)) {
		//add +1 to total lines
		tlines++;

		string test;
		istringstream iss(line);
		iss >> test;

		if (test.empty()) {
			continue;
		}
		//make sure all strings are lowercase to handle cD CD cd and Cd as the same CD comment
		for (char& c : test) {
			c = tolower(static_cast<unsigned char>(c));
		}

		if (line.substr(0, 2) == "::" || test == "rem") {
			commlines++;
		}

		else if (test == "cd") {
			cd++;
			vlines++;
		}
		else if (test == "dir") {
			dir++;
			vlines++;
		}
		else if (test == "copy") {
			copy++;
			vlines++;
		}
		else if (test == "del") {
			del++;
			vlines++;
		}

		else {
			ilines++;
			cout << endl;
			std::cout << "Error: Unrecognizable command in line " << tlines << ": " << test << "\n";
		}
	}
	if (tlines == 0) {
		cout << "File is empty.\n";
	}
	else {
		cout << endl;
		cout << "Total lines: " << tlines <<
			"\nCommented lines: " << commlines <<
			" \nValid Command lines: " << vlines <<
			"\nInvalid Command lines: " << ilines <<
			"\nDIR commands: " << dir << "\nCD commands: " <<
			cd << "\nCOPY commands: " << copy << "\nDEL commands: " << del;
	}
}
