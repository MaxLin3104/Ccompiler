#include"semantic_analyzer.h"

SemanticAnalyzer::SemanticAnalyzer()
{
	//����ȫ�ַ��ű�����ӵ���ǰ���ű�ջ��
	symbol_tables_.push_back(SymbolTable(GLOBAL_TABLE, "global_table"));
	current_symbol_table_stack_.push_back(0);

	//������ʱ�������ű�
	symbol_tables_.push_back(SymbolTable(TEMP_TABLE, "temp_table"));
	

	intermediate_code_temp_printer_.open("./gen_data/target_file/intermediate_code_temp.txt");
	if (!intermediate_code_temp_printer_.is_open()) {
		cerr << "������������м�������ʱ�ļ���ʧ�ܣ�" << endl;
	}

	next_state_num_ = 1;//��1��ʼ��0������ת��main��ָ��
	backpatching_level_ = 0;//��0��ʼ
	main_line_ = -1;
	target_code_generator_.TargetCodeGeneratorInit(symbol_tables_);//����������Ŀ������������У�ʵ�ֹ���

	while_cnt_ = 0;
	if_cnt_ = 0;
	w_j_label_num_ = 0;
}
SemanticAnalyzer::~SemanticAnalyzer()
{
	if (intermediate_code_temp_printer_.is_open())
		intermediate_code_temp_printer_.close();

	if (intermediate_code_printer_.is_open())
		intermediate_code_printer_.close();

	if (intermediate_code_temp_reader_.is_open())
		intermediate_code_temp_reader_.close();	

	remove("./gen_data/target_file/intermediate_code_temp.txt");
}

int SemanticAnalyzer::GetNextStateNum()
{
	return next_state_num_++;
}
int SemanticAnalyzer::PeekNextStateNum()
{
	return next_state_num_;
}

string SemanticAnalyzer::GetArgName(SymbolPos &sp, bool is_return)
{
	string arg_name = symbol_tables_[sp.table_pos].GetSymbolName(sp.symbol_pos);
	if (is_return == false)
	{
		if (VARIABLE == symbol_tables_[sp.table_pos].GetSymbolMode(sp.symbol_pos))//����
		{
			//arg_name = arg_name + "_" + std::to_string(sp.table_pos);
			arg_name = arg_name + "-" + symbol_tables_[sp.table_pos].GetTableName()+"Var";
		}
	}
	else
	{
		int pos = symbol_tables_[0].FindSymbol(symbol_tables_[sp.table_pos].GetTableName());
		Symbol &symbol = symbol_tables_[0].GetSymbol(pos);
		int symbol_pos_minus = sp.symbol_pos - 1;
		arg_name = arg_name + "-" + symbol.name + "_paramerter " + std::to_string(sp.symbol_pos);
		//if (VARIABLE == symbol_tables_[sp.table_pos].GetSymbolMode(sp.symbol_pos))//����
		//{
		//	int pos = symbol_tables_[0].FindSymbol(symbol_tables_[sp.table_pos].GetTableName());
		//	if (pos != -1 && symbol_tables_[0].GetSymbol(pos).mode == FUNCTION)
		//	{
		//		Symbol &symbol = symbol_tables_[0].GetSymbol(pos);
		//		int symbol_pos_minus = sp.symbol_pos - 1;
		//		if (symbol_pos_minus <= symbol.parameter_num)
		//			arg_name = arg_name + "-" + symbol.name + "_paramerter " + std::to_string(sp.symbol_pos);
		//		else
		//			arg_name = arg_name + "_" + std::to_string(sp.table_pos);
		//	}
		//	else
		//		arg_name = arg_name + "_" + std::to_string(sp.table_pos);
		//}
	}
	return arg_name;
}

bool SemanticAnalyzer::PrintQuadruples(const Quadruples &quadruples)
{
	//intermediate_code_temp_printer_ << quadruples.num << "(" << quadruples.op << ","
	//	<< quadruples.arg1 << "," << quadruples.arg2 << "," << quadruples.result << ")" << endl;
	if(backpatching_level_ == 0)//����Ҫ����
		intermediate_code_temp_printer_ << quadruples.num << " (" << quadruples.op << ", "
		<< quadruples.arg1 << ", " << quadruples.arg2 << ", " << quadruples.result << ")" << endl;
	else//��Ҫ����	
		quadruples_stack_.push_back(quadruples);
	
	return true;
}

bool SemanticAnalyzer::PrintQuadruples()
{
	intermediate_code_printer_.open("./gen_data/target_file/intermediate_code.txt");
	if (!intermediate_code_printer_.is_open()) {
		cerr << "������������м������ļ���ʧ�ܣ�" << endl;
		return false;
	}

	intermediate_code_temp_reader_.open("./gen_data/target_file/intermediate_code_temp.txt");
	if (!intermediate_code_temp_reader_.is_open()) {
		cerr << "��ȡ������������м�������ʱ�ļ���ʧ�ܣ�" << endl;
		return false;
	}

	//�ر���ʱ�ļ���
	if (intermediate_code_temp_printer_.is_open())
		intermediate_code_temp_printer_.close();
	////��ʱ�ļ��ص�ͷ��
	//intermediate_code_temp_printer_.clear();
	//intermediate_code_temp_printer_.seekg(0);
	
	//��ӵ�0��ָ�ֱ����ת��Main�������ڵ�λ��
	intermediate_code_printer_ << 0 << " (j, " << "-, " << "-, " << main_line_ << ")" << endl;

	while (!intermediate_code_temp_reader_.eof())
	{
		string str;
		std::getline(intermediate_code_temp_reader_, str);
		if (str == "")
			continue;
		intermediate_code_printer_ << str << endl;
	}

	if (intermediate_code_temp_reader_.is_open())
		intermediate_code_temp_reader_.close();

	//ɾ����ʱ�ļ�
	if (remove("./gen_data/target_file/intermediate_code_temp.txt") != 0)
		cerr << "�м������ʱ�ļ�ɾ��ʧ�ܣ�" << endl;

	return true;
}

bool SemanticAnalyzer::CreateSymbolTable(TABLE_TYPE table_type, const string &table_name)
{
	symbol_tables_.push_back(SymbolTable(table_type, table_name));
	current_symbol_table_stack_.push_back(symbol_tables_.size() - 1);
	return true;
}

bool SemanticAnalyzer::ExecuteSemanticCheck(vector<GrammarSymbolInfo>&symbol_info_stack, const Production &production)
{
	if ("Const_value" == production.left)//Const_value->num
	{	
		GrammarSymbolInfo symbol_info = symbol_info_stack.back();
		
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = symbol_info.txt_value;
		

		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for(int i = 0; i < length;++i)
			symbol_info_stack.pop_back();

		symbol_info_stack.push_back(conclude_syble_info);
	}
	
	else if ("Factor" == production.left && production.right[0] == "Const_value")//Factor->Const_value
	{
		//��Ҫ��Const_value������ʱ�������õ���ʱ�����������Ѿ��ڱ��е�λ�á�
		//txt_value ��ʾ����
		//pos ��ʾλ��
		//��Ҫ �м�������---------------------------------------------------------------------------------------------

		//��������
		GrammarSymbolInfo symbol_info = symbol_info_stack.back();

		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;

		int symbol_pos = symbol_tables_[1].AddSymbol(symbol_info.txt_value);
		string symbol_name = symbol_tables_[1].GetSymbolName(symbol_pos);

		conclude_syble_info.txt_value = symbol_name;
		conclude_syble_info.pos = { 1, symbol_pos };//1��ʾ��ʱ������

		
		PrintQuadruples(Quadruples(GetNextStateNum(), ":=", symbol_info.txt_value, "-", symbol_name));

		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for (int i = 0; i < length; ++i)
			symbol_info_stack.pop_back();
		symbol_info_stack.push_back(conclude_syble_info);

		//Ŀ���������
		target_code_generator_.LoadImmToReg(symbol_info.txt_value, conclude_syble_info.pos);
	}
	
	else if("Factor" == production.left && production.right[0] == "(")//Factor->(Expression)
	{
		//��������
		GrammarSymbolInfo symbol_expression = symbol_info_stack[symbol_info_stack.size() - 2];

		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = symbol_tables_[symbol_expression.pos.table_pos].GetSymbolName(symbol_expression.pos.symbol_pos);
		conclude_syble_info.pos = symbol_expression.pos;

		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for (int i = 0; i < length; ++i)
			symbol_info_stack.pop_back();
		symbol_info_stack.push_back(conclude_syble_info);
	}
	
	else if ("Factor" == production.left && production.right[0] == "identifier")//Factor->identifier FTYPE
	{
		//��������
		GrammarSymbolInfo &symbol_identifier = symbol_info_stack[symbol_info_stack.size() - 2];
		GrammarSymbolInfo symbol_ftype = symbol_info_stack.back();

		
		//�ڷ��ű�ջ��һ��������Ѱ�� identifier�Ƿ񱻶����

		int identifier_pos = -1;
		int identifier_layer = current_symbol_table_stack_.size() - 1;
		for (; identifier_layer >= 0; --identifier_layer)
		{
			SymbolTable search_table = symbol_tables_[current_symbol_table_stack_[identifier_layer]];
			identifier_pos = search_table.FindSymbol(symbol_identifier.txt_value);
			if (identifier_pos != -1)//��ʾ��ĳһ�����ҵ���
				break;
		}


		if (-1 == identifier_pos)//û�ҵ�����ʾ�����ڣ�˵��û�����ʹ�ã��������
		{
			cerr << "������󣡣���" << symbol_identifier.txt_value << "û�ж��壡" << endl;
			return false;
		}

		//��Indentifier��ֵpos
		symbol_identifier.pos.table_pos = current_symbol_table_stack_[identifier_layer];
		symbol_identifier.pos.symbol_pos = identifier_pos;

		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		if (symbol_ftype.txt_value == "")//��ʾFTYPE�Ǵ�FTYPE->$��ȡ��������Factor�Ǵӱ�ʶ����ȡ�����Ǻ�����
		{
			//Factor->identifier�������Ǻ�������
			//���������ʱ�����Ļ�����Ҫ����һ����ʱ����������š�
			if (TEMP == symbol_tables_[symbol_identifier.pos.table_pos].GetSymbolMode(symbol_identifier.pos.symbol_pos))
			{
				//ԭ��������ʱ���������������룬��ֱ�Ӵ���ʹ��
				conclude_syble_info.pos = { symbol_identifier.pos.table_pos, symbol_identifier.pos.symbol_pos };
				conclude_syble_info.txt_value = symbol_identifier.txt_value;
			}
			else
			{
				//������ʱ����
				int symbol_pos = symbol_tables_[1].AddSymbol(symbol_identifier.txt_value);
				string symbol_name = symbol_tables_[1].GetSymbolName(symbol_pos);


				//����ʱ�������洢��ǰ������ֵ
				PrintQuadruples(Quadruples(GetNextStateNum(), ":=", GetArgName(symbol_identifier.pos), "-", symbol_name));

				conclude_syble_info.pos = { 1, symbol_pos };//1��ʾ��ʱ������

				//Ŀ���������
				string tar_arg1_name = target_code_generator_.SetArgBeReady(conclude_syble_info.pos);//���������ʱ����
				SymbolPos sb_tmp = { symbol_identifier.pos.table_pos, symbol_identifier.pos.symbol_pos };
				string tar_arg2_name = target_code_generator_.SetArgBeReady(sb_tmp);

				target_code_generator_.PrintQuadruples(Instructions("move", tar_arg1_name, tar_arg2_name, ""));
			}

			//conclude_syble_info.txt_value = symbol_tables_[current_symbol_table_stack_[identifier_layer]].GetSymbolName(identifier_pos);		
			//conclude_syble_info.pos = { current_symbol_table_stack_[identifier_layer], identifier_pos };

		}
		else {
			//Factor->identifier FTYPE  �ǴӺ��������л�ȡ��
			//���ϴ���pos����
			int table_pos, symbol_pos = 0;
			table_pos = symbol_tables_[symbol_ftype.pos.table_pos].GetSymbol(symbol_ftype.pos.symbol_pos).func_symbol_pos;
			//conclude_syble_info.pos = { table_pos, symbol_pos };
			conclude_syble_info.txt_value = "";

			//������ʱ����
			int symbol_pos_t = symbol_tables_[1].AddSymbol(symbol_identifier.txt_value);
			string symbol_name = symbol_tables_[1].GetSymbolName(symbol_pos_t);

			//����ʱ�������洢��ǰ������ֵ
			int func_table_pos_t = symbol_tables_[0].GetSymbol(symbol_identifier.pos.symbol_pos).func_symbol_pos;
			SymbolPos return_symbol_pos = { func_table_pos_t , 0 };
			PrintQuadruples(Quadruples(GetNextStateNum(), ":=", GetArgName(return_symbol_pos), "-", symbol_name));
			conclude_syble_info.pos = { 1, symbol_pos_t };//1��ʾ��ʱ������


			//Ŀ���������
			target_code_generator_.ClearRegs();//���ú���ǰ����ռĴ���
			string target_func_name = symbol_tables_[0].GetSymbolName(identifier_pos);
			target_code_generator_.PrintQuadruples("jal " + target_func_name);//���ɺ�������ָ��
			//����ջ֡
			target_code_generator_.PrintQuadruples(Instructions("move", "$sp", "$fp", ""));//spָ��fp
			target_code_generator_.PrintQuadruples(Instructions("lw", "$fp", "($fp)", ""));//fpָ����fp
			int par_num = symbol_tables_[0].GetSymbol(symbol_identifier.pos.symbol_pos).parameter_num;//������������
			int sp_back_num = -1 - par_num; //spָ����˵���������ǰ��ջ���� 1���ص�ַ���Լ�ʵ�ʲ����ĸ���
			target_code_generator_.PushStack_sp(sp_back_num);//sp����

			//����ֵ�޸� v0 ->tmp
			string tar_arg1_name = target_code_generator_.SetArgBeReady(conclude_syble_info.pos);//���������ʱ����
			//SymbolPos sb_tmp = { symbol_identifier.pos.table_pos, symbol_identifier.pos.symbol_pos };
			string tar_arg2_name = target_code_generator_.SetArgBeReady(conclude_syble_info.pos);
			target_code_generator_.PrintQuadruples(Instructions("move", tar_arg1_name, "$v0", ""));
		}

		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for (int i = 0; i < length; ++i)
			symbol_info_stack.pop_back();
		symbol_info_stack.push_back(conclude_syble_info);
	}
	
	else if ("Factor_loop" == production.left && production.right[0] == "$")//Factor_loop->$
	{
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = "";//�գ��������Ϊ��
		//symbol_info_stack.pop_back();
		symbol_info_stack.push_back(conclude_syble_info);
	}
	
	else if ("Factor_loop" == production.left && production.right[0] == "Factor_loop")
		//Factor_loop->Factor_loop Factor *|Factor_loop Factor /
		//������Ҫ������ʱ����
	{
		//��������
		GrammarSymbolInfo symbol_factor = symbol_info_stack[symbol_info_stack.size() - 2];
		GrammarSymbolInfo symbol_factor_loop = symbol_info_stack[symbol_info_stack.size() - 3];


		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;

		if (symbol_factor_loop.txt_value == "")//�Ҳ���Factor_loop�Ǵ�$��Լ����
		{
			//������ʱ������������Ž�����������Ҵ���

			conclude_syble_info.txt_value = symbol_info_stack.back().symbol_name; //  * ���� /

			if (TEMP == symbol_tables_[symbol_factor.pos.table_pos].GetSymbolMode(symbol_factor.pos.symbol_pos))
			{
				//ԭ��������ʱ���������������룬��ֱ�Ӵ���ʹ��
				conclude_syble_info.pos = symbol_factor.pos;
			}
			else
			{
				//������ʱ����
				int symbol_pos = symbol_tables_[1].AddSymbol(symbol_factor.txt_value);
				string symbol_name = symbol_tables_[1].GetSymbolName(symbol_pos);

				//����ʱ�������洢��ǰ������ֵ
				PrintQuadruples(Quadruples(GetNextStateNum(), ":=", GetArgName(symbol_factor.pos), "-", symbol_name));
				conclude_syble_info.pos = { 1, symbol_pos };//1��ʾ��ʱ������

				//Ŀ���������
				string tar_arg1_name = target_code_generator_.SetArgBeReady(conclude_syble_info.pos);//���������ʱ����
				string tar_arg2_name = target_code_generator_.SetArgBeReady(symbol_factor.pos);

				target_code_generator_.PrintQuadruples(Instructions("move", tar_arg1_name, tar_arg2_name, ""));
			}
						
		}
		else//�Ҳ���Factor_loop�ж�������Ҫ���㣬ͬʱ����ʱ�������ϴ���
		{
			//������Ҫͨ���м��������
			//����ʱ���� op Factor��ֵ������ʱ����
			
			//�����м���룺��ʱ������factor ����󷵻ظ���ʱ����
			//��ȡ����arg�����ơ� 
			//������ʱ��������Ҫ���ϱ����ڵ����
			//arg2 ����a_2
			string arg2_name = GetArgName(symbol_factor.pos);
			string arg1_name = GetArgName(symbol_factor_loop.pos);

			string tar_arg2_name = target_code_generator_.SetArgBeReady(symbol_factor.pos);
			string tar_arg1_name = target_code_generator_.SetArgBeReady(symbol_factor_loop.pos);

			conclude_syble_info.txt_value = symbol_factor_loop.txt_value; //  * ���� /
			conclude_syble_info.pos = symbol_factor_loop.pos;
			
			//Ŀ��������м���빲ͬ����
			if ("*" == symbol_factor_loop.txt_value)
			{
				PrintQuadruples(Quadruples(GetNextStateNum(), "*", arg1_name, arg2_name, arg1_name));
				target_code_generator_.PrintQuadruples(Instructions("mult", tar_arg1_name, tar_arg2_name, ""));
				target_code_generator_.PrintQuadruples(Instructions("mflo", tar_arg1_name, "", ""));//ȡ��͵�32λ����32��ȥ
				
			}
			else// '/' ����
			{
				PrintQuadruples(Quadruples(GetNextStateNum(), "/", arg1_name, arg2_name, arg1_name));
				target_code_generator_.PrintQuadruples(Instructions("div", tar_arg1_name, tar_arg2_name, ""));
				target_code_generator_.PrintQuadruples(Instructions("mflo", tar_arg1_name, "", ""));//ȡ��͵�32λ����32��ȥ
			}
		}

		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for (int i = 0; i < length; ++i)
			symbol_info_stack.pop_back();
		symbol_info_stack.push_back(conclude_syble_info);
	}
	
	else if ("Item" == production.left)//Item->Factor_loop Factor
	{
		//��������
		GrammarSymbolInfo  symbol_factor_loop = symbol_info_stack[symbol_info_stack.size() - 2];
		GrammarSymbolInfo symbol_factor = symbol_info_stack.back();

		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		//conclude_syble_info.txt_value = symbol_factor.txt_value + symbol_factor_loop.txt_value;


		//Factor_loop Ϊ��
		if (symbol_factor_loop.txt_value == "")
		{
			conclude_syble_info.txt_value = "";
			conclude_syble_info.pos = symbol_factor.pos;


		}
		else//����
		{
			//Factor_loop �� Factor ����һ�£�Ȼ�󷵻�Factor_loop 

			string arg2_name = GetArgName(symbol_factor.pos);
			string arg1_name = GetArgName(symbol_factor_loop.pos);

			string tar_arg2_name = target_code_generator_.SetArgBeReady(symbol_factor.pos);
			string tar_arg1_name = target_code_generator_.SetArgBeReady(symbol_factor_loop.pos);


			if ("*" == symbol_factor_loop.txt_value)
			{
				PrintQuadruples(Quadruples(GetNextStateNum(), "*", arg1_name, arg2_name, arg1_name));
				target_code_generator_.PrintQuadruples(Instructions("mult", tar_arg1_name, tar_arg2_name, ""));
				target_code_generator_.PrintQuadruples(Instructions("mflo", tar_arg1_name, "", ""));//ȡ��͵�32λ����32��ȥ
			}
			else// '/' ����
			{
				PrintQuadruples(Quadruples(GetNextStateNum(), "/", arg1_name, arg2_name, arg1_name));
				target_code_generator_.PrintQuadruples(Instructions("div", tar_arg1_name, tar_arg2_name, ""));
				target_code_generator_.PrintQuadruples(Instructions("mflo", tar_arg1_name, "", ""));//ȡ��͵�32λ����32��ȥ
			}

			//conclude_syble_info.txt_value  == * ���� /
			conclude_syble_info.txt_value = symbol_factor_loop.txt_value;
			conclude_syble_info.pos = symbol_factor_loop.pos;
		}


		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for (int i = 0; i < length; ++i)
			symbol_info_stack.pop_back();
		symbol_info_stack.push_back(conclude_syble_info);
	}
	
	else if ("Item_loop" == production.left && production.right[0] == "$")//Item_loop->$
	{
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = "";//�գ��������Ϊ��
		//symbol_info_stack.pop_back();
		symbol_info_stack.push_back(conclude_syble_info);
	}
	
	else if ("Item_loop" == production.left && production.right[0] == "Item_loop")
	//Item_loop->Item_loop Item +|Item_loop Item -
	//������Ҫ������ʱ����
	{
		//��������
		GrammarSymbolInfo symbol_item = symbol_info_stack[symbol_info_stack.size() - 2];
		GrammarSymbolInfo symbol_item_loop = symbol_info_stack[symbol_info_stack.size() - 3];


		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;

		if (symbol_item_loop.txt_value == "")//�Ҳ���Item_loop�Ǵ�$��Լ����
		{
			//����û�б�Ҫ������ʱ������ֱ�Ӱ�ֵ��Item���ݵ�Item_loop
			conclude_syble_info.txt_value = symbol_info_stack.back().symbol_name; //  + ���� -
			conclude_syble_info.pos = symbol_item.pos;

			////������ʱ������������Ž�����������Ҵ���
			//conclude_syble_info.txt_value = symbol_info_stack.back().symbol_name; //  + ���� -

			////������ʱ����
			//int symbol_pos = symbol_tables_[1].AddSymbol(symbol_item.txt_value);
			//string symbol_name = symbol_tables_[1].GetSymbolName(symbol_pos);
			//
			////��������м����--------
			////����ʱ�������洢��ǰ������ֵ
			//PrintQuadruples(Quadruples(GetNextStateNum(), ":=", GetArgName(symbol_item.pos), "-", symbol_name));

			//conclude_syble_info.pos = { 1, symbol_pos };//1��ʾ��ʱ������
		}
		else//�Ҳ���Item_loop�ж�������Ҫ���㣬ͬʱ������ʱ����
		{
			//������Ҫͨ���м��������
			//����ʱ���� op Item��ֵ������ʱ����

			//�����м���룺��ʱ������Item ����󷵻ظ���ʱ����
			//��ȡ����arg�����ơ� 
			//������ʱ��������Ҫ���ϱ����ڵ����
			//arg2 ����a_2
			string arg2_name = GetArgName(symbol_item.pos);
			string arg1_name = GetArgName(symbol_item_loop.pos);

			string tar_arg2_name = target_code_generator_.SetArgBeReady(symbol_item.pos);
			string tar_arg1_name = target_code_generator_.SetArgBeReady(symbol_item_loop.pos);

			//Ŀ��������м���빲ͬ����
			if ("+" == symbol_item_loop.txt_value)
			{
				PrintQuadruples(Quadruples(GetNextStateNum(), "+", arg1_name, arg2_name, arg1_name));
				target_code_generator_.PrintQuadruples(Instructions("add", tar_arg1_name, tar_arg1_name, tar_arg2_name));
			}
			else// '-' ����
			{
				PrintQuadruples(Quadruples(GetNextStateNum(), "-", arg1_name, arg2_name, arg1_name));
				target_code_generator_.PrintQuadruples(Instructions("sub", tar_arg1_name, tar_arg1_name, tar_arg2_name));
			}

			conclude_syble_info.txt_value = symbol_info_stack.back().symbol_name; //  + ���� -
			conclude_syble_info.pos = symbol_item_loop.pos;

		}

		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for (int i = 0; i < length; ++i)
			symbol_info_stack.pop_back();
		symbol_info_stack.push_back(conclude_syble_info);
	}

	else if ("Add_expression" == production.left)//Add_expression->Item_loop Item
	{
		//��������
		GrammarSymbolInfo symbol_item_loop = symbol_info_stack[symbol_info_stack.size() - 2];
		GrammarSymbolInfo symbol_item = symbol_info_stack.back();

		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;


		//Item_loop Ϊ��
		if (symbol_item_loop.txt_value == "")
		{
			conclude_syble_info.txt_value = "";
			conclude_syble_info.pos = symbol_item.pos;
		}
		else//����
		{
			//Item_loop �� Item ����һ�£�Ȼ�󷵻�Item_loop 
			string arg2_name = GetArgName(symbol_item.pos);
			string arg1_name = GetArgName(symbol_item_loop.pos);

			string tar_arg2_name = target_code_generator_.SetArgBeReady(symbol_item.pos);
			string tar_arg1_name = target_code_generator_.SetArgBeReady(symbol_item_loop.pos);

			//Ŀ��������м���빲ͬ����
			if ("+" == symbol_item_loop.txt_value)
			{
				PrintQuadruples(Quadruples(GetNextStateNum(), "+", arg1_name, arg2_name, arg1_name));
				target_code_generator_.PrintQuadruples(Instructions("add", tar_arg1_name, tar_arg1_name, tar_arg2_name));
			}
			else// '-' ����
			{
				PrintQuadruples(Quadruples(GetNextStateNum(), "-", arg1_name, arg2_name, arg1_name));
				target_code_generator_.PrintQuadruples(Instructions("sub", tar_arg1_name, tar_arg1_name, tar_arg2_name));
			}

			//conclude_syble_info.txt_value  == + ���� -
			conclude_syble_info.txt_value = symbol_item_loop.txt_value;
			conclude_syble_info.pos = symbol_item_loop.pos;
		}


		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for (int i = 0; i < length; ++i)
			symbol_info_stack.pop_back();
		symbol_info_stack.push_back(conclude_syble_info);
	}
	
	else if ("Add_expression_loop" == production.left && production.right[0] == "$")//Add_expression_loop->$
	{
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = "";//�գ��������Ϊ��
		//symbol_info_stack.pop_back();
		symbol_info_stack.push_back(conclude_syble_info);
	}
	
	else if ("Add_expression_loop" == production.left && production.right[0] ==  "Add_expression_loop")
	//Add_expression_loop->Add_expression_loop Add_expression Relop
	//������Ҫ������ʱ����
	{
		//��������
		GrammarSymbolInfo symbol_add_expression = symbol_info_stack[symbol_info_stack.size() - 2];
		GrammarSymbolInfo symbol_add_expression_loop = symbol_info_stack[symbol_info_stack.size() - 3];


		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;

		if (symbol_add_expression_loop.txt_value == "")//�Ҳ���Factor_loop�Ǵ�$��Լ����
		{
			//����û�б�Ҫ������ʱ������ֱ�Ӱ�ֵ��Add_expression���ݵ�Add_expression_loop
			conclude_syble_info.txt_value = symbol_info_stack.back().txt_value; //  <|<=|>|>=|==|!=
			conclude_syble_info.pos = symbol_add_expression.pos;
		}
		else//�Ҳ���Add_expression_loop�ж�������Ҫ���㣬ͬʱ����ʱ�������ϴ���
		{
			string arg2_name = GetArgName(symbol_add_expression.pos);
			string arg1_name = GetArgName(symbol_add_expression_loop.pos);
			string op_name = "j" + symbol_add_expression_loop.txt_value; //  <|<=|>|>=|==|!=;

			int next_state_num = GetNextStateNum();	
			PrintQuadruples(Quadruples(next_state_num, op_name, arg1_name, arg2_name, std::to_string(next_state_num + 3)));
			PrintQuadruples(Quadruples(GetNextStateNum(), ":=", "0", "-", arg1_name));
			PrintQuadruples(Quadruples(GetNextStateNum(), "j", "-", "-", std::to_string(next_state_num + 4)));
			PrintQuadruples(Quadruples(GetNextStateNum(), ":=", "1", "-", arg1_name));

			string tar_arg2_name = target_code_generator_.SetArgBeReady(symbol_add_expression.pos);
			string tar_arg1_name = target_code_generator_.SetArgBeReady(symbol_add_expression_loop.pos);
			string op = symbol_add_expression_loop.txt_value;
			if ("<" == op)
				target_code_generator_.PrintQuadruples(Instructions("slt", tar_arg1_name, tar_arg1_name, tar_arg2_name));
			else if ("<=" == op)
				target_code_generator_.PrintQuadruples(Instructions("sle", tar_arg1_name, tar_arg1_name, tar_arg2_name));
			else if (">" == op)
				target_code_generator_.PrintQuadruples(Instructions("sgt", tar_arg1_name, tar_arg1_name, tar_arg2_name));
			else if (">=" == op)
				target_code_generator_.PrintQuadruples(Instructions("sge", tar_arg1_name, tar_arg1_name, tar_arg2_name));
			else if ("==" == op)
				target_code_generator_.PrintQuadruples(Instructions("seq", tar_arg1_name, tar_arg1_name, tar_arg2_name));
			else if ("!=" == op)
				target_code_generator_.PrintQuadruples(Instructions("snq", tar_arg1_name, tar_arg1_name, tar_arg2_name));
			else
			{
				cerr << "�����������" << "Add_expression_loop" << endl;
				return false;
			}
			//Ŀ���������
			//  <  | <=  |  >  | >= | ==  | !=
			// slt | sle | sgt | sge| seq | snq

			conclude_syble_info.txt_value = symbol_add_expression_loop.txt_value; //  <|<=|>|>=|==|!=
			conclude_syble_info.pos = symbol_add_expression_loop.pos;

		}

		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for (int i = 0; i < length; ++i)
			symbol_info_stack.pop_back();
		symbol_info_stack.push_back(conclude_syble_info);
	}
	

	else if ("Relop" == production.left)//Relop-><|<=|>|>= |= = |!=
	{
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = symbol_info_stack.back().txt_value;//<|<=|>|>= |= = |!=

		// ��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for (int i = 0; i < length; ++i)
			symbol_info_stack.pop_back();
		symbol_info_stack.push_back(conclude_syble_info);
	}

	else if ("Expression" == production.left)//Expression->Add_expression_loop Add_expression
	{
		//��������
		GrammarSymbolInfo symbol_add_expression_loop = symbol_info_stack[symbol_info_stack.size() - 2];
		GrammarSymbolInfo symbol_add_expression = symbol_info_stack.back();

		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;


		//Add_expression_loop Ϊ��
		if (symbol_add_expression_loop.txt_value == "")
		{
			conclude_syble_info.txt_value = symbol_add_expression.txt_value;
			conclude_syble_info.pos = symbol_add_expression.pos;
		}
		else//����
		{
			//Add_expression_loop �� Add_expression ����һ�£�Ȼ�󷵻�Add_expression_loop -----------------------�м����

			string arg2_name = GetArgName(symbol_add_expression.pos);
			string arg1_name = GetArgName(symbol_add_expression_loop.pos);
			string op_name = "j" + symbol_add_expression_loop.txt_value; //  <|<=|>|>=|==|!=;

			int next_state_num = GetNextStateNum();
			PrintQuadruples(Quadruples(next_state_num, op_name, arg1_name, arg2_name, std::to_string(next_state_num + 3)));
			PrintQuadruples(Quadruples(GetNextStateNum(), ":=", "0", "-", arg1_name));
			PrintQuadruples(Quadruples(GetNextStateNum(), "j", "-", "-", std::to_string(next_state_num + 4)));
			PrintQuadruples(Quadruples(GetNextStateNum(), ":=", "1", "-", arg1_name));


			string tar_arg2_name = target_code_generator_.SetArgBeReady(symbol_add_expression.pos);
			string tar_arg1_name = target_code_generator_.SetArgBeReady(symbol_add_expression_loop.pos);
			string op = symbol_add_expression_loop.txt_value;
			if ("<" == op)
				target_code_generator_.PrintQuadruples(Instructions("slt", tar_arg1_name, tar_arg1_name, tar_arg2_name));
			else if ("<=" == op)
				target_code_generator_.PrintQuadruples(Instructions("sle", tar_arg1_name, tar_arg1_name, tar_arg2_name));
			else if (">" == op)
				target_code_generator_.PrintQuadruples(Instructions("sgt", tar_arg1_name, tar_arg1_name, tar_arg2_name));
			else if (">=" == op)
				target_code_generator_.PrintQuadruples(Instructions("sge", tar_arg1_name, tar_arg1_name, tar_arg2_name));
			else if ("==" == op)
				target_code_generator_.PrintQuadruples(Instructions("seq", tar_arg1_name, tar_arg1_name, tar_arg2_name));
			else if ("!=" == op)
				target_code_generator_.PrintQuadruples(Instructions("snq", tar_arg1_name, tar_arg1_name, tar_arg2_name));
			else
			{
				cerr << "�����������" << "Add_expression_loop" << endl;
				return false;
			}

			//conclude_syble_info.txt_value  ==  <|<=|>|>=|==|!=
			conclude_syble_info.txt_value = symbol_add_expression_loop.txt_value;
			conclude_syble_info.pos = symbol_add_expression_loop.pos;
		}


		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for (int i = 0; i < length; ++i)
			symbol_info_stack.pop_back();
		symbol_info_stack.push_back(conclude_syble_info);
	}
	

	
	else if ("Internal_variable_stmt" == production.left )//Internal_variable_stmt->int identifier
	{
		GrammarSymbolInfo symbol_identifier = symbol_info_stack.back();
		//����ע��������
		SymbolTable &current_table = symbol_tables_[current_symbol_table_stack_.back()];

		//���ڵ�ǰ���ű���Ѱ�� identifier�Ƿ񱻶����
		if (-1 != current_table.FindSymbol(symbol_identifier.txt_value))//�ҵ�����ʾ�Ѵ��ڣ�˵���ظ����壬�������
		{
			cerr << "������󣡣���" << symbol_identifier.txt_value << "�ظ����壡" << endl;
			return false;
		}

		//����ȫ�ַ��ű��в鿴������뺯�������ͻ
		int search_symbol_pos = symbol_tables_[0].FindSymbol(symbol_identifier.txt_value);
		if (-1 != search_symbol_pos && symbol_tables_[0].GetSymbolMode(search_symbol_pos) == FUNCTION)
		{

			cerr << "������󣡣���" << symbol_identifier.txt_value << "�Ѿ�������Ϊ������" << endl;
			return false;
		}
		
		//û�б��������������ű���
		Symbol variable_symbol;
		variable_symbol.mode = VARIABLE;//����
		variable_symbol.name = symbol_identifier.txt_value;//����
		variable_symbol.type = INT;
		variable_symbol.value = "";//����ֵ��û�б���ֵ���˴�Ϊ��
		
		int symbol_pos = current_table.AddSymbol(variable_symbol);//������ű���

		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for(int i = 0; i < length;++i)
			symbol_info_stack.pop_back();
		
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;

		//�������������û��ʲô�ô�
		conclude_syble_info.txt_value = symbol_identifier.txt_value; //����������
		conclude_syble_info.pos = { current_symbol_table_stack_.back(), symbol_pos };//���ڷ��ű��˳���ڷ��ű��е�λ�ô�����

		symbol_info_stack.push_back(conclude_syble_info);

		//Ŀ��������ɣ�����һ���ֲ�������push��ջ��$sp=$sp+1*4
		target_code_generator_.PushStack_sp(1);
	}
	
	else if ("Assign_sentence" == production.left)//Assign_sentence->identifier = Expression ;
	{
		//��ȡidentifier��Expression���ķ���������
		GrammarSymbolInfo symbol_expression = symbol_info_stack[symbol_info_stack.size() - 2];
		GrammarSymbolInfo symbol_identifier = symbol_info_stack[symbol_info_stack.size() - 4];
		//SymbolTable current_table = symbol_tables_[current_symbol_table_stack_.back()];

		//�ڷ��ű�ջ��һ��������Ѱ�� identifier�Ƿ񱻶����

		int identifier_pos = -1;
		int identifier_layer = current_symbol_table_stack_.size() - 1;
		for (;identifier_layer >= 0; --identifier_layer)
		{
			SymbolTable search_table = symbol_tables_[current_symbol_table_stack_[identifier_layer]];
			identifier_pos = search_table.FindSymbol(symbol_identifier.txt_value);
			if (identifier_pos != -1)//��ʾ��ĳһ�����ҵ���
				break;
		}


		if (-1 == identifier_pos)//û�ҵ�����ʾ�����ڣ�˵��û�����ʹ�ã��������
		{
			cerr << "������󣡣���" << symbol_identifier.txt_value << "û�ж��壡" << endl;
			return false;
		}

		//�����ֵ���м����
		SymbolPos sp;
		sp.table_pos = current_symbol_table_stack_[identifier_layer];
		sp.symbol_pos = identifier_pos;
		string result_name = GetArgName(sp);
		string arg1_name = GetArgName(symbol_expression.pos);
		PrintQuadruples(Quadruples(GetNextStateNum(), ":=", arg1_name, "-", result_name));

		//Ŀ���������
		string tar_arg2_name = target_code_generator_.SetArgBeReady(symbol_expression.pos);
		string tar_arg1_name = target_code_generator_.SetArgBeReady(sp);
		target_code_generator_.PrintQuadruples(Instructions("move", tar_arg1_name, tar_arg2_name, ""));

		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for (int i = 0; i < length; ++i)
			symbol_info_stack.pop_back();

		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = "";//�����Ϣûʲô�ã��������ӽ�ȥ�������Ժ���õ�
		symbol_info_stack.push_back(conclude_syble_info);
	}
	
	else if ("Create_Function_table" == production.left)//Create_Function_table->$
	{
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = "";
		
		
		//Stmt->int identifier Stmt_type | void identifier Create_Function_table Function_stmt
		//��ʱsymbol_info_stack�����һ��Ϊidentifier�������������ơ�
		GrammarSymbolInfo symbol_identifier = symbol_info_stack.back();
		
		//����ȫ�ַ��ű��в鿴��������Ƿ񱻶������
		if (-1 != symbol_tables_[0].FindSymbol(symbol_identifier.txt_value))
		{
			cerr << "������󣡣���" << symbol_identifier.txt_value << "�ظ����壡" << production.left << endl;
			return false;
		}

		//CreateSymbolTable(FUNCTION_TABLE, symbol_identifier.txt_value);//�����µĺ������ű�������
		SymbolTable new_table(FUNCTION_TABLE, symbol_identifier.txt_value);
		symbol_tables_.push_back(new_table);
		//�Ѻ�������global_table�е�һ�����ţ�������
		Symbol new_function_symbol;
		new_function_symbol.mode = FUNCTION;
		new_function_symbol.name = symbol_identifier.txt_value;
		new_function_symbol.type = VOID;
		new_function_symbol.func_symbol_pos = symbol_tables_.size() - 1;
		symbol_tables_[0].AddSymbol(new_function_symbol);

		
		//�����µķ��ű���
		current_symbol_table_stack_.push_back(symbol_tables_.size() - 1);

		//�Ѻ�������ֵ��Ϊ��0����ӵ������ķ��ű���
		GrammarSymbolInfo symbol_return_type = symbol_info_stack[symbol_info_stack.size() - 2]; //int ���� void

		//���巵��ֵΪ���������뺯�����ű�ĵ�0��---
		Symbol variable_symbol;
		variable_symbol.mode = RETURN_VAR;//����
		variable_symbol.name = symbol_tables_[current_symbol_table_stack_.back()].GetTableName() + "-return_value";//����
		if (symbol_return_type.txt_value=="int")
			variable_symbol.type = INT;
		else if(symbol_return_type.txt_value == "void")
			variable_symbol.type = VOID;
		else {
			cerr << "������������ֻ����int �� void�������������! " << production.left << endl;
			return false;
		}
		variable_symbol.value = "";//����ֵ��û�б���ֵ���˴�Ϊ��
		
		//�鿴�Ƿ�ΪMain,�ǵĻ����ñ�ǵȴ���ת
		if (symbol_identifier.txt_value == "main")
			main_line_ = PeekNextStateNum();
		//�����������
		PrintQuadruples(Quadruples(GetNextStateNum(), symbol_identifier.txt_value, "-", "-", "-"));

		//������ֵ������ű� pos =(table_pos, 0)
		symbol_tables_[current_symbol_table_stack_.back()].AddSymbol(variable_symbol);

		//�Ҳ�Ϊ$�����Բ���pop
		symbol_info_stack.push_back(conclude_syble_info);

		//Ŀ���������
		//���ɺ������ı�ǩ
		target_code_generator_.PrintQuadruples(symbol_identifier.txt_value + " :");
		//�������ã������º������Ƚ���ջ֡����
		target_code_generator_.CreateStackFrame();

		//ѭ����ǩ�Ĵ洢
		w_j_label_nums.push_back(w_j_label_num_);
		w_j_label_num_ = 0;
	}

	else if ("Exit_Function_table" == production.left)//->$
	{
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = "";
		//Stmt->int identifier Stmt_type | void identifier Create_Function_table Function_stmt
		//Function_stmt->(Formal_parameter)Sentence_block Exit_Function_table
		GrammarSymbolInfo symbol_identifier = symbol_info_stack[symbol_info_stack.size() - 6];

		//�ӵ�ǰ���ű�ջ�е���
		current_symbol_table_stack_.pop_back();

		//�Ҳ�Ϊ$�����Բ���pop
		symbol_info_stack.push_back(conclude_syble_info);

		//ѭ����ǩ�Ļָ�
		w_j_label_num_ = w_j_label_nums.back();
		w_j_label_nums.pop_back();

		target_code_generator_.ClearRegs();//�˳�ǰ����ռĴ���


		if (symbol_identifier.txt_value != "main")
			target_code_generator_.PrintQuadruples(Instructions("jr", "$ra", "", ""));//�������ָ��
		else//main��������Ҫ���⴦��
			target_code_generator_.PrintSyscall();//main���������
	}

	
	else if ("Variavle_stmt" == production.left)//Variavle_stmt->;
	{
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = ";";
		
		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for (int i = 0; i < length; ++i)
			symbol_info_stack.pop_back();
		symbol_info_stack.push_back(conclude_syble_info);
	}
	
	else if ("Stmt_type" == production.left && production.right[0] == "Variavle_stmt")//Stmt_type->Variavle_stmt
	{
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = ";";

		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for (int i = 0; i < length; ++i)
			symbol_info_stack.pop_back();
		symbol_info_stack.push_back(conclude_syble_info);
	}


	else if ("Stmt" == production.left && production.right[0] == "int")//Stmt->int identifier Stmt_type
	{
		//��������
		GrammarSymbolInfo symbol_identifier = symbol_info_stack[symbol_info_stack.size() - 2];
		GrammarSymbolInfo symbol_Stmt_type = symbol_info_stack.back();

		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		if (symbol_Stmt_type.txt_value == ";")//��ʾ��������������Ǻ���
		{
			//�ڷ��ű�ջ��һ��������Ѱ�� identifier�Ƿ񱻶����
			int identifier_pos = -1;
			int identifier_layer = current_symbol_table_stack_.size() - 1;
			for (; identifier_layer >= 0; --identifier_layer)
			{
				SymbolTable search_table = symbol_tables_[current_symbol_table_stack_[identifier_layer]];
				identifier_pos = search_table.FindSymbol(symbol_identifier.txt_value);
				if (identifier_pos != -1)//��ʾ��ĳһ�����ҵ���
					break;
			}


			if (-1 != identifier_pos)//�ҵ��ˣ���ʾ���ڣ�˵���ظ����壬�������
			{
				cerr << "������󣡣���" << symbol_identifier.txt_value << "�ظ����壡" << production.left << endl;
				return false;
			}

			//û�б��������������ű���
			Symbol variable_symbol;
			variable_symbol.mode = VARIABLE;//����
			variable_symbol.name = symbol_identifier.txt_value;//����
			variable_symbol.type = INT;
			variable_symbol.value = "";//����ֵ��û�б���ֵ���˴�Ϊ��

			int symbol_pos = symbol_tables_[current_symbol_table_stack_.back()].AddSymbol(variable_symbol);//������ű���
			target_code_generator_.PushStack_sp(1);//����һ���ֲ�������push��ջ��$sp=$sp+1*4

			conclude_syble_info.txt_value = symbol_identifier.txt_value;

		}
		else {
			;//���庯��
			//�޲����� �����������Ѿ��洢����ֵ�Ĳ�����Create_Function_table�Ĺ�Լ����ɡ�
			//Stmt_type->Variavle_stmt | Create_Function_table Function_stmt
			//Create_Function_table->$
		}

		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for (int i = 0; i < length; ++i)
			symbol_info_stack.pop_back();
		symbol_info_stack.push_back(conclude_syble_info);
	}
	

	
	else if ("Return_expression" == production.left && production.right[0] == "$")//Return_expression->$
	{
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = "";//�գ��������Ϊ��

		//����������£�����ֵΪvoid�� ����Ҫ�жϵ�ǰ���ű��0����Ŀ�����͡�
		//�жϷ��������Ƿ���ȷ
		if (VOID != symbol_tables_[current_symbol_table_stack_.back()].GetSymbolType(0))
		{
			//�������ʹ������⡣
			cerr << "������󣡣���" << symbol_tables_[current_symbol_table_stack_.back()].GetTableName() 
				<< "��������ķ���ֵΪint���͡� ����return��䷵��void���ͣ�" << endl;
			return false;
		}
		
		//symbol_info_stack.pop_back();
		symbol_info_stack.push_back(conclude_syble_info);
	}

	else if ("Return_expression" == production.left && production.right[0] == "Expression")//Return_expression->Expression
	{

		//����������£�����ֵΪint�� ����Ҫ�жϵ�ǰ���ű��0����Ŀ�����͡�
		//�жϷ��������Ƿ���ȷ
		if (INT != symbol_tables_[current_symbol_table_stack_.back()].GetSymbolType(0))
		{
			//�������ʹ������⡣
			cerr << "������󣡣���" << symbol_tables_[current_symbol_table_stack_.back()].GetTableName() 
				<< "��������ķ���ֵΪvoid���͡� ����return��䷵��int���ͣ�" << endl;
			return false;
		}
		
		GrammarSymbolInfo symbol_expression = symbol_info_stack.back();
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = "int";//��ʾ��ֵ���أ�������""
		conclude_syble_info.pos = symbol_expression.pos;

		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for (int i = 0; i < length; ++i)
			symbol_info_stack.pop_back();
		symbol_info_stack.push_back(conclude_syble_info);

		//Ŀ���������
		//string tar_arg2_name = target_code_generator_.SetArgBeReady(symbol_expression.pos);
		string tar_arg_name = target_code_generator_.SetArgBeReady(symbol_expression.pos);
		target_code_generator_.PrintQuadruples(Instructions("move", "$v0", tar_arg_name, ""));//�ѷ���ֵ����v0
		target_code_generator_.PrintQuadruples(Instructions("lw", "$ra", "-4($fp)", ""));//�ѷ��ص�ַ����ra
	}

	else if ("Return_sentence" == production.left )//Return_sentence->return Return_expression ;
	{
		GrammarSymbolInfo symbol_return_expression = symbol_info_stack[symbol_info_stack.size() -2];

		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;

		SymbolTable &current_function_table = symbol_tables_[current_symbol_table_stack_.back()];//��ȡ��ǰ������
		int function_pos = symbol_tables_[0].FindSymbol(current_function_table.GetTableName());//������ȫ�ַ��ű��λ��
		Symbol &current_function_symbol = symbol_tables_[0].GetSymbol(function_pos);//������ȫ�ַ��ű�����Ϊ����

		if ("" != symbol_return_expression.txt_value)
		{
			SymbolPos result_pos;
			result_pos.table_pos = current_symbol_table_stack_.back();
			result_pos.symbol_pos = 0;//����ֵ��λ���� ��0��
			string result_name = GetArgName(result_pos);
			string arg1_name = GetArgName(symbol_return_expression.pos);

			//�ѷ���ֵд��������ĵ�0��
			PrintQuadruples(Quadruples(GetNextStateNum(), ":=", arg1_name, "-", result_name));


			conclude_syble_info.txt_value = "int";
			conclude_syble_info.pos = symbol_return_expression.pos;
		}
		else
			conclude_syble_info.txt_value = "";
		//�������ָ��
		PrintQuadruples(Quadruples(GetNextStateNum(), "return", "-", "-", current_function_table.GetTableName()));


		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for (int i = 0; i < length; ++i)
			symbol_info_stack.pop_back();
		symbol_info_stack.push_back(conclude_syble_info);
	}

	else if ("Parameter" == production.left )//Parameter->int identifier
	{
		GrammarSymbolInfo symbol_identifier = symbol_info_stack.back();
		//����ע��������
		SymbolTable &current_table = symbol_tables_[current_symbol_table_stack_.back()];

		int table_pos = symbol_tables_[0].FindSymbol(current_table.GetTableName());


		//���ڵ�ǰ���ű���Ѱ�� identifier�Ƿ񱻶����
		if (-1 != current_table.FindSymbol(symbol_identifier.txt_value))//�ҵ�����ʾ�Ѵ��ڣ�˵���ظ����壬�������
		{
			cerr << "������󣡣���" << symbol_identifier.txt_value << "�������� �ظ����壡" << endl;
			return false;
		}

		//����ȫ�ַ��ű��в鿴������뺯�������ͻ
		int search_symbol_pos = symbol_tables_[0].FindSymbol(symbol_identifier.txt_value);
		if (-1 != search_symbol_pos && symbol_tables_[0].GetSymbolMode(search_symbol_pos) == FUNCTION)
		{
			cerr << "������󣡣���" << symbol_identifier.txt_value << "�Ѿ�������Ϊ������" << production.left << endl;
			return false;
		}
		
		//û�б��������������ű���
		Symbol variable_symbol;
		variable_symbol.mode = VARIABLE;//����
		variable_symbol.name = symbol_identifier.txt_value;//����
		variable_symbol.type = INT;
		variable_symbol.value = "";//����ֵ��û�б���ֵ���˴�Ϊ��
		
		int symbol_pos = current_table.AddSymbol(variable_symbol);//������ű���

		//ȫ�ַ��ű��к����Ĳ���������Ҫ��һ   GetSymbol�������ص�������
		symbol_tables_[0].GetSymbol(table_pos).parameter_num++;

		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for(int i = 0; i < length;++i)
			symbol_info_stack.pop_back();
		
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;

		//�������������û��ʲô�ô�
		conclude_syble_info.txt_value = symbol_identifier.txt_value; //����������
		conclude_syble_info.pos = { current_symbol_table_stack_.back(), symbol_pos };//���ڷ��ű��˳���ڷ��ű��е�λ�ô�����

		symbol_info_stack.push_back(conclude_syble_info);
		
	}
	
	else if ("Sentence_block" == production.left)//Sentence_block->Sentence_block_m { Internal_stmt Sentence_string }
	{	
		//������һ��state����š�������ʹ��
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = std::to_string(PeekNextStateNum());

		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for(int i = 0; i < length;++i)
			symbol_info_stack.pop_back();

		symbol_info_stack.push_back(conclude_syble_info);
	}

	//else if ("Sentence_block_m" == production.left)//Sentence_block_m->$
	//{
	//	//��Ҫ������ռĴ���
	//	//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
	//	GrammarSymbolInfo conclude_syble_info;
	//	conclude_syble_info.symbol_name = production.left;
	//	conclude_syble_info.txt_value = "";
	//
	//
	//	//��ȡ����ʽ�Ҳ��ĳ���
	//	int length = 0;
	//	if ("$" != production.right[0])
	//		length = production.right.size();
	//
	//	for (int i = 0; i < length; ++i)
	//		symbol_info_stack.pop_back();
	//
	//	symbol_info_stack.push_back(conclude_syble_info);
	//
	//	target_code_generator_.ClearRegs();//����ѭ��ǰ����ռĴ���
	//}
	//

	

	else if ("While_sentence_m2" == production.left)//While_sentence_m2->$
	{	
	//While_sentence->while While_sentence_m1 ( Expression ) While_sentence_m2 Sentence_block
			
		GrammarSymbolInfo symbol_expression = symbol_info_stack[symbol_info_stack.size() -2 ];
		string symbol_name = symbol_tables_[symbol_expression.pos.table_pos].GetSymbolName(symbol_expression.pos.symbol_pos);

		//�����ȴ��������ٳ���
		PrintQuadruples(Quadruples(GetNextStateNum(), "j=", symbol_name, "0", "---j="));
		backpatching_point_pos_.push_back(quadruples_stack_.size() - 1);

		PrintQuadruples(Quadruples(GetNextStateNum(), "j", "-", "-", "---j"));
		backpatching_point_pos_.push_back(quadruples_stack_.size() - 1);

		//int next_state_num = PeekNextStateNum();
		//backpatching_value.push_back(next_state_num);
		
		//������һ��state����š�������ʹ��
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = std::to_string(PeekNextStateNum());

		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for(int i = 0; i < length;++i)
			symbol_info_stack.pop_back();

		symbol_info_stack.push_back(conclude_syble_info);

		//Ŀ���������
		string tar_arg1_name = target_code_generator_.SetArgBeReady(symbol_expression.pos);
		string j_label_end = j_label_.back();
		j_label_.pop_back();
		//��������0������ѭ��
		target_code_generator_.PrintQuadruples(Instructions("beq", tar_arg1_name, "$zero", j_label_end));
	
	}


	else if ("While_sentence_m1" == production.left)//While_sentence_m1->$
	{	
		//������һ��state����š�������ʹ��
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = std::to_string(PeekNextStateNum());

		//ͬʱ������㼶+1
		backpatching_level_++;

		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for(int i = 0; i < length;++i)
			symbol_info_stack.pop_back();

		symbol_info_stack.push_back(conclude_syble_info);

		//��������ǩ���ɺ�ѹ��ջ��
		string table_name = symbol_tables_[current_symbol_table_stack_.back()].GetTableName();
		int cnt = w_j_label_num_++;
		string label_1 = "Label_" + table_name + "_while_begin_" + std::to_string(cnt);
		string label_2 = "Label_" + table_name + "_while_end_" + std::to_string(cnt);
		//��дbegin
		w_label_.push_back(label_2);
		w_label_.push_back(label_1);
		//����end
		j_label_.push_back(label_1);
		j_label_.push_back(label_2);
		//Ŀ���������
		string w_label_begin = w_label_.back();
		w_label_.pop_back();

		target_code_generator_.PrintQuadruples("\n#whileѭ��������ռĴ���");//��һ��
		target_code_generator_.ClearRegs();//����ѭ��ǰ����ռĴ���
		//����while��begin��ǩ
		target_code_generator_.PrintQuadruples(w_label_begin+" :");

	}
	
	else if ("While_sentence" == production.left)
	//While_sentence->while While_sentence_m1 ( Expression ) While_sentence_m2 Sentence_block
	{		
		//��ת��while���ж������������Ԫʽ���뵽����ջ��
		GrammarSymbolInfo symbol_while_sentence_m1 = symbol_info_stack[symbol_info_stack.size() - 6];
		GrammarSymbolInfo symbol_while_sentence_m2 = symbol_info_stack[symbol_info_stack.size() - 2];
		GrammarSymbolInfo symbol_sentence_block = symbol_info_stack.back();
		PrintQuadruples(Quadruples(GetNextStateNum(), "j", "-", "-", symbol_while_sentence_m1.txt_value));

		//��ʼ���� �����ط�
		//�����
		int batch_pos = backpatching_point_pos_.back();
		backpatching_point_pos_.pop_back();
		quadruples_stack_[batch_pos].SetContent(quadruples_stack_[batch_pos].num, quadruples_stack_[batch_pos].op,
			quadruples_stack_[batch_pos].arg1, quadruples_stack_[batch_pos].arg2, symbol_while_sentence_m2.txt_value);
		//�ٳ���
		batch_pos = backpatching_point_pos_.back();
		backpatching_point_pos_.pop_back();
		quadruples_stack_[batch_pos].SetContent(quadruples_stack_[batch_pos].num, quadruples_stack_[batch_pos].op,
			quadruples_stack_[batch_pos].arg1, quadruples_stack_[batch_pos].arg2, std::to_string(PeekNextStateNum()));
		

		--backpatching_level_;//������ȼ�1
		//�ж��Ƿ���Ҫ���
		if (backpatching_level_ == 0)
		{
			for (auto iter = quadruples_stack_.begin(); iter != quadruples_stack_.end(); ++iter)
				PrintQuadruples(*iter);
			int length = quadruples_stack_.size();
			for (int i = 0; i < length; ++i)//����ջȫ����������������
				quadruples_stack_.pop_back();
		}

		/*int next_state_num = PeekNextStateNum();*/

		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = "";

		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for(int i = 0; i < length;++i)
			symbol_info_stack.pop_back();

		symbol_info_stack.push_back(conclude_syble_info);

		//Ŀ���������
		target_code_generator_.ClearRegs();//�˳�ѭ��ǰ����ռĴ���
		string j_label_begin = j_label_.back();
		j_label_.pop_back();
		target_code_generator_.PrintQuadruples("j " + j_label_begin);
		string w_label_end = w_label_.back();
		w_label_.pop_back();

		target_code_generator_.PrintQuadruples(w_label_end + " :");
		//��ռĴ��� --��֤��if�Ǹɾ���
		target_code_generator_.ClearRegs();
		target_code_generator_.PrintQuadruples("");//��һ��
		

	}

	else if ("If_sentence_m0" == production.left)//If_sentence_m0->$
	{	
		//��Ҫ��������������
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = std::to_string(PeekNextStateNum());

		//ͬʱ������㼶+1
		backpatching_level_++;

		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for(int i = 0; i < length;++i)
			symbol_info_stack.pop_back();

		symbol_info_stack.push_back(conclude_syble_info);

		target_code_generator_.PrintQuadruples("\n#if�ж�");//��һ��
		//target_code_generator_.PrintQuadruples("\n#if�жϣ�����ռĴ���");//��һ��
		//target_code_generator_.ClearRegs();//����ѭ��ǰ����ռĴ���
	}

	else if ("If_sentence_n" == production.left)//If_sentence_n->$ 
	{	
		//�����ת��if-else�����ĵط�,����ִ��else
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		
		//�ȴ��������������
		PrintQuadruples(Quadruples(GetNextStateNum(), "j", "-", "-", "---j-if-n"));
		backpatching_point_pos_.push_back(quadruples_stack_.size() - 1);

		conclude_syble_info.txt_value = std::to_string(PeekNextStateNum());//��if-else�ļٳ��ڵ�ַ���д���

		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for(int i = 0; i < length;++i)
			symbol_info_stack.pop_back();

		symbol_info_stack.push_back(conclude_syble_info);

		//Ŀ���������
		//ֻ���j else_end  �Լ� label if_end
		string w_label_if_end = w_label_.back();
		w_label_.pop_back();
		string j_label_else_end = j_label_.back();
		j_label_.pop_back();

		//��ռĴ��� --��֤��if�Ǹɾ���
		target_code_generator_.ClearRegs();
		//ע�������˳������ת���������ǩ
		//if������Ҫ��ת��else_end���Ӷ�����ִ��else
		target_code_generator_.PrintQuadruples(Instructions("j", j_label_else_end, "", ""));
		//����if_end��ǩ
		target_code_generator_.PrintQuadruples(w_label_if_end + " :");
	}

	else if ("If_sentence_m1" == production.left)//If_sentence_m1->$
	{	
	//If_sentence->if If_sentence_m0 ( Expression ) If_sentence_m1 Sentence_block If_expression
			
		GrammarSymbolInfo symbol_expression = symbol_info_stack[symbol_info_stack.size() -2 ];
		string symbol_name = symbol_tables_[symbol_expression.pos.table_pos].GetSymbolName(symbol_expression.pos.symbol_pos);

		//�����ȴ��������ٳ���
		PrintQuadruples(Quadruples(GetNextStateNum(), "j=", symbol_name, "0", "---j="));
		backpatching_point_pos_.push_back(quadruples_stack_.size() - 1);

		PrintQuadruples(Quadruples(GetNextStateNum(), "j", "-", "-", "---j"));
		backpatching_point_pos_.push_back(quadruples_stack_.size() - 1);

		
		//������һ��state����š�������ʹ��
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = std::to_string(PeekNextStateNum());

		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for(int i = 0; i < length;++i)
			symbol_info_stack.pop_back();

		symbol_info_stack.push_back(conclude_syble_info);


		//��������ǩ���ɺ�ѹ��ջ��
		string table_name = symbol_tables_[current_symbol_table_stack_.back()].GetTableName();
		int cnt = w_j_label_num_++;
		string label_1 = "Label_" + table_name + "_if_end_" + std::to_string(cnt);
		string label_2 = "Label_" + table_name + "_else_end_" + std::to_string(cnt);
		//if_end��д
		w_label_.push_back(label_2);
		w_label_.push_back(label_1);
		//if_end����
		j_label_.push_back(label_2);
		j_label_.push_back(label_1);
		//Ŀ���������
		string j_label_if_end = j_label_.back();
		j_label_.pop_back();

		////��ռĴ��� --��֤����if����else���Ǹɾ���
		//target_code_generator_.ResetRegs();
		//Ŀ���������
		string tar_arg1_name = target_code_generator_.SetArgBeReady(symbol_expression.pos);

		target_code_generator_.ClearRegs();//����ѭ��ǰ����ռĴ���
		//��������0������if
		target_code_generator_.PrintQuadruples(Instructions("beq", tar_arg1_name, "$zero", j_label_if_end));

	}

	else if ("If_expression" == production.left && production.right[0] == "$")//If_expression->$
	{	
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = "";//����Ϊ��


		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for(int i = 0; i < length;++i)
			symbol_info_stack.pop_back();

		symbol_info_stack.push_back(conclude_syble_info);

		//Ŀ���������
		//ֻ���һ��if_end��label���Ѷ����w_label��w_label��ջ�е�����
		string w_label_if_end = w_label_.back();
		w_label_.pop_back();
		w_label_.pop_back();//����else_end����Ϊ����if����else�����
		j_label_.pop_back();//����else_end��ͬ��

		//��ռĴ��� --��if�Ǹɾ���
		target_code_generator_.ClearRegs();
		//����if_end��ǩ
		target_code_generator_.PrintQuadruples(w_label_if_end + " :");
	}

	else if ("If_expression" == production.left && production.right[0] == "If_sentence_n")
	//If_expression->If_sentence_n else Sentence_block
	{	
		GrammarSymbolInfo symbol_if_sentence_n = symbol_info_stack[symbol_info_stack.size() - 3];
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = symbol_if_sentence_n.txt_value; //����һ�� ��

		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for(int i = 0; i < length;++i)
			symbol_info_stack.pop_back();

		symbol_info_stack.push_back(conclude_syble_info);

		//Ŀ���������
		//ֻ���һ��else_end��label
		string w_label_else_end = w_label_.back();
		w_label_.pop_back();

		//��ռĴ��� --��֤��else�Ǹɾ���
		target_code_generator_.ClearRegs();
		//����if_end��ǩ
		target_code_generator_.PrintQuadruples(w_label_else_end + " :");
	}

	else if ("If_sentence" == production.left)
	//If_sentence->if If_sentence_m0(Expression) If_sentence_m1 Sentence_block If_expression
	{		
		GrammarSymbolInfo symbol_if_sentence_m1 = symbol_info_stack[symbol_info_stack.size() - 3];
		GrammarSymbolInfo symbol_if_expression = symbol_info_stack.back();

		if ("" == symbol_if_expression.txt_value)//ֻ��if��û��else
		{
			//��ʼ���� �����ط�
			//�����
			int batch_pos = backpatching_point_pos_.back();
			backpatching_point_pos_.pop_back();
			quadruples_stack_[batch_pos].SetContent(quadruples_stack_[batch_pos].num, quadruples_stack_[batch_pos].op,
				quadruples_stack_[batch_pos].arg1, quadruples_stack_[batch_pos].arg2, symbol_if_sentence_m1.txt_value);
			//�ٳ���
			batch_pos = backpatching_point_pos_.back();
			backpatching_point_pos_.pop_back();
			quadruples_stack_[batch_pos].SetContent(quadruples_stack_[batch_pos].num, quadruples_stack_[batch_pos].op,
				quadruples_stack_[batch_pos].arg1, quadruples_stack_[batch_pos].arg2, std::to_string(PeekNextStateNum()));
		}
		else//if-else���
		{
			//��ʼ���� �����ط�
			//���if������if-else
			int batch_pos = backpatching_point_pos_.back();
			backpatching_point_pos_.pop_back();
			quadruples_stack_[batch_pos].SetContent(quadruples_stack_[batch_pos].num, quadruples_stack_[batch_pos].op,
				quadruples_stack_[batch_pos].arg1, quadruples_stack_[batch_pos].arg2, std::to_string(PeekNextStateNum()));
			
			//if�����
			batch_pos = backpatching_point_pos_.back();
			backpatching_point_pos_.pop_back();
			quadruples_stack_[batch_pos].SetContent(quadruples_stack_[batch_pos].num, quadruples_stack_[batch_pos].op,
				quadruples_stack_[batch_pos].arg1, quadruples_stack_[batch_pos].arg2, symbol_if_sentence_m1.txt_value);

			//if�ٳ���
			batch_pos = backpatching_point_pos_.back();
			backpatching_point_pos_.pop_back();
			quadruples_stack_[batch_pos].SetContent(quadruples_stack_[batch_pos].num, quadruples_stack_[batch_pos].op,
				quadruples_stack_[batch_pos].arg1, quadruples_stack_[batch_pos].arg2, symbol_if_expression.txt_value);
		}

		--backpatching_level_;//������ȼ�1
		//�ж��Ƿ���Ҫ���
		if (backpatching_level_ == 0)
		{
			for (auto iter = quadruples_stack_.begin(); iter != quadruples_stack_.end(); ++iter)
				PrintQuadruples(*iter);
			int length = quadruples_stack_.size();
			for (int i = 0; i < length; ++i)//����ջȫ����������������
				quadruples_stack_.pop_back();
		}

		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = "";

		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for(int i = 0; i < length;++i)
			symbol_info_stack.pop_back();

		symbol_info_stack.push_back(conclude_syble_info);

		//Ŀ���������
		//target_code_generator_.ClearRegs();//�˳�ifǰ����ռĴ���
	}

	
	else if ("Call_func_check" == production.left)//Call_func_check->$
	{
		//�������飬�Ƿ�Ϊ�Ϸ��ĺ�����
		//Factor->identifier ( Call_func_check Actual_parameter_list)
		//��������
		GrammarSymbolInfo symbol_identifier = symbol_info_stack[symbol_info_stack.size() - 2];


		//�ڷ��ű�ջ��һ��������Ѱ�� identifier�Ƿ񱻶����
		int identifier_pos = -1;
		int identifier_layer = current_symbol_table_stack_.size() - 1;
		for (; identifier_layer >= 0; --identifier_layer)
		{
			SymbolTable search_table = symbol_tables_[current_symbol_table_stack_[identifier_layer]];
			identifier_pos = search_table.FindSymbol(symbol_identifier.txt_value);
			if (identifier_pos != -1)//��ʾ��ĳһ�����ҵ���
				break;
		}
		if (-1 == identifier_pos )//û�ҵ�����ʾ�����ڣ�˵��û�����ʹ�ã��������
		{
			cerr << "������󣡣���" << symbol_identifier.txt_value << "û�ж��壡" << endl;
			return false;
		}

		if (FUNCTION != symbol_tables_[current_symbol_table_stack_[identifier_layer]].GetSymbolMode(identifier_pos))
		//�ҵ��ˣ����������ʶ�����Ǻ������������
		{
			cerr << "������󣡣���" << symbol_identifier.txt_value << "���Ǻ������޷������ã�" << endl;
			return false;
		}

		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = "";
		conclude_syble_info.pos = { current_symbol_table_stack_[identifier_layer], identifier_pos };//�洢�����ú�����λ��

		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for(int i = 0; i < length;++i)
			symbol_info_stack.pop_back();

		symbol_info_stack.push_back(conclude_syble_info);
	}
	

	else if ("Expression_loop" == production.left && production.right[0] == "$")//Expression_loop->$
	{	
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = "0";//����Ϊ0����ʾ�Ѿ�������0������


		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for(int i = 0; i < length;++i)
			symbol_info_stack.pop_back();

		symbol_info_stack.push_back(conclude_syble_info);
	}
	

	else if ("Expression_loop" == production.left && production.right[0] == "Expression_loop")
	//Expression_loop->Expression_loop Expression ,
	{	
	//Factor->identifier ( Call_func_check Expression_loop Expression ,
		//��������
		GrammarSymbolInfo symbol_call_func_check = symbol_info_stack[symbol_info_stack.size() - 4];
		GrammarSymbolInfo symbol_expression_loop = symbol_info_stack[symbol_info_stack.size() - 3];
		GrammarSymbolInfo symbol_expression = symbol_info_stack[symbol_info_stack.size() - 2];
		Symbol &called_function = symbol_tables_[symbol_call_func_check.pos.table_pos].GetSymbol(symbol_call_func_check.pos.symbol_pos);
		int parameters_num = called_function.parameter_num;
		int passed_parameters_num = std::stoi(symbol_expression_loop.txt_value);
		if (passed_parameters_num >= parameters_num)
		{//�������ݹ��࣡
			cerr << "������󣡣���" << called_function.name << "���ݵĲ������࣡" << endl;
			return false;
		}

		int table_pos = called_function.func_symbol_pos;
		
		SymbolPos result_pos;
		result_pos.table_pos = table_pos;
		result_pos.symbol_pos = passed_parameters_num + 1;//��0���Ƿ���ֵ

		string result_name = GetArgName(result_pos, true);

		string arg1_name = GetArgName(symbol_expression.pos);
		
		PrintQuadruples(Quadruples(GetNextStateNum(), ":=", arg1_name, "-", result_name));

		passed_parameters_num++;
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = std::to_string(passed_parameters_num);


		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for(int i = 0; i < length;++i)
			symbol_info_stack.pop_back();

		symbol_info_stack.push_back(conclude_syble_info);

		//Ŀ���������
		//string tar_arg2_name = target_code_generator_.SetArgBeReady(symbol_expression.pos);
		string tar_arg1_name = target_code_generator_.SetArgBeReady(symbol_expression.pos);//��ȡʵ�ε��ڴ��ַ����
		target_code_generator_.PrintQuadruples(Instructions("sw", tar_arg1_name, "($sp)", ""));
		target_code_generator_.PushStack_sp(1);//������sp�������ʵ��,���sp��Ҫ�ƶ�1*4

	}
	

	else if ("Actual_parameter_list" == production.left && production.right[0] == "Expression_loop")
	//Actual_parameter_list->Expression_loop Expression
	{	
	//Factor->identifier ( Call_func_check Expression_loop Expression
		//��������
		GrammarSymbolInfo symbol_call_func_check = symbol_info_stack[symbol_info_stack.size() - 3];
		GrammarSymbolInfo symbol_expression_loop = symbol_info_stack[symbol_info_stack.size() - 2];
		GrammarSymbolInfo symbol_expression = symbol_info_stack.back();
		Symbol &called_function = symbol_tables_[symbol_call_func_check.pos.table_pos].GetSymbol(symbol_call_func_check.pos.symbol_pos);
		int parameters_num = called_function.parameter_num;
		int passed_parameters_num = std::stoi(symbol_expression_loop.txt_value);
		if (passed_parameters_num >= parameters_num)
		{//�������ݹ��࣡
			cerr << "������󣡣���" << called_function.name << "���ݵĲ������࣡" << endl;
			return false;
		}

		int table_pos = called_function.func_symbol_pos;
		
		SymbolPos result_pos;
		result_pos.table_pos = table_pos;
		result_pos.symbol_pos = passed_parameters_num + 1;//��0���Ƿ���ֵ

		string result_name = GetArgName(result_pos, true);

		string arg1_name = GetArgName(symbol_expression.pos);
		
		PrintQuadruples(Quadruples(GetNextStateNum(), ":=", arg1_name, "-", result_name));

		passed_parameters_num++;


		//�ж��Ƿ�������٣�
		if (passed_parameters_num < parameters_num)
		{//�������ݹ��࣡
			cerr << "������󣡣���" << called_function.name << "���ݵĲ������٣�" << endl;
			return false;
		}

		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = std::to_string(passed_parameters_num);


		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for(int i = 0; i < length;++i)
			symbol_info_stack.pop_back();

		symbol_info_stack.push_back(conclude_syble_info);

		//Ŀ���������
		string tar_arg1_name = target_code_generator_.SetArgBeReady(symbol_expression.pos);//��ȡʵ�ε��ڴ��ַ����
		target_code_generator_.PrintQuadruples(Instructions("sw", tar_arg1_name, "($sp)", ""));
		target_code_generator_.PushStack_sp(1);//������sp�������ʵ��,���sp��Ҫ�ƶ�1*4
	}
	
	else if ("Actual_parameter_list" == production.left && production.right[0] == "$")//Actual_parameter_list->$
	{
		//Call_func->( Call_func_check 
		//��������
		GrammarSymbolInfo symbol_call_func_check = symbol_info_stack.back();
		Symbol &called_function = symbol_tables_[symbol_call_func_check.pos.table_pos].GetSymbol(symbol_call_func_check.pos.symbol_pos);
		int parameters_num = called_function.parameter_num;
		/*int passed_parameters_num = */
		if (0 != parameters_num)
		{//�������ݹ��࣡
			cerr << "������󣡣���" << called_function.name << "�����ĵ�����Ҫ����������û�д��������" << endl;
			return false;
		}
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = "0";//����Ϊ0����ʾ�Ѿ�������0������


		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for(int i = 0; i < length;++i)
			symbol_info_stack.pop_back();

		symbol_info_stack.push_back(conclude_syble_info);
	}

	else if ("Call_func" == production.left)//Call_func->(Call_func_check Actual_parameter_list)
	{
	//���call���
		//Factor->identifier ( Call_func_check Actual_parameter_list)
		//��������
		GrammarSymbolInfo symbol_identifier = symbol_info_stack[symbol_info_stack.size() - 5];
		GrammarSymbolInfo symbol_call_func_check = symbol_info_stack[symbol_info_stack.size() - 3];
		PrintQuadruples(Quadruples(GetNextStateNum(), "call", "-", "-", symbol_identifier.txt_value));
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = "";
		conclude_syble_info.pos = symbol_call_func_check.pos;


		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for(int i = 0; i < length;++i)
			symbol_info_stack.pop_back();

		symbol_info_stack.push_back(conclude_syble_info);
	}
	
	else if ("FTYPE" == production.left && production.right[0] == "Call_func")//FTYPE->Call_func
	{
		GrammarSymbolInfo symbol_call_func = symbol_info_stack.back();
		//�����ú�������posһ�������ϴ���
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = "call_func";//����ֻҪ����""�Ϳ���
		conclude_syble_info.pos = symbol_call_func.pos;
		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for (int i = 0; i < length; ++i)
			symbol_info_stack.pop_back();
		symbol_info_stack.push_back(conclude_syble_info);
	}
	
	else if ("Program" == production.left)//Program->Stmt_string
	{
		//������������������ȷ��
		//�ж��Ƿ����Main������
		if (main_line_ == -1)
		{
			cerr << "������󣡣���" << "main����û�ж��壡" << endl;
			return false;
		}
		PrintQuadruples();
		cerr << "�м����������ϣ�" << endl;
		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = "";
		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for (int i = 0; i < length; ++i)
			symbol_info_stack.pop_back();
		symbol_info_stack.push_back(conclude_syble_info);
	}

	else
	{
		
		//Stmt_string->Stmt_loop Stmt
		//Stmt_loop->Stmt_loop Stmt | $
		//Internal_stmt->$|Internal_variable_stmt ; Internal_stmt
		//Sentence_string->Sentence_loop Sentence
		//Sentence_loop->Sentence_loop Sentence | $
		//Sentence->If_sentence | While_sentence | Return_sentence | Assign_sentence
		//Stmt->void identifier Create_Function_table Function_stmt
		//Formal_parameter->Formal_parameter_list | void | $
		//Formal_parameter_list->Parameter_loop Parameter
		//Parameter_loop->Parameter_loop Parameter, | $
		//Function_stmt->( Formal_parameter ) Sentence_block Exit_Function_table


		//�ɹ�Լ��Ĳ���ʽ�󲿹���һ���ķ���������
		GrammarSymbolInfo conclude_syble_info;
		conclude_syble_info.symbol_name = production.left;
		conclude_syble_info.txt_value = "";
		//��ȡ����ʽ�Ҳ��ĳ���
		int length = 0;
		if ("$" != production.right[0])
			length = production.right.size();

		for (int i = 0; i < length; ++i)
			symbol_info_stack.pop_back();
		symbol_info_stack.push_back(conclude_syble_info);
	}
	

	//��������µĹ�Լ����������
	return true;
}