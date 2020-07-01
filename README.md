# NTree

**A C++98 Library (with future plans for C++14+ and C#) for creating and managing nodes in a tree structure**

## History

NTree was developed in the late 90's and early 00's as a structure to store a scene graph for an embedded drawing library as part of an MFC and MacOS 9 DNA analysis application.
The drawing library supported object drawing of shapes, lines, arrows and text.  This provided a quick way to annotate images.

The drawing lib also allowed objects to be grouped, and thus a tree structure was needed to render the scene.

## Architecture

- Tree structure with any number of nodes as children of other nodes.
- VisitAllTreeNodes() function to depth-first traverse the entire tree.
- Ability to only traverse a branch.
- Perform an action while traversing.
- Perform action on first node encounter.
- Perform action when leaving node (moving back up to parent).
- Recursive traversal upto 32 levels deep.
- Ability to "grow" or create the tree on-the-fly.
- Binary write/read of entire tree to FILE.
- XML write/read of entire tree to FILE.

## Goals

The library will be ported to C++14+ and also C#.

## Getting Started

The Sample application attempts to demonstrate how to create, use and persist an NTree.

SpellChecker takes a string of space delimited words and produces an NTree with two types of nodes 'LETR' and 'WORD' representing the dictionary.

(Need Picture)

The CheckSpelling() function verifies the word is spelled correctly.


