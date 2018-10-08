#include "MiniSQL.h"

buffer::buffer()										//ctor
{
	init();
}

buffer::~buffer()
{
	delete[] value;
}

void buffer::init()
{
	filename = "NULL";
	blockoffset = 0;
	//LRUvalue = 0;
	for (int i = 0; i < BLOCKSIZE; i++) value[i] = EMPTY;
	value[BLOCKSIZE] = '\0';
	isWritten = false;
	isPinned = false;
	isValid = false;
	clockFlag = false;
}

char buffer::getvalue(int index)
{
	if (0 <= index && index <= BLOCKSIZE)
		return value[index];
	else
		return '\0';
}

string buffer::getvalue(int start, int end)
{
	string temp = "";
	if (0 <= start && start <= end && end <= BLOCKSIZE)
		for (int i = start; i < end; ++i)
			temp += value[i];
	return string(temp);
}

insertPos::insertPos()
{
	bufferNUM = -1;
	position = -1;
}

insertPos::insertPos(int bufNum, int pos)
{
	bufferNUM = bufNum;
	position = pos;
}
