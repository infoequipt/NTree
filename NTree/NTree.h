/*
	The MIT License (MIT)
	Copyright © 2020 Douglas Corarito

	Permission is hereby granted, free of charge, to any person obtaining a copy of
	this software and associated documentation files (the “Software”), to deal in the
	Software without restriction, including without limitation the rights to use, copy,
	modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
	and to permit persons to whom the Software is furnished to do so, subject to the
	following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
	PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
	OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef _NTREE_
#define _NTREE_

// --------------------------------------------------------------------------------
/*
		NTree.h

		A NTree is composed of NTreeNodes (See NTreeNode.h).  The NTree object exists
		to add/remove, read/write and traverse the NTreeNodes.

		VisitAllNTreeNodes is the heart of the NTree system.  It handles the depth
		first search of each node in the tree.  See that function for more information.
*/
// --------------------------------------------------------------------------------
#include "NTreeNode.h"

#ifndef NULL
#define NULL 0
#endif

namespace tinyxml2 {
	class XMLElement;
}

// --------------------------------------------------------------------------------
/*
	The limiting factor in the recursion depth of the VisitAllNTreeNodes function is the
	number of bits available to save "visited" state for each node.  This is currently
	implemented using 1 bit in an unsigned long (32 bits total) so therefore the max
	number of times VisitAllNTreeNodes() can be entered is 32.
*/
// --------------------------------------------------------------------------------
const short kMaxRecursion = 32;

typedef bool (*NTreeNodeActionFunc)(NTreeNodePtr, void*);

class NTree;
typedef class NTree* NTreePtr;

class NTreeNodeRoot : public NTreeNode
{
public:

	enum
	{
		kType = 'ROOT',
		kID = 0xFFFF
	};

	NTreeNodeRoot(void);
	NTreeNodeRoot(NTreePtr);
	virtual ~NTreeNodeRoot();

	NTreePtr GetTree(void) const;
	void SetTree(NTreePtr);

private:

	NTreePtr fTree;
};


class NTree
{
public:

	enum
	{
		kActionOnEntry = true,
		kActionOnExit = false,
		kJustThisBranch = true,
		kEntireTree = false,
		kNumVisitedBytes = 8192
	};

	struct TreeWriteInfo
	{
		void* file;
		long offset;
		long version;
		NTreeNodePtr root;
		NTreeNodeWriteCB writeCB;
	};
	typedef struct TreeWriteInfo TreeWriteInfo;

	struct TreeReadInfo
	{
		void* file;
		long offset;
		long version;
		union
		{
			NTreeNodeReanimateFunc nodeReanimateFunc;
			NTreeNodeReanimateXMLFunc nodeReanimateXMLFunc;
		};
		NTreeNodePtr root;
		NTreeNodeReadCB readCB;
	};
	typedef struct TreeReadInfo TreeReadInfo;

	NTree(void);
	NTree(NTreeNodeRoot*);
	virtual ~NTree();

	virtual bool IsRoot(NTreeNodePtr);

	virtual long Read(void*, long&, long, NTreeNodeReanimateFunc, NTreeNodeReadCB);
	virtual long Write(void*, long&, long, NTreeNodeWriteCB);

	virtual long ReadXML(const char*, long&, long, NTreeNodeReanimateXMLFunc, NTreeNodeReadCB);
	virtual long WriteXML(const char*, long&, long, NTreeNodeWriteCB);

	virtual NTreeNodePtr FindNodeByID(NTreeNodeID);

	virtual NTreeNodeRoot* GetRoot(void) { return fRoot; }

	static NTreeNodePtr FindRoot(NTreeNodePtr);
	static NTreePtr GetTreeFromNode(NTreeNodePtr);

	virtual bool VisitAllNTreeNodes(NTreeNodePtr, NTreeNodeActionFunc, void*, bool, bool);

	virtual bool Prune(NTreeNodePtr);

#if defined(_DEBUG)
	void Dump(NTreeNodeActionFunc = nullptr);
	static bool DumpNode_ActionFunc(NTreeNodePtr, void*);
#endif

private:

	struct NodeIDSearchInfo
	{
		NTreeNodeID
			id;
		NTreeNodePtr
			foundNode;
	};
	typedef struct NodeIDSearchInfo NodeIDSearchInfo;

	struct VisitedBits;
	typedef struct VisitedBits VisitedBits;
	typedef VisitedBits* VisitedBitsPtr;
	struct VisitedBits
	{
		VisitedBitsPtr
			next;
		char*
			bits;
	};

	static bool SearchForNodeID_ActionFunc(NTreeNodePtr, NodeIDSearchInfo*);
	static bool WriteNTreeNode_ActionFunc(NTreeNodePtr, void*);
	static bool ReadNTreeNode_ActionFunc(NTreeNodePtr, TreeReadInfo*);
	static bool DisposeNTree_ActionFunc(NTreeNodePtr, void*);

	static long CreateNTreeNodeFromXMLElement(NTreeNode* inParent, TreeReadInfo* inTreeInfo, tinyxml2::XMLElement* inXMLElement);
	static bool WriteNTreeXMLNode_ActionFunc(NTreeNodePtr, void*);
	static bool ReadNTreeXMLNode_ActionFunc(NTreeNodePtr, TreeReadInfo*);

	long PushVisitedBits(void);
	long PopVisitedBits(void);
	long ClearVisitedBits(void) const;
	void SetVisitedBit(NTreeNodeID, bool) const;
	bool Visited(NTreeNodeID) const;

private:

	NTreeNodeRoot* fRoot;
	VisitedBitsPtr fVisitedBits;

};

#endif