#include "util.h"
#include <stack>
#include <vector>
#include <set>
#include <iostream>
using namespace std;

int Util::iCurrentFileLine = 0;
bool Util::fHasErrors = false;
int Symbol::s_iIdCount = 1;

string SGetWord(string& sLine)
{
	int i = 0;
	string sWord = "";
	while (sLine.length() > i && (isspace(sLine[i]) || ',' == sLine[i]))
		i++;
	if ('\'' == sLine[i]) {
		sWord += sLine[i++]; //get the '
		if ('\\' == sLine[i]) {
			sWord += sLine[i++]; //get the '\'
		}
		sWord += sLine[i++]; //get the character
		sWord += sLine[i++]; //get the other '

	}

	if ('[' == sLine[i])
	{
		while (']' != sLine[i])
			sWord += sLine[i++];
		sWord += sLine[i++];
	}
	else {
		while (i < sLine.length() && !isspace(sLine[i]) && !(',' == sLine[i]) && !(';' == sLine[i])) {
				sWord += sLine[i++];
		}
	}
	if (i < sLine.length()) {
		if (';' == sLine[i])
			sLine = "";
		else
			sLine = sLine.substr(i + 1, sLine.length());
	}
	else {
		sLine = "";
	}

	return sWord;
}


int IGetIntValue(const string& str)
{
	int iRet = INT_MAX;
	if ('0' == str[0] && 'x' == str[1])
	{

		for (int i = 2; i < str.length(); ++i)
		{
			unsigned short ch;
			if (str[i] >= '0' || str[i] <= '9')
				ch = str[i] - '0';
			else if (str[i] <= 'F')
				ch = str[i] - 'A' + 10;
			else
				ch = str[i] - 'a' + 10;
			iRet = (iRet << 4) | ch;
		}
	}
	else if ('0' == str[0] && 'b' == str[1])
	{
		for (int i = str.length() - 1; i > 1; --i)
		{
			iRet |= (str[i] - '0') << (str.length() - 1 - i);
		}
	}
	else
		iRet = atoi(str.c_str());
	return iRet;
}

// Calculates expression
//
int IComputeExpr(string& sLine, map<string, Symbol*> * pSymMap, string &sSymReloc, int iLocationCounter) {
	// Don't bother calculating if it's an undefined value
	//
	if ('?' == sLine[0])
	{
		sLine.erase(0, 1);
		sSymReloc = "";
		return 0;
	}
	if ("" == sLine)
	{
		sSymReloc = "";
		return 0;
	}
	stack<char> operators;
	stack<int> values;

	vector<string> tokens;
	set<char> setOfOperators;
	setOfOperators.insert('+');
	setOfOperators.insert('-');
	setOfOperators.insert('*');
	setOfOperators.insert('/');
	setOfOperators.insert('(');
	setOfOperators.insert(')');

	bool fAddLocationCounter = false;

	// If we start off with a negative value
	//
	if (sLine[0] == '-')
	{
		sLine = "0" + sLine;
	}
	// Extract all the operators and operands into a vector
	//
	while ("" != sLine) {
		int i = 0;
		// Skip the spaces
		//
		while (i < sLine.length() && isspace(sLine[i]))
			i++;
		if (sLine.length() == i)
			break;
		if (i < sLine.length() && ',' == sLine[i])
		{
			sLine = sLine.substr(i + 1, sLine.length());
			break;
		}
		if (i < sLine.length() && 'D' == sLine[i] && 'U' == sLine[i + 1] && 'P' == sLine[i + 2])
			break;
		string sWord;
		// If hexa constant
		//
		if (i < sLine.length() && '0' == sLine[i] && 'x' == sLine[i + 1])
		{
			sWord += "0x";
			i += 2;
			while (i < sLine.length() && ',' != sLine[i] && !isspace(sLine[i]) && setOfOperators.find(sLine[i]) == setOfOperators.end())
				sWord += sLine[i++];
		}
		// If binary constant
		//
		else if (i+1 < sLine.length() && '0' == sLine[i] && 'b' == sLine[i + 1])
		{
			sWord += "0b";
			i += 2;
			while (isdigit(sLine[i]))
				sWord += sLine[i++];
		}
		// If it's a decimal constant 
		//
		else if (i < sLine.length() && isdigit(sLine[i]))
		{
			while (isdigit(sLine[i]))
				sWord += sLine[i++];
		}

		// If it's a char treat it like a decimal const
		//
		else if (i < sLine.length() && '\'' == sLine[i])
		{
			int iValue = 0;
			if ('\\' == sLine[i + 1])
			{

				switch (sLine[i + 2])
				{
				case 'n':
					iValue = '\n';
					break;
				case 't':
					iValue = '\t';
					break;
				default:
					break;
					//TODO: ADD OTHER CASES HERE IF NEEDED
				}
				i += 4;
			}
			else {
				iValue = sLine[i + 1];
				i += 3;
			}
			sWord = to_string(iValue);
		}

		// If any of the statments above executed we should be at an operator
		// if not, than it's a symbol and we need to get it
		//
		else if (i < sLine.length() && setOfOperators.find(sLine[i]) == setOfOperators.end())
		{
			while (i < sLine.length() && ',' != sLine[i] && !isspace(sLine[i]) && setOfOperators.find(sLine[i]) == setOfOperators.end())
				sWord += sLine[i++];

			// If we read a symbol we've never seen before
			//
			if (pSymMap->find(sWord) == pSymMap->end())
			{
				cout << "ERROR: Undefined symbol in expression at line: " << Util::iCurrentFileLine << endl;
				Util::fHasErrors = true;
				return INT_MAX;
			}
		}


		// We have a token now
		//
		if ("" != sWord) {
			tokens.push_back(sWord);
			while (i < sLine.length() && isspace(sLine[i]) && ',' != sLine[i])
				i++;
			if (i == sLine.length())
				break;
			if (i < sLine.length() && ',' == sLine[i])
			{
				sLine = sLine.substr(i + 1, sLine.length());
				break;
			}
		}

		if (i < sLine.length() && setOfOperators.find(sLine[i]) != setOfOperators.end()) {
			string op;
			op += sLine[i];
			tokens.push_back(op);
			sLine = sLine.substr(i + 1, sLine.length());
		}
	}
	// Make sure the line is empty
	//
	sLine = "";

	// Turn it into a postfix
	//
	vector<string> postfix;
	for (int i = 0; i < tokens.size(); ++i) {
		if (setOfOperators.find(tokens[i][0]) == setOfOperators.end()) {
			// Operand, just print it
			//
			postfix.push_back(tokens[i]);
		}
		else {
			// Operator
			//
			if (operators.empty() || '(' == operators.top())
				operators.push(tokens[i][0]);
			else if (')' == tokens[i][0]) {
				// Pop until left parenthesis 
				//
				while ('(' != operators.top()) {
					string s;
					s += operators.top();
					postfix.emplace_back(s);
					operators.pop();
				}
				// Remove the '('
				//
				operators.pop();

			}
			else if ('*' == tokens[i][0] || '/' == tokens[i][0]) {
				// If there's same precedance on top of the stack
				//
				if ('*' == operators.top() || '/' == operators.top()) {
					string s;
					s += operators.top();
					postfix.emplace_back(s);
					operators.pop();
					operators.push(tokens[i][0]);
				}
				else {
					operators.push(tokens[i][0]);
				}
			}
			else if ('-' == tokens[i][0] || '+' == tokens[i][0]) {
				while (!operators.empty() && ('/' == operators.top() || '*' == operators.top())) {
					string s;
					s += operators.top();
					postfix.emplace_back(s);
					operators.pop();
				}
				if (!operators.empty() && ('-' == operators.top() || '+' == operators.top())) {
					string s;
					s += operators.top();
					postfix.emplace_back(s);
					operators.pop();
					operators.push(tokens[i][0]);
				}
				else {
					operators.push(tokens[i][0]);
				}
			}
			else if ('(' == tokens[i][0])
				operators.push(tokens[i][0]);

		}
	}
	while (!operators.empty()) {
		string s;
		s += operators.top();
		postfix.emplace_back(s);
		operators.pop();
	}


	sSymReloc = "";
	map<string, Symbol*>::iterator it1;
	map<string, Symbol*>::iterator it2;

	// Substitute all values for symbols
	// Assume that if two symbols susbstract they'll be in parenthasies
	//
	for (int i = postfix.size() - 1; i >= 0; --i)
	{
		// If we're substracting
		//
		if ("-" == postfix[i] && i >= 2)
		{
			if ((it1 = pSymMap->find(postfix[i - 1])) != pSymMap->end())
			{
				// We're trying to substract a symbol from the rest of expression
				// This is only possible if we're substracting from another symbol from the same section
				// And so we get a constant
				//
				if ((it2 = pSymMap->find(postfix[i - 2])) != pSymMap->end())
				{
					if (it1->second->m_iSectionId != it2->second->m_iSectionId)
					{
						cout << "ERROR: You're trying to substract two symbols from different sections! Line:" << Util::iCurrentFileLine << endl;
						Util::fHasErrors = true;
						return INT_MAX;
					}
					// This is OK state
					//
					postfix[i - 1] = to_string(it1->second->m_iOffset);
					postfix[i - 2] = to_string(it2->second->m_iOffset);
					i -= 2;
				}
				else
				{
					cout << "ERROR: Did you forget to put 2 symbols in () before substracting them? Line:" << Util::iCurrentFileLine << endl;
					Util::fHasErrors = true;
					return INT_MAX;
				}

			}
		}
		// We'll only hit this if a symbol is on its own, since the code above should handle when they're next to each other
		//
		else if ((it1 = pSymMap->find(postfix[i])) != pSymMap->end())
		{
			if ("" == sSymReloc)
			{
				if (-1 == it1->second->m_iSectionId)
					sSymReloc = it1->second->m_pSymReloc;
				else
					sSymReloc = it1->first;
				postfix[i] = to_string(it1->second->m_iOffset);
			}
			else
			{
				cout << "ERROR: There's more than one relocative symbol! Line: " << Util::iCurrentFileLine << endl;
				Util::fHasErrors = true;
				return INT_MAX;
			}
		}
	}

	stack<int> calculate;
	for (int i = 0; i < postfix.size(); ++i) {
		if (setOfOperators.find(postfix[i][0]) == setOfOperators.end()) {
			// Operand
			//
			int iDecValue = IGetIntValue(postfix[i]);
			if (INT_MAX == iDecValue) break;
			calculate.push(iDecValue);
		}
		else {
			int a = calculate.top();
			calculate.pop();
			int b = calculate.top();
			calculate.pop();

			switch (postfix[i][0]) {
			case '+':
				b += a;
				break;
			case '-':
				b -= a;
				break;
			case '*':
				b *= a;
				break;
			case '/':
				b /= a;
				break;
			default:
				//ILLEGAL STATE
				Util::fHasErrors = true;
				break;
			}
			calculate.push(b);
		}
	}

	if (!calculate.empty())
		return calculate.top();
	return INT_MAX;


}


// Used to test if instruction uses registry direct/indirect addressing
//
bool FUsesRegs(const string& str)
{
	int nospace = str.find_first_not_of(" ");
	str.substr(nospace, str.length());
	// Does it use PC?
	//
	if ("PC" == str || "[PC]" == str)
		return true;
	// Does it use SP?
	//
	if ("SP" == str || "[SP]" == str)
		return true;
	// Does it use R0..15?
	//
	string test("R");
	for (int i = 0; i < 16; ++i)
	{
		test += to_string(i);
		if (test == str)
			return true;
		test = "R";
	}

	test = "[R";
	for (int i = 0; i < 16; ++i)
	{
		test += to_string(i) + "]";
		if (test == str)
			return true;
		test = "[R";
	}


	return false;
}

bool FCheckDUP(const string& str)
{
	int i = 0;
	while (isspace(str[i]))
		i++;
	if ('D' == str[i] && 'U' == str[i + 1] && 'P' == str[i + 2])
		return true;
	return false;
}

unsigned int UiRegCode(string sWord)
{
	if ("PC" == sWord)
		return 0x11;
	if ('R' == sWord[0]) {
		char chCode[2];
		chCode[0] = sWord[1];
		chCode[1] = sWord[2];
		return atoi(chCode);
	}

	return 0x10;

}

void WriteMachineCode(int iValue, int iSize, vector<char>* vch, bool fLittleEndian)
{
	const int MASK = 0xFF;
	if (fLittleEndian)
	{
		for (int i = 0; i < iSize; ++i)
		{
			vch->push_back((iValue >> (i * 8)) & MASK);
		}
		
	}
	else
	{
		for (int i = iSize - 1; i >= 0; --i)
		{
			vch->push_back((iValue >> (i * 8)) & MASK);
		}
	}
}
