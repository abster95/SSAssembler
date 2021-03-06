#pragma once


//Class: CRelocTE
//purpose:
//Represents an element of the relocation table
//hungarian: rte

class CRelocTE {
public:
	CRelocTE(
		int iOffset,
		char chType,
		int iSectionId
	) : m_iOffset(iOffset), m_chType(chType), m_iSectionId(iSectionId) {};
	int m_iOffset;
	char m_chType;
	int m_iSectionId;
};