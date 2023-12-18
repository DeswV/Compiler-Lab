#include <iostream>
#include "GlobalVariable.h"
#include "LexicalAnalyzer.h"
#include "CodeGenerator.h"

void LexicalAnalyzerTest()
{
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

int main() {
	//TODO：从命令行参数中读取源文件路径和目标文件路径                 
	CLexicalAnalyzer LexicalAnalyzer{ SourceFilePath };
	LexicalAnalyzer.LexicalAnalyze();
	auto TerminatorSequence = LexicalAnalyzer.GetTerminatorSequence();
	CCodeGenerator CodeGenerator{ TerminatorSequence };
	CodeGenerator.GenerateCode();
	CodeGenerator.PrintInstructions();
	CodeGenerator.Output("OutputFilePath");
}

