#include "IndexManager.h"
static void zeroextend(string &a, int bit)
{
	while (a.length() < bit)
		a = '0' + a;
}
IndexLeaf::IndexLeaf()
{
	key = "";
	offsetInFile = 0;
	offsetInBlock = 0;
}

IndexLeaf::IndexLeaf(string k, int oinf, int oinb)
{
	key = k;
	offsetInFile = oinf;
	offsetInBlock = oinb;
}

BranchNode::BranchNode()
{
}

BranchNode::BranchNode(int bufNum)
{
	bufferNum = bufNum;
	recordNum = 0;
}

BranchNode::BranchNode(int bufNum, const Index & indexinfor)
{
	bufferNum = bufNum;
	isRoot = buf.block[bufferNum].value[0] == 'R';
	int totalRecord = getRecordNum();
	recordNum = 0;
	int position = 6;
	fatherptr = getPtr(position);
	ColumnLength = indexinfor.ColumnLength;
	for (int i = 0; i < totalRecord; ++i)
	{
		string key;
		position += POINTERLENGTH;
		for (int j = position;
			j < position + ColumnLength && buf.block[bufferNum].value[j] != EMPTY;
			++j)
			key += buf.block[bufferNum].value[j];
		position += ColumnLength;
		int childptr = getPtr(position);
		IndexBranch Node(key, childptr);
		Insert(Node);
	}
}

BranchNode::~BranchNode()
{
	//root
	buf.block[bufferNum].value[0] = isRoot ? 'R' : '_';
	//leaf
	buf.block[bufferNum].value[1] = '_';
	//recordNum
	char charRecordNum[5];
	itoa(recordNum, charRecordNum, 10);
	string strRecordNum(charRecordNum);
	zeroextend(strRecordNum, 4);
	strncpy(&buf.block[bufferNum].value[2], strRecordNum.c_str(), 4);

	//nodelist
	int pos = 6 + POINTERLENGTH;
	for (auto i = NodeList.begin(); i != NodeList.end(); ++i)
	{
		string key(i->key);
		while (key.length() < ColumnLength) key += EMPTY;
		strncpy(&buf.block[bufferNum].value[pos], key.c_str(), ColumnLength);
		pos += ColumnLength;

		char charChildPtr[5];
		itoa(i->childptr, charChildPtr, 10);
		string strChildPtr = charChildPtr;
		zeroextend(strChildPtr, POINTERLENGTH);
		strncpy(&buf.block[bufferNum].value[pos], strChildPtr.c_str(), POINTERLENGTH);
		pos += POINTERLENGTH;
	}

}

void BranchNode::Insert(IndexBranch Node)
{
	++recordNum;
	auto i = NodeList.begin();
	if (NodeList.size() != 0)
		while (i != NodeList.end() && i->key <= Node.key)
			i++;
	NodeList.insert(i, Node);
}

IndexBranch BranchNode::pop()
{
	--recordNum;
	auto back = NodeList.end();
	IndexBranch temp;
	temp = *back;
	NodeList.pop_back();
	return IndexBranch(temp);
}

IndexBranch BranchNode::getFront()
{

	return IndexBranch(NodeList.front());
}

void IndexManager::createIndex(const Table & tableinfor, Index & indexinfor)
{
	string filename = indexinfor.index_name + ".index";
	fstream fout(filename.c_str(), ios::out);
	fout.close();
	//create a indexfile

	int blockNum = buf.EmptyBuffer();			//get an empty block

												//initial the block information
	buf.block[blockNum].filename = filename;
	buf.block[blockNum].blockoffset = 0;
	buf.block[blockNum].isWritten = true;
	buf.block[blockNum].isValid = true;
	buf.block[blockNum].isPinned = false;
	buf.block[blockNum].clockFlag = true;

	//initial the block values
	buf.block[blockNum].value[0] = 'R';			//indicate the root
	buf.block[blockNum].value[1] = 'L';			//indicate the leaf
	for (int i = 2; i < 6; ++i)
		buf.block[blockNum].value[i] = '0';		//store the record number in block
	int initialoffset = POINTERLENGTH * 3 + 6;
	for (int i = 6; i < initialoffset; ++i)
		buf.block[blockNum].value[i] = '0';		//value[6:10] father pointer
												//value[11:15] , value[16:20] sibling parent pointer
	indexinfor.BlockNum++;

	filename = tableinfor.name + ".table";
	int RecordLength = tableinfor.TotalLength + 1;
	int RecordNum = BLOCKSIZE / RecordLength;
	string key;
	string stringrow;

	for (int blockoffset = 0; blockoffset < tableinfor.BlockNum; ++blockoffset)
	{
		int bufferNum = buf.getbufferNum(filename, blockoffset);

		for (int offset = 0; offset < RecordNum; ++offset)
		{
			int position = offset * RecordLength;
			stringrow = buf.block[bufferNum].getvalue(position, position + RecordLength);
			if (stringrow.c_str()[0] == EMPTY)
			{
				continue;
			}
			else
			{
				stringrow.erase(stringrow.begin());
				key = getColumnValue(tableinfor, indexinfor, stringrow);

				IndexLeaf Node(key, blockoffset, offset);
				InsertValue(indexinfor, Node);
			}
		}
	}
}

IndexBranch IndexManager::InsertValue(Index & indexinfor, IndexLeaf node, int blockoffset)
{
	IndexBranch tempBranch;
	string filename = indexinfor.index_name + ".index";
	int bufferNum = buf.getbufferNum(filename, blockoffset);
	bool isLeaf = (buf.block[bufferNum].value[1] == 'L');
	if (isLeaf)//is leaf
	{
		LeafNode leaf(bufferNum, indexinfor);
		leaf.Insert(node);

		int RecordLength = indexinfor.ColumnLength + POINTERLENGTH * 2;
		int MaxRecordNum = (BLOCKSIZE - POINTERLENGTH * 3 - 6) / RecordLength;

		if (leaf.recordNum > MaxRecordNum)
		{//need split
			int rootbufferNum = leaf.bufferNum;
			leaf.bufferNum = buf.CreateNewBlockInFile(indexinfor);
			int siblingbufferNum = buf.CreateNewBlockInFile(indexinfor);

			BranchNode root(rootbufferNum);
			LeafNode sibling(siblingbufferNum);

			root.isRoot = true;
			root.ColumnLength = leaf.ColumnLength;

			leaf.isRoot = sibling.isRoot = false;
			sibling.ColumnLength = leaf.ColumnLength;
			root.fatherptr = leaf.fatherptr = sibling.fatherptr = 0;

			sibling.LastBlock = buf.block[leaf.bufferNum].blockoffset;
			leaf.NextBlock = buf.block[sibling.bufferNum].blockoffset;
			while (sibling.NodeList.size() < leaf.NodeList.size())
			{
				IndexLeaf temp = leaf.pop();
				sibling.Insert(temp);
			}

			IndexBranch temp;
			temp.key = sibling.getFront().key;
			temp.childptr = buf.block[sibling.bufferNum].blockoffset;
			root.Insert(temp);
			temp.key = leaf.getFront().key;
			temp.childptr = buf.block[leaf.bufferNum].blockoffset;
			root.Insert(temp);
		}
	}
	else//not leaf
	{
		BranchNode Branch(bufferNum, indexinfor);
		auto i = Branch.NodeList.begin();
		if (node.key > i->key)
		{
			while (i != Branch.NodeList.end() && (++i)->key <= node.key);
			i--;
		}
		else
			i->key = node.key;

		IndexBranch branchnode = InsertValue(indexinfor, node, i->childptr);
		if (branchnode.key == "");
		else
		{
			Branch.Insert(branchnode);
			int RecordLength = indexinfor.ColumnLength + POINTERLENGTH;
			int MaxRecordNum = (BLOCKSIZE - POINTERLENGTH - 6) / RecordLength;
			if (Branch.recordNum <= MaxRecordNum);
			else
			{
				if (Branch.isRoot)
				{
					int rootbufferNum = Branch.bufferNum;
					Branch.bufferNum = buf.CreateNewBlockInFile(indexinfor);
					int siblingbufferNum = buf.CreateNewBlockInFile(indexinfor);
					BranchNode root(rootbufferNum);
					BranchNode sibling(siblingbufferNum);

					root.isRoot = true;
					Branch.isRoot = sibling.isRoot = 0;

					root.fatherptr = sibling.fatherptr = Branch.fatherptr = 0;
					root.ColumnLength = sibling.ColumnLength = Branch.ColumnLength;

					while (sibling.NodeList.size() < Branch.NodeList.size())
					{
						IndexBranch popNode = Branch.pop();
						sibling.Insert(popNode);
					}

					IndexBranch temp;
					temp.key = sibling.getFront().key;
					temp.childptr = buf.block[sibling.bufferNum].blockoffset;
					root.Insert(temp);
					temp.key = Branch.getFront().key;
					temp.childptr = buf.block[Branch.bufferNum].blockoffset;
					root.Insert(temp);
				}
				else
				{
					int buffNum = buf.CreateNewBlockInFile(indexinfor);
					BranchNode sibling(bufferNum);

					sibling.isRoot = false;
					sibling.fatherptr = Branch.fatherptr;
					sibling.ColumnLength = Branch.ColumnLength;

					while (sibling.NodeList.size()<Branch.NodeList.size())
					{
						IndexBranch popNode = Branch.pop();
						sibling.Insert(popNode);
					}

					tempBranch.key = sibling.getFront().key;
					tempBranch.childptr = buf.block[bufferNum].blockoffset;
				}
			}
		}
	}
	return IndexBranch(tempBranch);
}

Data IndexManager::selectEqual(const Table & tableinfor, const Index & indexinfor, string key, int blockoffset)
{
	Data d;
	string filename = indexinfor.index_name + ".index";
	int bufferNum = buf.getbufferNum(filename, blockoffset);
	bool isLeaf = buf.block[bufferNum].value[1] == 'L';
	if (isLeaf)
	{
		LeafNode leaf(bufferNum, indexinfor);
		for (auto i = leaf.NodeList.begin(); i != leaf.NodeList.end(); ++i)
		{
			if (i->key != key) break;
			filename = indexinfor.table_name + ".table";
			int recordBufferNum = buf.getbufferNum(filename, i->offsetInFile);
			int pos = (tableinfor.TotalLength + 1)*i->offsetInBlock;
			string stringrow;
			stringrow = buf.block[recordBufferNum].getvalue(pos, pos + tableinfor.TotalLength);
			if (stringrow[0] != EMPTY)
			{
				stringrow.erase(stringrow.begin());
				Row split = splitRow(tableinfor, stringrow);
				d.Rows.emplace_back(split);
				return d;
			}
		}
	}
	else
	{
		BranchNode branch(bufferNum, indexinfor);
		auto i = branch.NodeList.begin();
		while (i != branch.NodeList.end() && i->key <= key)
			++i;
		i--;
		d = selectEqual(tableinfor, indexinfor, key, i->childptr);
	}
	return Data(d);
}

Data IndexManager::selectBetween(const Table & tableinfor, const Index & indexinfor, string keystart, string keyend, int blockoffset)
{
	Data d;
	string filename = indexinfor.index_name + ".index";
	int bufferNum = buf.getbufferNum(filename, blockoffset);
	bool isLeaf = buf.block[bufferNum].value[1] == 'L';
	if (isLeaf)
	{
		while (1)
		{
			LeafNode leaf(bufferNum, indexinfor);
			auto i = leaf.NodeList.begin();
			for (; i != leaf.NodeList.end(); ++i)
			{
				if (i->key > keyend)
				{
					return d;
				}
				else
				{
					if (i->key >= keystart)
					{
						filename = indexinfor.table_name + ".table";
						int recordBufferNum = buf.getbufferNum(filename, i->offsetInFile);
						int pos = (tableinfor.TotalLength + 1)*i->offsetInBlock;
						string stringrow;
						stringrow = buf.block[recordBufferNum].getvalue(pos, pos + tableinfor.TotalLength);
						if (stringrow[0] != EMPTY)
						{
							stringrow.erase(stringrow.begin());
							Row split = splitRow(tableinfor, stringrow);
							d.Rows.emplace_back(split);
						}
					}
				}
			}
			if (leaf.NextBlock == 0)
				return d;
			else
			{
				filename = indexinfor.index_name + ".index";
				bufferNum = buf.getbufferNum(filename, leaf.NextBlock);
			}
		}
	}
	else
	{
		BranchNode branch(bufferNum, indexinfor);
		auto i = branch.NodeList.begin();
		if (i->key > keystart)
		{
			d = selectBetween(tableinfor, indexinfor, keystart, keyend, i->childptr);
			return d;
		}
		else
		{
			while (i != branch.NodeList.end() && i->key <= keystart)
				++i;
			i--;
			d = selectBetween(tableinfor, indexinfor, keystart, keyend, i->childptr);
			return d;
		}
	}
	return Data(d);
}

void IndexManager::dropIndex(Index & indexinfor)
{
	string filename = indexinfor.index_name + ".index";
	if (remove(filename.c_str()))							//delete the .index file
	{
		cout << "deleting indx failed";
	}
	buf.setInvalid(filename);
}

Row IndexManager::splitRow(Table tableinfor, string row)
{
	Row result;
	int start = 0, end = 0;
	for (int i = 0; i < tableinfor.AttriNum; ++i)
	{
		start = end;
		//end += tableinfor.attributes[i].length;
		while (end < tableinfor.Attributes[i].length && row[++end] != EMPTY);
		char* tempchar = new char[end - start + 1];

		strncpy(tempchar, &row[start], end - start);
		string tempstr(tempchar);
		result.Columns.emplace_back(tempstr);
	}
	return result;
}

string IndexManager::getColumnValue(const Table & tableinfor, const Index & indexinfor, string row)
{
	string result;
	int start = 0, end = 0;
	for (int i = 0; i < indexinfor.column; ++i)
		end += tableinfor.Attributes[i].length;
	start = end - tableinfor.Attributes[indexinfor.column - 1].length;

	for (int i = start; i < end&&row[i] != EMPTY; ++i) result += row[i];
	return string(result);
}

IndexBranch::IndexBranch()
{
	key = "";
	childptr = 0;
}

IndexBranch::IndexBranch(string k, int ptr)
{
	key = k;
	childptr = ptr;
}

BPlusTree::BPlusTree()
{
}

BPlusTree::BPlusTree(int bufNum)
{
	bufferNum = bufNum;
	recordNum = 0;
}

int BPlusTree::getPtr(int position)
{
	char stringptr[POINTERLENGTH + 1];
	strncpy(stringptr, &buf.block[bufferNum].value[position], POINTERLENGTH);
	int ptr = atoi(stringptr);
	if (ptr == 0)
	{
		cout << "Pointer wrong!" << endl;
	}
	return ptr;
}

int BPlusTree::getRecordNum()
{
	for (int i = 2; i < 6; i++)
		if (buf.block[bufferNum].value[i] == EMPTY)
			return 0;

	char stringnum[5];
	strncpy(stringnum, &buf.block[bufferNum].value[2], 4);
	return atoi(stringnum);
}

LeafNode::LeafNode(int bufNum)
{
	bufferNum = bufNum;
	recordNum = 0;
	NextBlock = LastBlock = 0;
}

LeafNode::LeafNode(int bufNum, const Index & indexinfor)
{
	bufferNum = bufNum;
	isRoot = buf.block[bufNum].value[0] == 'R';
	int totalRecord = getRecordNum();
	recordNum = 0;
	fatherptr = getPtr(6);
	LastBlock = getPtr(6 + POINTERLENGTH);
	NextBlock = getPtr(6 + POINTERLENGTH * 2);
	ColumnLength = indexinfor.ColumnLength;

	int pos = 6 + POINTERLENGTH * 3;
	for (int i = 0; i < totalRecord; ++i)
	{
		string key = "";
		int temppos = pos;
		while (temppos < pos + ColumnLength && buf.block[bufferNum].value[temppos] == EMPTY)
			key += buf.block[bufferNum].value[temppos++];
		pos += ColumnLength;
		int offsetInFile = getPtr(pos);
		pos += POINTERLENGTH;
		int offsetInBlock = getPtr(pos);
		pos += POINTERLENGTH;
		IndexLeaf node(key, offsetInFile, offsetInBlock);
		Insert(node);
	}
}

LeafNode::~LeafNode()
{
}

void LeafNode::Insert(IndexLeaf Node)
{
	++recordNum;
	auto i = NodeList.begin();

	if (NodeList.size() != 0)
		while (i != NodeList.end() && i->key <= Node.key)
			i++;
	NodeList.insert(i, Node);
}

IndexLeaf LeafNode::pop()
{
	--recordNum;
	auto back = NodeList.end();
	IndexLeaf temp;
	temp = *back;
	NodeList.pop_back();
	return IndexLeaf(temp);
}

IndexLeaf LeafNode::getFront()
{
	return IndexLeaf(NodeList.front());
}