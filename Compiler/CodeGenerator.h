#pragma once
#include "LexicalAnalyzer.h"
#include "Intruction.h"
#include <memory>


struct SVariable {
	std::string Name;
	uint32_t Offset;		//存储在函数栈中的偏移量
};

//一个子程序
struct SProcedure
{
	SProcedure* Parent;						//nullptr代表为主程序
	int16_t Level;							//层次
	std::string Name;						//子程序名，主程序的名字为"main"

	std::vector<SVariable> Variables;
	std::vector<SProcedure*> SubProcedures;

	std::vector<Instruction> Instructions;
	uint32_t Address;						//子程序的入口地址
};
/*
主程序的层次为0，主程序的局部变量的层次为0
*/

//记录下调用了CAL的指令，等待回填
struct SCallIntruction {
	SProcedure* Procedure;					//调用指令所在的子程序
	uint32_t CallInstructionOffset;			//调用指令的偏移量
	SProcedure* CalledProcedure;			//被调用的子程序
	int16_t LevelDifference;				//层次差
};

class CCodeGenerator
{
private:
	const std::vector<STerminator>& TerminatorSequence;	//输入的词法分析结果
	uint32_t CurrentIndex{};							//当前处理的词法分析结果的下标
	std::vector<Instruction> Instructions;				//最终得到的指令序列

	std::vector<std::shared_ptr<SProcedure>> Procedures;//所有的子程序；std::vector在扩容时会移动内存，因此使用智能指针
	std::vector<SCallIntruction> CallInstructions;		//所有的调用指令，用于回填

	/*
	* 一些辅助函数
	*/
	//得到下一个终结符类型，顺便检查是否存在下一个终结符，不存在则报错
	std::string GetNextTerminatorType();
	//向procedure.Variables中添加一个变量，顺便检查这个变量名是否合法；输入是定义了这个变量名的终结符的下标
	void AddVariable(SProcedure& procedure, uint32_t identTerminatorIndex);
	//同理，添加一个子程序到Procedures的末尾，同时将指针放入procedure.SubProcedures中；输入是定义了这个子程序名的终结符的下标
	void AddSubProcedure(SProcedure& procedure, uint32_t identTerminatorIndex);
	//输入的identTerminatorIndex是使用变量名的终结符的下标；返回levelDiff和offset，分别是变量的层次差和偏移量
	void FindVariable(SProcedure& procedure, uint32_t identTerminatorIndex, int16_t& levelDiff, uint32_t& offset);
	//与FindVariable类似
	void FindSubProcedure(SProcedure& procedure, uint32_t identTerminatorIndex, SProcedure*& calledProcedure, int16_t& levelDiff);

	/*
	* 各种语法分析函数
	*/
	void Program();

	//匹配一个终结符，如果是标识符或数字，将其值保存到numverValue或identifierName中
	void Match(const std::string& type, int32_t* numverValue = nullptr, std::string* identifierName = nullptr);
	void Procedure(SProcedure& procedure);

	void ConstDeclare(SProcedure& procedure);
	void VarDeclare(SProcedure& procedure);
	//子程序声明，输入的procedure是父程序
	void ProcedureDeclare(SProcedure& procedure);
	void Statement(SProcedure& procedure);
	void StatementSequence(SProcedure& procedure);
	void AssignStatement(SProcedure& procedure);
	void CallStatement(SProcedure& procedure);
	void BeginEndStatement(SProcedure& procedure);
	void IfStatement(SProcedure& procedure);
	void WhileStatement(SProcedure& procedure);
	void Condition(SProcedure& procedure);
	void OddCondition(SProcedure& procedure);
	void CompareCondition(SProcedure& procedure);
	//表达式；其对应的指令执行完毕后，栈顶的值就是表达式的值
	void Expression(SProcedure& procedure);
	//项
	void Term(SProcedure& procedure);
	//因子
	void Factor(SProcedure& procedure);
	/*
	主程序是特殊的，它的RA为0，调用RET时会退出程序
	在解释程序开始运行时，先手动向栈中压入三个0，占据DL、SL、RA的位置
	*/

public:
	CCodeGenerator(const std::vector<STerminator>& terminatorSequence);

	//同时完成语法分析、语义分析、代码生成；将生成的指令序列保存在Instructions中
	void GenerateCode();

	void PrintInstructions();

	//将Instructions中的指令序列输出到二进制文件中
	void Output(const std::string& FileName);
};
