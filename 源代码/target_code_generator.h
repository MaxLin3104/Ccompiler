#pragma once
#ifndef TARGET_CODE_GENERATOR
#define TARGET_CODE_GENERATOR
#include "utils.h"
#include "symbol_table.h"
#include <iostream>
#include <string>
#include <fstream>
using namespace std;

#define GF_INI_OFFSET 1024
class TargetCodeGenerator {
private:
	ofstream target_code_printer_;//Ŀ������ӡ��
	vector<SymbolTable> *symbol_tables_pt_;//���ű�����ָ�룬ÿһ�����ű��Ӧһ������   ----���������������ͬһ���� ʹ�õ�ʱ���ָ��
	
	vector<RegisterInfo> registers_ = { { "$t0","" },{ "$t1","" },{ "$t2","" },{ "$t3","" },{ "$t4","" },
										{ "$t5","" },{ "$t6","" },{ "$t7","" },{ "$t8","" },{ "$t9","" } };//��ʱ�Ĵ����ĳ�ʼ��

	int gf_offset;//gf��ƫ��, ���1����λ�����ڴ�4����λ��ǰ1024����Ԫ��1024*4������ȫ�ֱ���
	void SetMissTime(int reg);//����reg���0�����඼+1
public:
	TargetCodeGenerator();
	~TargetCodeGenerator();
	void TargetCodeGeneratorInit(vector<SymbolTable> &);
	bool PrintQuadruples(const Instructions &);//����ָ����д�ӡĿ�������ʱ�ļ�
	bool PrintQuadruples(const string&);//���ɶ�Ӧ��string����תlabel
	bool PushStack_sp(int num);//sp+4*num��ָ���������������������ʱ��ִ��
	bool CreateStackFrame();//�����������ú󣬽��뱻���ú���ʱ�����Ƚ���ջ֡�Ĺ����� ��Ŀ�����ص�ַ����fp
	bool InitialSpFpGp();//��ʼ��sp,fp,gp��ֵ
	int GetTempReg();//ʹ��LRU�㷨��ȡ������Ӧ�ñ�ʹ�õļĴ��������ؼĴ�����vector�е����
	bool LoadTempReg(int reg, SymbolPos &pos);//�ѷ��ű���ָ���ķ���д��ָ��reg
	bool IsSymbolLoaded(SymbolPos &)const;//�жϷ����Ƿ��Ѿ������ص��Ĵ�����
	string GetMemAddr(const SymbolPos &pos)const;//����Pos�����ڴ��ַ 4($gp), -4($fp) �ȵ�
	bool PrintSyscall();//main�Ľ���
	int LoadImmToReg(const string&, const SymbolPos &pos);//�����������ص���ʱ�Ĵ����У������ؼĴ��������
	string SetArgBeReady(SymbolPos pos);//�ò����������������ʱ�Ĵ��������ؼĴ�������
	bool ClearRegs();//���Ĵ����ϵľֲ�����ȫ��д�أ�������Ĵ���
	bool ResetRegs();//��10���Ĵ������ó�no_possessed
};


#endif
