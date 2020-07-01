#pragma once
#include "../NTree/NTreeNode.h"

extern NTreeNodeID GetNextNodeId();;

class LetterNode : public NTreeNode
{
public:

	char letter;

	LetterNode(char l) : NTreeNode(NTreeNodeType('LETR'), GetNextNodeId())
	{
		letter = l;
	}
};
