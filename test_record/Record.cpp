#include "Record.h"

Record::Record() {}

Record::~Record() {}

bool Record::Comparator(Table tableinfo, Row row, vector<Condition> conditions) { // 表信息，行信息，条件信息
	int i, k;
	for (i = 0; i < conditions.size(); i++) {
		int colNum = conditions[i].columnNum;
		int length1 = row.Columns[colNum].find_first_of('@');
		int length2 = conditions[i].value.length();
		string value1 = row.Columns[colNum].substr(0, length1);
		string value2 = conditions[i].value;
		int ivalue1, ivalue2;
		float fvalue1, fvalue2;

		switch (tableinfo.Attributes[colNum].type) {
		case CHAR:
			switch (conditions[i].op) {
			case Lt: // <
				if (value1 >= value2) return false;
				break;
			case Le: // <=
				if (value1 > value2) return false;
				break;
			case Gt: // >
				if (value1 <= value2) return false;
				break;
			case Ge: // >=
				if (value1 < value2) return false;
				break;
			case Eq: // ==
				if (value1 != value2) return false;
				break;
			case Ne: // !=
				if (value1 == value2) return false;
				break;
			default: break;
			}
			break;
		case INT: 
			ivalue1 = stoi(value1);
			ivalue2 = stoi(value2);
			switch (conditions[i].op) {
			case Lt: // <
				if (ivalue1 >= ivalue2) return false;
				break;
			case Le: // <=
				if (ivalue1 > ivalue2) return false;
				break;
			case Gt: // >
				if (ivalue1 <= ivalue2) return false;
				break;
			case Ge: // >=
				if (ivalue1 < ivalue2) return false;
				break;
			case Eq: // ==
				if (ivalue1 != ivalue2) return false;
				break;
			case Ne: // !=
				if (ivalue1 == ivalue2) return false;
				break;
			default: break;
			}
			break;
		case FLOAT: 
			fvalue1 = stof(value1);
			fvalue2 = stof(value2);
			switch (conditions[i].op) {
			case Lt:
				if (fvalue1 >= fvalue2) return false;
				break;
			case Le:
				if (fvalue1 > fvalue2) return false;
				break;
			case Gt:
				if (fvalue1 <= fvalue2) return false;
				break;
			case Ge:
				if (fvalue1 < fvalue2) return false;
				break;
			case Eq:
				if (fvalue1 != fvalue2) return false;
				break;
			case Ne:
				if (fvalue1 == fvalue2) return false;
				break;
			default: break;
			}
			break;
		default: break;
		}
	}
	return true;
}

Data Record::select(Table & tableinfo) { // select * from table_name
	string filename = tableinfo.name + ".table";
	string stringrow;
	Row splitedrow;
	Data datas;
	int length = tableinfo.TotalLength + 1;//加多一位来判断这条记录是否被删除了
	const int recordNum = BLOCKSIZE / length;

	for (int blockOffset = 0; blockOffset < tableinfo.BlockNum; blockOffset++) {
		int bufferNum = buf.getbufferNum(filename, blockOffset);
		if (bufferNum == -1) {
			bufferNum = buf.EmptyBuffer();
			buf.ReadBlock(filename, blockOffset, bufferNum);
		}
		for (int offset = 0; offset < recordNum; offset++) {
			int position = offset * length;
			stringrow = buf.block[bufferNum].getvalue(position, position + length);
			if (stringrow.c_str()[0] == EMPTY) continue; // indicate that this row of record have been deleted
			stringrow.erase(stringrow.begin()); // 把第一位去掉
			splitedrow = SplitRow(tableinfo, stringrow);
			datas.Rows.push_back(splitedrow);
		}
	}
	return datas;
}

Data Record::select(Table tableinfo, vector<Condition> conditions) {
	Data datas;
	if (conditions.size() == 0) {
		datas = select(tableinfo);
		return datas;
	}
	string filename = tableinfo.name + ".table";
	//buf.scanIn(tableinfo);
	string stringrow;
	Row splitedrow;
	int length = tableinfo.TotalLength + 1;//加多一位来判断这条记录是否被删除了
	const int recordNum = BLOCKSIZE / length;//加多一位来判断这条记录是否被删除了

	for (int blockOffset = 0; blockOffset < tableinfo.BlockNum; blockOffset++) {
		int bufferNum = buf.getbufferNum(filename, blockOffset);
		if (bufferNum == -1) {
			bufferNum = buf.EmptyBuffer();
			buf.ReadBlock(filename, blockOffset, bufferNum);
		}
		for (int offset = 0; offset < recordNum; offset++) {
			int position = offset * length;
			stringrow = buf.block[bufferNum].getvalue(position, position + length);
			if (stringrow[0] == EMPTY) continue;
			stringrow.erase(stringrow.begin());//把第一位去掉
			splitedrow = SplitRow(tableinfo, stringrow);
			if (Comparator(tableinfo, splitedrow, conditions)) //如果满足条件，就把结果push到datas里面去
				datas.Rows.push_back(splitedrow);
		}
	}
	return datas;
}

void Record::insertValue(Table & tableinfo, Row & splitedrow) {
	string stringrow = connectRow(tableinfo, splitedrow);
	insertPos iPos = buf.getInsertPosition(tableinfo);
	buf.block[iPos.bufferNUM].value[iPos.position] = NOTEMPTY;
	for (int i = 0; i < tableinfo.TotalLength; i++) {
		buf.block[iPos.bufferNUM].value[iPos.position + i + 1] = stringrow[i];
	}
	buf.block[iPos.bufferNUM].isWritten = 1;
}

int Record::deleteValue(Table tableinfo) {
	string filename = tableinfo.name + ".table";
	int count = 0;
	const int recordNum = BLOCKSIZE / (tableinfo.TotalLength + 1);	//加多一位来判断这条记录是否被删除了
	for (int blockOffset = 0; blockOffset < tableinfo.BlockNum; blockOffset++) {
		int bufferNum = buf.getbufferNum(filename, blockOffset);
		if (bufferNum == -1) {
			bufferNum = buf.EmptyBuffer();
			buf.ReadBlock(filename, blockOffset, bufferNum);
		}
		for (int offset = 0; offset < recordNum; offset++) {
			int position = offset * (tableinfo.TotalLength + 1);
			if (buf.block[bufferNum].value[position] != EMPTY) {
				buf.block[bufferNum].value[position] = EMPTY;
				count++;
			}
		}
	}
	return count;
}

int Record::deleteValue(Table tableinfo, vector<Condition> conditions) {
	string filename = tableinfo.name + ".table";
	string stringrow;
	Row splitedrow;
	int count = 0;
	const int recordNum = BLOCKSIZE / (tableinfo.TotalLength + 1);
	for (int blockOffset = 0; blockOffset < tableinfo.BlockNum; blockOffset++) {
		int bufferNum = buf.getbufferNum(filename, blockOffset);
		if (bufferNum == -1) {
			bufferNum = buf.EmptyBuffer();
			buf.ReadBlock(filename, blockOffset, bufferNum);
		}
		for (int offset = 0; offset < recordNum; offset++) {
			int position = offset * (tableinfo.TotalLength + 1);
			stringrow = buf.block[bufferNum].getvalue(position, position + tableinfo.TotalLength + 1);
			if (stringrow.c_str()[0] != EMPTY) {
				stringrow.erase(stringrow.begin());//把第一位去掉
				splitedrow = SplitRow(tableinfo, stringrow);
				if (Comparator(tableinfo, splitedrow, conditions)) {//如果满足条件，就把记录delete掉 
					buf.block[bufferNum].value[position] = DELETED;
					count++;
				}
			}
		}
		buf.block[bufferNum].isWritten = 1;
	}
	return count;
}


void Record::dropTable(Table tableinfo) {
	string filename = tableinfo.name + ".table";
	if (remove(filename.c_str()) != 0)
		perror("Error deleting file");
	else
		buf.setInvalid(filename);//when a file is deleted, a table or an index, all the value in buffer should be set invalid
}

void Record::CreateTable(Table tableinfo) {
	string filename = tableinfo.name + ".table";
	ofstream fout(filename);
	fout.close();
}

void Record::showDatas(const Data & datas) const {
	if (datas.Rows.size() == 0) {
		cout << "No Datas" << endl;
		return;
	}
	for (int i = 0; i < datas.Rows.size(); i++) {
		cout << (i + 1) << " "; // 序号 
		for (int j = 0; j < datas.Rows[i].Columns.size(); j++) {
			int endpos = datas.Rows[i].Columns[j].find_first_of(EMPTY);
			cout << datas.Rows[i].Columns[j].substr(0, endpos) << "\t" << endl;
		}
	}
}

Row Record::SplitRow(Table tableinfo, string row) {
	Row splitedrow;
	int pos = 0;//start position
	for (int i = 0; i < tableinfo.AttriNum; i++) {
		string col = row.substr(pos, tableinfo.Attributes[i].length);
		splitedrow.Columns.push_back(col);
		pos += tableinfo.Attributes[i].length;
	}
	return splitedrow;
}

string Record::connectRow(Table tableinfo, Row splitedrow) {
	string tmpRow;
	string stringrow = "";
	for (int i = 0; i < splitedrow.Columns.size(); i++) {
		tmpRow = splitedrow.Columns[i];
		int length = tmpRow.length();
		tmpRow.insert(length, tableinfo.Attributes[i].length - length, EMPTY); //补'@'以达到要求的长度
		stringrow += tmpRow;
	}
	return stringrow;
}
