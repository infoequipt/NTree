#pragma once
#include <string>
#include "../NTree/NTreeNode.h"

extern NTreeNodeID GetNextNodeId();;

class WordNode : public NTreeNode
{
public:

std::string word;

	WordNode(std::string* w) : NTreeNode(NTreeNodeType('WORD'), GetNextNodeId())
	{
		word.assign(*w);
	}
};
