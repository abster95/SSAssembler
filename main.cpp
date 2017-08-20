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
			else if ((it = CInstruction::s_pInstructionMap->find(sWord)) != CInstruction::s_pInstructionMap->end()) {
				iLocationCounter += 4;
				if (it->second->m_fMemAccess) {
					// We are interested only in the last word, since it tells us about the address type
					//
					for (int i = 0; i < it->second->m_cBytes; ++i) {
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
				iStartAddress = IComputeExpr(sLine, pSymMap, sSymReloc);
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

				while ("" != (sWord = SGetWord(sLine))){
					// Either the word after this is DUP or this is the value that should be written to this memory
					//
					int iAmount = IComputeExpr(sWord, pSymMap, sSymReloc);
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
			// If none of  code above executes then it should be symbol DEF expression
			//
			else
			{
				//  Symbol name is in our sWord, get the DEF keyword
				//
				if ("DEF" != SGetWord(sLine)){
				
					cout << "ERROR: Bad syntax! Line: "<< Util::iCurrentFileLine << std::endl;
					exit(1);
				}
				int iValue = IComputeExpr(sLine, pSymMap, sSymReloc);
				Symbol* pSymTemp = new Symbol(psymCurrSection->m_iSymbolId, iLocationCounter, iValue, 'L');
				pSymTemp->m_pSymReloc = sSymReloc;
				pSymMap->insert(pair<string, Symbol*>(sWord, pSymTemp));

			}

			// 

		}
		//at .end we've gone through everything we need
		if (".end" == sWord) {
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
				while ("" != sWord){
					if ((itSymMap = pSymMap->find(sWord)) != pSymMap->end())
					{
						itSymMap->second->m_chFlag = 'G';
					}
					else
					{
						cout << "ERROR: Unused symbol in .global" << endl;
						exit(1);
					}
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
			else if (string::npos == sWord.find(".data") && string::npos != sWord.find(".text") && string::npos != sWord.find(".rodata"))
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
				while ("" != (sWord = SGetWord(sLine))){
					// Either the word after this is DUP or this is the value that should be written to this memory
					//
					int iValue = IComputeExpr(sWord, pSymMap, sSymReloc);
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
						iValue = IComputeExpr(sWord, pSymMap, sSymReloc);
					}
					if ("" != sSymReloc)
					{
						itSymMap = pSymMap->find(sSymReloc);
						if (('$' == sSymReloc[0] && psymCurrSection->m_iOffset ==0) || (itSymMap != pSymMap->end() && ) )

					}
				}

			}
		}


		if (".end" == sWord) {
			cout << "End of first pass!" << endl;
			break;
		}
	}
	
}