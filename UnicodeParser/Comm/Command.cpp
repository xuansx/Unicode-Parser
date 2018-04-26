#include "Command.h"
#include "CharConverter.h"

#include "sstring.hpp"
#include <iostream>

Command::Command(const char* name)
	:name_(name)
{
	CommandDispatch::Instance().Register(this);
}
Command::Command(const std::string& name)
	:name_(name)
{
	CommandDispatch::Instance().Register(this);
}
//Command::Command(const char* name, bool /*noRegister*/)
//	: name_(name)
//{
//}
//Command::Command(const std::string& name, bool /*noRegister*/)
//	: name_(name)
//{
//}

Command::~Command()
{
	//if(GetType() == BaseCmd)
	CommandDispatch::Instance().Unregister(this);
}

//CommandEx::CommandEx(const char* name)
//	:Command(name,true), next_(nullptr)
//{
//	CommandDispatch::Instance().Register(this);
//}
//CommandEx::CommandEx(const std::string& name)
//	: Command(name, true), next_(nullptr)
//{
//	CommandDispatch::Instance().Register(this);
//}
//CommandEx::~CommandEx()
//{
//	CommandDispatch::Instance().Unregister(this);
//}

CommandDispatch::CommandDispatch()
	:exitCmd_(nullptr)//,exhead_(nullptr),extail_(nullptr)
{
}


CommandDispatch::~CommandDispatch()
{
}

void CommandDispatch::Startup()
{
	if (!exitCmd_)
	{
		exitCmd_ = new ExitCommand();
	}
	CharConverter::Instance().Load("../data/gbkuni.txt");
}
void CommandDispatch::Run()
{
	std::string	line;
	std::string cmdname;
	std::string cmdline;
	size_t splitpos = 0;
	while (std::getline(std::cin, line))
	{
		cmdline.clear();
		cmdname.clear();
		splitpos = std::string::npos;
		for (size_t i = 0; i < line.length(); i++)
		{
			if (line[i] == ' ')
			{
				splitpos = i;
				break;
			}
		}
		if (splitpos == std::string::npos)
		{
			cmdname = line;
		}
		else
		{
			cmdname = line.substr(0, splitpos);
			cmdline = line.substr(splitpos + 1, line.size() - splitpos - 1);
		}
		Command* cmd = findCommand(cmdname);
		if (!cmd)
		{
			continue;
		}
		if (cmd->Execute(cmdline) < 0)
		{
			break;
		}
	}
}
Command* CommandDispatch::findCommand(const std::string& name)
{
	std::shared_lock<std::shared_mutex> lock(smutex_);
	CommandMap::iterator iter = cmdMap_.find(name);
	if (iter == cmdMap_.end())
	{
		return nullptr;
	}
	return iter->second;
}
void CommandDispatch::Shutdown()
{
	if (exitCmd_)
	{
		delete exitCmd_;
		exitCmd_ = nullptr;
	}
}


bool CommandDispatch::Register(Command*cmd)
{
	if (!cmd)
	{
		return false;
	}
	{
		std::lock_guard<std::shared_mutex>	lock(smutex_);
		if (cmdMap_.find(cmd->GetName()) != cmdMap_.end())
		{
			return false;
		}
		cmdMap_.insert(std::make_pair(cmd->GetName(), cmd));
		return true;
	}
}
void CommandDispatch::Unregister(Command*cmd)
{
	if (!cmd)
	{
		return;
	}
	{
		std::lock_guard<std::shared_mutex>	lock(smutex_);
		CommandMap::iterator iter = cmdMap_.find(cmd->GetName());
		if (iter == cmdMap_.end())
		{
			return;
		}
		cmdMap_.erase(iter);
	}
}
//bool CommandDispatch::Register(CommandEx*cmd)
//{
//
//}
//void CommandDispatch::Unregister(CommandEx*cmd)
//{
//
//}