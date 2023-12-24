#include "CodeGenerator.h"

#include <fstream>
#include <string>
#include <iostream>

#include "Utils.h"
#include "LexicalAnalyzer.h"


std::string SScopedIdentifier::ToString() const {
	std::string result;
	if (bStartFromMain) {
		result += "::";
	}
	for (uint32_t i{}; i < Identifiers.size(); i++) {
		result += Identifiers[i];
		if (i != Identifiers.size() - 1)
			result += "::";
	}
	return result;
}

CCodeGenerator::CCodeGenerator(const std::vector<STerminator>& terminatorSequence) : TerminatorSequence(terminatorSequence)
{
}

void CCodeGenerator::GenerateCode()
{
	Program();
	if (CurrentIndex != TerminatorSequence.size()) {
		Error("Redundant characters after the period '.' on line " + std::to_string(TerminatorSequence[CurrentIndex].Line));
	}

	//将所有的子程序的指令序列合并到Instructions中
	for (auto& procedure : Procedures) {
		procedure->Address = Instructions.size();
		Instructions.insert(Instructions.end(), procedure->Instructions.begin(), procedure->Instructions.end());
	}
	//回填
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

	//检查标识符是否重名
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

	//检查标识符是否重名
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

void CCodeGenerator::FindVariable(SProcedure& procedure, const SScopedIdentifier& scopedIdentifier, SType& type, int16_t& levelDiff, uint32_t& offset, bool& isConst)
{
	if (scopedIdentifier.Identifiers.empty()) {
		Error("Compiler internal error: scopedIdentifier.Identifiers is empty");
	}

	//情况1：从最外层即主程序开始查找
	if (scopedIdentifier.bStartFromMain) {
		//先找到变量所在的子程序
		uint32_t numOfScopes = scopedIdentifier.Identifiers.size() - 1;
		SProcedure* procedurePtr = Procedures[0].get();
		for (uint32_t i{}; i < numOfScopes; i++) {
			bool found{};
			for (const auto& subprocedure : procedurePtr->SubProcedures) {
				if (subprocedure->Name == scopedIdentifier.Identifiers[i]) {
					found = true;
					procedurePtr = subprocedure;
					break;
				}
			}
			if (!found) {
				Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": cannot find variable '" + scopedIdentifier.ToString() + "'");
			}
		}
		//然后在该子程序中寻找变量
		const std::string& variableName = scopedIdentifier.Identifiers.back();
		for (const auto& variable : procedurePtr->Variables) {
			if (variable.Name == variableName) {
				type = variable.Type;
				levelDiff = procedurePtr->Level - procedure.Level;
				offset = variable.Offset;
				isConst = variable.bIsConst;
				return;
			}
		}
		Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": cannot find variable '" + scopedIdentifier.ToString() + "'");
	}
	//情况2：只有一个变量名，从当前子程序开始向上查找
	else if (scopedIdentifier.Identifiers.size() == 1) {
		const std::string& variableName = scopedIdentifier.Identifiers[0];
		SProcedure* procedurePtr = &procedure;
		while (procedurePtr) {
			for (auto& variable : procedurePtr->Variables) {
				if (variable.Name == variableName) {
					type = variable.Type;
					levelDiff = procedurePtr->Level - procedure.Level;
					offset = variable.Offset;
					isConst = variable.bIsConst;
					return;
				}
			}
			procedurePtr = procedurePtr->Parent;
		}

		Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": variable '" + variableName + "' has not been declared");
	}
	//情况3：除了变量名以外还有一个或多个作用域，先从下往上找到第一个标识符定义的作用域
	else {
		const std::string& firstIdentifier = scopedIdentifier.Identifiers[0];
		SProcedure* procedurePtr = &procedure;
		while (procedurePtr) {
			if (procedurePtr->Name == firstIdentifier) break;
			procedurePtr = procedurePtr->Parent;
		}
		if (!procedurePtr) {
			Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": identifier '" + firstIdentifier + "' has not been declared");
		}
		//然后向下依次寻找剩余的作用域
		uint32_t numOfLeftScopes = scopedIdentifier.Identifiers.size() - 2;
		for (uint32_t i{}; i < numOfLeftScopes; i++) {
			bool found{};
			for (const auto& subprocedure : procedurePtr->SubProcedures) {
				if (subprocedure->Name == scopedIdentifier.Identifiers[i + 1]) {
					found = true;
					procedurePtr = subprocedure;
					break;
				}
			}
			if (!found) {
				Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": cannot find variable '" + scopedIdentifier.ToString() + "'");
			}
		}
		//然后取出该变量
		const std::string& variableName = scopedIdentifier.Identifiers.back();
		for (const auto& variable : procedurePtr->Variables) {
			if (variable.Name == variableName) {
				type = variable.Type;
				levelDiff = procedurePtr->Level - procedure.Level;
				offset = variable.Offset;
				isConst = variable.bIsConst;
				return;
			}
		}
		Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": cannot find variable '" + scopedIdentifier.ToString() + "'");
	}
}

void CCodeGenerator::FindSubProcedure(SProcedure& procedure, uint32_t identTerminatorIndex, SProcedure*& calledProcedure, int16_t& levelDiff)
{
	std::string procedureName = TerminatorSequence[identTerminatorIndex].IdentifierName;

	//查找要调用的子程序，先看该子程序，再看该子程序的子程序，然后依次向上看祖先程序
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
	//主程序
	Procedures.push_back(std::make_shared<SProcedure>(nullptr, 0, "main"));	//主程序的Parent为nullptr，Level为0
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

void CCodeGenerator::ScopedIdentifier(SScopedIdentifier& scopedIdentifier)
{
	std::string nextTerminatorType = GetNextTerminatorType();
	if (nextTerminatorType == "::") {
		Match("::");
		scopedIdentifier.bStartFromMain = true;
	}
	else {
		scopedIdentifier.bStartFromMain = false;
	}
	std::string identifier;
	scopedIdentifier.Identifiers.clear();
	while (true) {
		Match("ident", nullptr, &identifier);
		scopedIdentifier.Identifiers.push_back(identifier);
		if (GetNextTerminatorType() != "::") break;
		else Match("::");
	}
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
	procedure.Instructions.push_back({ RET,0,0 });	//返回
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
	//匹配若干个“*”
	uint32_t numberOfStars{};
	while (true) {
		if (GetNextTerminatorType() == "*") {
			Match("*");
			numberOfStars++;
		}
		else break;
	}

	//匹配一个标识符
	Match("ident");
	uint32_t indexOfIdentTerminator = CurrentIndex - 1;

	//匹配若干个[dim]
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

	//构建该变量的类型
	SType type;
	type = BuildMultiLevelPointerType(numberOfStars, SType{ EType::Integer });
	type = BuildNDimArrayType(dimensions, 0, type);

	//记录该变量
	AddVariable(procedure, indexOfIdentTerminator, type);
	//在栈中为该变量分配空间
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
	std::vector<Instruction> instructionsOfLeft;					//暂存左值的指令序列
	SValue leftValue = Factor(procedure, instructionsOfLeft, true);	//得到左值的类型

	Match(":=");
	SValue rightValue = Expression(procedure, procedure.Instructions);

	// 3. 检查类型是否匹配
	if (leftValue.Type != rightValue.Type) {
		Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": cannot assign value to different types");
	}

	//将计算左值的指令放在右值的指令之后
	procedure.Instructions.insert(procedure.Instructions.end(), instructionsOfLeft.begin(), instructionsOfLeft.end());
	procedure.Instructions.push_back({ STR,0,0 });
}

void CCodeGenerator::CallStatement(SProcedure& procedure)
{
	Match("call");
	Match("ident");

	//查找要调用的子程序
	SProcedure* calledProcedure;
	int16_t levelDiff;
	FindSubProcedure(procedure, CurrentIndex - 1, calledProcedure, levelDiff);

	//在指令序列中添加空的CAL指令占位
	procedure.Instructions.push_back({ CAL,levelDiff,0 });
	//记录下调用了CAL的指令，等待回填
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
	Condition(procedure);	//Condition的代码执行完毕后，栈顶是条件的bool值
	Match("then");
	//在指令序列中添加空的JPC指令占位
	procedure.Instructions.push_back({ JPC,0,0 });
	uint32_t jpcInstructionOffset = procedure.Instructions.size() - 1;
	Statement(procedure);
	//回填
	procedure.Instructions[jpcInstructionOffset].a = procedure.Instructions.size() - jpcInstructionOffset;
}

void CCodeGenerator::WhileStatement(SProcedure& procedure)
{
	Match("while");
	uint32_t conditionOffset = procedure.Instructions.size();
	Condition(procedure);	//Condition的代码执行完毕后，栈顶是条件的bool值
	Match("do");
	//在指令序列中添加空的JPC指令占位；JPC将利用栈顶的bool值来判断是否跳转，同时将栈顶的bool值弹出
	//如果条件为假，跳转到while语句之后
	procedure.Instructions.push_back({ JPC,0,0 });
	uint32_t jpcInstructionOffset = procedure.Instructions.size() - 1;
	Statement(procedure);
	procedure.Instructions.push_back({ JMP,0,(int32_t)conditionOffset - (int32_t)procedure.Instructions.size() });	//跳转到条件判断
	//回填
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
	procedure.Instructions.push_back({ OPR,0,Odd });	//这条指令根据栈顶的值将1或0替代为栈顶
}

void CCodeGenerator::CompareCondition(SProcedure& procedure)
{
	SValue value1 = Expression(procedure, procedure.Instructions);

	//匹配一个比较运算符
	std::string nextTerminatorType = GetNextTerminatorType();
	int32_t OPR_a{};	//OPR指令的操作码
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
	//检查类型是否匹配
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

			//检查类型是否能够相加
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

			//检查类型是否能够相减
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

			//检查类型是否能够相乘
			if (value.Type.Type != EType::Integer || nextValue.Type.Type != EType::Integer) {
				Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": cannot mul such types of values");
			}
			instructions.push_back({ OPR,0,Mul });
		}
		else if (nextTerminatorType == "/") {
			Match("/");
			nextValue = Factor(procedure, instructions);

			//检查类型是否能够相除
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

SValue CCodeGenerator::Factor(SProcedure& procedure, std::vector<Instruction>& instructions, bool isLeftValue)
{
	std::string nextTerminatorType = GetNextTerminatorType();
	SValue value;	//跟踪当前处理的值的类型、是否是左值、是否是常量

	if ((nextTerminatorType == "ident" || nextTerminatorType == "::") || nextTerminatorType == "(") {
		//这两种情况的前半段是一样的
		if (nextTerminatorType == "ident" || nextTerminatorType == "::") {
			SScopedIdentifier scopedIdentifier;
			ScopedIdentifier(scopedIdentifier);
			SType type;
			int16_t levelDiff;
			uint32_t offset;
			bool isConst;
			FindVariable(procedure, scopedIdentifier, type, levelDiff, offset, isConst);

			//对于数组，将其转换为指针
			if (type.Type == EType::Array) {
				type.Type = EType::Pointer;
				value.bIsConst = false;
				instructions.push_back({ LOA,levelDiff,(int32_t)offset });
			}
			//对于指针或整数，直接取出相应内存位置的值即可
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

		//然后处理连续的若干个[index]
		SType indexType;
		while (true) {
			if (GetNextTerminatorType() == "[") {
				Match("[");
				indexType = Expression(procedure, instructions).Type;
				if (indexType.Type != EType::Integer) {
					Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": index must be integer");
				}
				Match("]");

				//检查是否能够进行索引
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

		//检查是否可以取负
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

		//判断是否是指针类型
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
		//匹配一个左值
		SValue nextValue = Factor(procedure, instructions, true);

		value.Type = SType{ EType::Pointer,std::make_shared<SType>(nextValue.Type) };
		value.bIsConst = false;
	}
	else {
		Error("Expected a factor on line " + std::to_string(TerminatorSequence[CurrentIndex].Line));
	}

	//对左值的特殊处理
	if (isLeftValue) {
		//检查是否是左值
		if (instructions.back().F == LOD || instructions.back().F == LOR) {
			//确实是左值，检查是否是常量
			if (value.bIsConst) Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": const cannot be lvalue");

			//修改最后一条指令使得指令执行结束后栈顶是该左值的地址而非该左值的值
			if (instructions.back().F == LOD) {
				int16_t levelDiff = instructions.back().L;
				int32_t offset = instructions.back().a;
				instructions.pop_back();
				instructions.push_back({ LOA,levelDiff,offset });
			}
			//instructions.back().F == LOR
			else {
				instructions.pop_back();
			}

		}
		//不是左值
		else {
			Error("Line " + std::to_string(TerminatorSequence[CurrentIndex - 1].Line) + ": expected lvalue here");
		}
	}

	return value;
}

