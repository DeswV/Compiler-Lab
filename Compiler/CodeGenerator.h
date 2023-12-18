#pragma once
#include "LexicalAnalyzer.h"
#include "Intruction.h"
#include <memory>


struct SVariable {
	std::string Name;
	uint32_t Offset;		//�洢�ں���ջ�е�ƫ����
};

//һ���ӳ���
struct SProcedure
{
	SProcedure* Parent;						//nullptr����Ϊ������
	int16_t Level;							//���
	std::string Name;						//�ӳ������������������Ϊ"main"

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

class CCodeGenerator
{
private:
	const std::vector<STerminator>& TerminatorSequence;	//����Ĵʷ��������
	uint32_t CurrentIndex{};							//��ǰ����Ĵʷ�����������±�
	std::vector<Instruction> Instructions;				//���յõ���ָ������

	std::vector<std::shared_ptr<SProcedure>> Procedures;//���е��ӳ���std::vector������ʱ���ƶ��ڴ棬���ʹ������ָ��
	std::vector<SCallIntruction> CallInstructions;		//���еĵ���ָ����ڻ���

	/*
	* һЩ��������
	*/
	//�õ���һ���ս�����ͣ�˳�����Ƿ������һ���ս�����������򱨴�
	std::string GetNextTerminatorType();
	//��procedure.Variables�����һ��������˳��������������Ƿ�Ϸ��������Ƕ�����������������ս�����±�
	void AddVariable(SProcedure& procedure, uint32_t identTerminatorIndex);
	//ͬ�����һ���ӳ���Procedures��ĩβ��ͬʱ��ָ�����procedure.SubProcedures�У������Ƕ���������ӳ��������ս�����±�
	void AddSubProcedure(SProcedure& procedure, uint32_t identTerminatorIndex);
	//�����identTerminatorIndex��ʹ�ñ��������ս�����±ꣻ����levelDiff��offset���ֱ��Ǳ����Ĳ�β��ƫ����
	void FindVariable(SProcedure& procedure, uint32_t identTerminatorIndex, int16_t& levelDiff, uint32_t& offset);
	//��FindVariable����
	void FindSubProcedure(SProcedure& procedure, uint32_t identTerminatorIndex, SProcedure*& calledProcedure, int16_t& levelDiff);

	/*
	* �����﷨��������
	*/
	void Program();

	//ƥ��һ���ս��������Ǳ�ʶ�������֣�����ֵ���浽numverValue��identifierName��
	void Match(const std::string& type, int32_t* numverValue = nullptr, std::string* identifierName = nullptr);
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
	//���ʽ�����Ӧ��ָ��ִ����Ϻ�ջ����ֵ���Ǳ��ʽ��ֵ
	void Expression(SProcedure& procedure);
	//��
	void Term(SProcedure& procedure);
	//����
	void Factor(SProcedure& procedure);
	/*
	������������ģ�����RAΪ0������RETʱ���˳�����
	�ڽ��ͳ���ʼ����ʱ�����ֶ���ջ��ѹ������0��ռ��DL��SL��RA��λ��
	*/

public:
	CCodeGenerator(const std::vector<STerminator>& terminatorSequence);

	//ͬʱ����﷨����������������������ɣ������ɵ�ָ�����б�����Instructions��
	void GenerateCode();

	void PrintInstructions();

	//��Instructions�е�ָ������������������ļ���
	void Output(const std::string& FileName);
};
