#pragma once
#include "MiniSQL.h"
class BufferManager
{
public:
	BufferManager();
	~BufferManager();
	buffer* block;
	int clockptr;
	void WriteBack(int bufferNum);
	int getbufferNum(string filename, int blockoffset);
	void ReadBlock(string filename, int blockoffset, int bufferNum);
	void WriteBlock(int bufferNum);
	int EmptyBuffer();
	int EmptyBuffer(string filename);
	//int EmptyBuffer(string filename);
	int CreateNewBlockInFile(Table& tableinfo);
	int CreateNewBlockInFile(Index& indexinfo);
	int SearchForBufferNum(string file, int blockoffset);
	void setInvalid(string filename);
	insertPos getInsertPosition(Table& tableinfo);
};