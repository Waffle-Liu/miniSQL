#ifndef _MINISQL_H
#define _MINISQL_H
#pragma warning(disable:4996)
#include <string>
#include <math.h>
#include <iostream>
#include <cstdbool>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <list>
using namespace std;

// DATATYPE
#define INT 1
#define FLOAT 2
#define CHAR 3
#define INTLEN 10
#define FLOATLEN 8
#define CHARLEN 255

// ATTRIBUTE
#define ISPRIMARYKEY 1
#define NOTPRIMARYKEY 0
#define ISUNIQUE 1
#define NOTUNIQUE 0
#define MAXPRIMARYKEYLENGTH  25    //should change sometime

// BLOCK
#define MAXBLOCKNUMBER 1000	//should be 10000
#define BLOCKSIZE 4096	//should be 4096
#define EMPTY '@'
#define END '@'
#define NOTEMPTY '1'
#define DELETED '@'
#define POINTERLENGTH 5

// OPERATION
#define UNKNOWN		0 // unknown operation
#define CRE_TAB		1 // CREATE TABLE
#define	DRP_TAB		2 // DROP TABLE 
#define	CRE_IND		3 // CREATE INDEX
#define DRP_IND		4 // DROP INDEX
#define SEL_NOCON	5 // SELECT * from TABLE
#define SEL_CON		6 // SELECT * from TABLE where CONDITION
#define INSERT		7 // INSERT 
#define DEL_NOCON	8 // DELETE from TABLE
#define DEL_CON		9 // DELETE from TABLE where CONDITION
#define QUIT		10 // QUIT
#define EXEFILE		11 // EXECUTE SQL FILE

// ERRORTYPE
#define ERR_NOKEYWORD			11  // sql: 语句第一个字是无效命令
// 语法错误
#define ERR_CREATE				12  // create: create后没有table或index
#define ERR_DROP				13  // drop: drop后没有table或index
#define ERR_TNAME				14  // create table: 表名是关键字
#define ERR_ILLEGLENAME			15  // create table: 属性名或者表名含有非法字符
#define ERR_OVERNAMESIZE		16  // create table: 属性名或者表名过长（超过16个字符）
#define ERR_LP					17  // 缺少左括号或者左括号附近有错误
#define ERR_RP					18  // 缺少右括号或者右括号附近有错误
#define ERR_TOOMANYATTR			19  // create table: 属性的数目过多
#define ERR_SIZE				20 // create table: char（n）中n不是整数
#define ERR_ATTR				21 // create table: 属性名是关键字
#define ERR_NOATTR				22 // create table: 定义为primary key的属性不存在
#define ERR_PRIMARYKEY			23 // create table: primary key定义附近出错
#define ERR_TYPE				24 // create table: 非法类型名（只支持int，char，float）
#define ERR_OVERSIZE			25 // create table: 超过char的最大长度
#define ERR_CREATEINDEX			26 // creat index语法错误
#define ERR_DROPTABLE			27 // drop table语法错误 
#define ERR_DropIndex			28 // drop index语法错误
#define ERR_INSERT				29 // insert语法错误
#define ERR_DELETE				30 // delete语法错误
#define ERR_DELETECONDITION		31 // delete中where子句语法错误
#define ERR_SELECTTABLE			32 // select语法错误
#define ERR_SELECTCONDITION		33 // select中where子句语法错误
#define ERR_END					34 // 非法结束符（必须是分号）
#define ERR_NOSENTENCE			35 // 没有可以供分析的语句
#define ERR_QUOTE				36 // 常量字符串缺少单引号
#define ERR_CONSTSTRING			37 // 常量字符串中有非法字符（'\n','\r'）
#define ERR_VALUEILLEGAL		38 // 数值数据（包括int，float）有非法字符
// 逻辑错误
#define ERR_EXISTTABLE			39 // create table: 定义的表已经存在
#define ERR_RECREATTR			40 // create table: 属性重复
#define ERR_REPETEKEY			41 // create table: primary key重复
#define ERR_EXSITINDEX			42 // create index: 索引已存在
#define ERR_NOEXISTTABLE		43 // 表不存在
#define ERR_NOEXISTARRT			44 // 属性不存在
#define ERR_NOEXISTINDEX		45 // delete index: 索引不存在
#define ERR_NOEXISTPRIKEY		46 // create table: primary key不存在
#define ERR_INSERTVALUENUM		47 // insert: 插入的值个数和属性个数不对应
#define ERR_INSERTNOTUNIQ		48 // insert: 值不唯一

// 大小
#define MAX_ATTRNUM		32  // 属性个数
#define MAX_STATNUM		200 // 语句个数
#define MAX_SET		100 // 
#define VALLEN		300 // 
#define NAMELEN		100 //

class Attribute {
public:
	string name;		// 名称
	int type;			// 类型
	int length;			// 长度
	bool IsPrimeryKey;	// 是否为主键
	bool IsUnique;		// 是否唯一
	Attribute() {
		name = "";
		length = 0;
		IsPrimeryKey = false;
		IsUnique = false;
	}
	Attribute(string n, int t, int l, bool isP, bool isU)
		:name(n), type(t), length(l), IsPrimeryKey(isP), IsUnique(isU) {}
	void clear()
	{
		type = length = 0;
		IsPrimeryKey = IsUnique = false;
	}
};

class Table {
public:
	string name;   // 名称
	int BlockNum;	// block数量（表格数）
//	int recordNum;	// record数量
	int AttriNum;	// attribute数量
	int TotalLength;	//total length of one record, should be equal to sum(attributes[i].length)
	vector<Attribute> Attributes; // 属性
	Table() : BlockNum(0), AttriNum(0), TotalLength(0) {}
	void clear()
	{
		name.clear();
		AttriNum = 0;
		BlockNum = 0;
		TotalLength = 0;
		Attributes.clear();
	}
};
 
class Index
{
public:
	string index_name;	//all the datas is store in file index_name.index
	string table_name;	//the name of the table on which the index is create
	int column;			//on which column the index is created
	int ColumnLength;
	int BlockNum;		//number of block the datas of the index occupied in the file index_name.table
	Index() : column(0), BlockNum(0) {}
	void clear()
	{
		column = 0;
		ColumnLength = 0;
		BlockNum = 0;
	}
};

class Row
{
public:
	vector<string> Columns;
};

class Data//这样就把Data搞成了一个二维数组
{
public:
	vector<Row> Rows;
};

enum Comparison { Lt, Le, Gt, Ge, Eq, Ne };//stants for less than, less equal, greater than, greater equal, equal, not equal respectivly
class Condition {
public:
	Comparison op; // comparison
	int columnNum; // 第columnNum列
	string value;
};

class buffer
{
public:
	string filename;
	int blockoffset;
	//	int LRUvalue;
	char* value = new char[BLOCKSIZE + 1];
	bool isWritten;
	bool isValid;
	bool isPinned;
	bool clockFlag;
	buffer();
	~buffer();
	void init();
	char getvalue(int index);
	string getvalue(int start, int end);
};

class insertPos {
public:
	int bufferNUM;
	int position;
	insertPos();
	insertPos(int bufNum, int pos);
};

#endif