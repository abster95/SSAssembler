#pragma once

#include <vector>
#include <string>
#include "relocation.h"
using namespace std;
class Symbol {
public:
	static int s_iIdCount;
	int m_iSymbolId;
	int m_iSectionId;
	int m_iOffset;
	int m_iValue;
	char m_chFlag;
	

	vector<char> machineCode;
	vector<CRelocTE*> reloc;
	string m_pSymReloc = string("");

	Symbol(int iSectionId, int iOffset, int iValue, char chFlag = 'L')
		: m_iSectionId(iSectionId), m_iOffset(iOffset), m_iValue(iValue), m_chFlag(chFlag)
	{
		m_iSymbolId = s_iIdCount++;
	}

};
