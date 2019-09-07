#include <iostream>
#include <string>
#include "lexical_analyzer.h"
#include "syntactic_analyzer.h"

#include "utils.h"

using namespace std;

int main()
{

	cout << "���ֶ�����Ŀ¼ gen_data/lexcial_analyzer" << endl;
	cout << "���ֶ�����Ŀ¼ gen_data/syntactic_analyzer" << endl;
	cout << "���ֶ�����Ŀ¼ gen_data/target_file" << endl << endl;
	cout << "�ʷ������Ľ�������lexcial_analyzer��" << endl;
	cout << "�﷨�����Ľ�������syntactic_analyzer��" << endl;
	cout << "�м������Ŀ�����Ľ�������target_file��" << endl << endl;
	string file_name = "./raw_data/";
	string str;
	cout << "��ָ����gen_data/Ŀ¼����Ҫ�����cppԴ�ļ���" << endl;
	cin >> str;
	file_name  = file_name + str;
	SyntacticAnalyzer sa;
	sa.StartAnalize(file_name);


	return 0;
}