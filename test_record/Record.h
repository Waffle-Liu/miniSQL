#pragma once
#include "Buffer.h"
#include "MiniSQL.h"

extern BufferManager buf;
class Record {
public:
	Record();
	~Record();
	
	bool Comparator(Table tableinfo, Row row, vector<Condition> conditions);
	
	Data select(Table& tableinfo); // select * from table_name
	
	Data select(Table tableinfo, vector<Condition> conditions); // select attr_name from table_name where attr_name op value

	void insertValue(Table& tableinfo, Row& splitedrow);

	int deleteValue(Table tableinfo);

	int deleteValue(Table tableinfo, vector<Condition> conditions);

	void dropTable(Table tableinfo);

	void CreateTable(Table tableinfo);

	void showDatas(const Data& datas) const;

private:
	Row SplitRow(Table tableinfo, string row);
	string connectRow(Table tableinfo, Row splitedrow);
};






