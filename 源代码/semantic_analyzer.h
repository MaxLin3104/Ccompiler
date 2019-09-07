#pragma once
#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include <vector>
#include "utils.h"
#include "symbol_table.h"
#include <fstream>
#include <iostream>
#include "target_code_generator.h"


using namespace std;

class SemanticAnalyzer
{
private:
	vector<SymbolTable> symbol_tables_;//���ű�������ÿһ�����ű��Ӧһ������
	vector<int> current_symbol_table_stack_;//��ǰ�����������������ز�εķ��ű�����ջ
	bool CreateSymbolTable(TABLE_TYPE, const string &);//����һ���µķ��ű����޸ĵ�ǰ���ű�

	int next_state_num_;//��һ����Ԫʽ����š������1��ʼ��0������ת��������ʼ��ַ����Ԫʽ
	ofstream intermediate_code_temp_printer_;//�м������ʱ�ļ���ӡ��
	fstream intermediate_code_temp_reader_;//�м������ʱ�ļ���ȡ
	ofstream intermediate_code_printer_;//�м�����ӡ��

	int backpatching_level_;//����Ĳ�Σ���0��ʼ�� �������ڻ����Ƕ��
	vector<Quadruples> quadruples_stack_;//��Ż�����Ԫʽ��������Ϻ�ͳһ�����
	vector<int> backpatching_value;//�����ֵ
	vector<int> backpatching_point_pos_;//������ڻ���ջ�еĵ�ַ
	int main_line_;//main���������
	bool PrintQuadruples(const Quadruples &);//������Ԫʽ���д�ӡ�м������ʱ�ļ�
	bool PrintQuadruples();//������ʱ�ļ���ӡ�м����

	vector<string> w_label_;//������¼while��if���Label�� �����Ǽ�¼д����Label
	vector<string> j_label_;//��¼Ҫ��ת��Label
	vector<int> w_j_label_nums;//��¼ÿ���while,if��Ŀ
	int w_j_label_num_;//��ʼΪ0��ÿ����һ����������0����ȥ��

	TargetCodeGenerator target_code_generator_;//Ŀ�����������

	//����Ŀ���������ת�õı�ǩ�ţ���ֹ�ظ�
	int while_cnt_;
	int if_cnt_;

	
public:
	SemanticAnalyzer();
	~SemanticAnalyzer();

	bool ExecuteSemanticCheck(vector<GrammarSymbolInfo>&, const Production &);//ִ������������,�����ݲ���ʽ�޸��ķ���������ջ
	int GetNextStateNum();//������һ����Ԫʽ����ţ�������ż�1
	int PeekNextStateNum();//�鿴��һ����Ԫʽ�����
	
	string GetArgName(SymbolPos &, bool is_return=false);//���м�������ɵ�ʱ�򣬻�ȡ���������ơ� ���ڲ�����ʱ�����ı�������Ҫ���ϱ����ţ����磺 a_2
	
};


#endif // !SEMANTIC_ANALYZER_H

