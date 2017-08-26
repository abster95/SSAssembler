#include "instruction.h"

map<string, CInstruction*>* CInstruction::s_pInstructionMap = nullptr;

CInstruction::CInstruction(int iOpCode, int cBytes, bool fMemAccess)
	: m_iOpCode(iOpCode), m_cBytes(cBytes), m_fMemAccess(fMemAccess)
{
}

void CInstruction::InitializeMap()
{
	if (nullptr != s_pInstructionMap) {
		s_pInstructionMap->clear();
	} else {
		s_pInstructionMap = new map<string, CInstruction*>();
	}

	s_pInstructionMap->insert(std::pair<string, CInstruction*>("ADD", new CInstruction(0x30, 3, false)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("SUB", new CInstruction(0x31, 3, false)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("MUL", new CInstruction(0x32, 3, false)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("DIV", new CInstruction(0x33, 3, false)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("MOD", new CInstruction(0x34, 3, false)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("AND", new CInstruction(0x35, 3, false)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("OR", new CInstruction(0x36, 3, false)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("XOR", new CInstruction(0x37, 3, false)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("NOT", new CInstruction(0x38, 2, false)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("ASL", new CInstruction(0x39, 3, false)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("ASR", new CInstruction(0x3A, 3, false)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("PUSH", new CInstruction(0x20, 1, false)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("POP", new CInstruction(0x21, 1, false)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("LOAD", new CInstruction(0x10, 2, true)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("LOADUB", new CInstruction(0x10, 2, true)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("LOADSB", new CInstruction(0x10, 2, true)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("LOADUW", new CInstruction(0x10, 2, true)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("LOADSW", new CInstruction(0x10, 2, true)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("STORE", new CInstruction(0x11, 2, true)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("STOREB", new CInstruction(0x11, 2, true)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("STOREW", new CInstruction(0x11, 2, true)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("JLEZ", new CInstruction(0x09, 2, true)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("JLZ", new CInstruction(0x08, 2, true)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("JGEZ", new CInstruction(0x07, 2, true)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("JGZ", new CInstruction(0x06, 2, true)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("JNZ", new CInstruction(0x05, 2, true)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("JZ", new CInstruction(0x04, 2, true)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("RET", new CInstruction(0x01, 0, false)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("CALL", new CInstruction(0x03, 1, true)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("JMP", new CInstruction(0x02, 1, true)));
	s_pInstructionMap->insert(std::pair<string, CInstruction*>("INT", new CInstruction(0x00, 1, true)));


}
