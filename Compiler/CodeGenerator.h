#pragma once
#include <memory>
#include <cstdint>
#include "LexicalAnalyzer.h"
#include "Instruction.h"
#include "Type.h"

struct SScopedIdentifier {
	std::vector<std::string> Identifiers;
	//�Ƿ��main��������ʼ
	bool bStartFromMain;
	/*
	����::p1::p2::a���Ǵ�main��������ʼ�ģ���p1::a�Ͳ���
	*/

	std::string ToString() const;
};

//���ڼ�¼һ���ӳ���ӵ�еı���
struct SVariable {
	std::string Name;
	SType Type;
	uint32_t Offset;		//�洢�ں���ջ�е�ƫ����
	bool bIsConst;
	/*
	����������������ô��Expression()�лὫ��ת��Ϊָ��
	*/
};

//������ʽ�����һ���м�ֵ����ֵ����ֵ
struct SValue {
	SType Type;
	bool bIsConst;
};

//һ���ӳ���
struct SProcedure
{
	SProcedure* Parent;						//nullptr����Ϊ������
	int16_t Level;							//���
	std::string Name;						//�ӳ������������������Ϊ"main"

	uint32_t StackOffset{ 3 };				//��һ���ֲ�������ջ�е�ƫ��������3��ʼ����ΪDL��SL��RAռ����0��1��2
	std::vector<SVariable> Variables;
	std::vector<SProcedure*> SubProcedures;

	std::vector<Instruction> Instructions;
	uint32_t Address;						//�ӳ������ڵ�ַ
};
/*
������Ĳ��Ϊ0��������ľֲ������Ĳ��Ϊ0
*/

//���ڼ�¼������CAL��ָ��ȴ�����
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
	void AddVariable(SProcedure& procedure, uint32_t identTerminatorIndex, const SType& type, bool isConst = false);
	//ͬ�����һ���ӳ���Procedures��ĩβ��ͬʱ��ָ�����procedure.SubProcedures�У������Ƕ���������ӳ��������ս�����±�
	void AddSubProcedure(SProcedure& procedure, uint32_t identTerminatorIndex);
	//�����scopedIdentifier�Ǵ�������ı�ʶ����Ҳ���Բ��������򣩣����������͡���βƫ�������Ƿ�Ϊ����
	void FindVariable(SProcedure& procedure, const SScopedIdentifier& scopedIdentifier, SType& type, int16_t& levelDiff, uint32_t& offset, bool& isConst);
	//��FindVariable����
	void FindSubProcedure(SProcedure& procedure, uint32_t identTerminatorIndex, SProcedure*& calledProcedure, int16_t& levelDiff);

	/*
	* �����﷨��������
	*/
	void Program();

	//ƥ��һ���ս��������Ǳ�ʶ�������֣�����ֵ���浽numverValue��identifierName��
	void Match(const std::string& type, int32_t* numverValue = nullptr, std::string* identifierName = nullptr);
	//ƥ��һ����������ı�ʶ����Ҳ���Բ��������򣩣������scopedIdentifier
	void ScopedIdentifier(SScopedIdentifier& scopedIdentifier);
	void Procedure(SProcedure& procedure);

	void ConstDeclare(SProcedure& procedure);
	void VarDeclare(SProcedure& procedure);
	void VarDefine(SProcedure& procedure);
	//�ӳ��������������procedure�Ǹ�����
	void ProcedureDeclare(SProcedure& procedure);
	void Statement(SProcedure& procedure);
	void StatementSequence(SProcedure& procedure);
	void AssignStatement(SProcedure& procedure);
	void CallStatement(SProcedure& procedure);
	void BeginEndStatement(SProcedure& procedure);
	void IfStatement(SProcedure& procedure);
	void WhileStatement(SProcedure& procedure);
	void PrintStatement(SProcedure& procedure);
	void Condition(SProcedure& procedure);
	void OddCondition(SProcedure& procedure);
	void CompareCondition(SProcedure& procedure);
	//���ʽ�����Ӧ��ָ��ִ����Ϻ�ջ����ֵ���Ǳ��ʽ��ֵ������ֵ�����˱��ʽ��ֵ������
	//����Ӧ��ָ��ӵ�instructions��
	//֮����Ҫ�������������ڶ��ڸ�ֵ���Ҫ�ȼ����ʽ�ұ��ټ����ʽ��ߣ������Ҫ�ݴ��ʽ�ұߵ�ָ��
	SValue Expression(SProcedure& procedure, std::vector<Instruction>& instructions);
	//��
	SValue Term(SProcedure& procedure, std::vector<Instruction>& instructions);
	//���ӣ���isLeftValue=false����ָ��ִ����Ϻ�ջ��Ϊ���ӵ�ֵ����ָ֮��ִ����Ϻ�ջ��Ϊ��ֵ�ĵ�ַ
	SValue Factor(SProcedure& procedure, std::vector<Instruction>& instructions, bool isLeftValue = false);
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
