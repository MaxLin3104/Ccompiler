#pragma once
#ifndef SYNTACTIC_ANALYZER
#define SYNTACTIC_ANALYZER

#include "lexical_analyzer.h"
#include "semantic_analyzer.h"
#include <map>


enum SLR_OPERATIONS { CONCLUDE, MOVE, ACCEPT, NONTERMINAL, ERROR };//��Լ���ƽ������ܡ�ѹ����ս��������
struct SlrOperation//ACTION����GOTO����
{
	SLR_OPERATIONS op;//��Լ���ƽ�������
	int state;//item�е���ţ���0��ʼ
	bool operator==(const SlrOperation &operation) const;//�ж����������Ƿ���ͬ
};

//LR��Ŀ
struct LrItem {
	int production_number;//��Ŀ����Ӧ�Ĳ���ʽ�� productions_ �����
	int point_pos;//��Ŀ��СԲ���λ�ã���0��ʼ�����ֵ���ڲ���ʽ�Ҳ�������Ŀ
	LrItem(int item_num, int point_num) { this->production_number = item_num, this->point_pos = point_num; };
	bool operator==(const LrItem &item) const;//�ж�����item�Ƿ���ͬ
	bool operator<(const LrItem &item) const;//û������<������ʱ���޷�����
};



class SyntacticAnalyzer {
private:
	bool print_detail_;
	vector<Grammar> grammars_;
	vector<Production> productions_;//��gammars_�еĲ���ʽ�𿪳ɶ������ʽ�������š�
	set<string> grammar_symbol_;//�﷨�����з��ţ������ս������ս����
	map<string, set<string>> first_map_, follow_map_;//First���ϡ�Follow����
	vector<set<LrItem>> normal_family_;//LR0��Ŀ���淶��
	set<LrItem> lr_items_;//LR0������Ŀ
	map<pair<int, string>, SlrOperation> action_goto_tables_;//action �� goto ����ں�
	ofstream syntactic_analyzer_printer_;//�﷨�������̴�ӡ
	vector<string> move_conclude_string_stack_;//��� �ƽ���Լ����ջ
	vector<int> state_sequence_stack_;//״̬����ջ
	SemanticAnalyzer sementic_analyzer_;//������������ڹ�Լ��ʱ��ʹ�ã�����������
	vector<GrammarSymbolInfo> grammar_symbol_info_stack_;//���������ʱ���õ����ķ���������ջ

	bool IsNonTerminalSymbol(const string &symbol);//�ж��Ƿ�Ϊ���ս��

	bool GenProduction();//��grammar.txt�����ɲ���ʽ
	void GenAugmentedGrammar();//�����ع��ķ�
	void GenFirstSet();//����First����
	void GenFollowSet();//����Follow����
	void GenGrammarSymbolSet();//�����ķ����ż���
	bool GenNormalFamilySet();//������Ŀ���淶�塢Action��Goto������false��˵���ķ�����SLR�����г�ͻ
	void GenLrItems();//����LR0������Ŀ
	bool BuildGrammar();//��grammar.txt�ļ����з����������������м��ϡ�
	set<LrItem> GenItemClosureSet(const LrItem &);//����ĳ����Ŀ�ıհ�����
	set<LrItem> GenItemsClosureSet(const set<LrItem> &);//����ĳ����Ŀ���ıհ�����
	set<string> GetProductionFirstSet(const vector<string> &);//��ȡĳ������First����


	void PrintBuildGrammarDetails();//��grammar.txt�ļ����з����������������м��ϡ�
	void PrintProdcutions(const string filename = "./gen_data/syntactic_analyzer/productions.txt");//��ӡ����ʽ
	void PrintGrammars();//��ӡ�ķ�
	void PrintFirst(const string filename = "./gen_data/syntactic_analyzer/fisrts.txt");//��ӡFirst����
	void PrintFollow(const string filename = "./gen_data/syntactic_analyzer/follows.txt");//��ӡFollow����
	void PrintGrammarSymbolSet(const string filename = "./gen_data/syntactic_analyzer/symbols.txt");//��ӡGrammarSymbol����
	void PrintLrItems(const string filename = "./gen_data/syntactic_analyzer/items.txt");//��ӡLrItems����
	void PrintClosure();//��ӡClosure���ϣ�debugʱ��ʹ��
	void PrintSlrError(const set<LrItem> &);//��ӡSLR�����д��ڳ�ͻ�Ĺ淶�壬���ִ����ʱ��ֱ����ʾ��CMD������
	void PrintNormalFamiliySet(const string filename = "./gen_data/syntactic_analyzer/normal_families.txt");//��ӡ���й淶��
	void PrintActionGotoTable(const string filename = "./gen_data/syntactic_analyzer/action_goto_tables.csv");//��ӡAction��Goto��

	void PrintAnalysisProcess(int step, const SlrOperation &sl_op);//��ӡ�﷨��������

public:
	SyntacticAnalyzer(bool show_detail = true);
	~SyntacticAnalyzer();
	bool StartAnalize(const string code_filename = "./raw_data/test.cpp");//��ʼ�����﷨����
	

};


#endif