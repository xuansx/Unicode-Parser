#ifndef Singleton_H_sim
#define Singleton_H_sim

template<typename BaseT>
class SingletonStaticT
{
public:
	typedef BaseT BaseObject;
	static BaseObject& Instance()
	{
		static BaseObject instance;
		return instance;
	}
};


template<typename BaseT>
class SingletonT:public BaseT
{
public:
	typedef SingletonT<BaseT> BaseObject;
	static BaseObject* Instance()
	{
		if (instance_ == nullptr)
		{
			instance_ = new BaseObject;
		}
		return instance_;
	}
	static	BaseObject*		instance_;
};

template<typename T>
SingletonT<T>* SingletonT<T>::instance_ = nullptr;


template<typename BaseT>
class SingletonDynT
{
public:
	typedef BaseT BaseObject;
	static BaseObject& Instance()
	{
		return &instance_;
	}
	static void CreateInstance()
	{
		if (instance_ == nullptr)
		{
			instance_ = new BaseObject;
		}
	}
	static void ReleaseInstance()
	{
		if (instance_!=nullptr)
		{
			delete instance_;
			instance_ = nullptr;
		}
	}
private:
static	BaseObject*		instance_;
};
template<typename T>
T* SingletonDynT<T>::instance_ = nullptr;

#endif

