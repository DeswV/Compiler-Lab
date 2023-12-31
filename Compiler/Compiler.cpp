﻿#include <iostream>
#include "GlobalVariable.h"
#include "LexicalAnalyzer.h"
#include "CodeGenerator.h"

//用于测试词法分析器
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

void ShowUsage() {
	std::cout << "Usage: " << std::endl<<std::endl;
	std::cout << "Compiler <SourceFilePath> <OutputFilePath>" << std::endl;
	exit(0);
}

int main(int argc,char** argv) {
	
	//从命令行参数中读取源文件路径和目标文件路径
	if(argc != 3) {
		ShowUsage();
	}
	else {
		SourceFilePath = argv[1];
		OutputFilePath = argv[2];
	}
	
	//LexicalAnalyzerTest();

	CLexicalAnalyzer LexicalAnalyzer{ SourceFilePath };
	LexicalAnalyzer.LexicalAnalyze();
	auto TerminatorSequence = LexicalAnalyzer.GetTerminatorSequence();
	CCodeGenerator CodeGenerator{ TerminatorSequence };
	CodeGenerator.GenerateCode();
	CodeGenerator.PrintInstructions();	//打印生成的指令
	CodeGenerator.Output(OutputFilePath);
}

