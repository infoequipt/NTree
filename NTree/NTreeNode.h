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
#ifndef _NTREENODE_
#define _NTREENODE_
#include "tinyxml2.h"

// --------------------------------------------------------------------------------
/*
		NTreeNode.h

		This object forms the nodes of a tree (NTree).  Each node is capable of
		having any number of children.  Although NTrees are not optimized for
		organizing large numbers of things, they are good for general hiearchies.

		This basic structure includes a pointer to the node's parent and an array
		of pointers to children.  There is a pointer to data in which to hang
		application specific data.

		See NTree.h for more information about NTrees.
*/
//--------------------------------------------------------------------------------

	/* The version of NTree.  This is used for reading the correct version of a file. */

	/* DCC - 18 DEC 07 - HOW TO INCREMENT THE NTREE VESION NUMBER:

			Copy the current version number line "const unsigned long kNTreeVersion = ..." to
			a new line and rename the variable with the version number appended.  Example:

				const unsigned long kNTreeVersion = 0x00000500;

			Becomes...

				const unsigned long kNTreeVersion500 = 0x00000500;

			Now increase the version number of the "kNTreeVersion" variable.

			kNTreeVerson always holds the latest version number.

			In code, checks should be made as follows:


				if ( inVersion < kNTreeVersion )
					{
						 [Handle older versions]
					}
				else
					{
						[Handle current version]
					}
	*/

	/* DCC - 18 DEC 07 - Up'd the version for 5.0 release. */
const unsigned long kNTreeVersion = 0x00000500;	/* version 5.0.0 */

	/* Each node is assigned a unique id.  This is usually just an incremented number
		each time a node is created. */
typedef unsigned int NTreeNodeType;
typedef unsigned short NTreeNodeID;

class NTreeNode;
#define Node NTreeNode

typedef class NTreeNode* NTreeNodePtr;

/* These two functions call platform specific functions that actually do the file i/o. */
typedef long (*NTreeNodeReadCB)(void*, void*, unsigned long, unsigned long, long);
typedef long (*NTreeNodeWriteCB)(void*, void*, unsigned long, unsigned long, long);
typedef NTreeNodePtr(*NTreeNodeReanimateFunc)(NTreeNodeType, NTreeNodeID);
typedef NTreeNodePtr(*NTreeNodeReanimateXMLFunc)(tinyxml2::XMLElement*);

class NTreeNode
{
public:

	enum
	{
		kVisited = (1 << 1),
		kUnassignedID = 0
	};


	/* Constructors & Destructor */
	// we needed to define a default constructor; this probably needs to change
	NTreeNode(void) { Initialize(); fType = -1; fID = kUnassignedID; }

	NTreeNode(NTreeNodeType, NTreeNodeID);
	NTreeNode(NTreeNodePtr, NTreeNodeID);
	virtual ~NTreeNode();

	/* Persistence */
	virtual long Read(void*, long&, long, NTreeNodeReanimateFunc, NTreeNodeReadCB);
	virtual long Write(void*, long&, long, NTreeNodeWriteCB);


	/* Children */
	virtual short GetNumChildren(void);
	virtual void SetNumChildren(short);
	virtual long InsertChild(NTreeNodePtr);
	virtual long InsertChild(NTreeNodePtr, short);
	virtual long RemoveChild(NTreeNodePtr);
	virtual long RemoveChild(short);
	virtual NTreeNodePtr GetChild(short);
	virtual void SetChild(short, NTreeNodePtr);

	/* Accessors */
	virtual NTreeNodeType GetType(void);
	virtual void SetType(NTreeNodeType);
	virtual NTreeNodeID GetID(void);
	virtual void SetID(NTreeNodeID);
	virtual short GetFlags(void);
	virtual void SetFlags(short);
	virtual NTreeNodePtr GetParent(void);
	virtual void SetParent(NTreeNodePtr);

	/* Utilities */
	virtual long Move(NTreeNodePtr, short);
	virtual short FindChildIndexByAddress(NTreeNodePtr);
	virtual bool IsRoot();

protected:

	void Initialize(void);
	long MoreChildren(short);
	long LessChildren(short);

private:

	NTreeNodeType fType;			// node type
	NTreeNodeID fID;			    // node id
	short fFlags;					// defined in NTreeNodeFlags.h
	NTreeNodePtr fParent;			// parent
	short fNumChildren;				// number of entries in the children array
	NTreeNodePtr* fChildren;        // Handle to block containing an array of NodePtr
};



#endif
