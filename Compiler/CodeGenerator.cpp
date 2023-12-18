#include "CodeGenerator.h"

#include <fstream>
#include <string>
#include <iostream>

#include "Utils.h"
#include "LexicalAnalyzer.h"


CCodeGenerator::CCodeGenerator(const std::vector<STerminator>& terminatorSequence) : TerminatorSequence(terminatorSequence)
{
}

void CCodeGenerator::GenerateCode()
{
	Program();
	if (CurrentIndex != TerminatorSequence.size()) {
		Error("Redundant characters after the period '.' on line " + std::to_string(TerminatorSequence[CurrentIndex].Line));
	}

	//�����е��ӳ����ָ�����кϲ���Instructions��
	for (auto& procedure : Procedures) {
		procedure.Address = Instructions.size();
		Instructions.insert(Instructions.end(), procedure.Instructions.begin(), procedure.Instructions.end());
	}
	//����
	for (auto& callInstruction : CallInstructions) {
		uint32_t callInstructionAddress = callInstruction.CallInstructionOffset + callInstruction.Procedure->Address;
		uint32_t calledProcedureAddress = callInstruction.CalledProcedure->Address;
		Instructions[callInstructionAddress] = { CAL,callInstruction.LevelDifference,(int32_t)calledProcedureAddress };
	}
}

void CCodeGenerator::PrintInstructions()
{
	for (auto& instruction : Instructions) {
		std::cout << instruction.F << ' ' << instruction.L << ' ' << instruction.a << std::endl;
	}
}

void CCodeGenerator::Output(const std::string& FileName)
{
	std::ofstream fout{ FileName,std::ios::binary };
	if (!fout.is_open()) {
		Error("Cannot open file " + FileName);
	}

	for (auto& instruction : Instructions) {
		fout.write((char*)&instruction, sizeof(instruction));
	}
}

std::string CCodeGenerator::GetNextTerminatorType() {
	if (CurrentIndex >= TerminatorSequence.size()) {
		Error("Unexpected end of file on line " + std::to_string(TerminatorSequence.back().Line));
	}
	return TerminatorSequence[CurrentIndex].Type;
}

void CCodeGenerator::AddVariable(SProcedure& procedure, uint32_t identTerminatorIndex)
{
	std::string identifierName = TerminatorSequence[identTerminatorIndex].IdentifierName;

	//����ʶ���Ƿ�����
	for (auto& variable : procedure.Variables) {
		if (variable.Name == identifierName) {
			Error("Line " + std::to_string(TerminatorSequence[identTerminatorIndex].Line) + ": identifier '" + identifierName + "' has already been declared as variable name");
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

void CCodeGenerator::AddSubProcedure(SProcedure& procedure, uint32_t identTerminatorIndex)
{
	std::string identifierName = TerminatorSequence[identTerminatorIndex].IdentifierName;

	//����ʶ���Ƿ�����
	for (auto& variable : procedure.Variables) {
		if (variable.Name == identifierName) {
			Error("Line " + std::to_string(TerminatorSequence[identTerminatorIndex].Line) + ": identifier '" + identifierName + "' has already been declared as variable name");
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

	Procedures.push_back({ &procedure,(int16_t)(procedure.Level + 1),identifierName });
	procedure.SubProcedures.push_back(&Procedures.back());
}

void CCodeGenerator::FindVariable(SProcedure& procedure, uint32_t identTerminatorIndex, int16_t& levelDiff, uint32_t& offset)
{
	std::string variableName = TerminatorSequence[identTerminatorIndex].IdentifierName;

	//���Ҹñ������ӵ�ǰ�ӳ���ʼ���ϲ���
	levelDiff = 0;
	SProcedure* currentProcedure = &procedure;
	while (currentProcedure) {
		for (auto& variable : currentProcedure->Variables) {
			if (variable.Name == variableName) {
				offset = variable.Offset;
				return;
			}
		}
		levelDiff--;
		currentProcedure = currentProcedure->Parent;
	}

	Error("Line " + std::to_string(TerminatorSequence[identTerminatorIndex].Line) + ": variable '" + variableName + "' has not been declared");
}

void CCodeGenerator::FindSubProcedure(SProcedure& procedure, uint32_t identTerminatorIndex, SProcedure*& calledProcedure, int16_t& levelDiff)
{
	std::string procedureName = TerminatorSequence[identTerminatorIndex].IdentifierName;

	//����Ҫ���õ��ӳ����ȿ����ӳ����ٿ����ӳ�����ӳ���Ȼ���������Ͽ����ȳ���
	bool found{};
	if (procedure.Name == procedureName) {
		found = true;
		calledProcedure = &procedure;
		levelDiff = 0;
	}
	else {
		for (auto procedurePtr : procedure.SubProcedures) {
			if (procedurePtr->Name == procedureName) {
				found = true;
				calledProcedure = procedurePtr;
				levelDiff = 1;
				break;
			}
		}
		if (!found) {
			SProcedure* currentProcedure = procedure.Parent;
			levelDiff = -1;
			while (currentProcedure) {
				if (currentProcedure->Name == procedureName) {
					found = true;
					calledProcedure = currentProcedure;
					break;
				}
				levelDiff--;
				currentProcedure = currentProcedure->Parent;
			}
		}
	}

	if (!found) {
		Error("Line " + std::to_string(TerminatorSequence[identTerminatorIndex].Line) + ": procedure '" + procedureName + "' has not been declared");
	}
}

void CCodeGenerator::Program()
{
	//������
	SProcedure mainProcedure;
	mainProcedure.Parent = nullptr;
	mainProcedure.Level = 0;
	mainProcedure.Name = "main";
	Procedures.push_back(mainProcedure);
	Procedure(Procedures[0]);

	Match(".");
}

void CCodeGenerator::Match(const std::string& type, int32_t* numverValue, std::string* identifierName)
{
	if (!IsPossibleTerminatorType(type)) {
		Error("Compiler internal error: unknown terminator type '" + type + "'");
	}

	std::string nextTerminatorType = GetNextTerminatorType();

	if (nextTerminatorType != type) {
		Error("Expected '" + type + "' but got '" + TerminatorSequence[CurrentIndex].Type + "' on line " + std::to_string(TerminatorSequence[CurrentIndex].Line));
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

void CCodeGenerator::Procedure(SProcedure& procedure)
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
	procedure.Instructions.push_back({ RET,0,0 });	//����
}

void CCodeGenerator::ConstDeclare(SProcedure& procedure)
{
	Match("const");

	std::string identifierName;
	int32_t numberValue;
	while (true) {
		Match("ident", nullptr, &identifierName);
		Match("=");
		Match("number", &numberValue);
		AddVariable(procedure, CurrentIndex - 3);

		//��������ֵ����ջ��
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

void CCodeGenerator::VarDeclare(SProcedure& procedure)
{
	Match("var");

	std::string identifierName;
	while (true) {
		Match("ident", nullptr, &identifierName);
		AddVariable(procedure, CurrentIndex - 1);

		//��ջ��Ϊ��������ռ�
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

void CCodeGenerator::ProcedureDeclare(SProcedure& procedure)
{
	Match("procedure");
	std::string procedureName;
	Match("ident", nullptr, &procedureName);
	Match(";");
	AddSubProcedure(procedure, CurrentIndex - 2);
	Procedure(Procedures.back());
	Match(";");
}

void CCodeGenerator::Statement(SProcedure& procedure)
{
	std::string nextTerminatorType = GetNextTerminatorType();

	if (nextTerminatorType == "ident") {
		AssignStatement(procedure);
	}
	else if (nextTerminatorType == "call") {
		CallStatement(procedure);
	}
	else if (nextTerminatorType == "begin") {
		BeginEndStatement(procedure);
	}
	else if (nextTerminatorType == "if") {
		IfStatement(procedure);
	}
	else if (nextTerminatorType == "while") {
		WhileStatement(procedure);
	}
	else {
		Error("Expected a statement on line " + std::to_string(TerminatorSequence[CurrentIndex].Line));
	}
}

void CCodeGenerator::StatementSequence(SProcedure& procedure)
{
	while (true) {
		Statement(procedure);
		Match(";");

		if (GetNextTerminatorType() == "end") {
			break;
		}
	}
}

void CCodeGenerator::AssignStatement(SProcedure& procedure)
{
	Match("ident");

	//���Ҹñ���
	int16_t levelDiff;
	uint32_t offset;
	FindVariable(procedure, CurrentIndex - 1, levelDiff, offset);

	Match(":=");
	Expression(procedure);	//Expression�Ĵ���ִ����Ϻ󣬱��ʽ��ֵ������ջ��
	procedure.Instructions.push_back({ STO,levelDiff,(int32_t)offset });
}

void CCodeGenerator::CallStatement(SProcedure& procedure)
{
	Match("call");
	Match("ident");

	//����Ҫ���õ��ӳ���
	SProcedure* calledProcedure;
	int16_t levelDiff;
	FindSubProcedure(procedure, CurrentIndex - 1, calledProcedure, levelDiff);

	//��ָ����������ӿյ�CALָ��ռλ
	procedure.Instructions.push_back({ CAL,levelDiff,0 });
	//��¼�µ�����CAL��ָ��ȴ�����
	CallInstructions.push_back({ &procedure,(uint32_t)(procedure.Instructions.size() - 1),calledProcedure,levelDiff });
}

void CCodeGenerator::BeginEndStatement(SProcedure& procedure)
{
	Match("begin");
	StatementSequence(procedure);
	Match("end");
}

void CCodeGenerator::IfStatement(SProcedure& procedure)
{
	Match("if");
	Condition(procedure);	//Condition�Ĵ���ִ����Ϻ�ջ����������boolֵ
	Match("then");
	//��ָ����������ӿյ�JPCָ��ռλ
	procedure.Instructions.push_back({ JPC,0,0 });
	uint32_t jpcInstructionOffset = procedure.Instructions.size() - 1;
	Statement(procedure);
	//����
	procedure.Instructions[jpcInstructionOffset].a = procedure.Instructions.size() - jpcInstructionOffset;
}

void CCodeGenerator::WhileStatement(SProcedure& procedure)
{
	Match("while");
	uint32_t conditionOffset = procedure.Instructions.size();
	Condition(procedure);	//Condition�Ĵ���ִ����Ϻ�ջ����������boolֵ
	Match("do");
	//��ָ����������ӿյ�JPCָ��ռλ��JPC������ջ����boolֵ���ж��Ƿ���ת��ͬʱ��ջ����boolֵ����
	//�������Ϊ�٣���ת��while���֮��
	procedure.Instructions.push_back({ JPC,0,0 });
	uint32_t jpcInstructionOffset = procedure.Instructions.size() - 1;
	Statement(procedure);
	procedure.Instructions.push_back({ JMP,0,(int32_t)conditionOffset - (int32_t)procedure.Instructions.size() });	//��ת�������ж�
	//����
	procedure.Instructions[jpcInstructionOffset].a = procedure.Instructions.size() - jpcInstructionOffset;
}

void CCodeGenerator::Condition(SProcedure& procedure)
{
	if (GetNextTerminatorType() == "odd") {
		OddCondition(procedure);
	}
	else {
		CompareCondition(procedure);
	}
}

void CCodeGenerator::OddCondition(SProcedure& procedure)
{
	Match("odd");
	Expression(procedure);
	procedure.Instructions.push_back({ OPR,0,Odd });	//����ָ�����ջ����ֵ��1��0���Ϊջ��
}

void CCodeGenerator::CompareCondition(SProcedure& procedure)
{
	Expression(procedure);

	//ƥ��һ���Ƚ������
	std::string nextTerminatorType = GetNextTerminatorType();
	int32_t OPR_a{};	//OPRָ��Ĳ�����
	if (nextTerminatorType == "=") {
		Match("=");
		OPR_a = Equal;
	}
	else if (nextTerminatorType == "<>") {
		Match("<>");
		OPR_a = NotEqual;
	}
	else if (nextTerminatorType == "<") {
		Match("<");
		OPR_a = LessThan;
	}
	else if (nextTerminatorType == "<=") {
		Match("<=");
		OPR_a = LessEqual;
	}
	else if (nextTerminatorType == ">") {
		Match(">");
		OPR_a = GreaterThan;
	}
	else if (nextTerminatorType == ">=") {
		Match(">=");
		OPR_a = GreaterEqual;
	}
	else {
		Error("Expected a compare operator on line " + std::to_string(TerminatorSequence[CurrentIndex].Line));
	}

	Expression(procedure);
	procedure.Instructions.push_back({ OPR,0,OPR_a });
}

void CCodeGenerator::Expression(SProcedure& procedure)
{
	Term(procedure);

	std::string nextTerminatorType;
	while (true) {
		nextTerminatorType = GetNextTerminatorType();
		if (nextTerminatorType == "+") {
			Match("+");
			Term(procedure);
			procedure.Instructions.push_back({ OPR,0,Add });
		}
		else if (nextTerminatorType == "-") {
			Match("-");
			Term(procedure);
			procedure.Instructions.push_back({ OPR,0,Sub });
		}
		else {
			break;
		}
	}
}

void CCodeGenerator::Term(SProcedure& procedure)
{
	Factor(procedure);

	std::string nextTerminatorType;
	while (true) {
		nextTerminatorType = GetNextTerminatorType();
		if (nextTerminatorType == "*") {
			Match("*");
			Factor(procedure);
			procedure.Instructions.push_back({ OPR,0,Mul });
		}
		else if (nextTerminatorType == "/") {
			Match("/");
			Factor(procedure);
			procedure.Instructions.push_back({ OPR,0,Div });
		}
		else {
			break;
		}
	}
}

void CCodeGenerator::Factor(SProcedure& procedure)
{
	std::string nextTerminatorType = GetNextTerminatorType();

	if (nextTerminatorType == "ident") {
		Match("ident");
		int16_t levelDiff;
		uint32_t offset;
		FindVariable(procedure, CurrentIndex - 1, levelDiff, offset);
		procedure.Instructions.push_back({ LOD,levelDiff,(int32_t)offset });
	}
	else if (nextTerminatorType == "number") {
		int32_t numberValue;
		Match("number", &numberValue);
		procedure.Instructions.push_back({ LIT,0,numberValue });
	}
	else if (nextTerminatorType == "-") {
		Match("-");
		Factor(procedure);
		procedure.Instructions.push_back({ OPR,0,Neg });
	}
	else if (nextTerminatorType == "(") {
		Match("(");
		Expression(procedure);
		Match(")");
	}
	else {
		Error("Expected a factor on line " + std::to_string(TerminatorSequence[CurrentIndex].Line));
	}
}
