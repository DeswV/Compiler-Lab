#pragma once
#include "LexicalAnalyzer.h"
#include "Intruction.h"


struct SVariable {
	std::string Name;
	uint32_t Offset;		//�洢�ں���ջ�е�ƫ����
};

//һ���ӳ���
struct SProcedure
{
	SProcedure* Parent;						//nullptr����Ϊ������
	int16_t Level;							//���

	std::string Name;
	std::vector<SVariable> Variables;
	std::vector<SProcedure*> SubProcedures;

	std::vector<Instruction> Instructions;
	uint32_t Address;						//�ӳ������ڵ�ַ
};
/*
������Ĳ��Ϊ0��������ľֲ������Ĳ��Ϊ0
*/

//��¼�µ�����CAL��ָ��ȴ�����
struct SCallIntruction {
	SProcedure* Procedure;					//����ָ�����ڵ��ӳ���
	uint32_t CallInstructionOffset;			//����ָ���ƫ����
	SProcedure* CalledProcedure;			//�����õ��ӳ���
	int16_t LevelDifference;				//��β�
};

class CodeGenerator
{
private:
	const std::vector<STerminator>& TerminatorSequence;	//����Ĵʷ��������
	uint32_t CurrentIndex{};							//��ǰ����Ĵʷ�����������±�
	std::vector<Instruction> Instructions;				//���յõ���ָ������

	std::vector<SProcedure> Procedures;					//���е��ӳ���
	std::vector<SCallIntruction> CallInstructions;		//���еĵ���ָ����ڻ���

	/*
	* һЩ��������
	*/
	//�õ���һ���ս�����ͣ�˳�����Ƿ������һ���ս�����������򱨴�
	std::string GetNextTerminatorType();
	//��procedure.Variables�����һ��������˳��������������Ƿ�Ϸ��������Ƕ�����������������ս�����±�
	void AddVariable(SProcedure& procedure, uint32_t identTerminatorIndex);

	/*
	* �����﷨��������
	*/
	void Program();
	
	//ƥ��һ���ս��������Ǳ�ʶ�������֣�����ֵ���浽numverValue��identifierName��
	void Match(const std::string& type, int32_t* numverValue=nullptr, std::string* identifierName=nullptr);
	void Procedure(SProcedure& procedure);

	void ConstDeclare(SProcedure& procedure);
	void VarDeclare(SProcedure& procedure);
	//�ӳ��������������procedure�Ǹ�����
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
	void Expression(SProcedure& procedure);
	void Term(SProcedure& procedure);					//��
	void Factor(SProcedure& procedure);					//����
	/*
	������������ģ�����RAΪ0������RETʱ���˳�����
	�ڽ��ͳ���ʼ����ʱ�����ֶ���ջ��ѹ������0��ռ��DL��SL��RA��λ��
	*/

public:
	CodeGenerator(const std::vector<STerminator>& terminatorSequence);

	//ͬʱ����﷨����������������������ɣ������ɵ�ָ�����б�����Instructions��
	void GenerateCode();

	//��Instructions�е�ָ������������������ļ���
	void Output(const std::string& FileName);
};
