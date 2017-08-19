#pragma once

#include <map>
#include <string>
using namespace std;

class CInstruction {
public:

	CInstruction(int iOpCode, int cBytes, bool fMemAccess);

	static map<string, CInstruction*> *s_pInstructionMap;
	static void InitializeMap();

	int m_iOpCode; //the operation code
	int m_cBytes; //how many bytes the instruction has
	bool m_fMemAccess; //if it uses any of the addressing modes
};