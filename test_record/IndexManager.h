#pragma once
#include "MiniSQL.h"
#include "Buffer.h"

extern BufferManager buf;
class IndexLeaf
{
public:
	IndexLeaf();
	IndexLeaf(string k, int oinf, int oinb);
	~IndexLeaf() = default;

	string key;
	int offsetInFile;
	int offsetInBlock;
};

class IndexBranch
{
public:
	IndexBranch();
	IndexBranch(string k, int ptr);

	string key;
	int childptr;
};

class BPlusTree
{
public:
	int bufferNum;				//the bufferNum of block containgning data
	int fatherptr;				//point to the parent node
	int recordNum;
	int ColumnLength;
	bool isRoot;

	BPlusTree();
	BPlusTree(int bufNum);
	int getPtr(int position);
	int getRecordNum();

};
class LeafNode :public BPlusTree
{
public:
	LeafNode(int bufNum);
	LeafNode(int bufNum, const Index& indexinfor);
	~LeafNode();
	void Insert(IndexLeaf Node);
	IndexLeaf pop();
	IndexLeaf getFront();

	int LastBlock;
	int NextBlock;
	list<IndexLeaf> NodeList;
};

class BranchNode :public BPlusTree
{
public:
	list<IndexBranch> NodeList;

	BranchNode();
	BranchNode(int bufNum);
	BranchNode(int bufNum, const Index& indexinfor);
	~BranchNode();
	void Insert(IndexBranch Node);
	IndexBranch pop();
	IndexBranch getFront();
};
class IndexManager
{
public:
	void createIndex(const Table& tableinfor, Index& indexinfor);
	IndexBranch InsertValue(Index& indexinfor, IndexLeaf node, int blockoffset = 0);
	Data selectEqual(const Table& tableinfor, const Index& indexinfor, string key, int blockoffset = 0);
	Data selectBetween(const Table& tableinfor, const Index& indexinfor, string keystart, string keyend, int blockoffset = 0);
	void dropIndex(Index& indexinfor);

private:
	Row splitRow(Table tableinfor, string row);
	string getColumnValue(const Table& tableinfor, const Index& indexinfor, string row);
};