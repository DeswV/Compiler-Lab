#pragma once
#include <memory>
#include <vector>


enum class EType :uint8_t {
	Integer,
	Array,
	Pointer
};

struct SType {
	EType Type;
	std::shared_ptr<SType> InnerType;	//����������ָ�룬����ָ��ָ������ͻ�����Ԫ�ص�����
	uint32_t ArraySize;					//��������飬��������Ĵ�С
	
	bool operator==(const SType& other) const;
	bool operator!=(const SType& other) const;
};

uint32_t GetSize(const SType& type);
//����һ���༶ָ�����ͣ�level����ָ��ļ���
//levelΪ0ʱ������innerType
SType BuildMultiLevelPointerType(uint32_t level, const SType& innerType);
//����һ��nά�������ͣ�dimensions����ÿһά�Ĵ�С��startIndex����dimensions�еĵ�һ��Ԫ�ص��±�
//startIndexΪdimensions.size()ʱ��������ά��Ϊ0ʱ������innerType
SType BuildNDimArrayType(const std::vector<uint32_t>& dimensions, uint32_t startIndex, const SType& innerType);


