#pragma once
#include <vector>
#include <string>
#include <unordered_set>


/*
在语法分析中，终结符有两种类型，一种是由其字符串就知道其意义的，即一种字符串就是一种类型，包括关键字、特殊符号；
另一种是由其字符串不代表其意义的，包括标识符（"ident"）、数字（"number"）。
*/
struct STerminator {
	size_t Line;				// 该终结符所在的行数
	std::string Type;
	int32_t NumberValue;		// 仅当Type为"number"时有效
	std::string IdentifierName;	// 仅当Type为"ident"时有效
};

class CLexicalAnalyzer {
private:
	std::vector<STerminator> TerminatorSequence;
	std::string SourceFile;
	size_t FileSize;
	char GetChar(size_t position);

public:
	CLexicalAnalyzer(const std::string& sourceFilePath);
	void LexicalAnalyze();
	const std::vector<STerminator>& GetTerminatorSequence();
};


//让语法分析过程可以查看的变量
extern const std::unordered_set<std::string> Keywords;
//让语法分析过程可以调用的函数
bool IsPossibleTerminatorType(const std::string& type);


