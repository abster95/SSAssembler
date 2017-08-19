#pragma once

class Symbol {
public:
	static int s_iIdCount;
	int m_iSymbolId;
	int m_iSectionId;
	int m_iOffset;
	int m_iValue;
	char m_chFlag;

	Symbol(int iSectionId, int iOffset, int iValue, char chFlag = 'L')
		: m_iSectionId(iSectionId), m_iOffset(iOffset), m_iValue(iValue), m_chFlag(chFlag)
	{
		m_iSymbolId = s_iIdCount++;
	}

};

