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
		procedure->Address = Instructions.size();
		Instructions.insert(Instructions.end(), procedure->Instructions.begin(), procedure->Instructions.end());
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
		switch (instruction.F) {
		case 0:
			std::cout << "INT";
			break;
		case 1:
			std::cout << "LIT";
			break;
		case 2:
			std::cout << "LOD";
			break;
		case 3:
			std::cout << "STO";
			break;
		case 4:
			std::cout << "CAL";
			break;
		case 5:
			std::cout << "JMP";
			break;
		case 6:
			std::cout << "JPC";
			break;
		case 7:
			std::cout << "OPR";
			break;
		case 8:
			std::cout << "RET";
			break;
		case 9:
			std::cout << "LOR";
			break;
		case 10:
			std::cout << "STR";
			break;
		case 11:
			std::cout << "LBP";
			break;
		case 12:
			std::cout << "WRT";
			break;
		case 13:
			std::cout << "LOA";
			break;
		default:
			break;
		}
		std::cout << ' ' << instruction.L << ' ' << instruction.a << std::endl;
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

void CCodeGenerator::AddVariable(SProcedure& procedure, uint32_t identTerminatorIndex, const SType& type, bool isConst)
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

	procedure.Variables.push_back({ identifierName,type,procedure.StackOffset,isConst });
	procedure.StackOffset += GetSize(type);
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

	Procedures.push_back(std::make_shared<SProcedure>(&procedure, (int16_t)(procedure.Level + 1), identifierName));
	procedure.SubProcedures.push_back(Procedures.back().get());
}

void CCodeGenerator::FindVariable(SProcedure& procedure, uint32_t identTerminatorIndex, SType& type, int16_t& levelDiff, uint32_t& offset, bool& isConst)
{
	std::string variableName = TerminatorSequence[identTerminatorIndex].IdentifierName;

	//���Ҹñ������ӵ�ǰ�ӳ���ʼ���ϲ���
	levelDiff = 0;
	SProcedure* currentProcedure = &procedure;
	while (currentProcedure) {
		for (auto& variable : currentProcedure->Variables) {
			if (variable.Name == variableName) {
				type = variable.Type;
				offset = variable.Offset;
				isConst = variable.bIsConst;
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
	Procedures.push_back(std::make_shared<SProcedure>(nullptr, 0, "main"));	//�������ParentΪnullptr��LevelΪ0
	Procedure(*Procedures[0]);

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
		AddVariable(procedure, CurrentIndex - 3, SType{ EType::Integer }, true);

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

	while (true) {
		VarDefine(procedure);

		if (GetNextTerminatorType() == ";") {
			Match(";");
			break;
		}
		else {
			Match(",");
		}
	}
}

void CCodeGenerator::VarDefine(SProcedure& procedure)
{
	//ƥ�����ɸ���*��
	uint32_t numberOfStars{};
	while (true) {
		if (GetNextTerminatorType() == "*") {
			Match("*");
			numberOfStars++;
		}
		else break;
	}

	//ƥ��һ����ʶ��
	Match("ident");
	uint32_t indexOfIdentTerminator = CurrentIndex - 1;

	//ƥ�����ɸ�[dim]
	std::vector<uint32_t> dimensions;
	while (true) {
		if (GetNextTerminatorType() == "[") {
			Match("[");
			int32_t numberValue;
			Match("number", &numberValue);
			if (numberValue <= 0) {
				Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": dimension must be positive");
			}
			dimensions.push_back(numberValue);
			Match("]");
		}
		else break;
	}

	//�����ñ���������
	SType type;
	type = BuildMultiLevelPointerType(numberOfStars, SType{ EType::Integer });
	type = BuildNDimArrayType(dimensions, 0, type);

	//��¼�ñ���
	AddVariable(procedure, indexOfIdentTerminator, type);
	//��ջ��Ϊ�ñ�������ռ�
	procedure.Instructions.push_back({ INT,0,(int32_t)GetSize(type) });
}

void CCodeGenerator::ProcedureDeclare(SProcedure& procedure)
{
	Match("procedure");
	std::string procedureName;
	Match("ident", nullptr, &procedureName);
	Match(";");
	AddSubProcedure(procedure, CurrentIndex - 2);
	Procedure(*Procedures.back());
	Match(";");
}

void CCodeGenerator::Statement(SProcedure& procedure)
{
	std::string nextTerminatorType = GetNextTerminatorType();

	if (nextTerminatorType == "ident" || nextTerminatorType == "number" || nextTerminatorType == "(" || nextTerminatorType == "*" || nextTerminatorType == "&") {
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
	else if (nextTerminatorType == "print") {
		PrintStatement(procedure);
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
	std::vector<Instruction> instructionsOfLeft;	//�ݴ���ֵ��ָ������
	SValue leftValue = Factor(procedure, instructionsOfLeft);	//�õ���ֵ�����ͺ��Ƿ��ǳ���

	// 1. ����Ƿ�����ֵ
	if (instructionsOfLeft.back().F != LOD && instructionsOfLeft.back().F != LOR) {
		Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": cannot assign to a non-lvalue");
	}
	//�޸�ָ��ʹ��ָ��ִ�н�����ջ���Ǹ���ֵ�ĵ�ַ���Ǹ���ֵ��ֵ
	if (instructionsOfLeft.back().F == LOD) {
		int16_t levelDiff = instructionsOfLeft.back().L;
		int32_t offset = instructionsOfLeft.back().a;
		instructionsOfLeft.pop_back();
		instructionsOfLeft.push_back({ LOA,levelDiff,offset });
	}
	//instructionsOfLeft.back().F == LOR
	else {
		instructionsOfLeft.pop_back();
	}
	// 2. ����Ƿ��ǳ���
	if (leftValue.bIsConst) {
		Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": cannot assign to a const value");
	}

	Match(":=");
	SValue rightValue = Expression(procedure, procedure.Instructions);

	// 3. ��������Ƿ�ƥ��
	if (leftValue.Type != rightValue.Type) {
		Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": cannot assign value to different types");
	}

	//��������ֵ��ָ�������ֵ��ָ��֮��
	procedure.Instructions.insert(procedure.Instructions.end(), instructionsOfLeft.begin(), instructionsOfLeft.end());
	procedure.Instructions.push_back({ STR,0,0 });
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

void CCodeGenerator::PrintStatement(SProcedure& procedure)
{
	Match("print");
	Match("(");
	while (true) {
		Expression(procedure, procedure.Instructions);
		procedure.Instructions.push_back({ WRT,0,0 });
		if (GetNextTerminatorType() != ",") break;
		else Match(",");
	}
	Match(")");
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
	Expression(procedure, procedure.Instructions);
	procedure.Instructions.push_back({ OPR,0,Odd });	//����ָ�����ջ����ֵ��1��0���Ϊջ��
}

void CCodeGenerator::CompareCondition(SProcedure& procedure)
{
	SValue value1 = Expression(procedure, procedure.Instructions);

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

	SValue value2 = Expression(procedure, procedure.Instructions);
	//��������Ƿ�ƥ��
	if (value1.Type != value2.Type) {
		Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": cannot compare different types of values");
	}
	procedure.Instructions.push_back({ OPR,0,OPR_a });
}

SValue CCodeGenerator::Expression(SProcedure& procedure, std::vector<Instruction>& instructions)
{
	SValue value = Term(procedure, instructions);
	SValue nextValue;

	std::string nextTerminatorType;
	while (true) {
		nextTerminatorType = GetNextTerminatorType();
		if (nextTerminatorType == "+") {
			Match("+");
			nextValue = Term(procedure, instructions);

			//��������Ƿ��ܹ����
			if (value.Type == nextValue.Type && value.Type.Type == EType::Integer) {
				instructions.push_back({ OPR,0,Add });
			}
			else if (value.Type.Type == EType::Pointer && nextValue.Type.Type == EType::Integer) {
				instructions.push_back({ LIT,0,(int32_t)GetSize(*value.Type.InnerType) });
				instructions.push_back({ OPR,0,Mul });
				instructions.push_back({ OPR,0,Add });
			}
			else {
				Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": cannot add such types of values");
			}
		}
		else if (nextTerminatorType == "-") {
			Match("-");
			nextValue = Term(procedure, instructions);

			//��������Ƿ��ܹ����
			if (value.Type == nextValue.Type && value.Type.Type == EType::Integer) {
				instructions.push_back({ OPR,0,Sub });
			}
			else if (value.Type.Type == EType::Pointer && nextValue.Type.Type == EType::Integer) {
				instructions.push_back({ LIT,0,(int32_t)GetSize(*value.Type.InnerType) });
				instructions.push_back({ OPR,0,Mul });
				instructions.push_back({ OPR,0,Sub });
			}
			else {
				Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": cannot sub such types of values");
			}
		}
		else {
			break;
		}
	}

	value.bIsConst = false;
	return value;
}

SValue CCodeGenerator::Term(SProcedure& procedure, std::vector<Instruction>& instructions)
{
	SValue value = Factor(procedure, instructions);
	SValue nextValue;

	std::string nextTerminatorType;
	while (true) {
		nextTerminatorType = GetNextTerminatorType();
		if (nextTerminatorType == "*") {
			Match("*");
			nextValue = Factor(procedure, instructions);

			//��������Ƿ��ܹ����
			if (value.Type.Type != EType::Integer || nextValue.Type.Type != EType::Integer) {
				Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": cannot mul such types of values");
			}
			instructions.push_back({ OPR,0,Mul });
		}
		else if (nextTerminatorType == "/") {
			Match("/");
			nextValue = Factor(procedure, instructions);

			//��������Ƿ��ܹ����
			if (value.Type.Type != EType::Integer || nextValue.Type.Type != EType::Integer) {
				Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": cannot div such types of values");
			}
			instructions.push_back({ OPR,0,Div });
		}
		else {
			break;
		}
	}

	return value;
}

SValue CCodeGenerator::Factor(SProcedure& procedure, std::vector<Instruction>& instructions)
{
	std::string nextTerminatorType = GetNextTerminatorType();
	SValue value;	//���ٵ�ǰ�����ֵ�����͡��Ƿ�����ֵ���Ƿ��ǳ���

	if (nextTerminatorType == "ident" || nextTerminatorType == "(") {
		//�����������ǰ�����һ����
		if (nextTerminatorType == "ident") {
			Match("ident");
			SType type;
			int16_t levelDiff;
			uint32_t offset;
			bool isConst;
			FindVariable(procedure, CurrentIndex - 1, type, levelDiff, offset, isConst);

			//�������飬����ת��Ϊָ��
			if (type.Type == EType::Array) {
				type.Type = EType::Pointer;
				value.bIsConst = false;
				instructions.push_back({ LOA,levelDiff,(int32_t)offset });
			}
			//����ָ���������ֱ��ȡ����Ӧ�ڴ�λ�õ�ֵ����
			else {
				value.bIsConst = isConst;
				instructions.push_back({ LOD,levelDiff,(int32_t)offset });
			}
			value.Type = type;
			
		}
		//nextTerminatorType == "("
		else {
			Match("(");
			value = Expression(procedure, instructions);
			Match(")");
		}

		//Ȼ�������������ɸ�[index]
		SType indexType;
		while (true) {
			if (GetNextTerminatorType() == "[") {
				Match("[");
				indexType = Expression(procedure, instructions).Type;
				if (indexType.Type != EType::Integer) {
					Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": index must be integer");
				}
				Match("]");

				//����Ƿ��ܹ���������
				if (value.Type.Type != EType::Pointer) {
					Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": cannot index a non-pointer type");
				}

				instructions.push_back({ LIT,0,(int32_t)GetSize(*value.Type.InnerType) });
				instructions.push_back({ OPR,0,Mul });
				instructions.push_back({ OPR,0,Add });

				value.Type = *value.Type.InnerType;
				if (value.Type.Type != EType::Array)
					instructions.push_back({ LOR,0,0 });
				else
					value.Type.Type = EType::Pointer;

				value.bIsConst = false;
			}
			else break;
		}
	}
	else if (nextTerminatorType == "number") {
		int32_t numberValue;
		Match("number", &numberValue);
		value.Type.Type = EType::Integer;
		value.bIsConst = false;
		instructions.push_back({ LIT,0,numberValue });
	}
	else if (nextTerminatorType == "-") {
		Match("-");
		SType type = Factor(procedure, instructions).Type;

		//����Ƿ����ȡ��
		if (type.Type != EType::Integer) {
			Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": cannot take the negative of a non-integer type");
		}
		value.Type = type;
		value.bIsConst = false;
		instructions.push_back({ OPR,0,Neg });
	}
	else if (nextTerminatorType == "*") {
		Match("*");
		SValue nextValue = Factor(procedure, instructions);

		//�ж��Ƿ���ָ������
		if (nextValue.Type.Type != EType::Pointer) {
			Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": cannot use * operator to a non-pointer type");
		}

		value.Type = *nextValue.Type.InnerType;
		value.bIsConst = false;
		if (value.Type.Type == EType::Array) value.Type.Type = EType::Pointer;
		else instructions.push_back({ LOR,0,0 });
	}
	else if (nextTerminatorType == "&") {
		Match("&");
		SValue nextValue = Factor(procedure, instructions);

		//�ж��Ƿ�����ֵ
		if (instructions.back().F != LOD && instructions.back().F != LOR) {
			Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": cannot use & operator to a non-lvalue");
		}
		if (instructions.back().F == LOD) {
			//����һ����������Ҫȡ���ĵ�ַ
			int16_t levelDiff = instructions.back().L;
			int32_t offset = instructions.back().a;
			instructions.pop_back();
			instructions.push_back({ LOA,levelDiff,offset });

			value.Type = SType{ EType::Pointer,std::make_shared<SType>(nextValue.Type) };
		}
		//instructions.back().F == LOR
		else {
			//����һ��ָ�����ջ������ָ������ָ��
			instructions.pop_back();
			value.Type = SType{ EType::Pointer,std::make_shared<SType>(nextValue.Type) };
		}
	}
	else {
		Error("Expected a factor on line " + std::to_string(TerminatorSequence[CurrentIndex].Line));
	}

	return value;
}

