#include "Utils.h"
#include <iostream>

void Error(const std::string& message)
{
	std::cerr <<"Error: " << message << std::endl;
	throw std::exception(message.c_str());
}
