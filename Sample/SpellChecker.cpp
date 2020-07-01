#define WIN32_LEAN_AND_MEAN
#include <cstdlib>
#include <cstddef>
#include <string>
#include <vector>

#include "../NTree/NTree.h"
#include "../NTree/NTreeNode.h"

#include "LetterNode.h"
#include "WordNode.h"
#include "AddWordData.h"
#include "SpellChecker.h"

using namespace std;

unsigned short _nextNodeId = 0;

SpellChecker::SpellChecker(string dictionary)
{
	Initialize();
	_words = MakeStringVector(dictionary);
	_tree = new NTree();
	_addWordData = new AddWordData();

	for (size_t i = 0; i < _words->size(); i += 1)
	{
		_addWordData->word = _words->at(i);
		_addWordData->index = 0;
		_tree->VisitAllNTreeNodes(
			_tree->GetRoot(),
			AddWord,
			_addWordData,
			NTree::kActionOnEntry,
			NTree::kEntireTree
		);
	}
}

SpellChecker::~SpellChecker()
{
	if (_words != nullptr)
	{
		for (size_t i = 0; i < _words->size(); i += 1) delete _words->at(i);
		_words->clear();
	}

	delete _words;
	delete _addWordData;
}

void SpellChecker::Initialize()
{
	_nextNodeId = 0;
	_words = nullptr;
	_tree = nullptr;
	_addWordData = nullptr;
}

vector<string*>* SpellChecker::MakeStringVector(string& text)
{
	vector<string*>* words = new vector<string*>();
	const char* DELIMS = " ";
	char* next_token = nullptr;
	char* token = strtok_s(const_cast<char*>(text.c_str()), DELIMS, &next_token);

	while (token != nullptr)
	{
		words->push_back(new string(token));
		token = strtok_s(nullptr, DELIMS, &next_token);
	}

	return words;
}


NTreeNodeID GetNextNodeId()
{
	_nextNodeId += 1;
	return _nextNodeId;
}


bool SpellChecker::AddWord(NTreeNodePtr node, void* parm)
{
	if (node->GetType() == NTreeNodeType('WORD'))
		return false;

	AddWordData* data = static_cast<AddWordData*>(parm);
	string* word = data->word;
	short index = data->index;

	if (!IsCorrectLineage(node, word, index))
		return false;

	if (index == word->length())
	{
		if (FindChildWithWord(node, word) == nullptr)
			node->InsertChild(new WordNode(word));

		return true;
	}

	const char letter = word->c_str()[index];
	if (FindChildWithLetter(node, letter) == nullptr)
	{
		NTreeNodePtr newNode = new LetterNode(letter);
		node->InsertChild(newNode);
	}

	data->index += 1;

	return false;
}

bool SpellChecker::IsCorrectLineage(NTreeNodePtr startNode, string* word, short index)
{
	if (startNode == nullptr || startNode->GetType() == NTreeNodeType('WORD'))
		return false;

	if (startNode->IsRoot())
		return true;

	LetterNode* letterNode = static_cast<LetterNode*>(startNode);
	short i = index - 1;
	NTreeNodePtr node = startNode;
	while ((i >= 0)
		&& !node->IsRoot()
		&& (tolower(word->c_str()[i]) == letterNode->letter))
	{
		i -= 1;
		node = node->GetParent();
		letterNode = static_cast<LetterNode*>(node);
	}

	return node->IsRoot();
}

NTreeNode* SpellChecker::FindChildWithLetter(NTreeNode* parent, char letter)
{
	int i = 0;
	bool exists = false;
	NTreeNode* child = nullptr;
	while (!exists && i < parent->GetNumChildren())
	{
		child = parent->GetChild(i);
		if (child->GetType() == NTreeNodeType('LETR'))
		{
			LetterNode* letterNode = static_cast<LetterNode*>(child);
			exists = letterNode->letter == letter;
		}

		i += 1;
	}

	return exists ? child : nullptr;
}

NTreeNode* SpellChecker::FindChildWithWord(NTreeNode* parent, string* word)
{
	int i = 0;
	bool exists = false;
	NTreeNode* child = nullptr;
	while (i < parent->GetNumChildren() && !exists)
	{
		child = parent->GetChild(i);
		if (child->GetType() == NTreeNodeType('WORD'))
		{
			WordNode* wordNode = static_cast<WordNode*>(child);
			exists = wordNode->word.compare(*word) == 0;
		}

		i += 1;
	}

	return exists ? child : nullptr;
}

bool SpellChecker::CheckSpelling(string* word)
{
	size_t i = 0;
	NTreeNode* node = _tree->GetRoot();
	char letter = 0x00;
	while (i < word->length())
	{
		letter = word->c_str()[i];
		node = FindChildWithLetter(node, letter);
		if (node == nullptr) return false;
		i += 1;
	}

	return FindChildWithWord(node, word) != nullptr;
}

void SpellChecker::Dump()
{
	_tree->Dump(DumpNode_ActionFunc);
}


bool SpellChecker::DumpNode_ActionFunc(NTreeNodePtr inNode, void* inParam)
{
	NTree::DumpNode_ActionFunc(inNode, inParam);

	if (inNode->GetType() == NTreeNodeType('LETR'))
	{
		LetterNode* letterNode = static_cast<LetterNode*>(inNode);
		_RPT1(_CRT_WARN, "%c", letterNode->letter);
	}
	else if (inNode->GetType() == NTreeNodeType('WORD'))
	{
		WordNode* wordNode = static_cast<WordNode*>(inNode);
		_RPT1(_CRT_WARN, "%s", wordNode->word.c_str());
	}

	_RPT0(_CRT_WARN, "\n");

	return false;
}