#include "Pl0VirtualMachine.h"
#include <fstream>
#include <iostream>

void Pl0VirtualMachine::Push(int32_t value)
{
	Stack[StackPointer] = value;
	StackPointer++;
}

int32_t Pl0VirtualMachine::Pop()
{
	StackPointer--;
	return Stack[StackPointer];
}

uint32_t Pl0VirtualMachine::GetVariableAddress(int16_t levelDiff, uint32_t offset)
{
	uint32_t basePointer = BasePointer;
	while (levelDiff < 0) {
		basePointer = Stack[basePointer + 2];
		levelDiff++;
	}
	return basePointer + offset;
}

void Pl0VirtualMachine::ExecINT(const Instruction& instruction)
{
	StackPointer += instruction.a;
}

void Pl0VirtualMachine::ExecLIT(const Instruction& instruction)
{
	Push(instruction.a);
}

void Pl0VirtualMachine::ExecLOD(const Instruction& instruction)
{
	uint32_t address = GetVariableAddress(instruction.L, instruction.a);
	Push(Stack[address]);
}

void Pl0VirtualMachine::ExecSTO(const Instruction& instruction)
{
	uint32_t address = GetVariableAddress(instruction.L, instruction.a);
	Stack[address] = Pop();
	std::cout << "STO: " << Stack[address] << std::endl;
}

void Pl0VirtualMachine::ExecCAL(const Instruction& instruction)
{
	Push(BasePointer);
	Push(ProgramCounter + 1);

	//找到SL
	int32_t SL;
	if (instruction.L == 0) {
		SL = Stack[BasePointer + 2];
	}
	else if (instruction.L == 1) {
		SL = BasePointer;
	}
	else {
		int32_t levelDiff = instruction.L;
		SL = BasePointer;
		while (levelDiff < 0) {
			SL = Stack[SL + 2];
			levelDiff++;
		}
		SL = Stack[SL + 2];
	}

	Push(SL);
	BasePointer = StackPointer - 3;
	ProgramCounter = instruction.a - 1;
}

void Pl0VirtualMachine::ExecJMP(const Instruction& instruction)
{
	ProgramCounter += instruction.a - 1;
}

void Pl0VirtualMachine::ExecJPC(const Instruction& instruction)
{
	if (Pop() == 0) {
		ProgramCounter += instruction.a - 1;
	}
}

void Pl0VirtualMachine::ExecOPR(const Instruction& instruction)
{
	int32_t a, b;	//临时变量

	switch (instruction.a) {
	case Add:
		Push(Pop() + Pop());
		break;
	case Sub:
		b = Pop();
		a = Pop();
		Push(a - b);
		break;
	case Mul:
		Push(Pop() * Pop());
		break;
	case Div:
		b = Pop();
		a = Pop();
		Push(a / b);
		break;
	case Neg:
		Push(-Pop());
		break;
	case LessThan:
		b = Pop();
		a = Pop();
		Push(a < b);
		break;
	case LessEqual:
		b = Pop();
		a = Pop();
		Push(a <= b);
		break;
	case Equal:
		b = Pop();
		a = Pop();
		Push(a == b);
		break;
	case NotEqual:
		b = Pop();
		a = Pop();
		Push(a != b);
		break;
	case GreaterEqual:
		b = Pop();
		a = Pop();
		Push(a >= b);
		break;
	case GreaterThan:
		b = Pop();
		a = Pop();
		Push(a > b);
		break;
	case Odd:
		Push(Pop() % 2);
		break;
	default:
		std::cerr << "Unknown OPR code: " << instruction.a << std::endl;
		exit(1);
	}
}

void Pl0VirtualMachine::ExecRET(const Instruction& instruction)
{
	uint32_t returnAddress = Stack[BasePointer + 1];
	if (returnAddress == 0) {
		//程序结束
		std::cout << "========= Program finished =========" << std::endl;
		exit(0);
	}

	StackPointer = BasePointer;
	ProgramCounter = returnAddress - 1;
	BasePointer = Stack[BasePointer];
}

void Pl0VirtualMachine::ExecLOR(const Instruction& instruction)
{
	uint32_t address = Pop();
	Push(Stack[address]);
}

void Pl0VirtualMachine::ExecSTR(const Instruction& instruction)
{
	uint32_t address = Pop();
	int32_t data = Pop();
	Stack[address] = data;

	std::cout << "STR: " << Stack[address] << std::endl;
}

void Pl0VirtualMachine::ExecLBP(const Instruction& instruction)
{
	Push(BasePointer);
}

void Pl0VirtualMachine::ExecWRT(const Instruction& instruction)
{
	std::cout << Pop() << std::endl;
}

void Pl0VirtualMachine::ExecLOA(const Instruction& instruction)
{
	uint32_t address = GetVariableAddress(instruction.L, instruction.a);
	Push(address);
}

Pl0VirtualMachine::Pl0VirtualMachine(const std::string& executableFile) : Stack(1024 * 1024), Instructions{}
{
	std::ifstream file(executableFile, std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "Cannot open file: " << executableFile << std::endl;
		exit(1);
	}

	// 获取文件大小
	size_t fileSize;
	file.seekg(0, std::ios::end);
	fileSize = file.tellg();
	if (fileSize % sizeof(Instruction) != 0)
	{
		std::cerr << "File size error" << std::endl;
		exit(1);
	}

	//读取指令
	uint32_t nInstructions = fileSize / sizeof(Instruction);
	file.seekg(0, std::ios::beg);
	for (uint32_t i{}; i < nInstructions; i++)
	{
		Instruction instruction;
		file.read((char*)&instruction, sizeof(Instruction));
		Instructions.push_back(instruction);
	}

	//为主程序预先压入三个0，占据DL、RA、SL的位置
	Push(0);
	Push(0);
	Push(0);
}

void Pl0VirtualMachine::Run()
{
	Instruction instruction;

	while (true) {
		instruction = Instructions[ProgramCounter];

		switch (instruction.F) {
		case INT:
			ExecINT(instruction);
			break;
		case LIT:
			ExecLIT(instruction);
			break;
		case LOD:
			ExecLOD(instruction);
			break;
		case STO:
			ExecSTO(instruction);
			break;
		case CAL:
			ExecCAL(instruction);
			break;
		case JMP:
			ExecJMP(instruction);
			break;
		case JPC:
			ExecJPC(instruction);
			break;
		case OPR:
			ExecOPR(instruction);
			break;
		case RET:
			ExecRET(instruction);
			break;
		case LOR:
			ExecLOR(instruction);
			break;
		case STR:
			ExecSTR(instruction);
			break;
		case LBP:
			ExecLBP(instruction);
			break;
		case WRT:
			ExecWRT(instruction);
			break;
		case LOA:
			ExecLOA(instruction);
			break;
		default:
			std::cerr << "Unknown instruction code: " << instruction.F << std::endl;
			exit(1);
		}

		ProgramCounter++;
	}
}
