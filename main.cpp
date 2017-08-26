#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "instruction.h"
#include "symbol.h"
#include "Util.h"
using namespace std;



int main(int argc, char* argv[]) {
	if (argc != 3) {
		cout << "Wrong parameters! Use: ./ssassembler inputFile outputFile" << endl;
		exit(1);
	}

	ifstream ifs = ifstream(argv[1], ifstream::in);
	if (!ifs.is_open()) {
		cout << "Can't read input file!" << endl;
		exit(1);
	}

	const int LINELENGTH = 2048;

	CInstruction::InitializeMap();
	map<string, CInstruction*>::iterator itInstruct;
	map<string, Symbol*> * pSymMap = new map<string, Symbol*>();
	int iStartAddress = 0;
	int iLocationCounter = 0;
	Symbol *psymCurrSection = nullptr;
	string sSymReloc;
	char rgchLine[LINELENGTH];

	while (!ifs.eof()) {
		ifs.getline(rgchLine, LINELENGTH);
		string sLine(rgchLine);
		++Util::iCurrentFileLine;
		string sWord;
		while ("" != sLine) {
			sWord = SGetWord(sLine);
			if ("" == sWord)
				break;
			// We're not interested in .global symbols and comments so skip this whole line
			//
			if (".global" == sWord || ".end" == sWord || sWord[0] == ';') break;

			// If this is a new section
			//
			if (0 == sWord.find(".text") || 0 == sWord.find(".data") || 0 == sWord.find(".rodata") || 0 == sWord.find(".bss")) {
				if (nullptr != psymCurrSection) {
					psymCurrSection->m_iValue = iLocationCounter - psymCurrSection->m_iOffset;
				}
				psymCurrSection = new Symbol(Symbol::s_iIdCount, iStartAddress, 0, 'L');
				pSymMap->insert(pair<string, Symbol*>(sWord, psymCurrSection));
				iLocationCounter = iStartAddress;
				iStartAddress = 0;
			}
			// If it's a lable
			//
			else if (':' == sWord[sWord.length() - 1]) {
				sWord = sWord.erase(sWord.length() - 1, 1);
				Symbol* psym = new Symbol((nullptr == psymCurrSection) ? -2 : psymCurrSection->m_iSymbolId, iLocationCounter, 0, 'L');
				pSymMap->insert(pair<string, Symbol*>(sWord, psym));
			}
			// If it's an instruction
			//
			else if ((itInstruct = CInstruction::s_pInstructionMap->find(sWord)) != CInstruction::s_pInstructionMap->end()) {
				iLocationCounter += 4;
				if (itInstruct->second->m_fMemAccess) {
					// We are interested only in the last word, since it tells us about the address type
					//
					for (int i = 0; i < itInstruct->second->m_cBytes; ++i) {
						sWord = SGetWord(sLine);
					}
					if (!FUsesRegs(sWord))
						iLocationCounter += 4;
				}
				sLine = "";
			}
			// If it's ORG dircetive
			//
			else if ("ORG" == sWord) {
				iStartAddress = IComputeExpr(sLine, pSymMap, sSymReloc, iLocationCounter);
				if (INT_MAX == iStartAddress)
				{
					cout << "ERROR: Bad expression format on line: " << Util::iCurrentFileLine << endl;
					Util::fHasErrors = true;
				}
				break;
			}

			else if ("DB" == sWord || "DW" == sWord || "DD" == sWord)
			{
				// Determine how many bytes to leave
				//
				int cBytes;
				switch (sWord[1])
				{
				case 'B':
					cBytes = 1;
					break;
				case 'W':
					cBytes = 2;
					break;
				case 'D':
					cBytes = 4;
					break;
				default:
					cBytes = 0;
					break;
				}

				while ("" != (sWord = SGetWord(sLine))) {
					// Either the word after this is DUP or this is the value that should be written to this memory
					//
					int iAmount = IComputeExpr(sWord, pSymMap, sSymReloc, iLocationCounter);
					if (FCheckDUP(sLine))
					{
						// The amount tells us how many bytes to leave here
						//
						iLocationCounter += iAmount * cBytes;


						// Get DUP
						//
						SGetWord(sLine);
						//The next word is what should be written to this memory
						//
						SGetWord(sLine);
					}
					else
					{
						iLocationCounter += cBytes;
					}
				}
			}
			// If none of  code above executes then it should be symbol DEF expression
			//
			else
			{
				//  Symbol name is in our sWord, get the DEF keyword
				//
				if ("DEF" != SGetWord(sLine)) {

					cout << "ERROR: Bad syntax! Line: " << Util::iCurrentFileLine << std::endl;
					exit(1);
				}
				int iValue = IComputeExpr(sLine, pSymMap, sSymReloc, iLocationCounter);
				Symbol* pSymTemp = new Symbol(-1, iValue, iValue, 'L');
				pSymTemp->m_pSymReloc = sSymReloc;
				pSymMap->insert(pair<string, Symbol*>(sWord, pSymTemp));

			}

			// 

		}
		//at .end we've gone through everything we need
		if (".end" == sWord) {
			if (nullptr != psymCurrSection)
				psymCurrSection->m_iValue = iLocationCounter;
			cout << "End of first pass!" << endl;
			break;
		}
		if (ifs.eof())
		{
			cout << "ERROR: End of file reached before \".end\" found!" << std::endl;
			exit(1);
		}
	}
	if (Util::fHasErrors)
	{
		cout << "File failed to compile due to errors" << endl;
		exit(1);
	}

	psymCurrSection = nullptr;
	sSymReloc = "";
	iLocationCounter = 0;

	ifs.clear();
	ifs.seekg(0, ios::beg);
	Util::iCurrentFileLine = 0;
	map<string, Symbol*>::iterator itSymMap;
	// Get a vector where symbol id is it's index for easier searching
	//
	vector<Symbol*> vSymbolsByIndex(pSymMap->size() + 1);
	for (itSymMap = pSymMap->begin(); itSymMap != pSymMap->end(); ++itSymMap) {
		vSymbolsByIndex[itSymMap->second->m_iSymbolId] = itSymMap->second;
	}
	itSymMap = pSymMap->end();
	while (!ifs.eof())
	{
		ifs.getline(rgchLine, LINELENGTH);
		string sLine(rgchLine);
		++Util::iCurrentFileLine;
		string sWord;
		while ("" != sLine)
		{
			sWord = SGetWord(sLine);
			if (".end" == sWord || "ORG" == sWord || ';' == sWord[0])
				break;
			if (".global" == sWord)
			{
				sWord = SGetWord(sLine);
				while ("" != sWord) {
					if ((itSymMap = pSymMap->find(sWord)) != pSymMap->end())
					{
						itSymMap->second->m_chFlag = 'G';
					}
					else
					{
						cout << "ERROR: Unused symbol in .global" << endl;
						exit(1);
					}
					sWord = SGetWord(sLine);
				}
			}
			// If .bss
			else if (0 == sWord.find(".bss"))
			{
				// Skip if .bss section until next section
				//
				while (string::npos == sLine.find(".data") && string::npos != sLine.find(".text") && string::npos != sLine.find(".rodata"))
				{
					ifs.getline(rgchLine, LINELENGTH);
					sLine = string(rgchLine);
				}
			}
			// Section start
			//
			else if (string::npos != sWord.find(".data") || string::npos != sWord.find(".text") || string::npos != sWord.find(".rodata"))
			{
				psymCurrSection = pSymMap->find(sWord)->second;
				iLocationCounter = psymCurrSection->m_iOffset;
			}

			else if ("DB" == sWord || "DW" == sWord || "DD" == sWord)
			{
				// Determine how many bytes to leave
				//
				int cBytes;
				switch (sWord[1])
				{
				case 'B':
					cBytes = 1;
					break;
				case 'W':
					cBytes = 2;
					break;
				case 'D':
					cBytes = 4;
					break;
				default:
					cBytes = 0;
					break;
				}
				while ("" != (sWord = SGetWord(sLine))) {
					// Either the word after this is DUP or this is the value that should be written to this memory
					//
					int iValue = IComputeExpr(sWord, pSymMap, sSymReloc, iLocationCounter);
					int iCount = 1;
					if (FCheckDUP(sLine))
					{
						// Get DUP
						//
						SGetWord(sLine);

						iCount = iValue;

						//The next word is what should be written to this memory
						//
						sWord = SGetWord(sLine);
						iValue = IComputeExpr(sWord, pSymMap, sSymReloc, iLocationCounter);
					}
					if ("" != sSymReloc)
					{
						itSymMap = pSymMap->find(sSymReloc);
						// If global symbol or not from an ORGed section we need to reloc
						//
						if (itSymMap != pSymMap->end() && (vSymbolsByIndex[itSymMap->second->m_iSectionId]->m_iSectionId == 0 || (0 == itSymMap->second->m_iSectionId))) {
							if ('G' == itSymMap->second->m_chFlag) {
								for (int i = 0; i < iCount; ++i) {
									// If global, put in own id
									//
									psymCurrSection->reloc.push_back(new CRelocTE(iLocationCounter + i*cBytes, 'A', itSymMap->second->m_iSymbolId));

								}
							}
							else {

								for (int i = 0; i < iCount; ++i) {
									// If global, put in section id
									//
									psymCurrSection->reloc.push_back(new CRelocTE(iLocationCounter + i*cBytes, 'A', itSymMap->second->m_iSectionId));

								}
							}
						}
					}
					// TODO: Add write to machine code here
					for (int i = 0; i < iCount; ++i)
					{
						WriteMachineCode(iValue, cBytes, &(psymCurrSection->machineCode), true);
					}
					iLocationCounter += cBytes * iCount;
				}

			}

			// Instruction
			else if ((itInstruct = CInstruction::s_pInstructionMap->find(sWord)) != CInstruction::s_pInstructionMap->end()) {
				unsigned int uiCode = static_cast<unsigned int>(itInstruct->second->m_iOpCode);
				uiCode <<= 8;
				unsigned int uiMode = 0;
				unsigned int uiType = 0;
				unsigned int uiSecondWord = 0;
				bool fHasSecondWord = false;

				if (6 == sWord.length()) {
					// UB SB UW SW...
					uiType = 1;
					if ('S' == sWord[4])
						uiType |= 4;
					if ('B' == sWord[5])
						uiType |= 2;

				}

				// Only the last word can be something other than a register
				for (int i = 0; i < itInstruct->second->m_cBytes - 1; ++i) {
					sWord = SGetWord(sLine);
					uiCode |= UiRegCode(sWord);
					uiCode <<= 5; // make room for next register
				}

				// Addressing the data
				//
				if (itInstruct->second->m_fMemAccess) {
					sWord = SGetWord(sLine);
					if (FUsesRegs(sWord)) {
						string stmp;
						// RegInd
						//
						if ('[' == sWord[0]) {
							uiMode = 2;
							string stmp;
							for (int i = 1; sWord[i] != ']'; ++i) {
								stmp += sWord[i];
							}
						} // if ('[' == sWord[0])
						// RegDir
						//
						else {
							stmp = sWord;
						}
						uiCode |= UiRegCode(stmp);
					} // if (FUsesRegs(sWord))
					else {
						// The last one wasn't a register
						// We need 4B more for the operand
						//
						iLocationCounter += 4;
						fHasSecondWord = true;
						bool fDollarSign =  false;
						if ('#' == sWord[0]) {
							// Remove the character
							//
							sWord.erase(0, 1);
							uiMode = 4;
						} // if ('#' == sWord)
						else if ('[' == sWord[0]) {
							uiMode = 7;
							sWord.erase(0, 1);
							string sReg = SGetWord(sWord);
							uiCode |= UiRegCode(sReg);
							int i = 0;
							while ('+' != sWord[i] && '-' != sWord[i])
								++i;
							// Get rid of the ']' and the '+'
							//
							if ('+' == sWord[i])
								sWord = sWord.substr(i + 1, sWord.length()-2);
							// Leave the '-' in
							//
							else
								sWord = sWord.substr(i, sWord.length()-2);
						} // else if ('[' == sWord[0])
						else if ('$' == sWord[0])
						{
							uiMode = 7;
							uiCode |= 0x11;
							fDollarSign = true;
							sWord.erase(0, 1);
						} // else if ('$' == sWord[0])
						else
						{
							uiMode = 6;
						}

						uiSecondWord = IComputeExpr(sWord, pSymMap, sSymReloc, iLocationCounter);
						if ("" != sSymReloc)
						{
							if (7 == uiMode && ((uiCode & 0x1f) == 0x11))
							{
								itSymMap = pSymMap->find(sSymReloc);
								uiSecondWord -= 4;
								// If it's not a dollar sign and sections mismatch it's relative reloc
								//
								if (!fDollarSign && itSymMap->second->m_iSectionId != psymCurrSection->m_iSymbolId)
								{
									// If current section isn't ORG-ed or the symbol is global or symbol's section isn't ORG-ed
									//
									if ((0 == psymCurrSection->m_iOffset) || ('G' == itSymMap->second->m_chFlag) || (0 == vSymbolsByIndex[itSymMap->second->m_iSectionId]->m_iOffset))
									{
										if ('G' == itSymMap->second->m_chFlag)
											psymCurrSection->reloc.push_back(new CRelocTE(iLocationCounter, 'R', itSymMap->second->m_iSymbolId));
										else
											psymCurrSection->reloc.push_back(new CRelocTE(iLocationCounter, 'R', itSymMap->second->m_iSectionId));
									} // if ((0 == psymCurrSection->m_iOffset) || ('G' == itSymMap->second->m_chFlag) || (0 == sad[itSymMap->second->m_iSectionId]->m_iOffset))
									else
									{
										uiSecondWord -= iLocationCounter;
										if ('G' == itSymMap->second->m_chFlag)
											uiSecondWord += itSymMap->second->m_iOffset;
									} 
								} // if (!fDollarSign && itSymMap->second->m_iSectionId != psymCurrSection->m_iSymbolId)
								else
								{
									uiSecondWord -= iLocationCounter;
								}
							} // if (7 == uiMode && ((uiCode & 0x1f) == 0x11))
							else
							{
								itSymMap = pSymMap->find(sSymReloc);
								if (((0 == psymCurrSection->m_iOffset) && fDollarSign) || ('G' == itSymMap->second->m_chFlag || (0 == vSymbolsByIndex[itSymMap->second->m_iSectionId]->m_iOffset)))
								{
									if (fDollarSign)
									{
										psymCurrSection->reloc.push_back(new CRelocTE(iLocationCounter, 'A', psymCurrSection->m_iSymbolId));
									}
									else
									{
										if ('G' == itSymMap->second->m_chFlag)
										{
											psymCurrSection->reloc.push_back(new CRelocTE(iLocationCounter, 'A', itSymMap->second->m_iSymbolId));
										}
										else
										{
											psymCurrSection->reloc.push_back(new CRelocTE(iLocationCounter, 'A', itSymMap->second->m_iSectionId));
										}
									}
								} // if (((0 == psymCurrSection->m_iOffset) && fDollarSign) || ('G' == itSymMap->second->m_chFlag || (0 == sad[itSymMap->second->m_iSectionId]->m_iOffset)))
								else if ('G' == itSymMap->second->m_chFlag)
									uiSecondWord += itSymMap->second->m_iOffset;
							}
						}
					} // !FUsesRegs
				} // if (itInstruct->second->m_fMemAccess)
				else if (itInstruct->second->m_cBytes > 0)
				{
					sWord = SGetWord(sLine);
					uiCode |= UiRegCode(sWord);
				}
				
				uiCode <<= (21 - 5 * (itInstruct->second->m_cBytes ? itInstruct->second->m_cBytes : 1));
				uiCode |= uiMode << 21;
				uiCode |= uiType << 3;
				WriteMachineCode(uiCode, 4, &(psymCurrSection->machineCode), false);
				if (fHasSecondWord)
				{
					WriteMachineCode(uiSecondWord, 4, &(psymCurrSection->machineCode), true);
				}
				iLocationCounter += 4;
				sLine = "";
			}

		}


		if (".end" == sWord) {
			cout << "End of second pass!" << endl;
			break;
		}
	}
	
	ifs.close();
	ofstream ofs = ofstream(argv[2], ofstream::out);
	if (!ofs.is_open())
	{
		cout << "ERROR: Can't open output file" << endl;
		exit(1);
	}

	ofs << "#TabelaSimbola" << endl;
	for (itSymMap = pSymMap->begin(); itSymMap != pSymMap->end(); ++itSymMap)
	{
		if ('.' == itSymMap->first[0]){
			ofs << "SEG\t" << itSymMap->second->m_iSymbolId << "\t" << itSymMap->first << "\t" << itSymMap->second->m_iSectionId << "\t0x" << hex << itSymMap->second->m_iOffset;
			ofs.unsetf(ios::hex);
			ofs << "\t0x" << hex << itSymMap->second->m_iValue;
			ofs.unsetf(ios::hex);
			if (string::npos != itSymMap->first.find(".text"))
			{
				ofs << "\tRWX" << endl;
			}
			else
			{
				ofs << "\tRW" << endl;
			}
		}
		else
		{
			ofs << "SYM\t" << itSymMap->second->m_iSymbolId << "\t" << itSymMap->first << "\t" << itSymMap->second->m_iSectionId << "\t0x" << hex << itSymMap->second->m_iOffset;
			ofs.unsetf(ios::hex);
			ofs << "\t" << itSymMap->second->m_chFlag << endl;
		}
	}
	ofs << endl;
	for (itSymMap = pSymMap->begin(); itSymMap != pSymMap->end(); ++itSymMap)
	{
		if ('.' == itSymMap->first[0])
		{
			ofs << "#rel" << itSymMap->first << endl;
			for (int i = 0; i < itSymMap->second->reloc.size(); ++i)
			{
				ofs << "0x" << hex << itSymMap->second->reloc[i]->m_iOffset;
				ofs.unsetf(ios::hex);
				ofs << "\t" << itSymMap->second->reloc[i]->m_chType << "\t" << itSymMap->second->reloc[i]->m_iSectionId << endl;
			}
		}
	}
	for (itSymMap = pSymMap->begin(); itSymMap != pSymMap->end(); ++itSymMap)
	{
		if ('.' == itSymMap->first[0])
		{
			ofs.unsetf(ios::hex);
			ofs << itSymMap->first << endl;
			ofs << hex;
			int iCounter = -1;
			for (int i = 0; i < itSymMap->second->machineCode.size(); ++i)
			{
				if (++iCounter >= 16)
				{
					ofs << endl;
					iCounter = 0;
				}
				unsigned short usUpper = (0xf0 & itSymMap->second->machineCode[i]) >> 4;
				unsigned short usLower = 0xf & itSymMap->second->machineCode[i];
				ofs << hex << usUpper << usLower;
				ofs.unsetf(ios::hex);
				ofs << " ";
			}
			ofs << endl;
		}
	}
	ofs << "#end";
	ofs.close();
	return 0;
}