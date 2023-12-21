#include "Utils.h"
#include <iostream>

void Error(const std::string& message)
{
	std::cerr <<"Error: " << message << std::endl;
	//throw std::exception{};	//debug时，抛出错误可以方便定位错误位置
	exit(1);
}
