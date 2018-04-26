#pragma once
#ifndef CommandDispatch_H_
#define CommandDispatch_H_
#include "Singleton.h"
#include <functional>
#include <unordered_map>
#include <shared_mutex>


struct Command
{
	enum eCmdType {
		BaseCmd,
		ExCmd = 0x10000,
	};
	Command(const char* name);
	Command(const std::string& name);
	//Command(const char* name,bool /*noRegister*/);
	//Command(const std::string& name, bool /*noRegister*/);
	const std::string& GetName()const
	{
		return name_;
	}
	virtual ~Command();	
	//virtual unsigned int GetType()const 
	//{
	//	return BaseCmd;
	//}
	virtual int Execute(const std::string& cmd) = 0;
private:
	std::string name_;
};
//struct CommandEx :public Command
//{
//public:
//	CommandEx(const char* name);
//	CommandEx(const std::string& name);
//	~CommandEx();
//	void SetNext(CommandEx* cmd)
//	{
//		next_ = cmd;
//	}
//	CommandEx* GetNext()const
//	{
//		return next_;
//	}
//	virtual unsigned int GetType()const
//	{
//		return ExCmd;
//	}
//	virtual int Startup() = 0;
//	virtual int Shutdown() = 0;
//private:
//	CommandEx*	next_;
//};
class CommandDispatch:public SingletonStaticT<CommandDispatch>
{
	struct ExitCommand:public Command
	{
		ExitCommand()
			:Command("exit")
		{}
		int Execute(const std::string& cmd)
		{
			return -1;
		}
	};
public:
	CommandDispatch();
	~CommandDispatch();
	void Startup();
	void Run();
	void Shutdown();
private:
	friend struct Command; 
	//friend struct CommandEx;
	bool Register(Command*cmd);
	void Unregister(Command*cmd);
	//bool Register(CommandEx*cmd);
	//void Unregister(CommandEx*cmd);
	Command* findCommand(const std::string& name);
private:
	typedef std::unordered_map<std::string, Command*>	CommandMap;
	CommandMap			cmdMap_;
	std::shared_mutex	smutex_;
	Command*			exitCmd_;
	//CommandEx*			exhead_;
	//CommandEx*			extail_;
};

#define COMMAND_CONSOLE()\
CommandDispatch::Instance().Startup();\
CommandDispatch::Instance().Run();\
CommandDispatch::Instance().Shutdown();


#endif // !CommandDispatch_H_
