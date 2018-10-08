#pragma once
#include "MiniSQL.h"
#include "Buffer.h"
#include "Interpreter.h"
#include "Record.h"
#include "Catalog.h"
#include "IndexManager.h"

Record record;
IndexManager index;
CatalogManager catalog;
Interpreter parsetree;
BufferManager buf;

bool IsComEnd(string input) { // 判断是否遇到了‘；’
	int pos = input.rfind(';', input.length() - 1);
	if (pos == -1) return false;
	else return true;
}

void ShowResult(Data data, Table tableinfo, vector<Attribute> column) {
	int i, j, k;
	int size = tableinfo.Attributes.size();
	int* length = new int[MAX_ATTRNUM];
	for (i = 0; i < size; i++) {
		if (tableinfo.Attributes[i].length > tableinfo.Attributes[i].name.length()) 
			length[i] = tableinfo.Attributes[i].length;
		else length[i] = tableinfo.Attributes[i].name.length();
	}
	if (column[0].name == "*") {
		cout << endl << "+";
		for (i = 0; i < tableinfo.AttriNum; i++) {
			for (j = 0; j < length[i] + 1; j++) {
				cout << "-";
			}
			cout << "+";
		}
		cout << endl;
		cout << "| ";
		for (i = 0; i < tableinfo.AttriNum; i++) {
			cout << tableinfo.Attributes[i].name;
			int lengthLeft = length[i] - tableinfo.Attributes[i].name.length();
			for (j = 0; j < lengthLeft; j++) {
				cout << ' ';
			}
			cout << "| ";
		}
		cout << endl;
		cout << "+";
		for (i = 0; i < tableinfo.AttriNum; i++) {
			for (j = 0; j < length[i] + 1; j++) {
				cout << "-";
			}
			cout << "+";
		}
		cout << endl;

		//内容
		for (i = 0; i < data.Rows.size(); i++) {
			cout << "| ";
			for (j = 0; j < tableinfo.AttriNum; j++) {
				int lengthLeft = length[j];
				for (k = 0; k < data.Rows[i].Columns[j].length(); k++) {
					if (data.Rows[i].Columns[j].c_str()[k] == EMPTY) break;
					else {
						cout << data.Rows[i].Columns[j].c_str()[k];
						lengthLeft--;
					}
				}
				for (k = 0; k < lengthLeft; k++) cout << " ";
				cout << "| ";
			}
			cout << endl;
		}

		cout << "+";
		for (i = 0; i < tableinfo.AttriNum; i++) {
			for (j = 0; j < length[i] + 1; j++) {
				cout << "-";
			}
			cout << "+";
		}
		cout << endl;
	}
	else {
		cout << endl << "+";
		for (i = 0; i < column.size(); i++) {
			int col;
			for (col = 0; col < tableinfo.Attributes.size(); col++) {
				if (tableinfo.Attributes[col].name == column[i].name) break;
			}
			for (j = 0; j < tableinfo.Attributes[col].length + 1; j++) {
				cout << "-";
			}
			cout << "+";
		}
		cout << endl;
		cout << "| ";
		for (i = 0; i < column.size(); i++) {
			int col;
			for (col = 0; col < tableinfo.Attributes.size(); col++) {
				if (tableinfo.Attributes[col].name == column[i].name) break;
			}
			cout << tableinfo.Attributes[col].name;
			int lengthLeft = length[col] - tableinfo.Attributes[col].name.length();
			for (j = 0; j < lengthLeft; j++) {
				cout << ' ';
			}
			cout << "| ";
		}
		cout << endl;
		cout << "+";
		for (i = 0; i < column.size(); i++) {
			int col;
			for (col = 0; col < tableinfo.Attributes.size(); col++) {
				if (tableinfo.Attributes[col].name == column[i].name) break;
			}
			for (j = 0; j < tableinfo.Attributes[col].length + 1; j++) {
				cout << "-";
			}
			cout << "+";
		}
		cout << endl;

		//内容
		for (i = 0; i < data.Rows.size(); i++) {
			cout << "| ";
			for (j = 0; j < column.size(); j++) {
				int col;
				for (col = 0; col < tableinfo.Attributes.size(); col++) {
					if (tableinfo.Attributes[col].name == column[j].name) break;
				}
				int lengthLeft = length[col];
				for (k = 0; k < data.Rows[i].Columns[col].length(); k++) {
					if (data.Rows[i].Columns[col].c_str()[k] == EMPTY) break;
					else {
						cout << data.Rows[i].Columns[col].c_str()[k];
						lengthLeft--;
					}
				}
				for (k = 0; k < lengthLeft; k++) cout << " ";
				cout << "| ";
			}
			cout << endl;
		}

		cout << "+";
		for (i = 0; i < column.size(); i++) {
			int col;
			for (col = 0; col < tableinfo.Attributes.size(); col++) {
				if (tableinfo.Attributes[col].name == column[i].name) break;
			}
			for (j = 0; j < length[i] + 1; j++) {
				cout << "-";
			}
			cout << "+";
		}
		cout << endl;
	}
	cout << data.Rows.size() << " rows in set" << endl;
}

bool Execute() {
	int i, j, k;
	Table tableinfo;
	Index indexinfo;
	string tempKeyValue;
	int tempPrimaryPosition = -1;
	int rowCount = 0;
	Data data;
	string command;
	string inputword;
	bool ComEnd = false;

	switch (parsetree.OpState) {
	case CRE_TAB:
		parsetree.TableIn.AttriNum = parsetree.TableIn.Attributes.size();
		catalog.CreateTable(parsetree.TableIn);
		record.CreateTable(parsetree.TableIn);
		cout << "Table " << parsetree.TableIn.name << " has been created successfully" << endl;
		break;
	case DRP_TAB:
		record.dropTable(parsetree.TableIn);
		for (int i = 0; i < parsetree.TableIn.AttriNum; i++) {//把这各表所有的index都删掉
			indexinfo = catalog.GetIndex(parsetree.TableIn.name);
			if (indexinfo.index_name != "")
				index.dropIndex(indexinfo);
		}
		catalog.DropTable(parsetree.TableIn.name);
		cout << "Table " << parsetree.TableIn.name << " has been dropped successfully" << endl;
		break;
	case INSERT:
		tableinfo = parsetree.TableIn;
		if (parsetree.PrimaryKeyPosition == -1 && parsetree.UniquePosition == -1) {
			record.insertValue(tableinfo, parsetree.rows);
			catalog.UpdateTable(tableinfo);
			cout << "One record has been inserted successfully" << endl;
			break;
		}
		if (parsetree.PrimaryKeyPosition != -1)
		{
			data = record.select(tableinfo, parsetree.conditions);
			if (data.Rows.size()>0) {
				cout << "Primary Key Redundancy occurs, thus insertion failed" << endl;
				break;
			}
		}
		if (parsetree.UniquePosition != -1) {

			data = record.select(tableinfo, parsetree.UniqueCondition);
			if (data.Rows.size()>0) {
				cout << "Unique Value Redundancy occurs, thus insertion failed" << endl;
				break;
			}
		}
		record.insertValue(tableinfo, parsetree.rows);
		catalog.UpdateTable(tableinfo);
		cout << "One record has been inserted successfully" << endl;
		break;
	case SEL_NOCON:
		tableinfo = parsetree.TableIn;
		data = record.select(tableinfo);
		if (data.Rows.size() != 0)
			ShowResult(data, tableinfo, parsetree.attributes);
		else {
			cout << "No data is found!!!" << endl;
		}
		break;
	case SEL_CON:
		tableinfo = parsetree.TableIn;
		if (parsetree.conditions.size() == 1) {
			for (int i = 0; i<parsetree.TableIn.Attributes.size(); i++) {
				if ((parsetree.TableIn.Attributes[i].IsPrimeryKey == true || parsetree.TableIn.Attributes[i].IsUnique == true) /*&& parsetree.m_colname == parsetree.TableIn.Attributes[i].name*/) {
					tempPrimaryPosition = i;
					indexinfo = catalog.GetIndex(tableinfo.name);
					break;
				}
			}
			if (tempPrimaryPosition == parsetree.conditions[0].columnNum&&parsetree.conditions[0].op == Eq&&indexinfo.table_name != "") {

				tempKeyValue = parsetree.conditions[0].value;
				data = index.selectEqual(tableinfo, indexinfo, tempKeyValue);
			}
			else {

				data = record.select(tableinfo, parsetree.conditions);
			}
		}
		else {
			data = record.select(tableinfo, parsetree.conditions);
		}
		if (data.Rows.size() != 0)
			ShowResult(data, tableinfo, parsetree.attributes);
		else {
			cout << "No data is found!!!" << endl;
		}
		break;
	case DEL_CON:
		rowCount = record.deleteValue(parsetree.TableIn, parsetree.conditions);
		cout << rowCount << "  rows have been deleted." << endl;
		break;
	case CRE_IND:
		tableinfo = parsetree.TableIn;
		indexinfo = parsetree.IndexIn;
		if (!tableinfo.Attributes[indexinfo.column].IsPrimeryKey && !tableinfo.Attributes[indexinfo.column].IsUnique) {//不是primary key，不可以建index
			cout << "Column " << tableinfo.Attributes[indexinfo.column].name << "  is not unique." << endl;
			break;
		}
		catalog.CreateIndex(indexinfo);
		index.createIndex(tableinfo, indexinfo);
		catalog.UpdateIndex(indexinfo);
		cout << "The index " << indexinfo.index_name << "has been created successfully" << endl;
		break;
	case DRP_IND:
		indexinfo = catalog.GetIndex(parsetree.IndexIn.index_name);
		if (indexinfo.index_name == "") {
			cout << "Index" << parsetree.IndexIn.index_name << "does not exist!" << endl;
		}
		index.dropIndex(indexinfo);
		catalog.DropIndex(parsetree.IndexIn.index_name);
		cout << "The index has been dropped successfully" << endl;
		break;
	case EXEFILE:
		while (parsetree.fin)
		{
			command = "";
			ComEnd = 0;
			while (!ComEnd) {
				if (parsetree.fin.eof()) break;
				getline(parsetree.fin, inputword, '\n');
				ComEnd = IsComEnd(inputword);
				command += inputword + " ";
			}
			if (parsetree.fin.eof()) break;
			command += '\0';
			parsetree.Parse(command);
			if (!Execute()) break;
		}
		parsetree.fin.close();
		break;
	case QUIT:
		cout << "You have already quit the system" << endl;
		getchar();
		return false;
		break;
	case EMPTY:
		cout << "Empty query! Please enter your command!" << endl;
		break;
	case UNKNOWN:
		cout << "The instruction is illegle." << endl;
		break;
	case ERR_NOEXISTTABLE:
		cout << "ERROR " << ERR_NOEXISTTABLE << ": Table does not exist" << endl;
		break;
	case ERR_NOEXISTARRT:
		cout << "ERROR " << ERR_NOEXISTARRT << ": Attribute does not exist" << endl;
		break;
	case ERR_NOEXISTINDEX:
		cout << "ERROR " << ERR_NOEXISTINDEX << ": Index does not exist" << endl;
		break;
	case ERR_NOEXISTPRIKEY:
		cout << "ERROR " << ERR_NOEXISTPRIKEY << ": Primary key does not exist" << endl;
		break;
	case ERR_EXISTTABLE:
		cout << "ERROR " << ERR_EXISTTABLE << ": Table already exists" << endl;
		break;
	case ERR_RECREATTR:
		cout << "ERROR " << ERR_RECREATTR << ": Attribute recreated" << endl;
		break;
	case ERR_REPETEKEY:
		cout << "ERROR " << ERR_REPETEKEY << ": Primary key recreated" << endl;
		break;
	case ERR_EXSITINDEX:
		cout << "ERROR " << ERR_EXSITINDEX << ": Index '" << parsetree.IndexIn.index_name << "' already exist" << endl;
		break;
	case ERR_OVERSIZE:
		cout << "ERROR " << ERR_OVERSIZE << ": The length of char is oversize " << endl;
		break;
	}
	return true;
}





