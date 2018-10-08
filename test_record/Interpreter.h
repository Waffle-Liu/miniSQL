#pragma once
#include "MiniSQL.h"
#include "Catalog.h"
#include"Record.h" 

using namespace std;

//static CCatalogManager* m_pCatalog = new CCatalogManager;
extern CatalogManager catalog;
extern Record  record;

class Interpreter
{
public:
	int OpState;
	string table_name;
	string index_name;
	string attriname;
	Row rows;
	vector<Attribute> attributes;
	vector<Condition> conditions;
	vector<Condition> UniqueCondition;
	int PrimaryKeyPosition;
	int UniquePosition;

	Table TableIn;
	Index IndexIn;
	fstream fin;
	Interpreter();
	~Interpreter();

	bool GetWord(string &command, string &word);
	bool GetStr(string &command, string &word);
	void Parse(string command);
	bool IsInt(string word);
};
