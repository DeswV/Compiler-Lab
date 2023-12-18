#pragma once
#include <cstdint>

struct Instruction
{
	uint16_t F;
	int16_t L;
	int32_t a;
};

constexpr uint16_t INT = 0;
constexpr uint16_t LIT = 1;
constexpr uint16_t LOD = 2;
constexpr uint16_t STO = 3;
constexpr uint16_t CAL = 4;
constexpr uint16_t JMP = 5;
constexpr uint16_t JPC = 6;
constexpr uint16_t OPR = 7;
constexpr uint16_t RET = 8;	//·µ»Ø
