#pragma once

#include <string>
#include "symbol.h"
#include <map>
using namespace std;

class Util
{
public:
	// Used to so we know if there was syntax errors on any assembler lines
	//
	static int iCurrentFileLine;
	static bool fHasErrors;
};


string SGetWord(string&);

int IComputeExpr(string& sLine, map<string, Symbol*> *pSymMap, string &sSymReloc);

bool FUsesRegs(const string& str);

int IGetIntValue(const string& str);

bool FCheckDUP(const string& str);