#pragma once
#include <vector>
#include <fstream>
#include <string>
#include <unordered_set>

extern std::unordered_set<std::string> Keywords;

/*
���﷨�����У��ս�����������ͣ�һ���������ַ�����֪��������ģ���һ���ַ�������һ�����ͣ������ؼ��֡�������ţ�
��һ���������ַ���������������ģ�������ʶ����"ident"�������֣�"number"����
*/
struct STerminator {
	size_t Line;				// ���ս�����ڵ�����
	std::string Type;
	int32_t NumberValue;		// ����TypeΪ"number"ʱ��Ч
	std::string IdentifierName;	// ����TypeΪ"ident"ʱ��Ч
};

class CLexicalAnalyzer {
private:
	std::vector<STerminator> TerminatorSequence;
	std::ifstream SourceFile;
	size_t FileSize;
	char GetChar(size_t position);

public:
	CLexicalAnalyzer(const std::string& sourceFilePath);
	void LexicalAnalyze();
	const std::vector<STerminator>& GetTerminatorSequence();
};


