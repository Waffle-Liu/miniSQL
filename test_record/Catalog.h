#pragma once
#include "MiniSQL.h"

class CatalogManager
{
private:
	int TableNum;
	vector<Table> tables;
	int IndexNum;
	vector<Index> indexs;
	void InitTable();
	void InitIndex();
	void StoreTable();
	void StoreIndex();

public:
	CatalogManager();
	~CatalogManager();
	void CreateTable(Table& t);
	void CreateIndex(Index& ind);
	void DropTable(string table_name);
	void DropIndex(string index_name);
	void UpdateTable(Table& t);
	void UpdateIndex(Index& ind);
	bool IsTable(string table_name);
	bool IsIndex(string index_name);
	bool IsIndexCol(string table_name, int colnum);

	Table GetTable(string table_name);
	Index GetIndex(string index_name);

};
