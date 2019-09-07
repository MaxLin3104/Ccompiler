#pragma once
#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>

using namespace std;

enum CATEGORY { FUNCTION, VARIABLE, CONST, TEMP, RETURN_VAR }; //���ű��з��ŵ����ࣺ��������������������ʱ���������ر���
enum VARIABLE_TYPE { INT, VOID };//

enum LEXICAL_TYPE {
	LCINT, LKEYWORD, LIDENTIFIER, LTYPE, LBORDER, LUNKNOWN, LEOF, LOPERATOR, LBEGIN,
	LCFLOAT, LCDOUBLE, LCCHAR, LCSTRING, LCERROR, LOERROR
};//C��ͷ�Ĵ���������,LCERROR��ʾ��������,LOERROR��ʾ���������LWERROR��ʾ���ʴ���,LNO��ʾ��


struct WordInfo//�ۺ�����
{
	LEXICAL_TYPE type;//������
	string value;//������ֵ�����߹ؼ��ֱ���
	string word_string;//�����������ʱ�ķ���

	//SymbolLocation location;// ���������ʱ���������ű�����Ӧ���������
	//vector<string> modifiers;//���η���������Ҫ������η����﷨���ã�����Type-(+)->const signed int ***,('+'��ʾ�ಽ�Ƶ�)������ʽ�Ҳ���ι�Լ��Type��Type��valueӦ����const��signed��int��*��*��*�ȶ��ֵ

	//���ں������õĲ���ʽ�õ�����ʾ������ȫ�ֱ��е����
	int functionIndex;
};


struct SymbolPos {
	int table_pos;//��������������ʱ���� ����table������table�е����
	int symbol_pos;//��������������ʱ���� ����table�е�λ��
	SymbolPos(int tp, int sp) { table_pos = tp; symbol_pos = sp; }
	SymbolPos() {};
};
struct GrammarSymbolInfo//�������ʱ���õ����ķ���������
{
	string symbol_name;//��Ӧ��grammar�е�����
	string txt_value;//��Ӧ���ı�����ֵ
	SymbolPos pos;//table,symbol��λ��
	string op;//��ʾ +, -, *, /
};

struct Grammar//�Ҳ��������
{
	string left;
	vector<string> right;
};

struct Production//����ʽ
{
	string left;
	vector<string> right;
};

struct Quadruples//��Ԫʽ
{
	int num;//�к�
	string op;
	string arg1;
	string arg2;
	string result;

	Quadruples();
	Quadruples(int n, const string &o, const string &a1, const string &a2, const string &r);
	void SetContent(int n, const string &o, const string &a1, const string &a2, const string &r);
};

struct Instructions//Ŀ�����ָ��
{
	string op;
	string arg1;
	string arg2;
	string arg3;
	Instructions();
	Instructions(const string &op, const string & arg1, const string & arg2, const string & arg3);
	void SetContent(const string &op, const string & arg1, const string & arg2, const string & arg3);
};

//struct SymbolOffset
//{
//	int table_pos;
//	int offset;
//	CATEGORY mode;
//	SymbolOffset();
//	SymbolOffset(int tableIndex, int offset, CATEGORY mode);
//};

struct RegisterInfo//��ʱ�Ĵ�������Ϣ ����Ĵ���ֵ���ڴ�֮�䴫��
{
	const string name;//"$t0"֮���
	string content;//�������ֵ
	SymbolPos content_info;//content����Ϣ,ͨ��Pos����λsymbol��λ�á��Ӷ���ȡsymbol����Ϣ
	int miss_time = 0;//δʹ�ô�����û�п��мĴ���ʱ���ڳ����δ��ʹ�õļĴ���
	bool is_possessed = false;//��ʾ�Ƿ�ռ�á� �ڴӺ���return֮����Ҫ�ͷžֲ�����ı�����ռ�õļĴ���
};

#endif