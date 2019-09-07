#pragma once
#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include "utils.h"
#include <vector>

//���ű��еķ���
struct Symbol
{
	CATEGORY mode;//MODE { FUNCTION, VARIABLE, CONST, TEMP }; //���ű��з��ŵ����ͣ���������������������ʱ����
	string name;
	VARIABLE_TYPE type;//����Ǳ��������Ǳ��������ͣ�����Ǻ��������Ǻ����ķ�������
	string value;//�洢����ֵ


	int parameter_num = 0;//����Ǻ������˱�����ʾ��������
	int entry_address;//������ڵ�ַ
	int func_symbol_pos;//�������ڷ��ű��е�λ��,������������ǵڼ��ű�
	int function_table_num;//������ʽ���� �ں������ű��е����

	int offset = -1;//TEMP:��$gpΪ��ʼ��ƫ�Ƶ�ַ �����1��ʾ4��bytes;  -1��ʾ��Ч  VARIABLE:��ջ֡�е�λ��
	int reg = -1;//MIPS��10���Ĵ��� t0-t9�� ��Ч 0-9  ʹ��-1��ʾ��
	//offset��reg����ʹ��
	//o-1, r-1		��ʾ��ûװ�ص�reg��Ҳû�б��浽�ڴ�
	//o-1, r>-1		��ʾ�Ĵ����ϵ�ֵ����Ч��
	//o>-1, r-1		��ʾreg�е�ֵ����Ч�ģ������ڴ����б���
};



enum TABLE_TYPE { CONST_TABLE, TEMP_TABLE, GLOBAL_TABLE, FUNCTION_TABLE, WHILE_TABLE, IF_TABLE };//��������ʱ������ȫ�֡�������ѭ���塢ѡ���֧

class SymbolTable
{
private:
	vector<Symbol> table_;//��vector����Ÿ������ţ��γ�һ�ŷ��ű�
	TABLE_TYPE table_type_;//���ű������
	string name_;//���ű������


public:
	SymbolTable(TABLE_TYPE, string = "");
	~SymbolTable();
	TABLE_TYPE GetTableType();//�����������ű������
	int AddSymbol(const Symbol &);//����ű�����ӷ��Ų����ط����ڷ��ű��е���š������ڷ��ţ��򷵻�-1
	int AddSymbol(const string &);//����ʱ���ű�����ӷ��Ų����ط����ڷ��ű��е����
	int AddSymbol();//����ʱ���ű�����ӷ��Ų����ط����ڷ��ű��е����

	int FindSymbol(const string &) const;//�ڷ��ű���Ѱ���Ƿ����ָ�����Ƶķ��ţ�û���򷵻�-1�����򷵻����
	int FindConst(const string &) const;//�ڷ��ű���Ѱ���Ƿ����ָ���ĳ�����û���򷵻�-1�����򷵻����
	bool SetValue(int pos, string &);//����pos������value
	string GetSymbolName(int pos) const;//����pos��ȡ��������
	CATEGORY GetSymbolMode(int pos)const;//����pos��ȡ����ģʽ
	VARIABLE_TYPE GetSymbolType(int pos)const;//����pos��ȡ��������
	string GetTableName()const;//��ȡ��ǰ���ű������
	Symbol &GetSymbol(int pos);//����pos��ȡ����
	vector<Symbol> &GetTable();//�����������ű�

};

#endif