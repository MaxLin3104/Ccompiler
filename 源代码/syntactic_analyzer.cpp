#include "syntactic_analyzer.h"

vector<string> split_string(const string &str, const string &pattern)
{
	vector<string> res;
	if (str == "")
		return res;
	//���ַ���ĩβҲ����ָ����������ȡ���һ��
	string strs = str + pattern;
	size_t pos = strs.find(pattern);

	while (pos != strs.npos)
	{
		string temp = strs.substr(0, pos);
		res.push_back(temp);
		//ȥ���ѷָ���ַ���,��ʣ�µ��ַ����н��зָ�
		strs = strs.substr(pos + 1, strs.size());
		pos = strs.find(pattern);
	}

	//debug: grammar��sp���
	//for (auto iter = res.begin(); iter != res.end(); ++iter)
	//	if ((*iter) == "")
	//		cout << "***************************";
	return res;
}

set<string> SyntacticAnalyzer::GetProductionFirstSet(const vector<string> & symbol_string)
{
	set<string> first;
	for (auto iter1 = symbol_string.begin(); iter1 != symbol_string.end(); ++iter1)
	{
		//������Ϊ$,����ǰ���ж�
		//if ((*iter1) == "$")
		//	break;

		if (!IsNonTerminalSymbol(*iter1))//ѭ�����ս��
		{
			first.insert(*iter1);
			break;//�ս����Ҫ����ѭ��
		}

		for (auto iter2 = first_map_[*iter1].begin(); iter2 != first_map_[*iter1].end(); ++iter2)//��������ʽ�Ҳ����Ŷ�Ӧ��First����
		{
			if ((*iter2) != "$" && first.find(*iter2) == first.end())//�ǿ�Ҳ���ظ�
				first.insert(*iter2);
		}


		//��ǰ����first������û�п�$���˳���������ʽ��ѭ����
		if (first_map_[*iter1].find("$") == first_map_[*iter1].end())
			break;

		//ÿһ�����ŵ�first�������գ����Ѿ�ѭ�������һ�����ţ����ʱ����������first����ҲӦ�ð�����
		if (iter1 + 1 == symbol_string.end() && first.find("$") == first.end())
			first.insert("$");
	}
	return first;
}

bool SyntacticAnalyzer::GenProduction()
{
	string grammar_filename = "./raw_data/grammar.txt";
	ifstream grammar_reader;

	grammar_reader.open(grammar_filename);

	if (!grammar_reader.is_open()) {
		cerr << "�﷨���������У�grammar�ļ���ʧ�ܣ�" << endl;
		return false;
	}

	string grammar_str;
	while (!grammar_reader.eof())
	{
		getline(grammar_reader, grammar_str);
		if (grammar_str.length() == 0)//�մ�
			continue;

		string left;
		vector<string> right;

		size_t left_pos = grammar_str.find_first_of("->");
		if (left_pos < 0)
		{
			cerr << "grammar���ִ���ĳ������ʽû��\" -> \" ���ţ���淶��" << endl;
			grammar_reader.close();
			return false;
		}
		left = grammar_str.substr(0, left_pos);

		//����"->"
		grammar_str = grammar_str.substr(left_pos + 2);


		while (true) //�������ʽ�Ҳ�
		{
			size_t right_pos = grammar_str.find_first_of("|");
			if (right_pos == string::npos)//���һ����ѡʽ
			{
				right.push_back(grammar_str);
				//productions_.push_back({ left, split_string(grammar_str, " ") });//������ʽ�ָ�


				vector<string> split_production = split_string(grammar_str, " ");//������ʽ�ָ�
				productions_.push_back({ left,  split_production });
				//����ս����grammar_symbol_����
				for (auto iter = split_production.begin(); iter != split_production.end(); ++iter)
					if (!IsNonTerminalSymbol(*iter) && (*iter)!= "$")
						grammar_symbol_.insert(*iter);
				
				break;
			}
			//�������һ����ѡʽ
			//string debug_s = grammar_str.substr(left_pos, right_pos - left_pos);
			right.push_back(grammar_str.substr(0, right_pos));

			vector<string> split_production = split_string(grammar_str.substr(0, right_pos), " ");//������ʽ�ָ�
			productions_.push_back({ left,  split_production });
			//����ս����grammar_symbol_����
			for (auto iter = split_production.begin(); iter != split_production.end(); ++iter)
				if (!IsNonTerminalSymbol(*iter) && (*iter) != "$")
					grammar_symbol_.insert(*iter);

			grammar_str = grammar_str.substr(right_pos + 1);
		}

		grammars_.push_back({ left, right });
	}
	
	grammar_reader.close();
	return true;

}

void SyntacticAnalyzer::GenAugmentedGrammar()
{
	//vector<Production> productions_
	/*string left = productions_[0].left + "'";*/
	string left = "Start";
	vector <string> right;
	right.push_back(productions_[0].left);
	productions_.insert(productions_.begin(), { left , right });
	return;
}

void SyntacticAnalyzer::GenFirstSet()
{
	for (auto iter = productions_.begin(); iter != productions_.end(); ++iter)//����ÿһ������ʽ
	{
		//vector<string> debug = iter->right;
		first_map_.insert({ iter->left, set<string>{} });//����FirstMap
		follow_map_.insert({ iter->left, set<string>{} });//����FollowMap
		if (iter->right[0] == "$")
		{
			if (first_map_[iter->left].find("$") == first_map_[iter->left].end())//��first��������� ��$
				first_map_[iter->left].insert("$");
		}
	}

	while (true)
	{
		bool is_change = false;
		for (auto iter1 = productions_.begin(); iter1 != productions_.end(); ++iter1)//����ÿһ������ʽ
		{
			for (auto iter2 = iter1->right.begin(); iter2 != iter1->right.end(); ++iter2)//��������ʽ�Ҳ�
			{
				if ((*iter2) == "$")
					break;

				if (!IsNonTerminalSymbol(*iter2))//ѭ�����ս��
				{
					if (first_map_[iter1->left].find(*iter2) == first_map_[iter1->left].end())
					{
						first_map_[iter1->left].insert(*iter2);
						is_change = true;
					}
					break;//�ս����Ҫ����ѭ��

				}
				//map ʹ��[]֮�����ԭ��û�ж�Ӧ��key�����Զ�������
				//string gebug_str = *iter2;
				//auto debug_iter1 = first_map_[*iter2].begin();
				//auto debug_iter2 = first_map_[*iter2].end();


				for (auto iter3 = first_map_[*iter2].begin(); iter3 != first_map_[*iter2].end(); ++iter3)//��������ʽ�Ҳ����Ŷ�Ӧ��First����
				{
					if ((*iter3) != "$" && first_map_[iter1->left].find(*iter3) == first_map_[iter1->left].end())//�ǿ�Ҳ���ظ�
					{
						first_map_[iter1->left].insert(*iter3);
						is_change = true;
					}
				}

				//��ǰ����first������û�п�$���˳���������ʽ��ѭ����
				if (first_map_[*iter2].find("$") == first_map_[*iter2].end())
					break;
				
				//ÿһ�����ŵ�first�������գ����Ѿ�ѭ�������һ�����ţ����ʱ�����ʽ�󲿵�firstҲ������
				if (iter2 + 1 == iter1->right.end() && first_map_[iter1->left].find("$") == first_map_[iter1->left].end())
				{
					first_map_[iter1->left].insert("$");
					is_change = true;
				}

			}
		}


		if (!is_change)//δ�����ı䣬����First���
			break;
	}
	return;
}

void SyntacticAnalyzer::GenFollowSet()
{
	//GetProductionFirstSet()
	follow_map_[productions_[0].left].insert("#");//�ķ���ʼ���ţ����#

	while (true)
	{
		bool is_change = false;
		for (auto iter1 = productions_.begin(); iter1 != productions_.end(); ++iter1)//����ÿ������ʽ
		{
			for (auto iter2 = iter1->right.begin(); iter2 != iter1->right.end(); ++iter2)//��������ʽ�Ҳ�ÿ������
			{
				if (!IsNonTerminalSymbol(*iter2))//��ǰ����Ϊ�ս��������
					continue;

				//���ս�������
				//======================================ע��===���=====
				set<string> first = GetProductionFirstSet(vector<string>(iter2 + 1, iter1->right.end()));
				for (auto iter = first.begin(); iter != first.end(); ++iter)
				{
					if ((*iter) != "$" && follow_map_[*iter2].find(*iter) == follow_map_[*iter2].end())//�ǿ��һ�û�м���
					{
						follow_map_[*iter2].insert(*iter);
						is_change = true;
					}				
				}
				if (first.empty() || first.find("$") != first.end())//�Ѳ���ʽ�󲿵�follow���ϼӵ���ǰ���ս����follow������
				{
					for (auto iter = follow_map_[iter1->left].begin(); iter != follow_map_[iter1->left].end(); ++iter)
					{
						if(follow_map_[*iter2].find(*iter) == follow_map_[*iter2].end())//��û�м����
						{
							follow_map_[*iter2].insert(*iter);
							is_change = true;
						}
					}
				}
				
			}
			
		}

		if (!is_change)//δ�����ı䣬����Follow���
			break;
	}


	return;
}

void SyntacticAnalyzer::GenGrammarSymbolSet()
{
	//�ѷ��ս��������ķ����ż�����
	for (auto iter = first_map_.begin(); iter != first_map_.end(); ++iter)
		grammar_symbol_.insert(iter->first);
	
	grammar_symbol_.insert("#");//�ս��#һ����ӽ��ķ����ż�����
	return;
}

void SyntacticAnalyzer::GenLrItems()
{
	for (auto iter = productions_.begin(); iter != productions_.end(); ++iter)
	{
		if (iter->right[0] == "$")//����ʽ�Ҳ��ǿմ���LR0��Ŀ ֻ��һ�� pos��-1�������ʾ
		{
			lr_items_.insert(LrItem{ iter - productions_.begin(), -1 });//�������ʽ����� -1����ʾ�գ�A->epsilon ֻ����A->��
			continue;
		}
		int production_lenth = iter->right.size();
		for (int count = 0; count <= production_lenth; ++count)
		{
			//auto debug = iter - productions_.begin();
			//auto debug2 = LrItem{ iter - productions_.begin(), count };
			//debug2 ������Ҫ��� LrItem��<����������������
			lr_items_.insert(LrItem{ iter - productions_.begin(), count });//�������ʽ�����СԲ��λ��	
			//PrintLrItems();
		}
	}
	return;
}

set<LrItem> SyntacticAnalyzer::GenItemClosureSet(const LrItem &input_item)
{
	vector<LrItem> item_stack;//�����ݴ�item
	item_stack.push_back(input_item);
	set<LrItem> item_set;//���շ��ص�Item

	while (!item_stack.empty())//�ݴ��ջ���գ�����Ҫһֱѭ��
	{
		//��ջ��ȡ��Item
		LrItem item = item_stack[item_stack.size() - 1];
		item_stack.pop_back();
		//���뵽�հ���
		item_set.insert(item);

		//A->��
		if (-1 == item.point_pos)
			continue;

		//СԲ��������棬��ʾ��Լ�� �������жϣ���ֹ�����vector��Խ�����
		if (item.point_pos == productions_[item.production_number].right.size())
			continue;
		//�ս��
		if (!IsNonTerminalSymbol(productions_[item.production_number].right[item.point_pos]))
			continue;

		string current_symbol = productions_[item.production_number].right[item.point_pos];
		//���ս��
		for (auto iter = lr_items_.begin(); iter != lr_items_.end(); ++iter)
		{
			//�Ե�ǰ���ս��Ϊ�󲿣�����СԲ���λ���ڵ�0λ ���� -1λ��A->����
			if (productions_[iter->production_number].left == current_symbol && (iter->point_pos == 0 || iter->point_pos == -1))
			{
				if(item_set.find(*iter) == item_set.end())//��֤֮ǰ����set�У������������ѭ��
					item_stack.push_back(*iter);
			}
		}
	}

	return item_set;
}

set<LrItem> SyntacticAnalyzer::GenItemsClosureSet(const set<LrItem> &items)
{
	vector<LrItem> item_stack;//�����ݴ�item
	for(auto iter = items.begin();iter!=items.end();++iter)
		item_stack.push_back(*iter);
	//item_stack.push_back(input_item);
	set<LrItem> item_set;//���շ��ص�Item

	while (!item_stack.empty())//�ݴ��ջ���գ�����Ҫһֱѭ��
	{
		//��ջ��ȡ��Item
		LrItem item = item_stack[item_stack.size() - 1];
		item_stack.pop_back();
		//���뵽�հ���
		item_set.insert(item);

		//A->��
		if (-1 == item.point_pos)
			continue;

		//СԲ��������棬��ʾ��Լ�� �������жϣ���ֹ�����vector��Խ�����
		if (item.point_pos == productions_[item.production_number].right.size())
			continue;
		//�ս��
		if (!IsNonTerminalSymbol(productions_[item.production_number].right[item.point_pos]))
			continue;

		string current_symbol = productions_[item.production_number].right[item.point_pos];
		//���ս��
		for (auto iter = lr_items_.begin(); iter != lr_items_.end(); ++iter)
		{
			//�Ե�ǰ���ս��Ϊ�󲿣�����СԲ���λ���ڵ�0λ ���� -1λ��A->����
			if (productions_[iter->production_number].left == current_symbol && (iter->point_pos == 0 || iter->point_pos == -1))
			{
				if (item_set.find(*iter) == item_set.end())//��֤֮ǰ����set�У������������ѭ��
					item_stack.push_back(*iter);
			}
		}
	}

	return item_set;
}

bool SyntacticAnalyzer::GenNormalFamilySet()
{
	//����ʼ�ַ���Closure��Ϊ��0���淶��
	//LrItem debug = *lr_items_.begin();

	vector<set<LrItem>> item_stack;//������ʱ���NormalFamiliySet
	item_stack.push_back(GenItemClosureSet(*lr_items_.begin()));
	normal_family_.push_back(item_stack[0]);

	while (!item_stack.empty())
	{
		//��ջ��ȡ��NormaliFamilySet
		set<LrItem> item = item_stack[item_stack.size() - 1];
		item_stack.pop_back();
		
		
		//�ҵ���ǰ�淶������
		int current_state = find(normal_family_.begin(), normal_family_.end(), item) - normal_family_.begin();

		for (auto iter1 = item.begin(); iter1 != item.end(); ++iter1)//���ڹ淶���е�ÿ��LR0��Ŀ
		{
			//СԲ���ڵ�-1λ�û�������棬��ʾ����Ҫ��Լ����Ŀ
			if (-1 == iter1->point_pos  || (productions_[iter1->production_number].right.size() == iter1->point_pos))
			{
				//����follow����
				string symbol = productions_[iter1->production_number].left;//����ʽ��
				//��follow���Ͻ��б���������action_goto���У�ͬʱ���д�����ж�
				for (auto iter2 = follow_map_[symbol].begin(); iter2 != follow_map_[symbol].end(); ++iter2)
				{
					//action_goto_table��û�У����ʱ����ӽ�ȥ
					if (action_goto_tables_.find({ current_state, *iter2 }) == action_goto_tables_.end())
					{
						//ͨ����ǰ�� ״̬+�����ַ����������󲿵�follow���ϣ�,��ӦΪ ��Լ��ʹ�õ�iter1->production_number����ʽ
						action_goto_tables_.insert({ { current_state, *iter2},  {CONCLUDE,iter1->production_number } });
					}
					else//��ʾ�Ѿ�����ӳ�����
					{
						//����ӳ�䵫��ȴ����ȣ�˵������SLR�ķ�
						if (!(action_goto_tables_[{ current_state, *iter2 }] == SlrOperation{ CONCLUDE,iter1->production_number }))
						{
							PrintSlrError(item);
							return false;
						}
					}
				}
			}
			else
			{
				//����ʽ�Ҳ��ĵ�ǰ����
				string current_right_symbol = productions_[iter1->production_number].right[iter1->point_pos];
				//��GO���������ã�����һ���淶��
				//set<LrItem> next_normal_family = GenItemClosureSet({ iter1->production_number, iter1->point_pos + 1 });
				//======================�޸�================================================================================
				set<LrItem> items;//�洢 ������ͬ�ַ���item
				for (auto iter2 = item.begin(); iter2 != item.end(); ++iter2)
				{
					//��ֹԽ��
					if (iter2->point_pos == -1 || (iter2->point_pos == productions_[iter2->production_number].right.size()))
						continue;
					//auto debug = productions_[iter1->production_number].right[iter1->point_pos];
					//auto debug2 = productions_[iter2->production_number].right[iter2->point_pos];
					if(productions_[iter1->production_number].right[iter1->point_pos] == 
						productions_[iter2->production_number].right[iter2->point_pos])
						items.insert({ iter2->production_number, iter2->point_pos + 1 });
					/*if (iter1->point_pos + 1 == iter2->point_pos + 1)*/
				}
				set<LrItem> next_normal_family = GenItemsClosureSet(items);


				//auto debug = find(normal_family_.begin(), normal_family_.end(), next_normal_family);
				//auto debug2 = normal_family_.begin();
				//auto debug3 = next_normal_family;
				//�жϵ�ǰ�淶���Ƿ��Ѿ�����
				if (find(normal_family_.begin(), normal_family_.end(), next_normal_family) == normal_family_.end())//������
				{
					normal_family_.push_back(next_normal_family);//����淶�弯����
					item_stack.push_back(next_normal_family);//����������ջ��
				}
				int next_state = find(normal_family_.begin(), normal_family_.end(), next_normal_family) - normal_family_.begin();

				//action_goto_table��û�У����ʱ����ӽ�ȥ
				if (action_goto_tables_.find({ current_state, current_right_symbol }) == action_goto_tables_.end())
				{
					//ͨ����ǰ�� ״̬+�����ַ����������󲿵�follow���ϣ�,��ӦΪ �ƽ�����ת����next_state��״̬
					action_goto_tables_.insert({ { current_state, current_right_symbol},  {MOVE, next_state } });
				}
				else//��ʾ�Ѿ�����ӳ�����
				{
					//����ӳ�䵫��ȴ����ȣ�˵������SLR�ķ�
					if (!(action_goto_tables_[{ current_state, current_right_symbol }] == SlrOperation{ MOVE, next_state }))
					{
						PrintSlrError(item);
						return false;
					}
				}


			}

			


		}
	}
	int current_state2 = -1;
	//����acc״̬
	for (auto iter1 = normal_family_.begin(); iter1 != normal_family_.end(); ++iter1)
	{
		for(auto iter2 = iter1->begin();iter2!=iter1->end();++iter2)
			if (LrItem(0, 1) == *iter2)
			{
				current_state2 = iter1 - normal_family_.begin();
				break;
			}
		if (current_state2 >= 0)
			break;
	}
	set<LrItem> item = { LrItem(0,1) };
	//auto debug = find(normal_family_.begin(), normal_family_.end(), item) == normal_family_.end();
	action_goto_tables_[{current_state2, "#"}] = { ACCEPT, current_state2 };

	//map<pair<int, string>, SlrOperation> action_goto_tables_;//action �� goto ����ں�
	//enum SLR_OPERATIONS { CONCLUDE, MOVE, ACCEPT, NONTERMINAL, ERROR };//��Լ���ƽ������ܡ�ѹ����ս��������
	//struct SlrOperation//ACTION����GOTO����
	//{
	//	SLR_OPERATIONS op;//��Լ���ƽ�������
	//	int state;//item�е���ţ���0��ʼ
	//};

	//һ�߹���淶�壬һ�߹���action_goto��

	return true;
}

bool SyntacticAnalyzer::BuildGrammar()
{
	GenProduction();
	GenAugmentedGrammar();
	GenFirstSet();
	GenFollowSet();
	GenGrammarSymbolSet();
	GenLrItems();
	return GenNormalFamilySet();
}

void SyntacticAnalyzer::PrintGrammars()
{
	for (auto iter1 = grammars_.begin(); iter1 != grammars_.end(); iter1++)
	{
		cout << (*iter1).left << "->";
		for (auto iter2 = (iter1->right).begin(); iter2 != (iter1->right).end(); iter2++)
			cout << (*iter2) << "|";
		cout << "\b " << endl;
	}
	return;
}

void SyntacticAnalyzer::PrintProdcutions(const string filename)
{
	ofstream printer;
	printer.open(filename);

	if (!printer.is_open()) {
		cerr << "�﷨���������У�" << filename << "�ļ���ʧ�ܣ�" << endl;
		return ;
	}
	//for (auto iter1 = grammars_.begin(); iter1 != grammars_.end(); iter1++)
	//{
	//	printer << (*iter1).left << "->";
	//	for (auto iter2 = (iter1->right).begin(); iter2 != (iter1->right).end(); iter2++)
	//	{
	//		printer << (*iter2);
	//		if((iter1->right).end() - iter2 != 1)
	//			printer << "|";
	//	}
	//	printer << endl;
	//}

	for (auto iter1 = productions_.begin(); iter1 != productions_.end(); iter1++)
	{
		printer << (*iter1).left << "->";
		for (auto iter2 = (iter1->right).begin(); iter2 != (iter1->right).end(); iter2++)
		{
			printer << (*iter2) << ' ';
		}
		printer << endl;
	}

	//productions_
	return;
}

void SyntacticAnalyzer::PrintFirst(const string filename)
{
	ofstream printer;
	printer.open(filename);

	if (!printer.is_open()) {
		cerr << "�﷨���������У�" << filename << "�ļ���ʧ�ܣ�" << endl;
		return;
	}
	printer << "First���ϣ�" << endl;
	for (auto iter1 = first_map_.begin(); iter1 != first_map_.end(); ++iter1)
	{
		//for (auto iter2 = iter1->begin(); )
		printer << (*iter1).first << ": ";
		int count = 0;
		int size_ = (*iter1).second.size();
		for (auto iter2 = (*iter1).second.begin(); iter2 != (*iter1).second.end(); ++iter2)
		{
			printer << (*iter2);
			if (size_ > count + 1)
				printer << " ";
		}
		printer << endl;
	}
	return;
}

void SyntacticAnalyzer::PrintFollow(const string filename)
{
	ofstream printer;
	printer.open(filename);

	if (!printer.is_open()) {
		cerr << "�﷨���������У�" << filename << "�ļ���ʧ�ܣ�" << endl;
		return;
	}
	printer << "Follow���ϣ�" << endl;
	for (auto iter1 = follow_map_.begin(); iter1 != follow_map_.end(); ++iter1)
	{
		//for (auto iter2 = iter1->begin(); )
		printer << (*iter1).first << ": ";
		int count = 0;
		int size_ = (*iter1).second.size();
		for (auto iter2 = (*iter1).second.begin(); iter2 != (*iter1).second.end(); ++iter2)
		{
			printer << (*iter2);
			if (size_ > count + 1)
				printer << " ";
		}
		printer << endl;
	}
	return;
}

void SyntacticAnalyzer::PrintGrammarSymbolSet(const string filename)
{	
	ofstream printer;
	printer.open(filename);
	if (!printer.is_open()) {
		cerr << "�﷨���������У�" << filename << "�ļ���ʧ�ܣ�" << endl;
		return;
	}

	printer << "�ķ����ţ��ս������ս������" << endl;
	for (auto iter = grammar_symbol_.begin(); iter != grammar_symbol_.end(); ++iter)
	{
		printer << *iter << endl;
	}
	return;
}

void SyntacticAnalyzer::PrintLrItems(const string filename)
{
	ofstream printer;
	printer.open(filename);
	if (!printer.is_open()) {
		cerr << "�﷨���������У�" << filename << "�ļ���ʧ�ܣ�" << endl;
		return;
	}
	printer << "LR0��Ŀ��" << endl;
	for (auto iter1 = lr_items_.begin(); iter1 != lr_items_.end(); ++iter1)
	{
		//if (productions_[iter1->production_number].right[0] == "$")//�����մ�
		//	continue;
		printer << productions_[iter1->production_number].left << "->";

		if (-1 == iter1->point_pos)//�մ���ֻ��һ����
		{
			printer << "��" << endl;
			continue;
		}

		int production_length = productions_[iter1->production_number].right.size();

		for (auto iter2 = productions_[iter1->production_number].right.begin();
			iter2 != productions_[iter1->production_number].right.end(); iter2++)
		{
			if (iter1->point_pos == (iter2 - productions_[iter1->production_number].right.begin()))
				printer << "�� ";
			printer << *iter2 << " ";

			//���һ���ַ������
			if (iter1->point_pos == production_length && 
				(productions_[iter1->production_number].right.end() - iter2 == 1))
				printer << "��";
		}

		printer << endl;
	}
	return;
}

void SyntacticAnalyzer::PrintClosure()
{
	cout << "Closure��Ŀ��" << endl;
	for (auto iter = lr_items_.begin(); iter != lr_items_.end(); ++iter)
	{
		set<LrItem> lr = GenItemClosureSet(*iter);

		for (auto iter1 = lr.begin(); iter1 != lr.end(); ++iter1)
		{
			cout << productions_[iter1->production_number].left << "->";

			if (-1 == iter1->point_pos)//�մ���ֻ��һ����
			{
				cout << "��" << endl;
				continue;
			}

			int production_length = productions_[iter1->production_number].right.size();

			for (auto iter2 = productions_[iter1->production_number].right.begin();
				iter2 != productions_[iter1->production_number].right.end(); iter2++)
			{
				if (iter1->point_pos == (iter2 - productions_[iter1->production_number].right.begin()))
					cout << "�� ";
				cout << *iter2 << " ";

				//���һ���ַ������
				if (iter1->point_pos == production_length &&
					(productions_[iter1->production_number].right.end() - iter2 == 1))
					cout << "��";
			}
			cout << endl;
		}
		cout << endl;
	}
	
}

void SyntacticAnalyzer::PrintSlrError(const set<LrItem> & normal_family)
{
	cerr << "����SLR�ķ������� ��ͻ��Ŀ�Ĺ淶�壺" << endl;
	set<LrItem> lr = normal_family;

	for (auto iter1 = lr.begin(); iter1 != lr.end(); ++iter1)
	{
		cerr << productions_[iter1->production_number].left << "->";

		if (-1 == iter1->point_pos)//�մ���ֻ��һ����
		{
			cerr << "��" << endl;
			continue;
		}

		int production_length = productions_[iter1->production_number].right.size();

		for (auto iter2 = productions_[iter1->production_number].right.begin();
			iter2 != productions_[iter1->production_number].right.end(); iter2++)
		{
			if (iter1->point_pos == (iter2 - productions_[iter1->production_number].right.begin()))
				cerr << "�� ";
			cerr << *iter2 << " ";

			//���һ���ַ������
			if (iter1->point_pos == production_length &&
				(productions_[iter1->production_number].right.end() - iter2 == 1))
				cout << "��";
		}
		cerr << endl;
	}
	cerr << endl;
}

void  SyntacticAnalyzer::PrintNormalFamiliySet(const string filename)
{
	ofstream printer;
	printer.open(filename);
	if (!printer.is_open()) {
		cerr << "�﷨���������У�" << filename << "�ļ���ʧ�ܣ�" << endl;
		return;
	}
	printer << "��Ŀ���淶�壺" << endl;
	for (auto iter1 = normal_family_.begin(); iter1 != normal_family_.end(); ++iter1)
	{
		printer << "�淶�� " << iter1 - normal_family_.begin() << " : " << endl;
		for (auto iter2 = iter1->begin(); iter2 != iter1->end(); ++iter2)
		{

			printer << productions_[iter2->production_number].left << "->";

			if (-1 == iter2->point_pos)//�մ���ֻ��һ����
			{
				printer << "��" << endl;
				continue;
			}

			int production_length = productions_[iter2->production_number].right.size();

			for (auto iter3 = productions_[iter2->production_number].right.begin();
				iter3 != productions_[iter2->production_number].right.end(); iter3++)
			{
				if (iter2->point_pos == (iter3 - productions_[iter2->production_number].right.begin()))
					printer << "�� ";
				printer << *iter3 << " ";

				//���һ���ַ������
				if (iter2->point_pos == production_length &&
					(productions_[iter2->production_number].right.end() - iter3 == 1))
					printer << "��";
			}
			printer << endl;
		}
		printer << endl;
	}
}

void SyntacticAnalyzer::PrintActionGotoTable(const string filename)
{
	ofstream printer;
	printer.open(filename);
	if (!printer.is_open()) {
		cerr << "�﷨���������У�" << filename << "�ļ���ʧ�ܣ�" << endl;
		return;
	}
	//�����ͷ
	printer << "  ";
	for (auto iter = grammar_symbol_.begin(); iter != grammar_symbol_.end(); ++iter)
	{
		if (IsNonTerminalSymbol(*iter))
			continue;
		if("," == (*iter))//����������CSV����� ","���µ�λ�����⡣
			printer << "," << "��";
		else
			printer << "," << (*iter) ;
	}
	for (auto iter = grammar_symbol_.begin(); iter != grammar_symbol_.end(); ++iter)
	{
		if (!IsNonTerminalSymbol(*iter))
			continue;
		printer << "," << (*iter) ;
	}
	printer << endl;

	for (unsigned int state = 0; state < normal_family_.size(); ++state)
	{
		printer << "state " << state;
		for (auto iter = grammar_symbol_.begin(); iter != grammar_symbol_.end(); ++iter)
		{
			if (IsNonTerminalSymbol(*iter))
				continue;

			//map �����ڵ����
			if (action_goto_tables_.find({ state, *iter }) == action_goto_tables_.end())
				printer << ",error";
			else {
				int next_state = action_goto_tables_[{state, *iter}].state;
				int op = action_goto_tables_[{state, (*iter)}].op;
				/*cout << "  ";*/
				if (op == MOVE)
					printer << ",s" << next_state;
				else if (op == CONCLUDE)
				{
					printer << ",r" << next_state;
				}
				else if (op == ACCEPT)
					printer << ",acc";
				else
					printer << ",???";
				//cout << "  ";
			}
		}
		for (auto iter = grammar_symbol_.begin(); iter != grammar_symbol_.end(); ++iter)
		{
			if (!IsNonTerminalSymbol(*iter))
				continue;
			/*cout << "  " << iter->left << "  ";*/
			//map �����ڵ����
			if (action_goto_tables_.find({ state, *iter }) == action_goto_tables_.end())
				printer << ",error";
			else {
				int next_state = action_goto_tables_[{state, *iter}].state;
				int op = action_goto_tables_[{state, *iter}].op;
				//cout << "  ";
				if (op == MOVE)
					printer << ",s" << next_state;
				else if (op == CONCLUDE)
				{
					printer << ",r" << next_state;
				}
				else if (op == ACCEPT)
					printer << ",acc";
				else
					printer << ",???";
				//cout << "  ";
			}
		}
		
		printer << endl;
	}
	return;
}

void SyntacticAnalyzer::PrintBuildGrammarDetails()
{
	if (print_detail_)
	{
		PrintProdcutions();
		PrintFirst();
		PrintFollow();
		PrintGrammarSymbolSet();
		PrintLrItems();
		PrintNormalFamiliySet();
		PrintActionGotoTable();
	}
	return;
}

void SyntacticAnalyzer::PrintAnalysisProcess(int step, const SlrOperation &sl_op)
{
	if (!print_detail_)
		return;
	syntactic_analyzer_printer_ << step << ',';
	//���״̬ջ
	for (auto iter = state_sequence_stack_.begin(); iter != state_sequence_stack_.end(); ++iter)
	{
		syntactic_analyzer_printer_ << *iter << ' ';
	}
	syntactic_analyzer_printer_ << ',';

	//����ַ���ջ
	for (auto iter = move_conclude_string_stack_.begin(); iter != move_conclude_string_stack_.end(); ++iter)
	{
		if ("," == *iter)
			syntactic_analyzer_printer_ << "��";
		else
			syntactic_analyzer_printer_ << *iter << ' ';
	}
	syntactic_analyzer_printer_ << ',';


	//�������
	//---------------------------------------------------------
	if(sl_op.op ==MOVE)
		syntactic_analyzer_printer_ << "�ƽ�";
	else if(sl_op.op == ACCEPT)
		syntactic_analyzer_printer_ << "����";
	else if (sl_op.op == CONCLUDE)
	{
		syntactic_analyzer_printer_ << "��Լ�� " << productions_[sl_op.state].left << "->";
		for (auto iter = productions_[sl_op.state].right.begin(); iter != productions_[sl_op.state].right.end(); ++iter)
		{
			if ("," == *iter)
				syntactic_analyzer_printer_ << "��";
			else
				syntactic_analyzer_printer_ << *iter << ' ';
		}
	}

	syntactic_analyzer_printer_ << endl;
}

bool SyntacticAnalyzer::IsNonTerminalSymbol(const string &symbol)
{
	if (symbol.length() == 0)
		return false;
	if (symbol[0] >= 'A' && symbol[0] <= 'Z')
		return true;
	return false;
}


SyntacticAnalyzer::SyntacticAnalyzer(bool show_detail)
{
	print_detail_ = show_detail;//ѡ���Ƿ��ڷ��������д�ӡ�﷨��������ϸ��Ϣ
	if (show_detail)
	{
		syntactic_analyzer_printer_.open("./gen_data/syntactic_analyzer/syntactic_analyser_process.csv");
		if (!syntactic_analyzer_printer_.is_open()) {
			cerr << "�﷨���������У���ʾ�﷨���������ļ���ʧ�ܣ�" << endl;
		}
		else
			syntactic_analyzer_printer_ << "����, ״̬ջ, ����ջ, ����˵��" << endl;
	}
	BuildGrammar();//����grammar.txt����SLR������ ACTION ��  GOTO
	PrintBuildGrammarDetails();//��ӡ����SLR���������ϸ�����Լ�����������Զ�����print_detail_���ж��Ƿ��ӡ��

	//���ôʷ�������
	//LexicalAnalyzer lexical_analyzer;
	//lexical_analyzer.IsReadyToAnalyze(true);
}

SyntacticAnalyzer::~SyntacticAnalyzer()
{
	if (syntactic_analyzer_printer_.is_open())
		syntactic_analyzer_printer_.close();
}


bool LrItem::operator==(const LrItem & item) const 
{
	return (this->production_number == item.production_number) && (this->point_pos == item.point_pos);
}

bool LrItem::operator<(const LrItem & item) const
{
	return this->production_number < item.production_number || this->production_number == item.production_number&&this->point_pos < item.point_pos;
}

bool SlrOperation::operator==(const SlrOperation & operation) const
{
	return (this->op == operation.op) && (this->state == operation.state);
}


//==================================================================================================================
//����SLR������
bool SyntacticAnalyzer::StartAnalize(const string code_filename)
{
	//ջ�ĳ�ʼ��
	state_sequence_stack_.push_back(0);//��ʼ״̬0ѹ��ջ��
	move_conclude_string_stack_.push_back("#");//��ʼ��#�ַ���ѹ��ջ��


	//struct GrammarSymbolInfo//�������ʱ���õ����ķ���������
	//{
	//	string symbol_name;//��Ӧ��grammar�е�����
	//	string txt_value;//��Ӧ���ı�����ֵ
	//	int num_value;//��Ӧ����ֵ����ֵ
	//};
	grammar_symbol_info_stack_.push_back({"Program"});//��ʼ���ķ���������

	//��ʼ���ʷ�������
	LexicalAnalyzer lexcial_analyzer;
	if (!lexcial_analyzer.IsReadyToAnalyze(true, code_filename))
		return false;

	int sytactic_step = 0;
	while (true)
	{
		//enum LEXICAL_TYPE {
		//	LCINT, LKEYWORD, LIDENTIFIER, LTYPE, LBORDER, LUNKNOWN, LEOF, LOPERATOR,

		//LEXICAL_TYPE type;//������
		//string value;//������ֵ�����߹ؼ��ֱ���
		
		WordInfo get_word = lexcial_analyzer.GetWord();
		string word_string = get_word.word_string;
		if (get_word.type == LUNKNOWN)//������
		{
			cerr << "�ʷ������������У�����unknown����" << endl;
			cerr << get_word.value << endl;
		}



		//enum SLR_OPERATIONS { CONCLUDE, MOVE, ACCEPT, NONTERMINAL, ERROR };//��Լ���ƽ������ܡ�ѹ����ս��������
		//struct SlrOperation//ACTION����GOTO����
		//{
		//	SLR_OPERATIONS op;//��Լ���ƽ�������
		//	int state;//item�е���ţ���0��ʼ
		//	bool operator==(const SlrOperation &operation) const;//�ж����������Ƿ���ͬ
		//};

		//map<pair<int, string>, SlrOperation> action_goto_tables_;//action �� goto ����ں�

		//vector<string> move_conclude_string_stack_;//��� �ƽ���Լ�� ��ջ
		//vector<int> state_sequence_stack_;//״̬����ջ

		while (true)
		{
			//ѡ��״̬����ջջ��
			int current_state = state_sequence_stack_[state_sequence_stack_.size() - 1];
			//action_goto���в����ڶ�Ӧ�Ĳ������﷨���������г��ִ��󣬱�����󲢷���
			if (action_goto_tables_.find({ current_state, word_string }) == action_goto_tables_.end())
			{
				cerr << "�﷨�����������У���������" << endl;
				cerr << get_word.value << endl;
				cerr << "state��" << current_state << " �� " << word_string << " ��action_goto_table �в�����Ӧ����!" << endl;
				return false;
			}

			if (MOVE == action_goto_tables_[{ current_state, word_string }].op)//�ƽ�����
			{
				state_sequence_stack_.push_back(action_goto_tables_[{ current_state, word_string }].state);
				move_conclude_string_stack_.push_back(word_string);
				PrintAnalysisProcess(sytactic_step, action_goto_tables_[{ current_state, word_string }]);
				sytactic_step++;
				grammar_symbol_info_stack_.push_back({ get_word.word_string, get_word.value });//�ķ�������Ϣѹ��

				break;
			}
			else if (CONCLUDE == action_goto_tables_[{ current_state, word_string }].op)//��Լ����
			{
				//��ȡ��Լ����ʽ�����
				int conclude_production_number = action_goto_tables_[{ current_state, word_string }].state;
				int production_length; //����ʽ�Ҳ�������Ŀ
				if (productions_[conclude_production_number].right[0] == "$")
					production_length = 0;
				else
					production_length = productions_[conclude_production_number].right.size();


				for (int i = 0; i < production_length; ++i)//������ջ������production_length��Ԫ��
				{
					//state_sequence_stack_.pop();
					//move_conclude_string_stack_.pop();
					state_sequence_stack_.erase(state_sequence_stack_.end() - 1);
					move_conclude_string_stack_.erase(move_conclude_string_stack_.end() - 1);
				}

				move_conclude_string_stack_.push_back(productions_[conclude_production_number].left);//���ڹ�Լ�Ĳ���ʽ���� ѹ��ջ��
				if (action_goto_tables_.find({ state_sequence_stack_[state_sequence_stack_.size() - 1], productions_[conclude_production_number].left }) == action_goto_tables_.end())//������goto
				{
					cerr << "�﷨�������㷨������������CONCLUDE��" << endl;
					cerr << get_word.value << endl;
					return false;
				}
				state_sequence_stack_.push_back(action_goto_tables_[{ state_sequence_stack_[state_sequence_stack_.size() - 1] , productions_[conclude_production_number].left }].state);//goto��Ӧ��״̬ѹ��
				
				//��������������˳�
				if (!sementic_analyzer_.ExecuteSemanticCheck(grammar_symbol_info_stack_, productions_[conclude_production_number]))
					return false;
				

			}
			else if (ACCEPT == action_goto_tables_[{ current_state, word_string }].op)//��������
			{
				PrintAnalysisProcess(sytactic_step, action_goto_tables_[{ current_state, word_string }]);
				sytactic_step++;
				cerr << "�﷨������ȷ��ɣ�" << endl;
				cerr << "���������ȷ��ɣ�" << endl;
				return true;
			}
			else//�﷨�������㷨����
			{
				cerr << "��������" << endl;
				cerr << "�﷨�������㷨���ڴ������飡" << endl;
				return false;
			}
			PrintAnalysisProcess(sytactic_step, action_goto_tables_[{ current_state, word_string }]);
			sytactic_step++;
		}
		
	}

	return true;
}