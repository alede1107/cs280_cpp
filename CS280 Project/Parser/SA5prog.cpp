#include <iostream>
#include <string>
#include <vector>
using namespace std;
#include "val_SP26.h"


void ExecuteDivide(const Value& val1, const Value& val2)
{
	cout << val1 << " / " << val2 << " is " <<  val1 / val2 << endl;
}

void ExecuteLThan(const Value& val1, const Value& val2)
{
	cout << val1 << " < " << val2 << " is " <<  (val1 < val2) << endl;
}

void ExecuteAdd(const Value& val1, const Value& val2)
{
	if(val1.IsString() || val1.IsChar())
	{
		cout << "\'" << val1 << "\'"<< " + ";
	}
	else
	{
		cout << val1 << " + ";
	}
	if(val2.IsString() || val2.IsChar())
	{
		cout << "\'" << val2 << "\'"<< " is ";
	}
	else
	{
		cout << val2 << " is ";
	}
	if((val1.IsString() || val1.IsChar()) && (val2.IsString() || val2.IsChar()))
	{
		cout << "\'" << (val1 + val2)<< "\'" << endl;
	}
	else
	{
		cout <<  (val1 + val2)<< endl;
	}
}

void ExecuteTrim(const Value& val1)
{
	if(val1.IsString())
		cout << "\'" << val1 << "\'" << " trimming " << " is " <<  "\'" << (val1.Trim()) << "\'" << endl;

	else
		cout << val1 << " trimming " << " is " <<  (val1.Trim()) << endl;
}


int main(int argc, char *argv[])
{
	Value ErrorVal;
	double real1 = 9.5;
	Value doubleVal1(real1);
	double real2 = 3.25;
	Value doubleVal2(real2);
	int int1 = 17;
	Value intVal1(int1);
	int int2 = 4;
	Value intVal2(int2);
	string str1 = "CS280    ";
	Value StrVal1(str1);
	string str2 = "Spring 2026     ";
	Value StrVal2(str2);
	bool b1 = true;
	Value bVal1(b1);
	bool b2 = false;
	Value bVal2(b2);
	char ch1 = 'Z';
	Value cVal1(ch1);
	char ch2 = 'i';
	Value cVal2(ch2);

	vector<Value> vals( {ErrorVal, doubleVal1, doubleVal2, intVal1, intVal2, StrVal1, StrVal2, bVal1, bVal2, cVal1, cVal2} );
	bool divflag = false, lthflag = false, addflag = false, trimflag = false;

	if (argc == 1)
	{
		cerr << "No Specified arguments." << endl;
		return 0;
	}

	string arg( argv[1] );
	if( arg == "-div" )
		divflag = true;
	else if( arg == "-lth" )
		lthflag = true;
	else if( arg == "-add" )
		addflag = true;
	else if( arg == "-trim" )
		trimflag = true;

	else {
			cerr << "UNRECOGNIZED FLAG " << arg << endl;
			return 0;
	}


	cout << "ErrorVal=" << ErrorVal << endl;

	cout << "doubleVal1=" << doubleVal1 << endl;
	cout << "doubleVal2=" << doubleVal2 << endl;
	cout << "intVal1=" << intVal1 << endl;
	cout << "intVal2=" << intVal2 << endl;
	cout << "StrVal1=" << "\"" << StrVal1 << "\"" << endl;
	cout << "StrVal2=" <<  "\"" << StrVal2 << "\"" << endl;
	cout << "bVal1=" << bVal1 << endl;
	cout << "bVal2=" << bVal2 << endl;
	cout << "cVal1=" << cVal1 << endl;
	cout << "cVal2=" << cVal2 << endl;

	if(divflag)
	{
		for(int i = 0; i < (int)vals.size(); i++ )
			for( int j = 0; j < (int)vals.size(); j++ )
				if( i != j )
					ExecuteDivide( vals[i], vals[j] );
	}

	if(lthflag)
	{
		for(int i = 0; i < (int)vals.size(); i++ )
		for( int j = 0; j < (int)vals.size(); j++ )
			if( i != j )
				ExecuteLThan( vals[i], vals[j] );
	}

	if(addflag)
	{
		for(int i = 0; i < (int)vals.size(); i++ )
		for( int j = 0; j < (int)vals.size(); j++ )
			if( i != j )
				ExecuteAdd( vals[i], vals[j] );
	}
	if(trimflag)
	{
		for(int i = 0; i < (int)vals.size(); i++ )
			ExecuteTrim( vals[i] );
	}
	return 0;
}
