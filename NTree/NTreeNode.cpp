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

#include "NTreeNode.h"
#include <cstdlib>


// ----- Constructors/Destructor -----

// --------------------------------------------------------------------------------
/*
	NTreeNode

	Creates and initializes a new node.  If outError returns non-zero, the object
	should be destroyed (because an error has occured).
*/
// --------------------------------------------------------------------------------
NTreeNode::NTreeNode(NTreeNodeType inType, NTreeNodeID inID)
{
	Initialize();
	fType = inType;
	fID = inID;
}

// --------------------------------------------------------------------------------
/*
	NTreeNode

	Creates and initializes a new node as a copy from the supplied node.  If
	outError returns non-zero, the object should be destroy, as an error has
	occurred.
*/
// --------------------------------------------------------------------------------
NTreeNode::NTreeNode
(
	NTreeNodePtr
	inNode,
	NTreeNodeID
	inNodeID
)
{
	Initialize();

	fType = inNode->GetType();
	fID = inNodeID;
	fFlags = inNode->GetFlags();
	fParent = inNode->GetParent();
	fNumChildren = inNode->GetNumChildren();
}

// --------------------------------------------------------------------------------
/*
  ~NTreeNode

  Deallocate node memory.
*/
// --------------------------------------------------------------------------------
NTreeNode::~NTreeNode()
{
	if (fChildren != nullptr)
	{
		free(fChildren);
		fChildren = nullptr;
		fNumChildren = 0;
	}
}


// --------------------------------------------------------------------------------
/*
	Initialize
*/
// --------------------------------------------------------------------------------
void
NTreeNode::Initialize(void)
{
	fType = 0;
	fID = 0;
	fFlags = 0;
	fParent = nullptr;
	fNumChildren = 0;
	fChildren = nullptr;
}

// ----- Persistance -----

// --------------------------------------------------------------------------------
/*
	Read

	Reads a node from permanent store.  Returns 0 if no error.

	On entry, ioOffset should be set to start reading.  Upon exit, ioOffset
	will be moved forward.
*/
// --------------------------------------------------------------------------------
long
NTreeNode::Read
(
	void* inFile,
	long& ioOffset,
	long inVersion,
	NTreeNodeReanimateFunc  inReanimateNodeFunc,
	NTreeNodeReadCB inReadCB
)
{
	long
		error = 0;
	unsigned long
		count;
	short
		i = 0;
	NTreeNodeType
		nodeType = 0;
	NTreeNodePtr
		child = nullptr;
	short
		numChildren = fNumChildren;

	count = sizeof(fType);
	error = (*inReadCB)(inFile, &fType, count, 1, ioOffset);
	if (error != 0)
		goto ErrorExit;
	ioOffset += count;

	count = sizeof(fID);
	error = (*inReadCB)(inFile, &fID, count, 1, ioOffset);
	if (error != 0)
		goto ErrorExit;
	ioOffset += count;

	count = sizeof(fFlags);
	error = (*inReadCB)(inFile, &fFlags, count, 1, ioOffset);
	if (error != 0)
		goto ErrorExit;
	ioOffset += count;

	count = sizeof(fNumChildren);
	error = (*inReadCB)(inFile, &fNumChildren, count, 1, ioOffset);
	if (error != 0)
		goto ErrorExit;
	ioOffset += count;


	fNumChildren = 0; // reset because, InsertChild will increment.
	for (i = 0; i < numChildren; i += 1)
	{
		count = sizeof(nodeType);
		error = (*inReadCB)(inFile, &nodeType, count, 1, ioOffset);
		if (error != 0)
		{
			goto ErrorExit;
		}
		ioOffset += count;

		child = (*inReanimateNodeFunc)(nodeType, 0);
		if (child == nullptr)
		{
			error = -1;
			goto ErrorExit;
		}
		if (error != 0)
		{
			goto ErrorExit;
		}

		child->SetFlags(GetFlags()); // Match parent flags.
		error = InsertChild(child);
		if (error != 0)
		{
			goto ErrorExit;

		}
	}


ErrorExit:
	return error;
}


// --------------------------------------------------------------------------------
/*
	Write

	Write the given node to disk. Returns 0 if no error.

	On entry, ioOffset should point to the starting position to write.  On exit,
	ioOffset will be moved ahead.
*/
// --------------------------------------------------------------------------------
long
NTreeNode::Write
(
	void* inFile,
	long& ioOffset,
	long inVersion,
	NTreeNodeReadCB inWriteCB
)
{
	long error = 0;
	unsigned long count;
	short i = 0;
	NTreeNodeType nodeType = 0;


	count = sizeof(fType);
	error = (*inWriteCB)(inFile, &fType, count, 1, ioOffset);
	if (error != 0)
		goto ErrorExit;
	ioOffset += count;

	count = sizeof(fID);
	error = (*inWriteCB)(inFile, &fID, count, 1, ioOffset);
	if (error != 0)
		goto ErrorExit;
	ioOffset += count;

	count = sizeof(fFlags);
	error = (*inWriteCB)(inFile, &fFlags, count, 1, ioOffset);
	if (error != 0)
		goto ErrorExit;
	ioOffset += count;

	count = sizeof(fNumChildren);
	error = (*inWriteCB)(inFile, &fNumChildren, count, 1, ioOffset);
	if (error != 0)
		goto ErrorExit;
	ioOffset += count;

	/* Write out child types */
	for (i = 0; i < fNumChildren; i += 1)
	{
		nodeType = GetChild(i)->GetType();

		count = sizeof(nodeType);
		error = (*inWriteCB)(inFile, &nodeType, count, 1, ioOffset);

		if (error != 0)
		{
			goto ErrorExit;
		}

		ioOffset += count;
	}

ErrorExit:
	return (error);
}

// ----- Children -----

// --------------------------------------------------------------------------------
/*
	MoreChildren

	Increases the child array by inNumChildrenToAdd.  If the child array cannot
	be grown, an error is returned.
*/
// --------------------------------------------------------------------------------
long
NTreeNode::MoreChildren(short inNumChildrenToAdd)
{
	long
		error = 0;
	NTreeNodePtr*
		newArray = nullptr;

	/* Create a new, larger array. */
	newArray = static_cast<NTreeNodePtr*>(calloc(fNumChildren + inNumChildrenToAdd, sizeof(NTreeNodePtr)));
	if (newArray == nullptr)
	{
		error = -1;
		goto ErrorExit;
	}

	/* If we have children, copy them over. */
	if (fNumChildren > 0)
	{
		short
			i;

		for (i = 0; i < fNumChildren; i += 1)
		{
			*(newArray + i) = *(fChildren + i);
		}
		free(fChildren);
		fChildren = nullptr;
	}

	/* Assign our new array */
	fChildren = newArray;
	fNumChildren += inNumChildrenToAdd;

ErrorExit:
	return error;
}

// --------------------------------------------------------------------------------
/*
	LessChildren

	Reduces the child array by inNumChildrenToReduce.  Returns 0 if no error.
*/
// --------------------------------------------------------------------------------
long
NTreeNode::LessChildren(short inNumChildrenToReduce)
{
	long
		error = 0;


	if (fNumChildren > 0)
	{
		short
			numChildren = fNumChildren - inNumChildrenToReduce;

		if (numChildren <= 0)
		{
			if (fChildren != nullptr)
			{
				free(fChildren);
			}
			fChildren = nullptr;
			fNumChildren = 0;
		}
		else
		{
			short i;
			NTreeNodePtr* newArray = nullptr;

			newArray = static_cast<NTreeNodePtr*>(malloc(numChildren * sizeof(NTreeNodePtr)));
			if (newArray == nullptr)
			{
				error = -1;
				goto ErrorExit;
			}

			for (i = 0; i < numChildren; i += 1)
			{
				*(newArray + i) = *(fChildren + i);
			}
			free(fChildren);
			fChildren = nullptr;

			fChildren = newArray;
			fNumChildren = numChildren;
		}
	}

ErrorExit:
	return error;
}

// --------------------------------------------------------------------------------
/*
	InsertChild

	Append child at end of child array.
*/
// --------------------------------------------------------------------------------
long
NTreeNode::InsertChild(NTreeNodePtr inNewChild)
{
	return InsertChild(inNewChild, fNumChildren);
}

// --------------------------------------------------------------------------------
/*
	InsertChild

	Insert child into child array at inAtIndex.  To append, use method above.
*/
// --------------------------------------------------------------------------------
long
NTreeNode::InsertChild
(
	NTreeNodePtr
	inNewChild,
	short
	inAtIndex
)
{
	long
		error = 0;


	error = MoreChildren(1);
	if (error != 0)
	{
		goto ErrorExit;
	}

	if (fNumChildren == 1)
	{
		SetChild(0, inNewChild);
	}
	else if (inAtIndex >= fNumChildren)
	{
		SetChild(fNumChildren - 1, inNewChild);
	}
	else
	{
		short
			i;

		for (i = fNumChildren - 1; i > inAtIndex; i -= 1)
		{
			*(fChildren + i) = *(fChildren + i - 1);
		}

		SetChild(inAtIndex, inNewChild);
	}

	// Point the child at new parent.
	inNewChild->SetParent(this);

ErrorExit:
	return (error);
}

// --------------------------------------------------------------------------------
/*
	RemoveChild

	Remove the child inChild from the child array.  This routine DOES NOT delete
	the child that is removed.
*/
// --------------------------------------------------------------------------------
long
NTreeNode::RemoveChild(short inChildIndex)
{
	long
		error = 0;


	if ((fNumChildren > 0) && (inChildIndex >= 0) && (inChildIndex < fNumChildren))
	{
		if ((fNumChildren == 1) || (inChildIndex == fNumChildren - 1))
		{
			LessChildren(1);
		}
		else
		{
			short
				i;

			for (i = inChildIndex; i < fNumChildren - 1; i += 1)
			{
				*(fChildren + i) = *(fChildren + i + 1);
			}

			LessChildren(1);
		}
	}

	return error;
}

// --------------------------------------------------------------------------------
/*
	RemoveChild

	Remove the child inChild from the child array.  This routine DOES NOT delete
	the child that is removed.
*/
// --------------------------------------------------------------------------------
long
NTreeNode::RemoveChild(NTreeNodePtr inChild)
{
	return RemoveChild(FindChildIndexByAddress(inChild));
}


// --------------------------------------------------------------------------------
/*
	Move

	Move this node elsewhere - given by inNewParent and index.
*/
// --------------------------------------------------------------------------------
long
NTreeNode::Move
(
	NTreeNodePtr
	inNewParent,
	short
	inIndex
)
{
	long
		error = 0;
	NTreeNodePtr
		parent = GetParent();


	if (parent != nullptr)
	{
		/* Remove this node from current parent */
		error = GetParent()->RemoveChild(this);
		if (error != 0)
		{
			goto ErrorExit;
		}

		/* Add to new parent */
		error = inNewParent->InsertChild(this, inIndex);
	}

ErrorExit:
	return error;
}

// ----- Accessors -----

// --------------------------------------------------------------------------------
/*
	GetType
*/
// --------------------------------------------------------------------------------
NTreeNodeType
NTreeNode::GetType(void)
{
	return fType;
}

// --------------------------------------------------------------------------------
/*
	SetType
*/
// --------------------------------------------------------------------------------
void
NTreeNode::SetType(NTreeNodeType inType)
{
	fType = inType;
}

// --------------------------------------------------------------------------------
/*
	GetID
*/
// --------------------------------------------------------------------------------
NTreeNodeID
NTreeNode::GetID(void)
{
	return fID;
}

// --------------------------------------------------------------------------------
/*
	SetID
*/
// --------------------------------------------------------------------------------
void
NTreeNode::SetID(NTreeNodeID inID)
{
	fID = inID;
}

// --------------------------------------------------------------------------------
/*
	GetFlags

	Returns the nodes flags.
*/
// --------------------------------------------------------------------------------
short
NTreeNode::GetFlags(void)
{
	return fFlags;
}

// --------------------------------------------------------------------------------
/*
	SetFlags

	Sets the nodes flags.
*/
// --------------------------------------------------------------------------------
void
NTreeNode::SetFlags(short inFlags)
{
	fFlags = inFlags;
}


// --------------------------------------------------------------------------------
/*
	GetParent

	Returns the parent of the given node.
*/
// --------------------------------------------------------------------------------
NTreeNodePtr
NTreeNode::GetParent(void)
{
	return fParent;
}

// --------------------------------------------------------------------------------
/*
	SetParent

	Sets the parent of the given node.
*/
// --------------------------------------------------------------------------------
void
NTreeNode::SetParent(NTreeNodePtr inParent)
{
	fParent = inParent;
}

// --------------------------------------------------------------------------------
/*
	GetNumChildren

	Returns the node's number of children.
*/
// --------------------------------------------------------------------------------
short
NTreeNode::GetNumChildren(void)
{
	return fNumChildren;
}

// --------------------------------------------------------------------------------
/*
	SetNumChildren

	Sets the number of children in a node.
*/
// --------------------------------------------------------------------------------
void
NTreeNode::SetNumChildren(short inNumChildren)
{
	fNumChildren = inNumChildren;
}

// --------------------------------------------------------------------------------
/*
	GetChild

	Returns the child node at the given index.
*/
// --------------------------------------------------------------------------------
NTreeNodePtr
NTreeNode::GetChild(short inChildIndex)
{
	return *(fChildren + inChildIndex);
}

// --------------------------------------------------------------------------------
/*
	SetChild

	Sets the child references at the given node.
*/
// --------------------------------------------------------------------------------
void
NTreeNode::SetChild
(
	short
	inChildIndex,
	NTreeNodePtr
	inChild
)
{
	*(fChildren + inChildIndex) = inChild;
}

// ----- Utilities -----

// --------------------------------------------------------------------------------
/*
	FindChildIndexByData
*/
// --------------------------------------------------------------------------------
/*
short
NTreeNode::FindChildIndexByData( void * inData )
{
  short
	i = 0;


  while( (i < fNumChildren) && ( (fChildren + i).fData != inData ) ) { i += 1; }

  return( i >= fNumChildren ? -1 : i );
}
*/

// --------------------------------------------------------------------------------
/*
	FindChildIndexByAddress

	Returns the index of the child inChild.  Search on address of child.  If not
	found, return -1
*/
// --------------------------------------------------------------------------------
short NTreeNode::FindChildIndexByAddress(NTreeNodePtr inChild)
{
	short
		index = -1;

	if (fNumChildren > 0)
	{
		short
			i = 0;

		while ((i < fNumChildren) && (*(fChildren + i) != inChild))
		{
			i += 1;
		}

		if (i < fNumChildren)
		{
			index = i;
		}
	}

	return index;
}

// --------------------------------------------------------------------------------
/*
	IsRoot

	Returns the true if root node
*/
// --------------------------------------------------------------------------------
bool NTreeNode::IsRoot()
{
	return GetType() == NTreeNodeType('ROOT');
}
