#include "Catalog.h"

void CatalogManager::InitTable()
{
	string table_name = "table.catelog";
	fstream fin;
	fin.open(table_name, ios::in);
	fin >> TableNum;
	for (int i = 0; i < TableNum; i++)
	{
		Table temptable;
		fin >> temptable.name;
		fin >> temptable.AttriNum;
		fin >> temptable.BlockNum;
		for (int j = 0; j < temptable.AttriNum; j++)
		{
			Attribute tempattri;
			fin >> tempattri.name;
			fin >> tempattri.type;
			fin >> tempattri.length;
			fin >> tempattri.IsPrimeryKey;
			fin >> tempattri.IsUnique;
			temptable.Attributes.push_back(tempattri);
			temptable.TotalLength += tempattri.length;
		}
		tables.push_back(temptable);
	}
	fin.close();
}

void CatalogManager::InitIndex()
{
	string index_name = "index.catalog";
	fstream fin;
	fin.open(index_name, ios::in);
	fin >> IndexNum;
	for (int i = 0; i < IndexNum; i++)
	{
		Index tempindex;
		fin >> tempindex.index_name;
		fin >> tempindex.table_name;
		fin >> tempindex.column;
		fin >> tempindex.ColumnLength;
		fin >> tempindex.BlockNum;
		indexs.push_back(tempindex);
	}
	fin.close();
}

void CatalogManager::StoreTable()
{
	string table_name = "table.catelog";
	fstream fout;
	fout.open(table_name, ios::out);
	fout << TableNum << endl;
	for (int i = 0; i < TableNum; i++)
	{
		fout << tables[i].name << " " << tables[i].AttriNum << " " << tables[i].BlockNum << ' ';
		for (int j = 0; j < tables[i].AttriNum; j++)
		{
			fout << tables[i].Attributes[j].name << ' ';
			fout << tables[i].Attributes[j].type << ' ';
			fout << tables[i].Attributes[j].length << ' ';
			fout << tables[i].Attributes[j].IsPrimeryKey << ' ';
			fout << tables[i].Attributes[j].IsUnique << ' ';
		}
		fout << endl;
	}
	fout.close();
}

void CatalogManager::StoreIndex()
{
	string index_name = "index.catalog";
	fstream fout;
	fout.open(index_name, ios::out);
	for (int i = 0; i < IndexNum; i++)
	{
		fout << indexs[i].index_name << ' ';
		fout << indexs[i].table_name << ' ';
		fout << indexs[i].column << ' ';
		fout << indexs[i].ColumnLength << ' ';
		fout << indexs[i].BlockNum << ' ';
	}
	fout << endl;
	fout.close();
}

CatalogManager::CatalogManager()
{
	InitTable();
	InitIndex();
}

CatalogManager::~CatalogManager()
{
	StoreTable();
	StoreIndex();
}

void CatalogManager::CreateTable(Table &t)
{
	TableNum++;
	for (int i = 0; i < t.Attributes.size(); i++)
	{
		t.TotalLength += t.Attributes[i].length;
	}
	tables.push_back(t);
}

void CatalogManager::CreateIndex(Index &ind)
{
	IndexNum++;
	indexs.push_back(ind);
}

void CatalogManager::DropTable(string table_name)
{
	for (int i = 0; i < TableNum; i++)
	{
		if (tables[i].name == table_name)
		{
			tables.erase(tables.begin() + i);
			TableNum--;
			break;
		}
	}

	for (int j = 0; j < IndexNum; j++)
	{
		if (indexs[j].table_name == table_name)
		{
			indexs.erase(indexs.begin() + j);
			IndexNum--;
			break;
		}
	}
}

void CatalogManager::DropIndex(string index_name)
{
	for (int i = 0; i < IndexNum; i++)
	{
		if (indexs[i].index_name == index_name)
		{
			indexs.erase(indexs.begin() + i);
			IndexNum--;
			break;
		}
	}
}

void CatalogManager::UpdateTable(Table & t)
{
	for (int i = 0; i < TableNum; i++)
	{
		if (tables[i].name == t.name)
		{
			tables[i].AttriNum = t.AttriNum;
			tables[i].BlockNum = t.BlockNum;
			tables[i].TotalLength = t.TotalLength;
			tables[i].Attributes = t.Attributes;
		}
	}
}

void CatalogManager::UpdateIndex(Index & ind)
{
	for (int i = 0; i < IndexNum; i++)
	{
		if (indexs[i].index_name == ind.index_name)
		{
			indexs[i].table_name = ind.table_name;
			indexs[i].column = ind.column;
			indexs[i].ColumnLength = ind.ColumnLength;
			indexs[i].BlockNum = ind.BlockNum;
		}
	}
}

bool CatalogManager::IsTable(string table_name)
{
	for (int i = 0; i < TableNum; i++)
	{
		if (tables[i].name == table_name)
			return true;
	}
	return false;
}

bool CatalogManager::IsIndex(string index_name)
{
	for (int i = 0; i < IndexNum; i++)
	{
		if (indexs[i].index_name == index_name)
			return true;
	}
	return false;
}

bool CatalogManager::IsIndexCol(string table_name, int colnum)
{
	for (int i = 0; i < indexs.size(); i++)
	{
		if (indexs[i].table_name == table_name && indexs[i].column == colnum)
			return true;
	}
	return false;
}

Table CatalogManager::GetTable(string table_name)
{
	Table empty;
	for (int i = 0; i < TableNum; i++)
	{
		if (tables[i].name == table_name)
			return tables[i];
	}
	return empty;
}

Index CatalogManager::GetIndex(string index_name)
{
	Index empty;
	for (int i = 0; i < IndexNum; i++)
	{
		if (indexs[i].index_name == index_name)
		{
			return indexs[i];
		}
	}
	return empty;
}

