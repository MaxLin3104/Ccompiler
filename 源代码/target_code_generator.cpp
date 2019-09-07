#include "target_code_generator.h"

TargetCodeGenerator::TargetCodeGenerator()
{
	target_code_printer_.open("./gen_data/target_file/target_code.txt");
	if (!target_code_printer_.is_open()) {
		cerr << "�����������Ŀ�������ļ���ʧ�ܣ�" << endl;
	}
	InitialSpFpGp();//��ʼ�����ã�û��Mars�ڴ�������ִ���
	gf_offset = GF_INI_OFFSET;
}
void TargetCodeGenerator::TargetCodeGeneratorInit(vector<SymbolTable> &symbol_tables_)
{
	this->symbol_tables_pt_ = &symbol_tables_;
}
TargetCodeGenerator::~TargetCodeGenerator()
{
	if (target_code_printer_.is_open())
		target_code_printer_.close();
}

bool TargetCodeGenerator::InitialSpFpGp()
{
	//lui $sp, 0x1001
	//move $fp, $sp  ��ʼ��fpֱ�Ӹ�0�� ���޸�----------------------------------------------------------------
	PrintQuadruples(Instructions("lui", "$sp", "0x1001", ""));
	PrintQuadruples(Instructions("lui", "$gp", "0x1000", ""));
	/*PrintQuadruples(Instructions("move", "$fp", "$sp", ""));*/
	PrintQuadruples(Instructions("j", "main", "", ""));
	return true;
}

bool TargetCodeGenerator::PrintQuadruples(const Instructions &instr)
{
	target_code_printer_ << instr.op << " " << instr.arg1 << " "
		<< instr.arg2 << " " << instr.arg3 << endl;
	
	return true;
}

bool TargetCodeGenerator::PrintSyscall()
{
	target_code_printer_ << "syscall" << endl;
	return true;
}

bool TargetCodeGenerator::PrintQuadruples(const string&label)
{
	target_code_printer_ << label << endl;
	return true;
}

bool TargetCodeGenerator::PushStack_sp(int num)
{
	int size = num * 4;
	PrintQuadruples(Instructions("addi", "$sp", "$sp", std::to_string(size)));
	//���� addi $sp $sp 12
	return true;
}

bool TargetCodeGenerator::CreateStackFrame()
{
	target_code_printer_ << "#����ջ֡" << endl;
	//���淵�ص�ַ sw $ra ($sp)
	PrintQuadruples(Instructions("sw", "$ra", "($sp)", ""));
	//������fp sw $ra ($sp)
	PrintQuadruples(Instructions("sw", "$fp", "4($sp)", ""));
	//����fp addi $fp $sp 4
	PrintQuadruples(Instructions("addi", "$fp", "$sp", "4"));
	//sp + 2*4
	PushStack_sp(2);
	target_code_printer_ << endl << endl;
	return true;
}

void TargetCodeGenerator::SetMissTime(int reg)
{
	//����reg���0�����඼+1
	int length = registers_.size();
	for (int i = 0; i < length; ++i)
	{
		if (reg == i)
			registers_[i].miss_time = 0;
		else
			registers_[i].miss_time += 1;
	}
	return;
}

int TargetCodeGenerator::GetTempReg()
{
	//��ʾ�Ĵ���û��ȫ��ռ��
	int reg = -1;
	int miss_t = -1;//û��ʹ�õĴ���

	//�Ĵ���ȫ����ռ����
	int reg_p = -1;
	int miss_t_p = -1;

	for (auto iter = registers_.begin(); iter != registers_.end(); ++iter)
	{
		//�Ĵ������ڿ��õ�
		if (iter->is_possessed == false)//��û��ռ��
		{
			if (iter->miss_time > miss_t)//û��ʹ�õĴ���
			{
				reg = iter - registers_.begin();
				miss_t = iter->miss_time;
			}
		}
		//�Ĵ���ȫ����ռ��
		if (iter->miss_time > miss_t_p)//û��ʹ�õĴ���
		{
			reg_p = iter - registers_.begin();
			miss_t_p = iter->miss_time;
		}
	}

	if (reg != -1)//��ʾ����û��ռ�õļĴ������ҳ������һ������ʹ�õļĴ�����ֱ�ӷ���reg
		return reg;
	else//��ʾ���мĴ�������ռ����
		return reg_p;
}

bool TargetCodeGenerator::IsSymbolLoaded(SymbolPos &pos)const
{
	vector<SymbolTable> &symbol_tables = *symbol_tables_pt_;
	Symbol &sb = symbol_tables[pos.table_pos].GetSymbol(pos.symbol_pos);
	if (sb.reg == -1)
		return false;
	else
		return true;
}

bool TargetCodeGenerator::LoadTempReg(int reg, SymbolPos &pos)
{
	vector<SymbolTable> &symbol_tables = *symbol_tables_pt_;
	////�ȼ��pos ��Ӧ�ķ����Ƿ��Ѿ��ڼĴ���-------------------------�����Ȳ������������������ʱ��͵��ȼ��
	Symbol &sb = symbol_tables[pos.table_pos].GetSymbol(pos.symbol_pos);
	
	
	if (true == registers_[reg].is_possessed)//��ռ���ˣ���Ҫ��д�ص��ڴ档
	{
		string sw_mem_addr = GetMemAddr(registers_[reg].content_info);
		PrintQuadruples(Instructions("sw", registers_[reg].name, sw_mem_addr, ""));
		//��ʾ���ڼĴ�������
		SymbolPos reg_pos = registers_[reg].content_info;
		symbol_tables[reg_pos.table_pos].GetSymbol(reg_pos.symbol_pos).reg = -1;
	}
	
	string mem_addr = GetMemAddr(pos);
	PrintQuadruples(Instructions("lw", registers_[reg].name, mem_addr, ""));

	//���üĴ�����Ϣ
	registers_[reg].content_info = pos;
	registers_[reg].is_possessed = true;
	SetMissTime(reg);
	
	//����symbol��Ϣ
	sb.reg = reg;

	return true;
	//if (false == registers_[reg].is_possessed)//û��ռ�ã�����ֱ��д��reg
	//{
	//	string mem_addr = GetMemAddr(pos);
	//	PrintQuadruples(Instructions("lw", registers_[reg].name, mem_addr, ""));
	//	return true;
	//}
	//else//��ռ���ˣ���Ҫ��д���ڴ档
	//{
	//	string sw_mem_addr = GetMemAddr(registers_[reg].content_info);
	//	PrintQuadruples(Instructions("sw", registers_[reg].name, sw_mem_addr, ""));
	//	string mem_addr = GetMemAddr(pos);
	//	PrintQuadruples(Instructions("lw", registers_[reg].name, mem_addr, ""));
	//}
}

string TargetCodeGenerator::GetMemAddr(const SymbolPos &pos) const//����Pos�����ڴ��ַ 4($gp), -4($fp) �ȵ�
{
	string addr = "";
	//��Ҫ���ǵ��������ʱ������ȫ�ֱ������ֲ��������βΡ�    ����ֵ��---------------------������-------------------
	vector<SymbolTable> &symbol_tables = *symbol_tables_pt_;
	if (pos.table_pos == 1)//��ʱ����
	{
		addr = "($gp)";
		int offset_gp = GF_INI_OFFSET + pos.symbol_pos;
		addr = std::to_string(offset_gp * 4) + addr;
	}
	else if (pos.table_pos == 0)//ȫ�ֱ���
	{
		addr = "($gp)";
		
		int offset_gp = -1;
		vector<Symbol> &global_table = symbol_tables[0].GetTable();
		for (auto iter = global_table.begin(); iter != global_table.end(); ++iter)
		{
			if (iter->mode == FUNCTION)
				continue;
			offset_gp++;
			if (iter - global_table.begin() == pos.symbol_pos)
				break;
		}
		addr = std::to_string(offset_gp * 4) + addr;

		//cerr << "TargetCodeGenerator::GetMemAddr  ȫ�ֱ������ȴ��޸ģ�" << endl;
	}
	else//�ֲ��������βΣ�����ֵ
	{
		int func_pos_in_global = symbol_tables[0].FindSymbol(symbol_tables[pos.table_pos].GetTableName());
		Symbol &func = symbol_tables[0].GetSymbol(func_pos_in_global);//��ȡ�������ڵķ��ű�

		if (pos.symbol_pos == 0)//����ֵ
		{
			Symbol return_value = symbol_tables[pos.table_pos].GetSymbol(0);
			if (return_value.type == INT)
				addr = "$v0";
			else//type == VOID
				addr = "";

			//cerr << "TargetCodeGenerator::GetMemAddr  ����ֵ���ȴ��޸ģ�" << endl;
		}
		else
		{
			
			int par_num = func.parameter_num;//��ȡ�βθ���
			if (pos.symbol_pos <= par_num)//λ�����βθ���֮�ڣ���ʾ�β�
			{
				addr = "($fp)";
				int offset_fp = -2 - par_num + pos.symbol_pos;//($fp)��fp  -4($fp)�淵�ص�ַ
				addr = std::to_string(offset_fp * 4) + addr;
			}
			else//�ֲ�����
			{
				addr = "($fp)";
				int offset_fp = pos.symbol_pos - par_num;//($fp)��fp  4($fp)���0���ֲ�����
				addr = std::to_string(offset_fp * 4) + addr;
			}
		}
	}
	return addr;
}

int TargetCodeGenerator::LoadImmToReg(const string&imm, const SymbolPos &pos)
{
	vector<SymbolTable> &symbol_tables = *symbol_tables_pt_;
	int reg = GetTempReg();//��ȡ��һ�����õ�

	if (true == registers_[reg].is_possessed)//��ռ���ˣ���Ҫ��д�ص��ڴ档
	{
		string sw_mem_addr = GetMemAddr(registers_[reg].content_info);
		PrintQuadruples(Instructions("sw", registers_[reg].name, sw_mem_addr, ""));
		//��ʾ���ڼĴ�������
		SymbolPos reg_pos = registers_[reg].content_info;
		symbol_tables[reg_pos.table_pos].GetSymbol(reg_pos.symbol_pos).reg = -1;
	}

	PrintQuadruples(Instructions("addi", registers_[reg].name, "$zero", imm));//����������

	Symbol &sb = symbol_tables[pos.table_pos].GetSymbol(pos.symbol_pos);
	//���üĴ�����Ϣ
	registers_[reg].content_info = pos;
	registers_[reg].is_possessed = true;
	SetMissTime(reg);

	//����symbol��Ϣ
	sb.reg = reg;

	return true;
}

string TargetCodeGenerator::SetArgBeReady(SymbolPos pos)
{
	string name = "$t";
	vector<SymbolTable> &symbol_tables = *symbol_tables_pt_;
	Symbol &sb = symbol_tables[pos.table_pos].GetSymbol(pos.symbol_pos);//��ȡ�÷���

	if (sb.reg == -1)//��Ҫװ��
	{
		int reg = GetTempReg();
		LoadTempReg(reg, pos);	
	}
	name += std::to_string(sb.reg);

	return name;
}

bool TargetCodeGenerator::ClearRegs()
{
	vector<SymbolTable> &symbol_tables = *symbol_tables_pt_;
	for (auto iter = registers_.begin(); iter != registers_.end(); ++iter)
	{
		if (iter->is_possessed == false)
			continue;
		SymbolPos pos = iter->content_info;
		Symbol &sb = symbol_tables[pos.table_pos].GetSymbol(pos.symbol_pos);//��ȡ�÷���
		sb.reg = -1;
		iter->is_possessed = false;
		iter->miss_time = 0;
		iter->content_info = { -1, -1 };
		if (sb.mode == TEMP)
			continue;
		//������ʱ��������Ҫ��mips��д��
		int reg = iter - registers_.begin();
		string mem_addr = GetMemAddr(pos);
		PrintQuadruples(Instructions("sw", registers_[reg].name, mem_addr, ""));
	}
	return true;
}

bool TargetCodeGenerator::ResetRegs()
{
	vector<SymbolTable> &symbol_tables = *symbol_tables_pt_;
	for (auto iter = registers_.begin(); iter != registers_.end(); ++iter)
	{
		if (iter->is_possessed == false)
			continue;
		SymbolPos pos = iter->content_info;
		Symbol &sb = symbol_tables[pos.table_pos].GetSymbol(pos.symbol_pos);//��ȡ�÷���
		sb.reg = -1;
		iter->is_possessed = false;
		iter->miss_time = 0;
		iter->content_info = { -1, -1 };
	}
	return true;
}