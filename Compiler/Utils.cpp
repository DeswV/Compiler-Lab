#include "Utils.h"
#include <iostream>

void Error(const std::string& message)
{
	std::cerr <<"Error: " << message << std::endl;
	exit(1);
}
