#include <fstream>
#include <iostream>
#include <string>
#include <map>
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
	map<string, CInstruction*>::iterator it;
	map<string, Symbol*> * pSymMap = new map<string, Symbol*>();
	int iStartAddress = 0;
	int iLocationCounter = 0;
	Symbol *psymCurrSection = nullptr;
	Symbol *pSymReloc;
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
			else if ((it = CInstruction::s_pInstructionMap->find(sWord)) != CInstruction::s_pInstructionMap->end()) {
				iLocationCounter += 4;
				if (it->second->m_fMemAccess) {
					// We are interested only in the last word, since it tells us about the address type
					//
					for (int i = 0; i < it->second->m_cBytes; ++i) {
						sWord = SGetWord(sLine);
					}
					if (FUsesRegs(sWord))
						iLocationCounter += 4;
				}
			}
			// If it's ORG dircetive
			//
			else if ("ORG" == sWord) {
				iStartAddress = IComputeExpr(sLine, pSymMap, pSymReloc);
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
				default:
					cBytes = 0;
					break;
				}

				while ("" != (sWord = SGetWord(sLine))){
					// Either the word after this is DUP or this is the value that should be written to this memory
					//
					int iAmount = IComputeExpr(sWord, pSymMap, pSymReloc);
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
					} else
					{
						iLocationCounter += cBytes;
					}
				}
			}

			// 

		}
		//at .end we've gone through everything we need
		if (sWord == ".end") {
			cout << "End of first pass!" << endl;
			break;
		}
		if (ifs.eof())
		{
			cout << "ERROR: End of file reached before \".end\" found!" << std::endl;
			exit(1);
		}
		if (Util::fHasErrors)
		{
			cout << "File failed to compile due to errors" << endl;
			exit(1);
		}
	}
}