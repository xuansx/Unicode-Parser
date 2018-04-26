#pragma once
#include "FileIO.h"
#include "signature.hpp"
#include "LinkedList.h"
#include "Memory/ZoneAllocator.h"
#include <vector>
#include <stack>

namespace nameslist {

struct Token
{
	enum eType {
		Token_Code = 1,
		Token_AT,//@
		Token_TAB,//"\t"
		Token_LF,//'\r'or'\n'
		Token_SP,//' '
		Token_AngleL,
		Token_AngleR,
		Token_BracketL,
		Token_BracketR,
		Token_String,
	};
};
enum eCharLineType
{
	EMPTY_LINE,
	ALIAS_LINE,//=
	FORMAL_ALIAS_LINE,//%
	COMMENT_LINE,//*
	CROSS_REF,//x
	VARIATION_LINE,//~
	DECOMPOSITION,//:
	COMPAT_MAPPING,//#
};

class CharNameInfo:public ZoneObject
{
public:
	enum CharType {
		Pseudo,
		Printable,
	};
	void SetType(bool printable)
	{
		if (printable)
		{
			type = Printable;
		}
		else
		{
			type = Pseudo;
		}
	}
	char32_t	code;
	CharType	type;
	signature	name;
	signature	comment;
};

/*
FILE_COMMENT:	";"  LINE

EMPTY_LINE:	LF
// Empty and ignored lines as well as
// file comments are ignored

IGNORED_LINE:	TAB ";" LINE
// Ignore LINE

SIDEBAR_LINE: 	";;" LINE
// Output LINE as marginal note
*/

typedef std::vector<Token::eType>	TokenVector;
typedef std::vector<signature>		SignatureVector;
struct ParserBase
{
	constexpr static char	CH_TAB = '\t';
	constexpr static char	CH_SPACE = ' ';
	constexpr static char	CH_HYPHEN = '-';
	template <typename FuncType>
	bool parseStringBy(const char* source, signature& sig, FuncType& condFunc)
	{
		sig.reset(source);
		while (*source)
		{
			if (condFunc(*source))
			{
				sig.grow();
			}
			else
			{
				break;
			}
			if (isEndLine(*source))
			{
				break;
			}
			++source;
		}
		return !sig.empty();
	}
	template <typename FuncType>
	bool parseStringBy(const char* source, signature& sig,char endCh, FuncType& condFunc)
	{
		sig.reset(source);
		while (*source)
		{
			if (condFunc(*source)&& endCh != *source)
			{
				sig.grow();
			}
			else
			{
				break;
			}
			if (isEndLine(*source))
			{
				break;
			}
			++source;
		}
		return !sig.empty();
	}
	bool parseNAME(const char* source, signature& sig)
	{
		return parseStringBy(source, sig, [](int c)->bool
		{
			if (isupper(c) ||
				isdigit(c) ||
				c == ' ' ||
				c == '-')
			{
				return true;
			}
			else
			{
				return false;
			}
		});
	}
	bool parseName(const char* source, signature& sig)
	{
		return parseStringBy(source, sig, [](int c)->bool
		{
			if (isalnum(c) ||
				c == ' ' ||
				c == '-')
			{
				return true;
			}
			else
			{
				return false;
			}
		});
	}
	bool parseLCNAME(const char* source, signature& sig)
	{
		return parseStringBy(source, sig, [](int c)->bool
		{
			if (islower(c) ||
				isdigit(c) ||
				c == ' ' ||
				c == '-')
			{
				return true;
			}
			else
			{
				return false;
			}
		});
	}
	bool parseLCNAMEWithOutHyphen(const char* source, signature& sig)
	{
		bool lastCode = false;
		sig.reset(source);
		const char* nextStr = nullptr;
		while (*source)
		{
			if (islower(*source) ||
				isdigit(*source))
			{
				sig.grow();
			}
			else if (*source == ' ')
			{
				nextStr = source;
				++nextStr;
				if (*nextStr == '-')
				{
					++nextStr;
					if (*nextStr == ' ')
					{
						break;
					}
				}
				sig.grow();
			}
			else if (*source == '-')
			{
				nextStr = source + 1;
				sig.grow();
				signature code;
				if (parseCode(nextStr,code))
				{
					++source;
					lastCode = true;
					break;
				}
			}
			else
			{
				break;
			}
			if (isEndLine(*source))
			{
				break;
			}
			++source;
		}
		if (lastCode)
		{
			while (*source)
			{
				if (isxdigit(*source))
				{
					sig.grow();
				}
				else
				{
					break;
				}
				if (isEndLine(*source))
				{
					break;
				}
				++source;
			}
		}
		return !sig.empty();
		//return parseStringBy(source, sig, [&](int c)->bool
		//{
		//	if (islower(c) ||
		//		isdigit(c) ||
		//		c == ' ')
		//	{
		//		lastCh = c;
		//		return true;
		//	}
		//	if (c == '-' && lastCh != ' ')
		//	{
		//		lastCh = c;
		//		return true;
		//	}
		//	else
		//	{
		//		return false;
		//	}
		//});
	}
	bool parseSTRING(const char* source, signature& sig)
	{
		return parseStringBy(source, sig, [](int c)->bool
		{
			if (c < 0)//À©Õ¹ASCII
			{
				return true;
			}
			if (!iscntrl(c))
			{
				return true;
			}
			else
			{
				return false;
			}
		});
	}
	bool parseLABEL(const char* source, signature& sig)
	{
		return parseStringBy(source, sig, [](int c)->bool
		{
			if (iscntrl(c)||
				c == '('||
				c == ')')
			{
				return false;
			}
			return true;
		});
	}
	bool parseCOMMENT(const char* source, signature& sig)
	{
		if (*source != '(')
		{
			return false;
		}
		sig.grow();
		++source;
		if (!parseLABEL(source,sig))
		{
			return false;
		}
		if (*source != ')')
		{
			return false;
		}
		sig.grow();
		//++source;
		return true;
	}
	bool parseCode(const char* source, signature& sig)
	{
		bool res =  parseStringBy(source, sig, [](int c)->bool
		{
			if (isxdigit(c))
			{
				return true;
			}
			else
			{
				return false;
			}
		});
		if (res && isCodeRight(sig))
		{
			return true;
		}
		return false;
	}
	bool isEndLine(int c)const
	{
		return c == '\r' || c == '\n';
	}
	bool isCodeRight(signature& code)
	{
		if (code.length() < 4 || code.length() > 6)
		{
			return false;
		}
		return true;
	}
};
struct CharNameParser:public ParserBase
{
	/*
	NAME_LINE:	CHAR TAB NAME LF
	// The CHAR and the corresponding image are echoed,
	// followed by the name as given in NAME

	| CHAR TAB "<" LCNAME ">" LF
	// Control and noncharacters use this form of
	// lowercase, bracketed pseudo character name

	| CHAR TAB NAME SP COMMENT LF
	// Names may have a comment, which is stripped off
	// unless the file is parsed for an ISO style list

	| CHAR TAB "<" LCNAME ">" SP COMMENT LF
	// Control and noncharacters may also have comments
	and
	RESERVED_LINE:	CHAR TAB "<reserved>" LF
	*/
	const char* Parse(const char* source, bool*ok);
	void ToData(CharNameInfo* chn);
	void ClearData()
	{
		code.reset();
		name.reset();
		comment.reset();
	}
	bool isPrintable;
	signature code;
	signature name;
	signature comment;
};

struct CharLine:public LinkedNodeProto<CharLine>,public ZoneObject
{
public:
	void* operator new(size_t size, ZoneMemory* zone) { return zone->New(size); }
	CharLine(eCharLineType ty)
		:type(ty)
	{
	}
	eCharLineType	type;
private:
	// Hidden to prevent accidental usage. It would have to load the
	// current zone from the TLS.
	void* operator new(size_t size) {};
};
struct LineParser:public ParserBase
{
	const char* preLineHead(const char* source,char lineC, bool* ok)
	{
		if (*source != lineC)
		{
			*ok = false;
			return source;
		}
		++source;
		if (*source != ' ')
		{
			*ok = false;
			return source;
		}
		++source;
		*ok = true;
		return source;
	}
	virtual const char* Parse(const char* source, bool* ok) = 0;
	virtual CharLine*	ToData(ZoneMemory* zone) = 0;
	virtual void		ClearData() = 0;
	virtual eCharLineType GetType()const = 0;
};


struct CommentCharLine :public CharLine
{
	CommentCharLine()
		:CharLine(COMMENT_LINE)
	{}
	std::string comment;
};
struct CommentLineParser:public LineParser
{
	CommentLineParser()
		:continueLine(nullptr)
	{}
	/*
	COMMENT_LINE:	TAB "*" SP EXPAND_LINE
	// * is replaced by BULLET, output line as comment
	|TAB EXPAND_LINE
	// Output line as comment
	*/
	const char* Parse(const char* source, bool* ok);
	CharLine*	ToData(ZoneMemory* zone);
	void		ClearData()
	{
		comment.clear();
		continueLine = nullptr;
	}
	eCharLineType GetType()const
	{
		return COMMENT_LINE;
	}
	std::string comment;
	CommentCharLine* continueLine;
};

struct AliasCharLine :public CharLine
{
	AliasCharLine()
		:CharLine(ALIAS_LINE) {}
	signature name;
};
struct AliasCharLineParser:public LineParser
{
	/*
	TAB "=" SP LINE
	// Replace = by itself, output line as alias
	*/	
	const char* Parse(const char* source, bool* ok);
	CharLine*	ToData(ZoneMemory* zone);
	void		ClearData()
	{
		name.reset();
	}
	eCharLineType GetType()const
	{
		return ALIAS_LINE;
	}
	signature name;
};

struct FormalAliasCharLine :public CharLine
{
	FormalAliasCharLine()
		:CharLine(FORMAL_ALIAS_LINE) {}
	signature name;
};
struct FormalAliasParser :public LineParser
{
	/*
	TAB "%" SP NAME LF
	// Replace % by U+203B, output line as formal alias
	*/
	const char* Parse(const char* source, bool* ok);
	CharLine*	ToData(ZoneMemory* zone);
	void		ClearData()
	{
		name.reset();
	}
	eCharLineType GetType()const
	{
		return FORMAL_ALIAS_LINE;
	}
	signature name;
};

struct CrossRefLine :public CharLine
{
	CrossRefLine()
		:CharLine(CROSS_REF) {}
	int code;
	bool	  isPoseudo;
	signature lcname;
};
struct CrossRefParser :public LineParser
{
/*
CROSS_REF:	TAB "x" SP CHAR SP LCNAME LF
			| TAB "x" SP CHAR SP "<" LCNAME ">" LF
			// x is replaced by a right arrow

			| TAB "x" SP "(" LCNAME SP "-" SP CHAR ")" LF
			| TAB "x" SP "(" "<" LCNAME ">" SP "-" SP CHAR ")" LF
			// x is replaced by a right arrow;
			// (second type as used for control and noncharacters)

			// In the forms with parentheses the "(","-" and ")" are removed
			// and the order of CHAR and LCNAME is reversed;
			// i.e. all inputs result in the same order of output

			| TAB "x" SP CHAR LF
			// x is replaced by a right arrow
			// (this type is the only one without LCNAME
			// and is used for ideographs)
*/
	const char* Parse(const char* source, bool* ok);
	CharLine*	ToData(ZoneMemory* zone);
	void		ClearData()
	{
		isPoseudo = false;
		codeSig.reset();
		lcname.reset();
	}
	eCharLineType GetType()const
	{
		return CROSS_REF;
	}
	bool	  isPoseudo;
	signature codeSig;
	signature lcname;
};

struct VariationLine :public CharLine
{
	VariationLine()
		:CharLine(VARIATION_LINE)
	{}
	int code;
	int varsel;//select code's mean.
	signature label;
	signature lcname;
};
struct VariationLineParser :public LineParser
{
	/*
	VARIATION_LINE:	TAB "~" SP CHAR VARSEL SP LABEL LF
	| TAB "~" SP CHAR VARSEL SP LABEL "(" LCNAME ")"LF
	// output standardized variation sequence or simply the char code in case of alternate
	// glyphs, followed by the alternate glyph or variation glyph and the label and context
	*/
	const char* Parse(const char* source, bool* ok);
	CharLine*	ToData(ZoneMemory* zone);
	void		ClearData()
	{
		code.reset();
		varsel.reset();
		label.reset();
		lcname.reset();
	}
	eCharLineType GetType()const
	{
		return VARIATION_LINE;
	}
	signature code;
	signature varsel;
	signature label;
	signature lcname;
};

struct DecompositionLine :public CharLine
{
	enum {
		kMaxDecompositionField = 6,//18
	};
	DecompositionLine(signature* v,int length)
		:CharLine(DECOMPOSITION),count(length)
	{
		for (int i = 0; i < length; i++)
		{
			code[i] = v->to_int(16);
			++v;
		}
	}
	int code[kMaxDecompositionField];
	int count;
	signature postfix;
};
struct DecompositionParser :public LineParser
{
	/*
	DECOMPOSITION:	TAB ":" SP EXPAND_LINE
	// Replace ':' by EQUIV, expand line into
	// decomposition
	*/
	const char* Parse(const char* source, bool* ok);
	CharLine*	ToData(ZoneMemory* zone);
	eCharLineType GetType()const
	{
		return DECOMPOSITION;
	}
	void ClearData()
	{
		count = 0;
		for (int i = 0; i < DecompositionLine::kMaxDecompositionField; i++)
		{
			codeV->reset();
		}
		postfix.reset();
	}
	signature codeV[DecompositionLine::kMaxDecompositionField];
	signature postfix;
	int		  count;
};

struct CompatMappingLine :public CharLine
{
	enum
	{
		kMaxCodeField = 6
		//18,when kMaxCodeField = 6:the flow unparsed data will be add to postfix 
	};
	CompatMappingLine(signature& pro,signature&post,signature* v,int length)
		:CharLine(COMPAT_MAPPING),profix(pro),postfix(post),count(length)
	{
		for (int i = 0; i < length; i++)
		{
			code[i] = v->to_int(16);
			++v;
		}
	}
	signature profix;
	signature postfix;
	int		  code[kMaxCodeField];
	int		  count;
	bool	  isPoseudo;
};
struct CompatMappingParser:public LineParser
{
	/*
	COMPAT_MAPPING:	TAB "#" SP EXPAND_LINE
			| TAB "#" SP "<" LCTAG ">" SP EXPAND_LINE
			// Replace '#' by APPROX, output line as mapping;
			// check for balanced < >
	EXPAND_LINE:	{ESC_CHAR | CHAR | STRING | ESC +}+ LF
			// Instances of CHAR (see Notes) are replaced by 
			// CHAR NBSP x NBSP where x is the single Unicode
			// character corresponding to CHAR.
			// If character is combining, it is replaced with
			// CHAR NBSP <circ> x NBSP where <circ> is the 
			// dotted circle
	*/
	const char* Parse(const char* source, bool* ok);
	CharLine*	ToData(ZoneMemory* zone);
	eCharLineType GetType()const
	{
		return COMPAT_MAPPING;
	}
	void ClearData()
	{
		count = 0;
		for (int i = 0; i < CompatMappingLine::kMaxCodeField; i++)
		{
			codeV->reset();
		}
		isPoseudo = false;
	}
	signature profix;
	signature postfix;
	signature codeV[CompatMappingLine::kMaxCodeField];
	int		  count;
	bool	  isPoseudo;
};

class CharNames:public ZoneObject,public LinkedNodeProto<CharNames>
{
public:
	void* operator new(size_t size, ZoneMemory* zone) { return zone->New(size); }
	CharNames()
		:lineCount(0)
	{
	}

	void AddLine(CharLine* line)
	{
		lineList.PushBack(line);
		++lineCount;
	}
	uint32_t GetLineCount()const
	{
		return lineCount;
	}
	CharNameInfo				name;
	LinkedListProto<CharLine>	lineList;
	uint32_t					lineCount;
private:
	// Hidden to prevent accidental usage. It would have to load the
	// current zone from the TLS.
	void* operator new(size_t size) {};
}; 
typedef LinkedListProto<CharNames>	CharNamesList;

class CharNamesFactory {
public:
	CharNamesFactory()
	{}
	void Startup();
	void Shutdown();
	void Restart()
	{
		zone_.DeleteAll();
		for (size_t i = 1; i < MAX_LINE_TYPE; i++)
		{
			lineParserV[i]->ClearData();
		}
	}
	CharNames* createCharNames()
	{
		return new(getZone())CharNames();
	}
	LineParser* getParser(char c);
	CharNameParser* getNameParser()
	{
		return &nameParser_;
	}
	ZoneMemory* getZone()
	{
		return &zone_;
	}
private:
	enum {
		MAX_LINE_TYPE = COMPAT_MAPPING + 1
	};
	static char LinesHeadChar[MAX_LINE_TYPE];
	void initParserV()
	{
		lineParserV[0] = nullptr;
		lineParserV[ALIAS_LINE]			= &aliasLineParser;
		lineParserV[FORMAL_ALIAS_LINE]	= &formalAliasLineParser;
		lineParserV[COMMENT_LINE]		= &commentLineParser;
		lineParserV[CROSS_REF]			= &crossRefLineParser;
		lineParserV[VARIATION_LINE]		= &variationLineParser;
		lineParserV[DECOMPOSITION]		= &decompositionLineParser;
		lineParserV[COMPAT_MAPPING]		= &compatMappingLineParser;
	}
	LineParser*			lineParserV[MAX_LINE_TYPE];
	CharNameParser		nameParser_;
	ZoneMemory			zone_;
	CommentLineParser	commentLineParser;
	AliasCharLineParser	aliasLineParser;
	FormalAliasParser	formalAliasLineParser;
	CrossRefParser		crossRefLineParser;
	VariationLineParser variationLineParser;
	DecompositionParser	decompositionLineParser;
	CompatMappingParser	compatMappingLineParser;
};


class NamesListParser
{
public:
	enum eScope {
		NoneScope,
		BlockScope,
		CharScope,
		LineScope,
	};
	enum eLineHead {
		Line_AT,
		Line_TAB,
		Line_Code,
		Line_Comment,
		Line_Other,
	};
	enum eError {
		EOK,
		EMemoryException,
		ECodeFormatNotRight,
		EBadCharName,
		EBadLineHead,
		EBadLineEnd,
		EUnknownCharLine,
		EBadLineFormat,
		EBadAliasLine,//=
		EBadFormalAliasLine,//%
		EBadCommentLine,//*
		EBadCrossRef,//x
		EBadVariationLine,//~
		EBadDecomposition,//:
		EBadCompatMapping,//#
	};
	NamesListParser();
	~NamesListParser();
	void Startup();
	void Shutdown();
	bool Load(const char* filename)
	{
		if (!bufFile_.ReadToBuffer(filename))
		{
			return false;
		}
		readPtr_ = bufFile_.GetBuffer();
		lastLineParser_ = nullptr;
		return true;
	}
	
	bool Parse();
	signature ToSignature()
	{
		return signature(bufFile_.GetBuffer(), bufFile_.GetLength());
	}
	eError LastError()const
	{
		return lastError_;
	}
	uint32 GetRow()const
	{
		return currentRow_;
	}
	CharNamesList& GetNamesList()
	{
		return charNamesList_;
	}
private:
	bool readyParse()
	{
		charLineFactory_.Restart();
		readPtr_ = bufFile_.GetBuffer();
		ok_ = true;
		currentRow_ = 1;
		lastLineParser_ = nullptr;
		currentCharNames_ = nullptr;
		charNamesList_.Clean();
		lastError_ = EOK;
		currentScope_ = NoneScope;
		return true;
	}
	bool isEndLine(int c)const
	{
		return c == '\r' || c == '\n';
	}
	bool skipLine();
	//void parseBlock();
	void parseChar();
	void parseLine();
	eLineHead checkLineHead()
	{
		switch (*readPtr_)
		{
		case '@':
			return Line_AT;
		case '\t':
			return Line_TAB;
		case ';':
		case '\r':
		case '\n':
			return Line_Comment;
		default:
		{
			if (isxdigit(*readPtr_))
			{
				const char* p = readPtr_;
				int count = 1;
				++p;
				while (*p && isxdigit(*p))
				{
					++count;
					++p;
				}
				if (count >= 4 && count <= 6)
				{
					return Line_Code;
				}
				else
				{
					setError(ECodeFormatNotRight);
					return Line_Other;
				}
			}
			else
			{
				return Line_Other;
			}
		}
		}
	}
	void setError(eError err)
	{
		lastError_ = err;
	}
	bool appendLineData(LineParser* lineParser)
	{
		CharLine* newCharLine = lineParser->ToData(charLineFactory_.getZone());
		if (!newCharLine)
		{
			return false;
		}
		currentCharNames_->AddLine(newCharLine);
		return true;
	}
	void increateCurrentRow()
	{
		++currentRow_;
	}
private:
	bool			ok_;
	uint32			currentRow_;
	LineParser*		lastLineParser_;
	CharNames*		currentCharNames_;
	CharNamesList	charNamesList_;
	BufferFile		bufFile_;
	const char*		readPtr_;
	eError			lastError_;
	eScope			currentScope_;
	CharNameParser	nameP_;
	CharNamesFactory	charLineFactory_;
};

NamesListParser::NamesListParser()
	:readPtr_(nullptr), currentScope_(NoneScope), 
	lastLineParser_(nullptr), currentCharNames_(nullptr)
{
}

NamesListParser::~NamesListParser()
{
}




}//end of namespace nameslist
