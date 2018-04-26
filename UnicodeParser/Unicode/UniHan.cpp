#include "UniHan.h"
#include "Command.h"
#include "Log.h"

namespace unihan {

bool NumbericValue::SetType(signature& typeSignature)
{
	static signature typeStringes[3] = { "kAccountingNumeric","kOtherNumeric","kPrimaryNumeric" };
	if (typeStringes[0] == typeSignature)
	{
		type = kAccountingNumeric;
	}
	else if (typeStringes[1] == typeSignature)
	{
		type = kOtherNumeric;
	}
	else if (typeStringes[2] == typeSignature)
	{
		type = kPrimaryNumeric;
	}
	else
	{
		return false;
	}
	return true;
}
bool NumbericValue::Init(sigseparator_8& separator)
{
	separator.split('\t');
	if (separator.count() != 2)
	{
		return false;
	}

	signature codeSignature = separator.get(0);
	signature typeSignature = separator.get(1);
	signature numberSignature = separator.get(2);
	static signature codeProfix("U+", 2);
	if (!codeProfix.equal(codeSignature.begin(), 2))
	{
		return false;
	}
	codeSignature.move_begin_to_tail(2);
	code = codeSignature.to_int(16);
	if (!SetType(typeSignature))
	{
		return false;
	}
	number = numberSignature.to_long();
	return true;
}

NumbericValue* NumbericValueParser::FindValue(int32_t code)
{
	NumbericValue* value = nullptr;
	for (uint32_t i = 0; i < GetLineCount(); i++)
	{
		value = GetValue(i);
		if (!value)
		{
			break;
		}
		if (value->code == code)
		{
			return value;
		}
	}
	return nullptr;
}

bool Variant::SetType(signature& typeSignature)
{
	static signature typeStringes[5] = 
	{	"kSemanticVariant",
		"kSimplifiedVariant",
		"kSpecializedSemanticVariant",
		"kTraditionalVariant",
		"kZVariant"
	};
	if (typeStringes[0] == typeSignature)
	{
		type = kSemanticVariant;
	}
	else if (typeStringes[1] == typeSignature)
	{
		type = kSimplifiedVariant;
	}
	else if (typeStringes[2] == typeSignature)
	{
		type = kSpecializedSemanticVariant;
	}
	else if (typeStringes[3] == typeSignature)
	{
		type = kTraditionalVariant;
	}
	else if (typeStringes[4] == typeSignature)
	{
		type = kZVariant;
	}
	else
	{
		return false;
	}
	return true;
}
bool Variant::Init(sigseparator_8& separator)
{
	separator.split('\t');
	if (separator.count() < 2)
	{
		return false;
	}

	signature codeSignature = separator.get(0);
	signature typeSignature = separator.get(1);
	signature variantSignature = separator.get(2);
	static signature codeProfix("U+", 2);
	if (!codeProfix.equal(codeSignature.begin(), 2))
	{
		return false;
	}
	codeSignature.move_begin_to_tail(2);
	code = codeSignature.to_int(16);

	if (!SetType(typeSignature))
	{
		return false;
	}
	separator.reset_source(variantSignature);
	separator.split(' ');
	for (size_t i = 0; i <= separator.count(); i++)
	{
		signature& variant = separator.get(i);
		if (!codeProfix.equal(variant.begin(), 2))
		{
			return false;
		}
		variant.move_begin_to_tail(2);
		variants[i] = variant.to_int(16);
		++count;
		if (count > kMaxVariantCount)
		{
			return false;
		}
	}
	return true;
}


class NumbericValueCommand:public Command
{
public:
	NumbericValueCommand()
		:Command("hnum")
	{}
	bool Load()
	{
		if (!parser.Load("../data/ucd/Unihan_NumericValues.txt"))
		{
			LOG_TRACE("Unihan NumericValues parse failed!");
			return false;
		}
		else
		{
			LOG_TRACE("Unihan NumericValues load ok!");
			return true;
		}
	}
	int Execute(const std::string& cmd)
	{
		if (cmd == "a")
		{
			if (!Load())
			{
				return -1;
			}
			if (!parser.Parse())
			{
				LOG_TRACE("Unihan NumericValues parse error...!");
				return -1;
			}
			LOG_TRACE("Unihan NumericValues parse ok!");
			Print();
			return 1;
		}
		if (cmd == "l")
		{
			if (!Load())
			{
				return -1;
			}
			return 1;
		}
		if (cmd == "p")
		{
			if (!parser.Parse())
			{
				LOG_TRACE("Unihan NumericValues parse error...!");
				return -1;
			}
			LOG_TRACE("Unihan NumericValues parse ok!");
		}
		if (cmd == "s")
		{
			Print();
		}
		return 0;
	}
	void Print()
	{
		NumbericValue* value = nullptr;
		for (uint32_t i = 0; i < parser.GetLineCount(); i++)
		{
			value = parser.GetValue(i);
			if (value)
			{
				LOG_TRACE("Code:"<<value->code<<"\t"<<value->number);
			}
			else
			{
				break;
			}
		}
	}
	NumbericValueParser parser;
};
static NumbericValueCommand staticInstanceNumbericValueCommand;

class VariantCommand :public Command
{
public:
	VariantCommand()
		:Command("hvar")
	{}
	bool Load()
	{
		if (!parser.Load("../data/ucd/Unihan_Variants.txt"))
		{
			LOG_TRACE("Unihan Variants parse failed!");
			return false;
		}
		else
		{
			LOG_TRACE("Unihan Variants load ok!");
			return true;
		}
	}
	bool Parse()
	{
		if (!parser.Parse())
		{
			LOG_TRACE("Unihan Variants parse error...!");
			return false;
		}
		LOG_TRACE("Unihan Variants parse ok!");
		return true;
	}
	void Print()
	{
		Variant* value = nullptr;
		for (uint32_t i = 0; i < parser.GetLineCount(); i++)
		{
			value = parser.GetValue(i);
			if (value && value->count >= 3)
			{
				LOG_TRACE("Code:" << value->code << "\t" << value->count);
			}
		}
	}
	int Execute(const std::string& cmd)
	{
		if (cmd == "a")
		{
			if (!Load())
			{
				return 0;
			}
			if (!Parse())
			{
				return 0;
			}
			Print();
			return 1;
		}
		if (cmd == "l")
		{
			if (!Load())
			{
				return -1;
			}
			return 1;
		}
		if (cmd == "p")
		{
			if (!Parse())
			{
				return -1;
			}
			return 1;
		}
		if (cmd == "s")
		{
			Print();
		}
		return 0;
	}
	VariantParser	parser;
};

static VariantCommand staticInstanceVariantCommand;
}//end of namespace unihan
