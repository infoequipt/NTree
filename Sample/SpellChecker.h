#pragma once
#include <vector>
#include "AddWordData.h"
#include "../NTree/NTree.h"

class SpellChecker
{
private:

	std::vector<std::string*>* _words;
	NTree* _tree = new NTree();
	AddWordData* _addWordData;

	void Initialize();

	std::vector<std::string*>* MakeStringVector(std::string& text);
	static bool AddWord(NTreeNodePtr node, void* parm);
	static bool IsCorrectLineage(NTreeNodePtr startNode, std::string* word, short index);
	static NTreeNode* FindChildWithLetter(NTreeNode* parent, char letter);
	static NTreeNode* FindChildWithWord(NTreeNode* parent, std::string* word);
	static bool DumpNode_ActionFunc(NTreeNodePtr inNode, void* inParam);

public:

	SpellChecker(std::string dictionary);
	~SpellChecker();

	bool CheckSpelling(std::string* word);
	void Dump();
};

