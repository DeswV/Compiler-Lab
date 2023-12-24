#pragma once
#include <vector>
#include <string>
#include <random>
#include "Instruction.h"

class Pl0VirtualMachine
{
private:
	uint32_t ProgramCounter{};
	uint32_t BasePointer{};
	uint32_t StackPointer{};
	std::vector<int32_t> Stack;
	std::vector<Instruction> Instructions;

	std::mt19937 mt{ std::random_device{}() };

	void Push(int32_t value);
	int32_t Pop();
	//ȡ�ñ����ĵ�ַ
	uint32_t GetVariableAddress(int16_t levelDiff, uint32_t offset);

	void ExecINT(const Instruction& instruction);
	void ExecLIT(const Instruction& instruction);
	void ExecLOD(const Instruction& instruction);
	void ExecSTO(const Instruction& instruction);
	void ExecCAL(const Instruction& instruction);
	void ExecJMP(const Instruction& instruction);
	void ExecJPC(const Instruction& instruction);
	void ExecOPR(const Instruction& instruction);
	void ExecRET(const Instruction& instruction);
	void ExecLOR(const Instruction& instruction);
	void ExecSTR(const Instruction& instruction);
	void ExecLBP(const Instruction& instruction);
	void ExecWRT(const Instruction& instruction);
	void ExecLOA(const Instruction& instruction);
	void ExecRAN_N(const Instruction& instruction);
	void ExecRAN(const Instruction& instruction);

public:
	Pl0VirtualMachine(const std::string& executableFile);
	void Run();
};

