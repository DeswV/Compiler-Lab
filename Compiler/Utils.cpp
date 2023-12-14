#include "Utils.h"
#include <iostream>

void Error(const std::string& message)
{
	std::cout << message << std::endl;
	exit(1);
}
