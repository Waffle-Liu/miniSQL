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
#define ERR_NOKEYWORD			11  // sql: ����һ��������Ч����
// �﷨����
#define ERR_CREATE				12  // create: create��û��table��index
#define ERR_DROP				13  // drop: drop��û��table��index
#define ERR_TNAME				14  // create table: �����ǹؼ���
#define ERR_ILLEGLENAME			15  // create table: ���������߱������зǷ��ַ�
#define ERR_OVERNAMESIZE		16  // create table: ���������߱�������������16���ַ���
#define ERR_LP					17  // ȱ�������Ż��������Ÿ����д���
#define ERR_RP					18  // ȱ�������Ż��������Ÿ����д���
#define ERR_TOOMANYATTR			19  // create table: ���Ե���Ŀ����
#define ERR_SIZE				20 // create table: char��n����n��������
#define ERR_ATTR				21 // create table: �������ǹؼ���
#define ERR_NOATTR				22 // create table: ����Ϊprimary key�����Բ�����
#define ERR_PRIMARYKEY			23 // create table: primary key���帽������
#define ERR_TYPE				24 // create table: �Ƿ���������ֻ֧��int��char��float��
#define ERR_OVERSIZE			25 // create table: ����char����󳤶�
#define ERR_CREATEINDEX			26 // creat index�﷨����
#define ERR_DROPTABLE			27 // drop table�﷨���� 
#define ERR_DropIndex			28 // drop index�﷨����
#define ERR_INSERT				29 // insert�﷨����
#define ERR_DELETE				30 // delete�﷨����
#define ERR_DELETECONDITION		31 // delete��where�Ӿ��﷨����
#define ERR_SELECTTABLE			32 // select�﷨����
#define ERR_SELECTCONDITION		33 // select��where�Ӿ��﷨����
#define ERR_END					34 // �Ƿ��������������Ƿֺţ�
#define ERR_NOSENTENCE			35 // û�п��Թ����������
#define ERR_QUOTE				36 // �����ַ���ȱ�ٵ�����
#define ERR_CONSTSTRING			37 // �����ַ������зǷ��ַ���'\n','\r'��
#define ERR_VALUEILLEGAL		38 // ��ֵ���ݣ�����int��float���зǷ��ַ�
// �߼�����
#define ERR_EXISTTABLE			39 // create table: ����ı��Ѿ�����
#define ERR_RECREATTR			40 // create table: �����ظ�
#define ERR_REPETEKEY			41 // create table: primary key�ظ�
#define ERR_EXSITINDEX			42 // create index: �����Ѵ���
#define ERR_NOEXISTTABLE		43 // ������
#define ERR_NOEXISTARRT			44 // ���Բ�����
#define ERR_NOEXISTINDEX		45 // delete index: ����������
#define ERR_NOEXISTPRIKEY		46 // create table: primary key������
#define ERR_INSERTVALUENUM		47 // insert: �����ֵ���������Ը�������Ӧ
#define ERR_INSERTNOTUNIQ		48 // insert: ֵ��Ψһ

// ��С
#define MAX_ATTRNUM		32  // ���Ը���
#define MAX_STATNUM		200 // ������
#define MAX_SET		100 // 
#define VALLEN		300 // 
#define NAMELEN		100 //

class Attribute {
public:
	string name;		// ����
	int type;			// ����
	int length;			// ����
	bool IsPrimeryKey;	// �Ƿ�Ϊ����
	bool IsUnique;		// �Ƿ�Ψһ
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
	string name;   // ����
	int BlockNum;	// block�������������
//	int recordNum;	// record����
	int AttriNum;	// attribute����
	int TotalLength;	//total length of one record, should be equal to sum(attributes[i].length)
	vector<Attribute> Attributes; // ����
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

class Data//�����Ͱ�Data�����һ����ά����
{
public:
	vector<Row> Rows;
};

enum Comparison { Lt, Le, Gt, Ge, Eq, Ne };//stants for less than, less equal, greater than, greater equal, equal, not equal respectivly
class Condition {
public:
	Comparison op; // comparison
	int columnNum; // ��columnNum��
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