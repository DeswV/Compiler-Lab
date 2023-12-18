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
constexpr uint16_t RET = 8;	//返回

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
