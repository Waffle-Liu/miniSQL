#include "Buffer.h"

BufferManager::BufferManager()
{
	block = new buffer[MAXBLOCKNUMBER];
	clockptr = 0;
}

BufferManager::~BufferManager()
{
	for (int i = 0; i < MAXBLOCKNUMBER; ++i)
		WriteBack(i);
	delete[] block;
}

void BufferManager::WriteBack(int bufferNum)
{
	if (!block[bufferNum].isWritten)
	{
		return;
	}
	else
	{
		clockptr = bufferNum;

		string filename = block[bufferNum].filename;
		fstream fout(filename.c_str(), ios::in | ios::out);
		int offset = block[bufferNum].blockoffset;

		fout.seekp(BLOCKSIZE*offset, fout.beg);
		fout.write(block[bufferNum].value, BLOCKSIZE);
		fout.close();

		block[bufferNum].init();
	}

}

int BufferManager::getbufferNum(string filename, int blockoffset)
{
	int bufferNum = SearchForBufferNum(filename, blockoffset);
	if (bufferNum == -1)
	{
		bufferNum = EmptyBuffer(filename);
		ReadBlock(filename, blockoffset, bufferNum);
	}
	return bufferNum;
}

void BufferManager::ReadBlock(string filename, int blockoffset, int bufferNum)
{
	fstream fin(filename.c_str(), ios::in);

	block[bufferNum].filename = filename;
	block[bufferNum].blockoffset = blockoffset;
	block[bufferNum].clockFlag = true;
	block[bufferNum].isWritten = false;
	block[bufferNum].isValid = true;

	fin.seekp(BLOCKSIZE*blockoffset, fin.beg);
	fin.read(block[bufferNum].value, BLOCKSIZE);
	fin.close();

	clockptr = bufferNum;

}

void BufferManager::WriteBlock(int bufferNum)
{
	block[bufferNum].isWritten = true;
	block[bufferNum].clockFlag = true;

	clockptr = bufferNum;
}

int BufferManager::EmptyBuffer()
{

	while (block[clockptr].clockFlag || block[clockptr].isPinned)	//if the block is pinned, keep searching
	{
		block[clockptr].clockFlag = 0;
		clockptr++;
		if (clockptr == MAXBLOCKNUMBER) clockptr = 0;
	}
	if (!block[clockptr].isValid)
	{
		block[clockptr].init();
		block[clockptr].isValid = true;
	}
	else
	{
		WriteBack(clockptr);
		block[clockptr].init();
		block[clockptr].isValid = true;
	}
	return clockptr;
}

int BufferManager::EmptyBuffer(string filename)
{
	int originptr = clockptr;
	bool flag = false;							//find a empty buffer with name not filename
	while (1)
	{
		while (block[clockptr].clockFlag || block[clockptr].isPinned) // if the block is pinned, keep searching
		{
			block[clockptr].clockFlag = false;
			clockptr++;
			if (clockptr == MAXBLOCKNUMBER) clockptr = 0;
		}
		if (block[clockptr].filename == filename)
		{
			++clockptr;
			if (flag) break;
			if (originptr == clockptr) flag = true;		//once the clockptr point to the originptr twice, it means the buffer has been token up
			continue;
		}
		else
			break;
	}

	if (!flag)											//not find a empty buffer with a filename not perticular filename
	{
		if (!block[clockptr].isValid)
		{
			block[clockptr].init();
			block[clockptr].isValid = true;
		}
		else
		{
			WriteBack(clockptr);
			block[clockptr].init();
			block[clockptr].isValid = true;
		}
		return clockptr;
	}
	else
	{
		std::cout << "The buffer runs out!";
		exit(0);
	}
}

/*void BufferManager::useBlock(int bufferNum)
{
block[bufferNum].clockFlag = 1;
//block[bufferNum].isValid = 1;
}*/

insertPos BufferManager::getInsertPosition(Table & tableinfor)
{
	if (tableinfor.BlockNum != 0)
	{
		string filename = tableinfor.name + ".table";
		int length = tableinfor.TotalLength + 1;					//the extra bit is to indicate the valid of data
		int blockoffset = tableinfor.BlockNum - 1;					//get the last block of the file

		int bufferNum = getbufferNum(filename, blockoffset);

		//now we got the bufferNum of the last block in buffer

		int recordNum = BLOCKSIZE / length;
		for (int i = 0; i < recordNum; ++i)
		{
			int pos = length * i;
			if (block[bufferNum].value[pos] == EMPTY)
			{
				insertPos temp(bufferNum, pos);
				return temp;
			}
		}
	}
	//the blockNUM==0, a new block should be create
	//or the last block in file has been used up, we need to create a new block in file
	insertPos temp(CreateNewBlockInFile(tableinfor), 0);
	return temp;
}


int BufferManager::CreateNewBlockInFile(Table & tableinfo)
{
	int bufferNum = EmptyBuffer();
	block[bufferNum].init();
	block[bufferNum].filename = tableinfo.name + ".table";
	block[bufferNum].isValid = block[bufferNum].isWritten = true;
	block[bufferNum].blockoffset = tableinfo.BlockNum++;
	return bufferNum;
}

int BufferManager::CreateNewBlockInFile(Index & indexinfo)
{
	string filename = indexinfo.index_name + ".index";
	int bufferNum = EmptyBuffer(filename);
	block[bufferNum].init();
	block[bufferNum].isValid = block[bufferNum].isWritten = true;
	block[bufferNum].filename = filename;
	block[bufferNum].blockoffset = indexinfo.BlockNum++;
	return bufferNum;
}

int BufferManager::SearchForBufferNum(string file, int blockoffset)
{
	for (int i = 0; i < MAXBLOCKNUMBER; ++i)
	{
		if (block[i].filename == file && block[i].blockoffset == blockoffset)
			return i;
	}
	return -1;//find nothing
}

void BufferManager::setInvalid(string filename)
{
	for (int i = 0; i < MAXBLOCKNUMBER; ++i)
	{
		if (block[i].filename == filename)
		{
			block[i].isValid = false;
			block[i].isWritten = false;
		}
	}
}
