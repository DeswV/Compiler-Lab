#include "Utils.h"
#include <iostream>

void Error(const std::string& message)
{
	std::cout <<"Error: " << message << std::endl;
	exit(1);
}
