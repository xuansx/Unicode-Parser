#include "NamesList.h"
#include "Command.h"
#include "Log.h"

namespace nameslist {

//CharNameParser----------------------------------
/*
1:CHAR TAB NAME LF
2:CHAR TAB "<" LCNAME ">" LF
3:CHAR TAB NAME SP COMMENT LF
4:CHAR TAB "<" LCNAME ">" SP COMMENT LF
*/
const char* CharNameParser::Parse(const char* source, bool*ok)
{
	if (!parseCode(source, code))
	{
		*ok = false;
		return source;
	}
	source += code.length();
	if (!isCodeRight(code))
	{
		*ok = false;
		return source;
	}
	if (*source != CH_TAB)
	{
		*ok = false;
		return source;
	}
	++source;

	if (!isupper(*source))
	{//pseudo name
		if (*source != '<')
		{
			*ok = false;
			return source;
		}
		++source;
		if (!parseLCNAME(source, name))
		{
			*ok = false;
			return source;
		}
		source += name.length();
		if (*source != '>')
		{
			*ok = false;
			return source;
		}
		isPrintable = false;
		++source;
	}
	else
	{
		if (!parseNAME(source, name))
		{
			*ok = false;
			return source;
		}
		source += name.length();
		isPrintable = true;
	}
	if (isEndLine(*source))
	{
		*ok = true;
		return source;
	}
	if (*source != CH_SPACE)
	{
		*ok = false;
		return source;
	}
	if (!parseCOMMENT(source, comment))
	{
		*ok = false;
		return source;
	}
	if (!isEndLine(*source))
	{
		*ok = false;
		return source;
	}
	*ok = true;
	return source;
}
void CharNameParser::ToData(CharNameInfo* chn)
{
	chn->code = code.to_int(16);
	chn->SetType(isPrintable);
	chn->name = name;
	chn->comment = comment;
}
//------------------------------------------------
//CommentLineParser-------------------------------
/*
COMMENT_LINE:	TAB "*" SP EXPAND_LINE
// * is replaced by BULLET, output line as comment
|TAB EXPAND_LINE
// Output line as comment
*/
const char* CommentLineParser::Parse(const char* source, bool* ok)
{
	if (*source == '*')
	{
		continueLine = nullptr;
		++source;
		if (*source != ' ')
		{
			*ok = false;
			return source;
		}
	}
	signature cmt;
	if (!parseSTRING(source,cmt))
	{
		*ok = false;
		return source;
	}
	source+= cmt.length();
	if (!continueLine)
		comment.append(cmt.begin(), cmt.length());
	else
		continueLine->comment.append(cmt.begin(), cmt.length());
	if (!isEndLine(*source))
	{
		*ok = false;
		return source;
	}
	*ok = true;
	return source;
}
CharLine*	CommentLineParser::ToData(ZoneMemory* zone)
{
	continueLine = new(zone)CommentCharLine();
	if (continueLine)
	{
		continueLine->comment = comment;
	}
	return continueLine;
}
//
//AliasCharLineParser---------------------------------------
/*
TAB "=" SP LINE
// Replace = by itself, output line as alias
*/
const char* AliasCharLineParser::Parse(const char* source, bool* ok)
{
	source = preLineHead(source, '=', ok);
	if (!parseSTRING(source, name))
	{
		*ok = false;
		return source;
	}
	source += name.length();
	if (!isEndLine(*source))
	{
		*ok = false;
		return source;
	}
	*ok = true;
	return source;
}
CharLine*	AliasCharLineParser::ToData(ZoneMemory* zone)
{
	AliasCharLine* aliasLine =  new(zone)AliasCharLine();
	aliasLine->name = name;
	return aliasLine;
}
//--------------------------------
//FormalAliasParser---------------------------------------
/*
TAB "%" SP NAME LF
// Replace % by U+203B, output line as formal alias
*/
const char* FormalAliasParser::Parse(const char* source, bool* ok)
{
	source = preLineHead(source, '%', ok);
	if (!parseNAME(source, name))
	{
		*ok = false;
		return source;
	}
	source += name.length();
	if (!isEndLine(*source))
	{
		*ok = false;
		return source;
	}
	*ok = true;
	return source;
}
CharLine*	FormalAliasParser::ToData(ZoneMemory* zone)
{
	FormalAliasCharLine* faLine = new(zone)FormalAliasCharLine();
	faLine->name = name;
	return faLine;
}
//------------------------------------------------------
//CrossRefParser----------------------------------------
/*
1:
TAB "x" SP CHAR SP LCNAME LF
| TAB "x" SP CHAR LF
| TAB "x" SP CHAR SP "<" LCNAME ">" LF
2:
| TAB "x" SP "(" LCNAME SP "-" SP CHAR ")" LF
| TAB "x" SP "(" "<" LCNAME ">" SP "-" SP CHAR ")" LF
*/
const char* CrossRefParser::Parse(const char* source, bool* ok)
{
	source = preLineHead(source, 'x', ok);
	if (!*ok)
	{
		return source;
	}
	if (isxdigit(*source))
	{
		if (!parseCode(source,codeSig))
		{
			*ok = false;
			return source;
		}
		if (!isCodeRight(codeSig))
		{
			*ok = false;
			return source;
		}
		source += codeSig.length();
		if (isEndLine(*source))
		{
			*ok = true;
			return source;
		}
		
		if (*source != CH_SPACE)
		{
			*ok = false;
			return source;
		}
		++source;
		if (*source == '<')
		{
			isPoseudo = true;
			++source;
		}
		if (!parseLCNAME(source,lcname))
		{
			*ok = false;
			return source;
		}
		source += lcname.length();
		if (isPoseudo && *source == '>')
		{
			++source;
		}
		if (isEndLine(*source))
		{
			*ok = true;
			return source;
		}
		else
		{
			*ok = false;
			return source;
		}
	}
	else if(*source == '(')
	{
		++source;
		if (*source == '<')
		{
			isPoseudo = true;
			++source;
			if (!parseLCNAME(source, lcname))
			{
				*ok = false;
				return source;
			}
			source += lcname.length();
			if (*source != '>')
			{
				*ok = false;
				return source;
			}
			++source;
		}
		else
		{
			if (!parseLCNAMEWithOutHyphen(source, lcname))
			{
				*ok = false;
				return source;
			}
			lcname.trace_tail();
			source += lcname.length();
		}
		if (*source != CH_SPACE)
		{
			*ok = false;
			return source;
		}
		++source;

		if (*source != '-')
		{
			*ok = false;
			return source;
		}
		++source;
		if (*source != CH_SPACE)
		{
			*ok = false;
			return source;
		}
		++source;
		if (!parseCode(source, codeSig))
		{
			*ok = false;
			return source;
		}
		if (!isCodeRight(codeSig))
		{
			*ok = false;
			return source;
		}
		source += codeSig.length();
		if (*source != ')')
		{
			*ok = false;
			return source;
		}
		++source;
	}
	if (!isEndLine(*source))
	{
		*ok = false;
		return source;
	}
	*ok = true;
	return source;
}
CharLine*	CrossRefParser::ToData(ZoneMemory* zone)
{
	CrossRefLine *cfLine = new(zone)CrossRefLine();
	cfLine->code = codeSig.to_int(16);
	cfLine->lcname = lcname;
	cfLine->isPoseudo = isPoseudo;
	return cfLine;
}
//-----------------------------------------------------------
//VariationLineParser----------------------------------------
/*
VARIATION_LINE:	TAB "~" SP CHAR VARSEL SP LABEL LF
| TAB "~" SP CHAR VARSEL SP LABEL "(" LCNAME ")"LF
*/
const char* VariationLineParser::Parse(const char* source, bool* ok)
{
	source = preLineHead(source, '~', ok);
	if (!*ok)
	{
		return source;
	}
	if (!parseCode(source, code))
	{
		*ok = false;
		return source;
	}
	if (!isCodeRight(code))
	{
		*ok = false;
		return source;
	}
	source += code.length();
	if (*source != CH_SPACE)
	{
		*ok = false;
		return source;
	}
	++source;

	if (parseCode(source, varsel))
	{
		source += varsel.length();
		if (*source != CH_SPACE)
		{
			*ok = false;
			return source;
		}
		++source;
	}
	if (!parseLABEL(source,label))
	{
		*ok = false;
		return source;
	}
	source += label.length();
	if (*source == '(')
	{
		++source;
		if (!parseLCNAME(source,lcname))
		{
			*ok = false;
			return source;
		}
		source += lcname.length();
		if (*source != ')')
		{
			*ok = false;
			return source;
		}
		++source;
	}
	if (!isEndLine(*source))
	{
		*ok = false;
		return source;
	}
	*ok = true;
	return source;
}
CharLine*	VariationLineParser::ToData(ZoneMemory* zone)
{
	VariationLine* vLine = new(zone)VariationLine();
	vLine->code = code.to_int(16);
	vLine->varsel = varsel.to_int(16);
	vLine->label = label;
	vLine->lcname = lcname;
	return vLine;
}
//-------------------------------------------------------------------
//DecompositionParser------------------------------------------------
/*
DECOMPOSITION:	TAB ":" SP EXPAND_LINE
// Replace ':' by EQUIV, expand line into
// decomposition
*/
const char* DecompositionParser::Parse(const char* source, bool* ok)
{
	source = preLineHead(source, ':', ok);
	if (!*ok)
	{
		return source;
	}
	signature sig;
	while (parseCode(source,sig))
	{
		source += sig.length();
		codeV[count++] = sig;
		if (*source != CH_SPACE)
		{
			break;
		}
		++source;
		if (!isxdigit(*source))
		{
			break;
		}
		if (count > DecompositionLine::kMaxDecompositionField)
		{
			ASSERT(0);
		}
	}
	if (!isEndLine(*source))
	{
		if (!parseSTRING(source, postfix))
		{
			*ok = false;
			return source;
		}
		source += postfix.length();
	}
	if (!isEndLine(*source))
	{
		*ok = false;
		return source;
	}
	*ok = true;
	return source;
}
CharLine*	DecompositionParser::ToData(ZoneMemory* zone)
{
	DecompositionLine* dLine = new(zone)DecompositionLine(codeV,count);
	return dLine;
}
//-----------------------------------------------------------------------
//CompatMappingParser----------------------------------------------------
/*
COMPAT_MAPPING:	TAB "#" SP EXPAND_LINE
				| TAB "#" SP "<" LCTAG ">" SP EXPAND_LINE
// Replace '#' by >,
EXPAND_LINE:	{ESC_CHAR | CHAR | STRING | ESC +}+ LF
*/
const char* CompatMappingParser::Parse(const char* source, bool* ok)
{
	source = preLineHead(source, '#', ok);
	if (!*ok)
	{
		return source;
	}
	if (*source == '<')
	{
		++source;
		isPoseudo = true;
		if (!parseName(source,profix))
		{
			*ok = false;
			return source;
		}
		source += profix.length();
		if (*source != '>')
		{
			*ok = false;
			return source;
		}
		++source;
		if (*source != CH_SPACE)
		{
			*ok = false;
			return source;
		}
		++source;
	}

	signature sig;
	while (parseCode(source, sig))
	{
		source += sig.length();
		codeV[count++] = sig;
		if (*source != CH_SPACE)
		{
			break;
		}
		++source;
		if (count >= CompatMappingLine::kMaxCodeField)
		{
			break;
		}
	}
	if (!isEndLine(*source))
	{
		if (!parseSTRING(source,postfix))
		{
			*ok = false;
			return source;
		}
		source += postfix.length();
	}
	*ok = true;
	return source;
}
CharLine*	CompatMappingParser::ToData(ZoneMemory* zone)
{
	CompatMappingLine* cmLine = new(zone)CompatMappingLine(profix, postfix, codeV, count);
	cmLine->isPoseudo = isPoseudo;
	return cmLine;
}
//----------------------------------------------------------
//CharNamesFactory
char CharNamesFactory::LinesHeadChar[CharNamesFactory::MAX_LINE_TYPE] =
{
	' ',
	'=',
	'%',
	'*',
	'x',
	'~',
	':',
	'#'
};
LineParser* CharNamesFactory::getParser(char c)
{
	for (int i = 0; i < MAX_LINE_TYPE; i++)
	{
		if (c == LinesHeadChar[i])
		{
			lineParserV[i]->ClearData();
			return lineParserV[i];
		}
	}
	return nullptr;
}
void CharNamesFactory::Startup()
{
	zone_.Initialize("ucd", kInMemory);
	initParserV();
}
void CharNamesFactory::Shutdown()
{
	zone_.DeleteAll();
}
void NamesListParser::Startup()
{
	charLineFactory_.Startup();
}
void NamesListParser::Shutdown()
{
	charLineFactory_.Shutdown();
}
bool NamesListParser::Parse()
{
	readyParse();
	while (*readPtr_)
	{
		eLineHead head = checkLineHead();
		switch (head)
		{
		case NamesListParser::Line_AT:
		{
			currentScope_ = BlockScope;
			skipLine();
			//parseBlock();
			break;
		}
		case NamesListParser::Line_TAB:
		{
			if (currentScope_ == CharScope ||
				currentScope_ == LineScope)
			{
				currentScope_ = LineScope;
				if (!currentCharNames_)
				{
					setError(EUnknownCharLine);
					return false;
				}
				parseLine();
			}
			else if (currentScope_ == BlockScope)
			{
				skipLine();
			}
			else
			{
				setError(EBadLineFormat);
				return false;
			}
			break;
		}
		case NamesListParser::Line_Code:
		{
			currentScope_ = CharScope;
			CharNames* newCharNames = charLineFactory_.createCharNames();
			if (!newCharNames)
			{
				setError(EMemoryException);
			}
			charNamesList_.PushBack(newCharNames);
			currentCharNames_ = newCharNames;
			parseChar();
			break;
		}
		case NamesListParser::Line_Comment:
		{
			currentScope_ = NoneScope;
			skipLine();
			break;
		}
		case NamesListParser::Line_Other:
		{
			setError(EBadLineFormat);
			return false;
		}
		default:
		{
			setError(EBadLineFormat);
			return false;
		}
		}
		if (!isEndLine(*readPtr_))
		{
			setError(EBadLineEnd);
			return false;
		}
		++readPtr_;
		increateCurrentRow();
	}
	return true;
}

void NamesListParser::parseChar()
{
	nameP_.ClearData();
	readPtr_ = nameP_.Parse(readPtr_, &ok_);
	if (!ok_)
	{
		setError(EBadCharName);
	}
	nameP_.ToData(&(currentCharNames_->name));
}
void NamesListParser::parseLine()
{
	if (*readPtr_ != '\t')
	{
		setError(EBadLineHead);
		return;
	}
	++readPtr_;
	LineParser* newLineParser = charLineFactory_.getParser(*readPtr_);
	if (!newLineParser)
	{
		if (lastLineParser_ &&
			lastLineParser_->GetType() == COMMENT_LINE)
		{
			const char* resultPtr =
				lastLineParser_->Parse(readPtr_, &ok_);
			if (ok_)
			{
				readPtr_ = resultPtr;
				//if (!appendLineData(lastLineParser_))
				//{
				//	setError(EMemoryException);
				//}
			}
			else
			{
				setError(EBadCommentLine);
			}
			return;
		}
		setError(EBadCommentLine);
	}
	//
	lastLineParser_ = newLineParser;
	const char* resultPtr =
		lastLineParser_->Parse(readPtr_, &ok_);
	if (ok_)
	{
		readPtr_ = resultPtr;
		if (!appendLineData(lastLineParser_))
		{
			setError(EMemoryException);
		}
	}
	else
	{
		LOG_WARNING("Error Current Line:"<<currentRow_);
		setError(EBadCommentLine);
		return;
	}
}

bool NamesListParser::skipLine()
{
	while (*readPtr_)
	{
		if (*readPtr_ == '\r' ||
			*readPtr_ == '\n')
		{
			//++readPtr_;
			break;
		}
		++readPtr_;
	}
	return true;
}


class NamesListParserCmd :public Command
{
public:
	NamesListParserCmd()
		:Command("nl")//unameslist
	{
		parser.Startup();
	}
	~NamesListParserCmd()
	{
		parser.Shutdown();
	}
	bool Load()
	{
		if (!parser.Load("../data/ucd/NamesList.txt"))
		{
			LOG_TRACE("UCD NamesList.txt parse failed!");
			return false;
		}
		else
		{
			LOG_TRACE("UCD NamesList.txt load ok!");
			return true;
		}
	}
	void Trans()
	{
		uint32_t count = 0;
		CharNamesList& list = parser.GetNamesList();
		list.DoForEach([&](CharNames* node)
		{
			if (node->GetLineCount() > 5)
			{
				LOG_TRACE("Big CharNames:" << node->name.code<<"\t"<<node->name.name<<"\t"<<node->GetLineCount());
			}
			++count;
		});
		LOG_TRACE("UCD NamesList.txt Count:"<< count);
	}
	int Execute(const std::string& cmd)
	{
		if (cmd =="l")
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
				LOG_TRACE("UCD NamesList.txt parse error...!");
				return -1;
			}
			LOG_TRACE("UCD NamesList.txt parse ok!");
		}
		if (cmd == "t")
		{
			Trans();
		}
		return 0;
	}
	NamesListParser	parser;
};
static NamesListParserCmd nameslistCmd;
}//end of namespace nameslist

