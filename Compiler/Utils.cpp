#include "Utils.h"
#include <iostream>

void Error(const std::string& message)
{
	std::cerr <<"Error: " << message << std::endl;
	//throw std::exception{};	//debugʱ���׳�������Է��㶨λ����λ��
	exit(1);
}
