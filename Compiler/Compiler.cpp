#include <iostream>
#include "GlobalVariable.h"
#include "LexicalAnalyzer.h"

int main()
{
	//TODO：从命令行读取SourceFile和OutputFile
	CLexicalAnalyzer LexicalAnalyzer{ SourceFilePath };
	LexicalAnalyzer.LexicalAnalyze();
	auto TerminatorSequence = LexicalAnalyzer.GetTerminatorSequence();
	for (auto& Terminator : TerminatorSequence) {
		std::cout<<"Line " << Terminator.Line << " ";
		std::cout << Terminator.Type << " ";
		if (Terminator.Type == "number") {
			std::cout << Terminator.NumberValue<<' ';
		}
		else if (Terminator.Type == "ident") {
			std::cout << Terminator.IdentifierName<<' ';
		}
		std::cout << std::endl;
	}
}

