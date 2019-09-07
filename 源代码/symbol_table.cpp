#include "symbol_table.h"

SymbolTable::SymbolTable(TABLE_TYPE t_type, string iname)
{
	table_type_ = t_type;
	name_ = iname;
}

SymbolTable::~SymbolTable()
{
}

vector<Symbol> &SymbolTable::GetTable()
{
	return table_;
}

TABLE_TYPE SymbolTable::GetTableType()
{
	return table_type_;
}

Symbol &SymbolTable::GetSymbol(int pos)
{
	return table_[pos];
}

int SymbolTable::AddSymbol(const Symbol & sb)
{
	//˫���գ��������ٽ���һ���жϣ��ټ�����ű�֮ǰ�鿴�Ƿ��Ѿ����ڡ�
	//������ھͷ���-1����ʾ����ʧ��
	if (-1 != FindSymbol(sb.name))
		return -1;

	//ԭ�Ȳ����ڵ���������ʱ������ű�����ӷ���
	table_.push_back(sb);
	return table_.size() - 1;
}

int SymbolTable::AddSymbol(const string &str)
{
	string tmp_name = "T" + std::to_string(table_.size());
	Symbol sb;
	sb.name = tmp_name;
	sb.mode = TEMP;
	sb.value = str;
	table_.push_back(sb);
	return table_.size() - 1;
}

int SymbolTable::AddSymbol()
{
	string tmp_name = "T" + std::to_string(table_.size());
	Symbol sb;
	sb.name = tmp_name;
	sb.mode = TEMP;
	table_.push_back(sb);
	return table_.size() - 1;
}

string SymbolTable::GetTableName()const
{
	return name_;
}

string SymbolTable::GetSymbolName(int pos) const
{
	return table_[pos].name;
}

int SymbolTable::FindSymbol(const string &name) const
{
	for (auto iter = table_.begin(); iter != table_.end(); ++iter)
	{
		if (name == iter->name)//���ŵ�������ͬ����ʾ�ҵ�
			return iter - table_.begin();
	}

	return -1;//û���ҵ�
}

int SymbolTable::FindConst(const string &const_value) const
{
	for (auto iter = table_.begin(); iter != table_.end(); ++iter)
	{
		if (iter->mode != CONST)
			continue;
		if (const_value == iter->value)//����������ֵ���
			return iter - table_.begin();
	}

	return -1;//û���ҵ�
}

bool SymbolTable::SetValue(int pos, string &value)
{
	table_[pos].value = value;
	return true;
}
CATEGORY SymbolTable::GetSymbolMode(int pos)const
{
	return table_[pos].mode;
}
VARIABLE_TYPE SymbolTable::GetSymbolType(int pos)const
{
	return table_[pos].type;
}