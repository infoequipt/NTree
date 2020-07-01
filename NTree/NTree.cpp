// --------------------------------------------------------------------------------
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
	PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
	OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
// --------------------------------------------------------------------------------
#include "pch.h"
#include "framework.h"

#include <stdlib.h>
#include "NTreeNode.h"
#include "NTree.h"
#include <crtdbg.h>
#include "tinyxml2.h"

#ifdef _DEBUG
#ifdef _WIN32
#else
#include <iostream>
#endif
#endif

using namespace tinyxml2;


// Comment this out to turn off debugging for this module.
//#define DEBUGSTR(a) DebugStr(a)

#if mAssertOn
static bool gImUsingThisMethod = false;
#endif


// --------------------------------------------------------------------------------
/*
	* NTreeNodeRoot
*/
// --------------------------------------------------------------------------------
NTreeNodeRoot::NTreeNodeRoot(void) : NTreeNode(NTreeNodeType(NTreeNodeRoot::kType), NTreeNodeID(NTreeNodeRoot::kID))
{
	fTree = nullptr;
}

// --------------------------------------------------------------------------------
/*
	* NTreeNodeRoot
*/
// --------------------------------------------------------------------------------
NTreeNodeRoot::NTreeNodeRoot(NTreePtr inTree) : NTreeNode(NTreeNodeType(NTreeNodeRoot::kType), NTreeNodeID(NTreeNodeRoot::kID))
{
	fTree = inTree;
}

// --------------------------------------------------------------------------------
/*
	* ~NTreeNodeRoot
*/
// --------------------------------------------------------------------------------
NTreeNodeRoot::~NTreeNodeRoot()
{
}

// --------------------------------------------------------------------------------
/*
	* GetTree
*/
// --------------------------------------------------------------------------------
NTreePtr
NTreeNodeRoot::GetTree() const
{
	return fTree;
}

// --------------------------------------------------------------------------------
/*
	* GetTree
*/
// --------------------------------------------------------------------------------
void
NTreeNodeRoot::SetTree(NTreePtr inTree)
{
	fTree = inTree;
}

// --------------------------------------------------------------------------------
/*
	* NewNTree

	Returns true if an error occurred.
*/
// --------------------------------------------------------------------------------
NTree::NTree(void)
{
	fRoot = new NTreeNodeRoot(this);
	fVisitedBits = nullptr;
	PushVisitedBits();
}

// --------------------------------------------------------------------------------
/*
	* NewNTree

	Returns true if an error occurred.
*/
// --------------------------------------------------------------------------------
NTree::NTree(NTreeNodeRoot* inRoot)
{
	fRoot = inRoot;
	fVisitedBits = nullptr;
	PushVisitedBits();
}

// --------------------------------------------------------------------------------
/*
	* ~NTree

	Returns true if an error occurred.
*/
// --------------------------------------------------------------------------------
NTree::~NTree()
{
	bool result = true;

	result = NTree::VisitAllNTreeNodes(fRoot, NTreeNodeActionFunc(DisposeNTree_ActionFunc), this, kActionOnExit, kEntireTree);

	while (fVisitedBits != nullptr)
		PopVisitedBits();

	if (fRoot)
	{
		delete fRoot;
		fRoot = nullptr;
	}
}

// --------------------------------------------------------------------------------
/*
	* DisposeNTreeNode

	Disposes of NodeData then the node itself.
*/
// --------------------------------------------------------------------------------
bool
NTree::DisposeNTree_ActionFunc
(
	NTreeNodePtr
	inNode,
	void*
	inTree
)
{
	long
		error = 0;
	NTreePtr
		tree = NTreePtr(inTree);

	if (tree->IsRoot(inNode) == false)
	{
		NTreeNodePtr
			parent = inNode->GetParent();

		if (parent != nullptr)
		{
			error = parent->RemoveChild(inNode);
			delete inNode;
		}
	}

	return (error == 0) ? false : true;
}

// --------------------------------------------------------------------------------
/*
	* Read

	Returns true if an error occurred.
*/
// --------------------------------------------------------------------------------
long
NTree::Read
(
	void* inFile,
	long& ioOffset,
	long inVersion,
	NTreeNodeReanimateFunc inNodeReanimateFunc,
	NTreeNodeReadCB inReadCB
)
{
	long error = 0;
	TreeReadInfo info;

	info.file = inFile;
	info.offset = ioOffset;
	info.version = inVersion;
	info.nodeReanimateFunc = inNodeReanimateFunc;
	info.root = fRoot;
	info.readCB = inReadCB;

	while (fRoot->GetNumChildren() > 0)
		fRoot->RemoveChild(static_cast<short>(0));

	/* The NewNTree() function sets the node id to kNTreeRootID.  The ReadNTreeNode() function
	needs this to be changed to work correctly. */
	fRoot->SetID(NTreeNode::kUnassignedID);

	VisitAllNTreeNodes(fRoot, NTreeNodeActionFunc(ReadNTreeNode_ActionFunc), &info, kActionOnEntry, kEntireTree);

	ioOffset = info.offset;

	return error;
}

// --------------------------------------------------------------------------------
/*
	* ReadNTreeNode

	Returns true if an error occurred.
*/
// --------------------------------------------------------------------------------
bool
NTree::ReadNTreeNode_ActionFunc
(
	NTreeNodePtr inNode,
	TreeReadInfo* inInfo
)
{
	long
		error = 0;

	if (inNode->GetID() == NTreeNode::kUnassignedID)
	{
		error = inNode->Read(inInfo->file, inInfo->offset, inInfo->version, inInfo->nodeReanimateFunc, inInfo->readCB);
		if (error != 0)
		{
			goto ErrorExit;
		}
	}

ErrorExit:
	return (error == 0) ? false : true;
}

// --------------------------------------------------------------------------------
/*
	* Write

	Returns true if an error occurred.
*/
// --------------------------------------------------------------------------------
long
NTree::Write
(
	void* inFile,
	long& ioOffset,
	long inVersion,
	NTreeNodeWriteCB inWriteCB
)
{
	long
		error = 0;
	TreeWriteInfo
		info;


	info.file = inFile;
	info.offset = ioOffset;
	info.version = inVersion;
	info.root = fRoot;
	info.writeCB = inWriteCB;

	if (VisitAllNTreeNodes(fRoot, WriteNTreeNode_ActionFunc, &info, kActionOnEntry, kEntireTree))
	{
		goto ErrorExit;
	}

ErrorExit:
	ioOffset = info.offset;
	return ((error == 0) ? false : true);
}

// --------------------------------------------------------------------------------
/*
	* WriteNTreeNode_ActionFunc

	Returns true if an error occurred.
*/
// --------------------------------------------------------------------------------
bool NTree::WriteNTreeNode_ActionFunc(NTreeNodePtr inNode, void* inInfo)
{
	long error = 0;
	TreeWriteInfo* info = static_cast<TreeWriteInfo*>(inInfo);

	error = inNode->Write(info->file, info->offset, info->version, info->writeCB);

	return (error == 0) ? false : true;
}


// --------------------------------------------------------------------------------
/*
	ReadXML

	Returns true if an error occurred.
*/
// --------------------------------------------------------------------------------
long
NTree::ReadXML
(
	const char* inFilename,
	long& ioOffset,
	long inVersion,
	NTreeNodeReanimateXMLFunc inNodeReanimateXMLFunc,
	NTreeNodeReadCB inReadCB
)
{
	long error = 0;
	XMLDocument* document = new XMLDocument();
	XMLElement* rootElement = NULL;

	error = document->LoadFile(inFilename);
	if (error != 0) { goto ErrorExit; }

	TreeReadInfo info;
	info.file = nullptr;
	info.offset = ioOffset;
	info.version = inVersion;
	info.nodeReanimateXMLFunc = inNodeReanimateXMLFunc;
	info.root = fRoot;
	info.readCB = inReadCB;

	while (fRoot->GetNumChildren() > 0)
		fRoot->RemoveChild(static_cast<short>(0));

	/* The NewNTree() function sets the node id to kNTreeRootID.  The ReadNTreeNode() function
	needs this to be changed to work correctly. */
	fRoot->SetID(NTreeNode::kUnassignedID);

	rootElement = document->RootElement();
	if (rootElement != nullptr)
	{
		error = CreateNTreeNodeFromXMLElement(GetRoot(), &info, rootElement);
	}

ErrorExit:
	return error;
}

// --------------------------------------------------------------------------------
/*
  PopulateNTreeNodeFromXMLElement
*/
// --------------------------------------------------------------------------------
long NTree::CreateNTreeNodeFromXMLElement(NTreeNode* inParent, TreeReadInfo* inTreeInfo, XMLElement* inXMLElement)
{
	long error = 0;

	if ((inParent == nullptr) || (inTreeInfo == nullptr) || (inXMLElement == nullptr))
	{
		error = -1;
		goto ErrorExit;
	}

	if (inXMLElement->NoChildren())
	{
		XMLElement* siblingElement = inXMLElement->NextSiblingElement();
		if (siblingElement != nullptr)
		{
			NTreeNodePtr sibling = (*(inTreeInfo->nodeReanimateXMLFunc))(siblingElement);
			inParent->InsertChild(sibling);
			error = CreateNTreeNodeFromXMLElement(sibling, inTreeInfo, siblingElement);
		}
	}
	else
	{
		XMLElement* childElement = inXMLElement->FirstChildElement();
		NTreeNodePtr child = (*(inTreeInfo->nodeReanimateXMLFunc))(childElement);
		inParent->InsertChild(child);
		error = CreateNTreeNodeFromXMLElement(child, inTreeInfo, childElement);
	}

ErrorExit:
	return error;
}


// --------------------------------------------------------------------------------
/*
  WriteXML
*/
// --------------------------------------------------------------------------------
long NTree::WriteXML(const char*, long&, long, NTreeNodeWriteCB)
{
	return 0;
}


// --------------------------------------------------------------------------------
/*
	* FindNodeByID
*/
// --------------------------------------------------------------------------------
NTreeNodePtr NTree::FindNodeByID(NTreeNodeID inID)
{
	NodeIDSearchInfo
		info;

	info.id = inID;
	info.foundNode = nullptr;

	VisitAllNTreeNodes(fRoot, NTreeNodeActionFunc(SearchForNodeID_ActionFunc), &info, true, false);

	return (info.foundNode);
}

// --------------------------------------------------------------------------------
/*
	* SearchForNodeID_ActionFunc
*/
// --------------------------------------------------------------------------------
bool
NTree::SearchForNodeID_ActionFunc
(
	NTreeNodePtr
	inNode,
	NTree::NodeIDSearchInfo*
	info
)
{
	if (inNode->GetID() == info->id)
	{
		info->foundNode = inNode;
		return true;
	}

	return false;
}

#if 1
// --------------------------------------------------------------------------------
/*
	* VisitAllNTreeNodes (re-entrant up to 32 times)

	You may start the visitation on any node in the tree.
	Returns true if an action proc aborted (an error occured).
*/
// --------------------------------------------------------------------------------
bool NTree::VisitAllNTreeNodes
(
	NTreeNodePtr inStartNode,                     // NTreeNodePtr to start on.
	NTreeNodeActionFunc inNodeActionProc,         // Action procedure to perform at every node.
	void* inNodeActionParm,                       // Parameter to be passed to action procedure.
	bool inActionOnEntry,                         // If true, action procedure is called BEFORE any child is visited,
												  //  (i.e. when the node is first encountered).
												  // If false, the action procedure is executed AFTER all children,
												  //	(i.e. before the node moves up to parent).
	bool inDoOnlyThisBranch                       // If false, the search begins at inStartNode and visits the entire tree.
												  // If true, only inStartNode and descendents of inStartNode are visited.

)
{
	NTreeNodePtr
		node = inStartNode;
	short
		flags;
	bool
		abort = false;
	bool
		endOfBranchCausedAbort = false;
	NTreeNodePtr
		parent = nullptr;
	bool
		nodeIsStartNode = false;
	static short
		entryCount = 0; // Number of times this routine has been entered.


	  /* Update our entry count. */
	entryCount += 1;
	if (entryCount > kMaxRecursion)
	{
		goto Exit;
	}

	/* The NTree has one set of bits available at all times.  If we enter this recursively,
		we need to create more.  Else, we simply clear the bit array. */
	if (entryCount > 1)
	{
		PushVisitedBits();
	}
	else
	{
		ClearVisitedBits();
	}

	// Loop until node == nullptr, this means we have visited all nodes.
	while (node != nullptr)
	{
		flags = node->GetFlags();

		// Has this node been visited?
		if (Visited(node->GetID()) == true)
		{
			// We do this just in case the action function wants to dispose of the node.
			parent = node->GetParent();
			nodeIsStartNode = (node == inStartNode) ? true : false;

			// If the caller wants an action on exit, do it now.
			if (!inActionOnEntry)
			{
				if (inNodeActionProc != nullptr)
				{
					abort = (*inNodeActionProc)(node, inNodeActionParm);
					if (abort == true)
					{
						goto Exit;
					}
					// node is possibly invalid here.
				}
			}

			// If we're about to move up to a new parent, check to see if the node we are at
			//  is the node we started with.  If this is the case and inDoOnlyThisBranch
			//  is true, then we are done processing - abort (don't do any more actions).
			if (nodeIsStartNode && inDoOnlyThisBranch)
			{
				abort = true;
				endOfBranchCausedAbort = true;
				goto Exit;
			}

			// Move up to parent.
			node = parent;
		}
		else
		{
			bool
				found = false;
			NTreeNodePtr
				child = nullptr;
			bool
				didAction = false;
			short
				numChildren = node->GetNumChildren();
			short
				childIndex = numChildren - 1;


			// If we don't have any children, this could be because a) this node REALLY
			//  doesn't have any children (its a 'leaf') or b) we're possibly in the
			//  middle of some creation routine that plans to add children as part of
			//  its action proc.
			if (numChildren == 0)
			{
				if (inActionOnEntry)
				{
					if (inNodeActionProc != nullptr)
					{
						abort = (*inNodeActionProc)(node, inNodeActionParm);
						if (abort == true)
						{
							goto Exit;
						}

						// We did an action routine.  Make sure it doesn't get called again.
						didAction = true;

						// The action proc may have added children, update our count.
						numChildren = node->GetNumChildren();
					}
				}
			}

			// Find first unvisited child, and visit (if we are not steering)
			{
				// Search backward for any children that might have been visited.  Use this as the starting point to find
				// the first unvisited child.
				while ((childIndex >= 0) && !found)
				{
					child = node->GetChild(childIndex);

					if (Visited(child->GetID()) == true)
					{
						found = true;
					}
					else
					{
						childIndex -= 1;
					}
				}

				// If we found one, increment the index.  This is start point for finding the first unvisited child.
				if (found == true)
				{
					childIndex += 1;
					if (childIndex >= numChildren)
					{
						childIndex = 0;
					}
				}
				else
				{
					childIndex = 0;
				}

				found = false;
				while ((childIndex < numChildren) && !found)
				{
					child = node->GetChild(childIndex);

					if (Visited(child->GetID()) == false)
					{
						found = true;
					}
					else
					{
						childIndex += 1;
					}
				}

				// If we have found at least one child and we are about to drop to the first child,
				//  then check for action on entry.  Do this only if we haven't called an action proc yet.
				if (found && (childIndex == 0) && !didAction)
				{
					if (inActionOnEntry)
					{
						if (inNodeActionProc != nullptr)
						{
							abort = (*inNodeActionProc)(node, inNodeActionParm);
							if (abort == true)
							{
								goto Exit;
							}
						}
					}
				}
			}

			// Did we find one? 
			if (found == true)
			{
				// Yes, go visit.
				node = child;
			}
			else
			{
				SetVisitedBit(node->GetID(), true);
			}
		}
	}

Exit:

	/* Decrement the instance counter.  This lets us know we have finished a traversal and we're leaving. */
	if (entryCount > 0)
	{
		if (entryCount > 1)
		{
			PopVisitedBits();
		}
		entryCount -= 1;
	}

	/* Return true only if we did not abort due to an end of of branch */
	return (abort == true ? (endOfBranchCausedAbort == true ? false : true) : false);
}
#endif

// --------------------------------------------------------------------------------
/*
	* FindRoot

	Given any NTreeNode, this routine finds the NTree root.  In the event that the
	node is not attached to any tree, this routine returns nullptr.
*/
// --------------------------------------------------------------------------------

NTreeNodePtr
NTree::FindRoot(NTreeNodePtr inStartNode)
{
	NTreeNodePtr
		node = inStartNode;

	if (node != nullptr)
	{
		while (node->GetParent() != nullptr)
		{
			node = node->GetParent();
		}
	}

	return (node->GetType() == NTreeNodeRoot::kType) ? node : nullptr;
}

// --------------------------------------------------------------------------------
/*
	* GetTreeFromNode

	Given any NTreeNode, this routine finds the NTree root and returns a pointer
	to the tree.
*/
// --------------------------------------------------------------------------------
NTreePtr
NTree::GetTreeFromNode(NTreeNodePtr inStartNode)
{
	NTreeNodePtr
		node = FindRoot(inStartNode);

	return (node != nullptr) ? static_cast<NTreeNodeRoot*>(node)->GetTree() : nullptr;
}

// --------------------------------------------------------------------------------
/*
*/
// --------------------------------------------------------------------------------
long
NTree::PushVisitedBits(void)
{
	long
		error = 0;
	VisitedBitsPtr
		newVB = nullptr;


	newVB = VisitedBitsPtr(malloc(sizeof(VisitedBits)));
	if (newVB == nullptr)
	{
		error = -1;
		goto ErrorExit;
	}

	newVB->next = nullptr;

	newVB->bits = static_cast<char*>(malloc(kNumVisitedBytes));
	if (newVB->bits == nullptr)
	{
		error = -1;
		goto ErrorExit;
	}

	newVB->next = fVisitedBits;
	fVisitedBits = newVB;

	ClearVisitedBits();

ErrorExit:
	return error;
}

// --------------------------------------------------------------------------------
/*
*/
// --------------------------------------------------------------------------------
long
NTree::PopVisitedBits(void)
{
	long
		error = 0;

	if (fVisitedBits != nullptr)
	{
		VisitedBitsPtr
			vb = fVisitedBits;

		fVisitedBits = fVisitedBits->next;
		free(vb->bits);
		free(vb);
	}

	return error;
}

// --------------------------------------------------------------------------------
/*
*/
// --------------------------------------------------------------------------------
long
NTree::ClearVisitedBits(void) const
{
	long
		error = 0;

	if (fVisitedBits != nullptr)
	{
		long
			i;
		long
			numLongs = kNumVisitedBytes / sizeof(long);
		long*
			lptr = reinterpret_cast<long*>(fVisitedBits->bits);

		for (i = 0; i < numLongs; i += 1)
		{
			*(lptr + i) = 0;
		}
	}

	return error;
}

// --------------------------------------------------------------------------------
/*
*/
// --------------------------------------------------------------------------------
void
NTree::SetVisitedBit(NTreeNodeID inNodeID, bool inState) const
{
	long
		byteNum = static_cast<unsigned long>(inNodeID) / 8;
	short
		bitNum = static_cast<short>(static_cast<short>(inNodeID) - static_cast<short>(byteNum * 8));
	char
		mask = 1 << (7 - bitNum);
	char*
		vb = fVisitedBits->bits;

	if (inState == true)
	{
		*(vb + byteNum) |= mask;
	}
	else
	{
		*(vb + byteNum) &= ~mask;
	}
}

// --------------------------------------------------------------------------------
/*
*/
// --------------------------------------------------------------------------------
bool
NTree::Visited(NTreeNodeID inNodeID) const
{
	long
		byteNum = static_cast<unsigned long>(inNodeID) / 8;
	short
		bitNum = static_cast<short>(static_cast<short>(inNodeID) - static_cast<short>(byteNum * 8));
	char
		mask = 1 << (7 - bitNum);
	char*
		vb = fVisitedBits->bits;

	return (*(vb + byteNum) & mask) != 0 ? true : false;
}

// --------------------------------------------------------------------------------
/*
	IsRoot
*/
// --------------------------------------------------------------------------------
bool
NTree::IsRoot(NTreeNodePtr inNode)
{
	return (inNode->GetID() == NTreeNodeRoot::kID) ? true : false;
}

// --------------------------------------------------------------------------------
/*
	Prune

	Recursively deletes nodes starting from inStartNode

	Returns true if an error occurred.
*/
// --------------------------------------------------------------------------------
bool
NTree::Prune(NTreeNodePtr inStartNode)
{
	return VisitAllNTreeNodes(inStartNode, NTreeNodeActionFunc(DisposeNTree_ActionFunc), this, kActionOnExit, kJustThisBranch);
}


#if defined(_DEBUG)
#ifdef _WIN32

// --------------------------------------------------------------------------------
/*
	Dump

	Displays an indent list of tree nodes in the Visual Studio "Output" window.
*/
// --------------------------------------------------------------------------------
void NTree::Dump(NTreeNodeActionFunc inDumpFunc)
{
	_RPT0(_CRT_WARN, "********** NTREE DUMP START\n");
	NTreeNodeActionFunc dumpFunc = inDumpFunc != nullptr ? inDumpFunc : DumpNode_ActionFunc;
	VisitAllNTreeNodes(GetRoot(), dumpFunc, this, kActionOnEntry, kEntireTree);
	_RPT0(_CRT_WARN, "********** NTREE DUMP END\n");
}

// --------------------------------------------------------------------------------
/*
	(WINDOWS) DumpNode_ActionFunc
*/
// --------------------------------------------------------------------------------
bool NTree::DumpNode_ActionFunc
(
	NTreeNodePtr inNode,
	void* inParam
)
{
	long error = 0;
	NTree* tree = static_cast<NTree*>(inParam);

	/* Indent level */
	NTreeNode* node = inNode;
	while (node != tree->GetRoot())
	{
		node = node->GetParent();
		_RPT0(_CRT_WARN, ".");
	}

	/* Node Type */
	NTreeNodeType type = inNode->GetType();
	_RPT1(_CRT_WARN, "%c", char(type >> 24));
	_RPT1(_CRT_WARN, "%c", char(type >> 16));
	_RPT1(_CRT_WARN, "%c", char(type >> 8));
	_RPT1(_CRT_WARN, "%c", char(type));

	_RPT0(_CRT_WARN, " ");

	/* Node ID */
	NTreeNodeID id = inNode->GetID();
	_RPT1(_CRT_WARN, "%d", id);

	_RPT0(_CRT_WARN, " ");

	/* Flags */
	short flags = inNode->GetFlags();
	_RPT1(_CRT_WARN, "%X", flags);

	_RPT0(_CRT_WARN, " ");

	/* Number of Children */
	short numberOfChildren = inNode->GetNumChildren();
	_RPT1(_CRT_WARN, "%d", numberOfChildren);

	_RPT0(_CRT_WARN, "\n");

	return false;
}

#else

// --------------------------------------------------------------------------------
/*
	Dump
*/
// --------------------------------------------------------------------------------
void NTree::Dump(void)
{
	std::cout << "********** NTREE DUMP START\n";
	VisitAllNTreeNodes(GetRoot(), NTreeNodeActionFunc(DumpNode_ActionFunc), this, kActionOnEntry, kEntireTree);
	std::cout << "********** NTREE DUMP END\n";
}

// --------------------------------------------------------------------------------
/*
	(MACINTOSH) DumpNode_ActionFunc
*/
// --------------------------------------------------------------------------------
bool
NTree::DumpNode_ActionFunc
(
	NTreeNodePtr
	inNode,
	void*
	inParam
)
{
	long error = 0;
	NTree* tree = (NTree*)inParam;

	/* Indent level */
	NTreeNode* node = inNode;
	while (node != tree->GetRoot())
	{
		node = node->GetParent();
		std::cout << char('.');
	}

	/* Node Type */
	NTreeNodeType type = inNode->GetType();
	std::cout << char(type >> 24);
	std::cout << char(type >> 16);
	std::cout << char(type >> 8);
	std::cout << char(type);

	std::cout << char(' ');

	/* Node ID */
	NTreeNodeID id = inNode->GetID();
	std::cout << int(id);

	std::cout << char(' ');

	/* Flags */
	short flags = inNode->GetFlags();
	std::cout << short(flags);

	std::cout << char(' ');

	/* Number of Children */
	short numberOfChildren = inNode->GetNumChildren();
	std::cout << short(numberOfChildren);

	std::cout << char('\n');

	return false;
}

#endif


#endif

