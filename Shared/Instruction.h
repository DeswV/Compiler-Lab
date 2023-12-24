#pragma once
#include <cstdint>

struct Instruction
{
	uint16_t F;
	int16_t L;
	int32_t a;
};

//F中的操作码
constexpr uint16_t INT = 0;
constexpr uint16_t LIT = 1;
constexpr uint16_t LOD = 2;
constexpr uint16_t STO = 3;
constexpr uint16_t CAL = 4;
constexpr uint16_t JMP = 5;
constexpr uint16_t JPC = 6;
constexpr uint16_t OPR = 7;
constexpr uint16_t RET = 8;		//返回
constexpr uint16_t LOR = 9;		//加载栈顶元素指向的内容
constexpr uint16_t STR = 10;	//存储次栈顶元素的值到栈顶指向的位置
constexpr uint16_t LBP = 11;	//将BasePointer压栈
constexpr uint16_t WRT = 12;	//输出一个数
constexpr uint16_t LOA = 13;	//加载变量的地址
constexpr uint16_t RAN_N = 14;
constexpr uint16_t RAN = 15;


//OPR指令的a中的操作码
constexpr int32_t Add = 0;
constexpr int32_t Sub = 1;
constexpr int32_t Mul = 2;
constexpr int32_t Div = 3;
constexpr int32_t Neg = 4;
constexpr int32_t LessThan = 5;
constexpr int32_t LessEqual = 6;
constexpr int32_t Equal = 7;
constexpr int32_t NotEqual = 8;
constexpr int32_t GreaterEqual = 9;
constexpr int32_t GreaterThan = 10;
constexpr int32_t Odd = 11;
