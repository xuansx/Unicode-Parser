#pragma once
#include "UParser.h"

namespace unihan {
class NumbericValue
{
public:
	enum eNumbericType{
		kAccountingNumeric,
		kOtherNumeric,
		kPrimaryNumeric
	};
	bool SetType(signature& typeSignature);
	bool Init(sigseparator_8& separator);
	int32_t			code;
	eNumbericType	type;
	int64_t			number;
};


class NumbericValueParser:public DataLineParser<NumbericValue,80>
{
public:
	NumbericValue* FindValue(int32_t code);
};

struct Variant
{
	enum eVariantType
	{
		kSemanticVariant,
		kSimplifiedVariant,
		kSpecializedSemanticVariant,
		kTraditionalVariant,
		kZVariant,
	};
	enum {
		kMaxVariantCount = 5,
	};
	Variant()
		:code(0),count(0)
	{
		memset(variants, 0, sizeof(variants));
	}
	int32 code;
	eVariantType type;
	uint32	count;
	int32	variants[kMaxVariantCount];
	bool SetType(signature& typeSignature);
	bool Init(sigseparator_8& separator);
};

class VariantParser :public DataLineParser<Variant, 12380>
{
public:
};

}//end of namespace unihan
