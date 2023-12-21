#pragma once
#include <cstdint>
#include <memory>
#include <vector>


enum class EType :uint8_t {
	Integer,
	Array,
	Pointer
};

struct SType {
	EType Type;
	std::shared_ptr<SType> InnerType;	//如果是数组或指针，代表指针指向的类型或数组元素的类型
	uint32_t ArraySize;					//如果是数组，代表数组的大小

	bool operator==(const SType& other) const;
	bool operator!=(const SType& other) const;
};

uint32_t GetSize(const SType& type);
//创建一个多级指针类型，level代表指针的级数
//level为0时，返回innerType
SType BuildMultiLevelPointerType(uint32_t level, const SType& innerType);
//创建一个n维数组类型，dimensions代表每一维的大小，startIndex代表dimensions中的第一个元素的下标
//startIndex为dimensions.size()时，即数组维度为0时，返回innerType
SType BuildNDimArrayType(const std::vector<uint32_t>& dimensions, uint32_t startIndex, const SType& innerType);