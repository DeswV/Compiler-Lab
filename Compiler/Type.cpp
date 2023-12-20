#include "Type.h"


bool SType::operator==(const SType& other) const
{
	if (Type != other.Type) return false;
	if (Type == EType::Integer) return true;
	else if (Type == EType::Array) return ArraySize == other.ArraySize && *InnerType == *other.InnerType;
	else if (Type == EType::Pointer) return *InnerType == *other.InnerType;
	else return false;
}

bool SType::operator!=(const SType& other) const
{
	return !(*this == other);
}

uint32_t GetSize(const SType& type)
{
	if (type.Type == EType::Integer) return 1;
	else if (type.Type == EType::Array) return type.ArraySize * GetSize(*type.InnerType);
	else if (type.Type == EType::Pointer) return 1;
	else return 0;
}

SType BuildMultiLevelPointerType(uint32_t level, const SType& innerType)
{
	if (level == 0) return innerType;

	SType result{
		EType::Pointer,
		std::make_shared<SType>(BuildMultiLevelPointerType(level - 1,innerType))
	};
	return result;
}

SType BuildNDimArrayType(const std::vector<uint32_t>& dimensions, uint32_t startIndex, const SType& innerType)
{
	if (startIndex == dimensions.size()) return innerType;
	else {
		SType result{
			EType::Array,
			std::make_shared<SType>(BuildNDimArrayType(dimensions, startIndex + 1, innerType)),
			dimensions[startIndex]
		};
		return result;
	}
}
