#define WIN32_LEAN_AND_MEAN
#include <string>

#include "SpellChecker.h"

using namespace std;

extern string DICTIONARY;

int main()
{
	SpellChecker* spellChecker = new SpellChecker(DICTIONARY);

	spellChecker->Dump();

	string wordToCheck("fish");
	bool correct = spellChecker->CheckSpelling(&wordToCheck);
	_RPT1(_CRT_WARN, "%s is spelled %s\n", wordToCheck.c_str(), correct ? "correctly" : "incorrectly");

	wordToCheck = "fushu";
	correct = spellChecker->CheckSpelling(&wordToCheck);
	_RPT1(_CRT_WARN, "%s is spelled %s\n", wordToCheck.c_str(), correct ? "correctly" : "incorrectly");

	wordToCheck = "dogged";
	correct = spellChecker->CheckSpelling(&wordToCheck);
	_RPT1(_CRT_WARN, "%s is spelled %s\n", wordToCheck.c_str(), correct ? "correctly" : "incorrectly");

	wordToCheck = "doged";
	correct = spellChecker->CheckSpelling(&wordToCheck);
	_RPT1(_CRT_WARN, "%s is spelled %s\n", wordToCheck.c_str(), correct ? "correctly" : "incorrectly");
	
	delete spellChecker;
}

