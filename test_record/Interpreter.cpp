#include "Interpreter.h"

Interpreter::Interpreter()
{
}

Interpreter::~Interpreter()
{
}

bool Interpreter::GetWord(string &command, string &word)
{
	word.clear();
	char temp = ' ';
	int posc, posw = 0;

	for (posc = 0; posc < command.length(); posc++) {
		if (temp == ' ' || temp == '\t' || temp == '\0')
			temp = command[posc];
		else break;
	}
	if (posc == command.length() && (temp == ' ' || temp == '\t' || temp == '\0'))
		return false;

	switch (temp)
	{
	case '(':
	case ')':
	case '*':
	case '=':
	case ',':
	case ';':
	case '\'':
		word += temp;
		command.erase(0, posc);
		break;
	case '>':
		word += temp;
		temp = command[posc++];
		if (temp == '=')
		{
			word += temp;
			command.erase(0, posc);
		}
		else
		{
			command.erase(0, posc - 1);
		}
		break;
	case '<':
		word += temp;
		temp = command[posc];
		posc++;
		if (temp == '=' || temp == '>')
		{
			word += temp;
			command.erase(0, posc);
		}
		else
		{
			command.erase(0, posc - 1);
		}
		break;
	default:
		do
		{
			word += temp;
			if (posc < command.length())
				temp = command[posc++];
			else
			{
				command.erase(0, posc);
				word[posw++] = '\0';
				return true;
			}
		} while (temp != '(' && temp != ')' && temp != ',' && temp != '*'
			&& temp != ' '   && temp != '<'   &&temp != '>'  &&temp != '='
			&& temp != '\''  && temp != '\t'  && temp != 10 && temp != 13 && temp != ';');
		command.erase(0, posc - 1);
	}
	
	return true;
}

bool Interpreter::GetStr(string & command, string & word)
{
	int posc = 0;
	char temp;
	word.clear();
	if (command[0] == '\'')
	{
		word = "";
		return true;
	}
	else 
		for (posc = 0; posc < command.length(); posc++)
		{
			if (command[posc] != '\'')
			{
				temp = command[posc];
				word += temp;
			}
			else
			{
				command.erase(0, posc);
				return true;
			}
		}
	return false;
}


void Interpreter::Parse(string command)
{
	string temp = command;
	string word = "";
	string tempcol;
	bool flag;
	Attribute tempattr;
	Condition tempcond;
	int findp, findu;

	TableIn.clear();
	IndexIn.clear();
	findp = findu = 0;
	rows.Columns.clear();
	attributes.clear();
	conditions.clear();
	UniqueCondition.clear();
	PrimaryKeyPosition = UniquePosition = -1;
	OpState = UNKNOWN;

	flag = GetWord(temp, word);
	if (!flag)
	{
		OpState = UNKNOWN;//空命令
		return;
	}
	else if (word == "quit")
	{
		flag = GetWord(temp, word);
		if (word == ";")
		{
			OpState = QUIT;//退出
		}
		return;
	}
	else if (word == "create")
	{
		flag = GetWord(temp, word);
		if (!flag) return;
		if (word == "table")
		{
			flag = GetWord(temp, word);
			if (!flag) return;
			table_name = word;
			if (catalog.IsTable(table_name))
			{
				OpState = ERR_EXISTTABLE;//表存在
				return;
			}
			TableIn.name = table_name;
			flag = GetWord(temp, word);
			if (!flag) return;
			if (word != "(") return;

			flag = GetWord(temp, word);
			if (!flag) return;
			if (word == "primary" || word == "unique")
			{
				OpState = ERR_ILLEGLENAME; // 非法字符
				return;
			}
			tempattr.name = word;
			flag = GetWord(temp, word);
			if (!flag) return;
			if (word == "int")
			{
				tempattr.type = INT;
				tempattr.length = INTLEN;
				flag = GetWord(temp, word);
				if (!flag) return;
				if (word == "unique")
				{
					tempattr.IsUnique = true;
					flag = GetWord(temp, word);
					if (!flag) return;
				}
			}
			else if (word == "float")
			{
				tempattr.type = FLOAT;
				tempattr.length = FLOATLEN;
				flag = GetWord(temp, word);
				if (!flag) return;
				if (word == "unique")
				{
					tempattr.IsUnique = true;
					flag = GetWord(temp, word);
					if (!flag) return;
				}
			}
			else if (word == "char")
			{
				tempattr.type = CHAR;
				tempattr.length = CHARLEN;
				flag = GetWord(temp, word);
				if (!flag) return;
				if (word != "(") return;

				flag = GetWord(temp, word);
				if (!flag) return;
				if (!IsInt(word)) return;
				tempattr.length = stoi(word);
				if (tempattr.length > 256 || tempattr.length < 1)
				{
					OpState = ERR_OVERSIZE;//char超出界限
					return;
				}
				flag = GetWord(temp, word);
				if (!flag) return;
				if (word != ")") return;
				flag = GetWord(temp, word);
				if (!flag) return;
				if (word == "unique")
				{
					tempattr.IsUnique = true;
					flag = GetWord(temp, word);
					if (!flag) return;
				}
			}
			else return;
			TableIn.Attributes.push_back(tempattr);
			tempattr.clear();
			while (word == ",")
			{
				flag = GetWord(temp, word);
				if (!flag) return;
				if (word == "primary")
				{
					flag = GetWord(temp, word);
					if (!flag) return;
					if (word != "key") return;
					flag = GetWord(temp, word);
					if (!flag) return;
					if (word != "(") return;
					flag = GetWord(temp, word);
					if (!flag) return;
					for (int i = 0; i < TableIn.Attributes.size(); i++)
					{
						if (word == TableIn.Attributes[i].name)
						{
							findp = 1;
							TableIn.Attributes[i].IsPrimeryKey = true;
							TableIn.Attributes[i].IsUnique = true;
						}
					}
					if (!findp)
					{
						OpState = ERR_NOEXISTPRIKEY;//没有找到主键
						return;
					}
					findp = 0;
					flag = GetWord(temp, word);
					if (!flag) return;
					if (word != ")") return;
					flag = GetWord(temp, word);
					if (!flag) return;
				}
				else if (word == "unique")
				{
					flag = GetWord(temp, word);
					if (!flag) return;
					if (word != "(") return;
					flag = GetWord(temp, word);
					if (!flag) return;
					for (int i = 0; i < TableIn.Attributes.size(); i++)
					{
						if (word == TableIn.Attributes[i].name)
						{
							findu = 1;
							TableIn.Attributes[i].IsUnique = true;
						}
					}
					if (!findu)
					{
						OpState = ERR_NOEXISTTABLE;//没有找到该表
						return;
					}
					findu = 0;
					flag = GetWord(temp, word);
					if (!flag) return;
					if (word != ")") return;
					flag = GetWord(temp, word);
					if (!flag) return;
				}
				else
				{
					tempattr.name = word;
					flag = GetWord(temp, word);
					if (!flag) return;
					if (word == "int")
					{
						tempattr.type = INT;
						tempattr.length = INTLEN;
						flag = GetWord(temp, word);
						if (!flag) return;
						if (word == "unique")
						{
							tempattr.IsUnique = true;
							flag = GetWord(temp, word);
							if (!flag) return;
						}
					}
					else if (word == "float")
					{
						tempattr.type = FLOAT;
						tempattr.length = FLOATLEN;
						flag = GetWord(temp, word);
						if (!flag) return;
						if (word == "unique")
						{
							tempattr.IsUnique = true;
							flag = GetWord(temp, word);
							if (!flag) return;
						}
					}
					else if (word == "char")
					{
						tempattr.type = CHAR;
						tempattr.length = CHARLEN;
						flag = GetWord(temp, word);
						if (!flag) return;
						if (word != "(") return;

						flag = GetWord(temp, word);
						if (!flag) return;
						if (!IsInt(word)) return;
						tempattr.length = stoi(word);
						if (tempattr.length > 256 || tempattr.length < 1)
						{
							OpState = ERR_OVERSIZE;//char超出界限
							return;
						}
						flag = GetWord(temp, word);
						if (!flag) return;
						if (word != ")") return;
						flag = GetWord(temp, word);
						if (!flag) return;
						if (word == "unique")
						{
							tempattr.IsUnique = true;
							flag = GetWord(temp, word);
							if (!flag) return;
						}
					}
					else return;
					TableIn.Attributes.push_back(tempattr);
					tempattr.clear();
				}
			}
			if (word != ")") return;
			flag = GetWord(temp, word);
			if (!flag) return;
			if (word != ";") return;
			flag = GetWord(temp, word);
			if (!flag)
			{
				OpState = CRE_TAB;//表创建成功
				return;
			}
		}
		else if (word == "index")
		{
			OpState = CRE_IND;//创建索引
			flag = GetWord(temp, word);
			if (!flag) return;
			index_name = word;
			IndexIn.index_name = word;
			if (catalog.IsIndex(index_name))
			{
				OpState = ERR_EXSITINDEX;//index存在
				return;
			}
			flag = GetWord(temp, word);
			if (!flag) return;
			if (word != "on") return;
			flag = GetWord(temp, word);
			if (!flag) return;
			table_name = word;
			if (!catalog.IsTable(table_name))
			{
				OpState = ERR_NOEXISTTABLE;//表不存在
				return;
			}
			TableIn = catalog.GetTable(table_name);
			IndexIn.table_name = word;
			flag = GetWord(temp, word);
			if (!flag) return;
			if (word != "(") return;
			flag = GetWord(temp, word);
			if (!flag) return;
			int tempcolnum = -1;
			for (int i = 0; i<TableIn.Attributes.size(); i++)
			{
				if (TableIn.Attributes[i].name == word)
				{
					tempcolnum = i;
					break;
				}
			}
			if (tempcolnum == -1)
			{
				OpState = ERR_NOEXISTARRT;//属性不存在
				return;
			}
			if (catalog.IsIndexCol(table_name, tempcolnum))
			{
				OpState = ERR_EXSITINDEX;//索引已存在
				return;
			}
			IndexIn.BlockNum = 0;
			IndexIn.column = tempcolnum;
			IndexIn.ColumnLength = TableIn.Attributes[tempcolnum].length;
			flag = GetWord(temp, word);
			if (!flag) return;
			if (word != ")") return;
			flag = GetWord(temp, word);
			if (!flag) return;
			if (word != ";") return;
			flag = GetWord(temp, word);
			if (!flag)
			{
				OpState = CRE_IND;//索引创建成功
				return;
			}
		}
	}
	else if (word == "select")
	{
//		OpState = SEL_;//select
		flag = GetWord(temp, word);
		if (!flag) return;
		tempattr.name = word;
		attributes.push_back(tempattr);
		flag = GetWord(temp, word);
		if (!flag) return;
		while (word == ",")
		{
			flag = GetWord(temp, word);
			if (!flag) return;
			tempattr.name = word;
			attributes.push_back(tempattr);
			flag = GetWord(temp, word);
			if (!flag) return;
		}
		if (word != "from") return;
		flag = GetWord(temp, word);
		if (!flag) return;
		table_name = word;

		if (!catalog.IsTable(table_name))
		{
			OpState = ERR_NOEXISTTABLE;//表不存在
			return;
		}
		TableIn = catalog.GetTable(table_name);
		flag = GetWord(temp, word);
		if (word == ";")
		{
			OpState = SEL_NOCON;//结束
			return;
		}

		if (word != "where") return;
		flag = GetWord(temp, word);
		if (!flag) return;
		for (int i = 0; i < TableIn.Attributes.size(); i++)
		{
			if (word == TableIn.Attributes[i].name)
			{
				tempcond.columnNum = i;
				break;
			}
		}
		attriname = word;
		if (tempcond.columnNum == -1)
		{
			OpState = ERR_NOEXISTARRT;//列错误
			return;
		}
		flag = GetWord(temp, word);
		if (!flag) return;
		if (word == "=") tempcond.op = Eq;
		else if (word == "<=") tempcond.op = Le;
		else if (word == ">=") tempcond.op = Ge;
		else if (word == "<") tempcond.op = Lt;
		else if (word == ">") tempcond.op = Gt;
		else if (word == "<>") tempcond.op = Ne;
		else return;

		flag = GetWord(temp, word);
		if (!flag) return;
		if (word == "\'")
		{
			flag = GetStr(temp, word);
			if (!flag) return;
			tempcond.value = word;
			flag = GetWord(temp, word);
			if (!flag) return;
			if (word != "\'") return;
		}
		else
			tempcond.value = word;

		conditions.push_back(tempcond);
		flag = GetWord(temp, word);
		if (word == ";")
		{
			OpState = SEL_CON; // 查询成功
			return;
		}
		while (word == "and")
		{
			flag = GetWord(temp, word);
			if (!flag) return;
			for (int i = 0; i < TableIn.Attributes.size(); i++)
			{
				if (word == TableIn.Attributes[i].name)
				{
					tempcond.columnNum = i;
					break;
				}
			}
			attriname = word;
			if (tempcond.columnNum == -1)
			{
				OpState = ERR_NOEXISTARRT; // 列错误
				return;
			}
			flag = GetWord(temp, word);
			if (!flag) return;
			if (word == "=") tempcond.op = Eq;
			else if (word == "<=") tempcond.op = Le;
			else if (word == ">=") tempcond.op = Ge;
			else if (word == "<") tempcond.op = Lt;
			else if (word == ">") tempcond.op = Gt;
			else if (word == "<>") tempcond.op = Ne;
			else return;

			flag = GetWord(temp, word);
			if (!flag) return;
			if (word == "\'")
			{
				flag = GetStr(temp, word);
				if (!flag) return;
				tempcond.value = word;
				flag = GetWord(temp, word);
				if (!flag) return;
				if (word != "\'") return;
			}
			else
				tempcond.value = word;

			conditions.push_back(tempcond);
			flag = GetWord(temp, word);
			if (word == ";")
			{
				OpState = SEL_CON; // 查询成功
				return;
			}
		}

	}
	else if (word == "insert")
	{
		flag = GetWord(temp, word);
		if (!flag) return;
		if (word != "into") return;
		flag = GetWord(temp, word);
		if (!flag) return;
		table_name = word;
		if (!catalog.IsTable(table_name))
		{
			OpState = ERR_NOEXISTTABLE; // 表不存在
			return;
		}
		TableIn = catalog.GetTable(table_name);
		flag = GetWord(temp, word);
		if (!flag) return;
		if (word != "values") return;
		flag = GetWord(temp, word);
		if (!flag) return;
		if (word != "(") return;
		flag = GetWord(temp, word);
		if (!flag) return;
		if (word == "\'")
		{
			flag = GetStr(temp, word);
			if (!flag) return;
			tempcol = word;
			flag = GetWord(temp, word);
			if (!flag) return;
			if (word != "\'") return;
		}
		else
		{
			tempcol = word;
		}
		flag = GetWord(temp, word);
		if (!flag) return;
		rows.Columns.push_back(tempcol);

		while (word == ",")
		{
			flag = GetWord(temp, word);
			if (!flag) return;
			if (word == "\'")
			{
				flag = GetStr(temp, word);
				if (!flag) return;
				tempcol = word;
				flag = GetWord(temp, word);
				if (!flag) return;
				if (word != "\'") return;
			}
			else
			{
				tempcol = word;
			}

			rows.Columns.push_back(tempcol);
			flag = GetWord(temp, word);
			if (!flag) return;
		}
		if (word != ")") return;
		if (rows.Columns.size() != TableIn.Attributes.size())
		{
			OpState = ERR_INSERTVALUENUM; // insert: 插入的值个数和属性个数不对应
			return;
		}

		for (int i = 0; i < TableIn.Attributes.size(); i++)
		{
			if (TableIn.Attributes[i].IsPrimeryKey)
			{
				PrimaryKeyPosition = i;
				tempcond.columnNum = i;
				tempcond.value = rows.Columns[i];
				tempcond.op = Eq;
				conditions.push_back(tempcond);
			}
			if (TableIn.Attributes[i].IsUnique)
			{
				UniquePosition = i;
				tempcond.columnNum = i;
				tempcond.value = rows.Columns[i];
				tempcond.op = Eq;
				UniqueCondition.push_back(tempcond);
			}
		}
		flag = GetWord(temp, word);
		if (!flag) return;
		if (word != ";") return;
		flag = GetWord(temp, word);
		if (!flag)
		{
			OpState = INSERT;//插入成功
			return;
		}
	}
	else if (word == "delete")
	{
		flag = GetWord(temp, word);
		if (!flag) return;
		if (word != "from") return;
		flag = GetWord(temp, word);
		if (!flag) return;
		table_name = word;
		if (!catalog.IsTable(table_name))
		{
			OpState = ERR_NOEXISTTABLE;//表不存在
			return;
		}
		TableIn = catalog.GetTable(table_name);
		flag = GetWord(temp, word);
		if (word == ";")
		{
			OpState = DEL_NOCON;//删除成功
			return;
		}
		if (word != "where") return;
		flag = GetWord(temp, word);
		if (!flag) return;
		int tempint = -1;
		for (int i = 0; i < TableIn.Attributes.size(); i++)
		{
			if (word == TableIn.Attributes[i].name)
			{
				tempint = i;
				break;
			}
		}
		if (tempint == -1)
		{
			OpState = ERR_NOEXISTTABLE;//找不到表
			return;
		}
		tempcond.columnNum = tempint;
		flag = GetWord(temp, word);
		if (!flag) return;
		if (word == "=") tempcond.op = Eq;
		else if (word == "<=") tempcond.op = Le;
		else if (word == ">=") tempcond.op = Ge;
		else if (word == "<") tempcond.op = Lt;
		else if (word == ">") tempcond.op = Gt;
		else if (word == "<>") tempcond.op = Ne;
		else return;

		flag = GetWord(temp, word);
		if (!flag) return;
		if (word == "\'")
		{
			flag = GetStr(temp, word);
			if (!flag) return;
			tempcond.value = word;
			flag = GetWord(temp, word);
			if (!flag) return;
			if (word != "\'") return;
		}
		else
			tempcond.value = word;

		conditions.push_back(tempcond);
		flag = GetWord(temp, word);
		if (word == ";")
		{
			OpState = DEL_CON;
			return;
		}
		while (word == "and")
		{
			flag = GetWord(temp, word);
			if (!flag) return;
			for (int i = 0; i < TableIn.Attributes.size(); i++)
			{
				if (word == TableIn.Attributes[i].name)
				{
					tempcond.columnNum = i;
					break;
				}
			}
			attriname = word;
			if (tempcond.columnNum == -1)
			{
				OpState = ERR_NOEXISTARRT; // 列错误，属性不存在
				return;
			}
			flag = GetWord(temp, word);
			if (!flag) return;
			if (word == "=") tempcond.op = Eq;
			else if (word == "<=") tempcond.op = Le;
			else if (word == ">=") tempcond.op = Ge;
			else if (word == "<") tempcond.op = Lt;
			else if (word == ">") tempcond.op = Gt;
			else if (word == "<>") tempcond.op = Ne;
			else return;

			flag = GetWord(temp, word);
			if (!flag) return;
			if (word == "\'")
			{
				flag = GetStr(temp, word);
				if (!flag) return;
				tempcond.value = word;
				flag = GetWord(temp, word);
				if (!flag) return;
				if (word != "\'") return;
			}
			else
				tempcond.value = word;

			conditions.push_back(tempcond);
			flag = GetWord(temp, word);
			if (!flag) return;
			for (int i = 0; i < TableIn.Attributes.size(); i++)
			{
				if (word == TableIn.Attributes[i].name)
				{
					tempcond.columnNum = i;
					break;
				}
			}
			attriname = word;
			if (tempcond.columnNum == -1)
			{
				OpState = ERR_NOEXISTARRT;//列错误
				return;
			}
			flag = GetWord(temp, word);
			if (!flag) return;
			if (word == "=") tempcond.op = Eq;
			else if (word == "<=") tempcond.op = Le;
			else if (word == ">=") tempcond.op = Ge;
			else if (word == "<") tempcond.op = Lt;
			else if (word == ">") tempcond.op = Gt;
			else if (word == "<>") tempcond.op = Ne;
			else return;

			flag = GetWord(temp, word);
			if (!flag) return;
			if (word == "\'")
			{
				flag = GetWord(temp, word);
				if (!flag) return;
				tempcond.value = word;
				flag = GetWord(temp, word);
				if (!flag) return;
				if (word != "\'") return;
			}
			else
				tempcond.value = word;

			conditions.push_back(tempcond);
			flag = GetWord(temp, word);
			if (word == ";")
			{
				OpState = DEL_CON;//删除成功
				return;
			}
		}
	}
	else if (word == "drop")
	{
		flag = GetWord(temp, word);
		if (!flag) return;
		if (word == "table")
		{
			flag = GetWord(temp, word);
			if (!flag) return;
			table_name = word;
			if (!catalog.IsTable(table_name))
			{
				OpState = ERR_NOEXISTTABLE; // 表不存在
				return;
			}
			TableIn = catalog.GetTable(table_name);
			flag = GetWord(temp, word);
			if (word == ";")
			{
				OpState = DRP_TAB;//删表成功
				return;
			}
		}
		else if (word == "index")
		{
			flag = GetWord(temp, word);
			if (!flag) return;
			index_name = word;
			if (!catalog.IsIndex(index_name))
			{
				OpState = ERR_NOEXISTINDEX;//索引不存在
				return;
			}
			IndexIn = catalog.GetIndex(index_name);
			flag = GetWord(temp, word);
			if (word == ";")
			{
				OpState = DRP_IND;//删索引成功
				return;
			}
		}
	}
	else if (word == "execfile")
	{
		flag = GetWord(temp, word);
		if (!flag) return;
		fin.open(word, ios::in);
		flag = GetWord(temp, word);
		if (word == ";")
		{
			OpState = EXEFILE;
			return;
		}
	}
	return;
}

bool Interpreter::IsInt(string word)
{
	int i = 0;
	if (word[0] == '-') i = 1;
	for (; i < word.length(); i++)
	{
		if ('0' <= word[i] && word[i] <= '9')
			continue;
		else return false;
	}
	return true;
}

