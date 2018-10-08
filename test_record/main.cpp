#include "Buffer.h"
#include "Catalog.h"
#include "IndexManager.h"
#include "Interpreter.h"
#include "MiniSQL.h"
#include "Record.h"
#include "API.h"

int main() {
	string command = "";
	string inputword = "";
	bool ComEnd = false;

	cout << "Welcome to the MiniSQL monitor." << endl;
	while (1) {
		command = "";
		ComEnd = 0;
		cout << "MiniSQL> ";

		while (!ComEnd) {
			getline(cin, inputword);
			ComEnd = IsComEnd(inputword);
			if (!ComEnd) cout << "      -> ";
			command += inputword + " ";
		}
		command += '\0';
		parsetree.Parse(command);
		if(!Execute()) break;
	}

/*	//批量测试
	string infile = "in.txt";
	fstream fin;
	fin.open(infile, ios::in);

	while (fin)
	{
		command = "";
		ComEnd = 0;
		while (!ComEnd) {
			getline(fin, inputword, '\n');
			ComEnd = IsComEnd(inputword);
			command += inputword + " ";
		}
		command += '\0';
		parsetree.Parse(command);
		if (!Execute()) break;
	}*/

	system("pause");
	return 0;
}


