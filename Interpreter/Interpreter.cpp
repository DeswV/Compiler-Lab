#include <iostream>
#include "Pl0VirtualMachine.h"

void ShowUsage()
{
	std::cout << "Usage: \n\n";
	std::cout << "Interpreter <ExecutableFilePath>\n";
	exit(0);
}

int main(int argc, char** argv)
{
	
	//从命令行参数中获取文件路径
	if(argc != 2)
		ShowUsage();

	Pl0VirtualMachine vm{argv[1]};
	vm.Run();
	

	/*
	Pl0VirtualMachine vm{ "test" };
	vm.Run();
	*/
}
