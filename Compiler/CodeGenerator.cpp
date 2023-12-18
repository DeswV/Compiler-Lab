#include "CodeGenerator.h"

#include <fstream>
#include <string>

#include "Utils.h"
#include "LexicalAnalyzer.h"


CodeGenerator::CodeGenerator(const std::vector<STerminator>& terminatorSequence) : TerminatorSequence(terminatorSequence)
{
}

void CodeGenerator::GenerateCode()
{
	Program();
	if (CurrentIndex != TerminatorSequence.size()) {
		Error("Redundant characters after the period '.' on line " + std::to_string(TerminatorSequence[CurrentIndex].Line));
	}

	//将所有的子程序的指令序列合并到Instructions中
	for (auto& procedure : Procedures) {
		procedure.Address = Instructions.size();
		Instructions.insert(Instructions.end(), procedure.Instructions.begin(), procedure.Instructions.end());
	}
	//回填
	for (auto& callInstruction : CallInstructions) {
		uint32_t callInstructionAddress = callInstruction.CallInstructionOffset + callInstruction.Procedure->Address;
		uint32_t calledProcedureAddress = callInstruction.CalledProcedure->Address;
		Instructions[callInstructionAddress] = { CAL,callInstruction.LevelDifference,(int32_t)calledProcedureAddress };
	}
}

void CodeGenerator::Output(const std::string& FileName)
{
	std::ofstream fout{ FileName,std::ios::binary };
	if (!fout.is_open()) {
		Error("Cannot open file " + FileName);
	}

	for (auto& instruction : Instructions) {
		fout.write((char*)&instruction, sizeof(instruction));
	}
}

std::string CodeGenerator::GetNextTerminatorType() {
	if (CurrentIndex >= TerminatorSequence.size()) {
		Error("Unexpected end of file on line " + std::to_string(TerminatorSequence.back().Line));
	}
	return TerminatorSequence[CurrentIndex].Type;
}

void CodeGenerator::AddVariable(SProcedure& procedure, uint32_t identTerminatorIndex)
{
	std::string identifierName = TerminatorSequence[identTerminatorIndex].IdentifierName;

	//检查标识符是否重名
	for (auto& variable : procedure.Variables) {
		if (variable.Name == identifierName) {
			Error("Duplicate variable name '" + identifierName + "' on line " + std::to_string(TerminatorSequence[identTerminatorIndex].Line));
		}
	}
	for (auto& subProcedure : procedure.SubProcedures) {
		if (subProcedure->Name == identifierName) {
			Error("Line " + std::to_string(TerminatorSequence[identTerminatorIndex].Line) + ": identifier '" + identifierName + "' has already been declared as subprocedure name");
		}
	}
	if (Keywords.contains(identifierName)) {
		Error("Line " + std::to_string(TerminatorSequence[identTerminatorIndex].Line) + ": identifier '" + identifierName + "' has already been declared as keyword");
	}
	if (procedure.Name == identifierName) {
		Error("Line " + std::to_string(TerminatorSequence[identTerminatorIndex].Line) + ": identifier '" + identifierName + "' has already been declared as procedure name");
	}

	procedure.Variables.push_back({ identifierName,(uint32_t)(procedure.Variables.size() + 3) });
}

void CodeGenerator::Program()
{
	//主程序
	SProcedure mainProcedure;
	mainProcedure.Parent = nullptr;
	mainProcedure.Level = 0;
	Procedures.push_back(mainProcedure);
	Procedure(Procedures[0]);

	Match(".");
}

void CodeGenerator::Match(const std::string& type, int32_t* numverValue, std::string* identifierName)
{
	std::string nextTerminatorType = GetNextTerminatorType();

	if (nextTerminatorType != type) {
		Error("Expected " + type + " but got " + TerminatorSequence[CurrentIndex].Type + " on line " + std::to_string(TerminatorSequence[CurrentIndex].Line));
	}
	if (type == "number") {
		if (numverValue != nullptr) {
			*numverValue = TerminatorSequence[CurrentIndex].NumberValue;
		}
	}
	else if (type == "ident") {
		if (identifierName != nullptr) {
			*identifierName = TerminatorSequence[CurrentIndex].IdentifierName;
		}
	}
	CurrentIndex++;
}

void CodeGenerator::Procedure(SProcedure& procedure)
{
	while (true) {
		std::string nextTerminatorType = GetNextTerminatorType();

		if (nextTerminatorType == "const") {
			ConstDeclare(procedure);
		}
		else if (nextTerminatorType == "var") {
			VarDeclare(procedure);
		}
		else if (nextTerminatorType == "procedure") {
			ProcedureDeclare(procedure);
		}
		else {
			Statement(procedure);
			break;
		}
	}
}

void CodeGenerator::ConstDeclare(SProcedure& procedure)
{
	Match("const");

	std::string identifierName;
	int32_t numberValue;
	while (true) {
		Match("ident", nullptr, &identifierName);
		Match("=");
		Match("number", &numberValue);
		AddVariable(procedure, CurrentIndex - 3);

		//将常量的值存入栈中
		procedure.Instructions.push_back({ LIT,0,numberValue });

		if (GetNextTerminatorType() == ";") {
			Match(";");
			break;
		}
		else {
			Match(",");
		}
	}
}

void CodeGenerator::VarDeclare(SProcedure& procedure)
{
	Match("var");

	std::string identifierName;
	while (true) {
		Match("ident", nullptr, &identifierName);
		AddVariable(procedure, CurrentIndex - 1);

		//在栈中为变量分配空间
		procedure.Instructions.push_back({ INT,0,1 });

		if (GetNextTerminatorType() == ";") {
			Match(";");
			break;
		}
		else {
			Match(",");
		}
	}
}

void CodeGenerator::ProcedureDeclare(SProcedure& procedure)
{

}
