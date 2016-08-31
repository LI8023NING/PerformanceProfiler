/************************************
 ** Auth: 李宁
 ** Date: 2016/8/27
 ** Desc: 性能剖析器
 ************************************/
#pragma once

#include<iostream>
#include<string>
#include<unordered_map>  //C++11底层哈希表
#include<windows.h>

#include <algorithm>  //STL算法
#include<map>
#include <thread>
#include <mutex>

#include<assert.h>
#include<time.h>          //clock() 

using namespace std;

//兼容性问题
#ifdef _WIN32
#include <windows.h>  //_WIN32自带线程
#else
#include <pthread.h>  //Linux下线程
#endif // _WIN32

typedef long long LongType;

//单例模式 
//保证一个类只有一个实例，并提供一个访问它的全局访问点。
//首先，需要保证一个类只有一个实例；在类中，要构造一个实例，就必须调用类的构造函数，
//如此，为了防止在外部调用类的构造函数而构造实例，需要将构造函数的访问权限标记为protected或private；
//最后，需要提供要给全局访问点，就需要在类中定义一个static函数，返回在类内部唯一构造的实例。

//单例的基类  饿汉模式
template<class T>
class Singleton
{
public:
	static T* GetInstance()  //这个函数返回类内构造的唯一实例
	{
		assert(_sInstance);
		return _sInstance;
	}

protected:
	static T* _sInstance;   //T是什么类型，那么_sInstance就是什么类型的对象 很神奇
};

template<class T>
T* Singleton<T>::_sInstance = new T;  //给基类成员分配空间

class SaveAdapter  //保存抽象类  日志保存何处
{
public:
	virtual void Save(const char* format, ...) = 0;    //抽象类  接口类
}; 

//适配器
//保存控制台
class ConsoleSaveAdapter : public SaveAdapter
{
public:
	virtual void Save(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		vfprintf(stdout, format,args);  //处理可变参数
		va_end(args);                    
	}

	~ConsoleSaveAdapter()
	{}
};

//保存文件
class FileSaveAdapter : public SaveAdapter
{
public:
	FileSaveAdapter(const char* filename)
	{
		fout = fopen(filename,"w");      //将剖析结果写入文件
		assert(fout);
	}

	~FileSaveAdapter()
	{
		if (fout)
		{
			fclose(fout);
		}
	}

	virtual void Save(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		vfprintf(fout, format, args);  //处理可变参数
		va_end(args);
	}

protected:
	//防止拷贝  如何防止：
	//1.自己定义接口，就不会调用系统默认的接口
	//2.定义在保护中，外界不可调用
	FileSaveAdapter(FileSaveAdapter&);
	FileSaveAdapter& operator=(FileSaveAdapter&);

protected:
	FILE* fout;
};

//另外添加接口  比如保存数据到数据库
class SqlSaveAdapter :public SaveAdapter
{};

// 配置管理
// 需要开启哪些功能，用按位互不重复的数 未操作决定谁可用
enum ConfigOptions
{
	NONE = 0,
	PERFORMANCE_PROFILER = 1,  //相当于开关 告知此功能要不要开启
	SAVE_TO_CONSOLE = 2,       //输出控制台开启
	SAVE_TO_FILE = 4,          //输出文件开启
	SORT_BY_COSTTIME = 8,      //按花费时间排序选项开启
	SORT_BY_CALLCOUNT = 16,    //按调用次数排序选项开启
};

//ConfigManager ：配置管理
class ConfigManager : public Singleton<ConfigManager>  //单例模式 继承下来唯一对象
{
public:
	void SetOptions(int options)  //设置选项
	{
		_options = options;  //设置的也只是互不重复的位值
	}

	void AddOption(int option)  //对于增加选项来说 32这个位假如用了 就在往上开 64了
	{
		_options |= option;
	}

	void DelOption(int option)  //删
	{
		_options &= (~option);
	}

	int GetOptions()  //获取
	{
		return _options;
	}

protected:
	friend class Singleton<ConfigManager>;  //后边的类对象可以访问本类函数

	ConfigManager()    //构造定位保护
		:_options(NONE)
	{}

	ConfigManager(const ConfigManager&);
	ConfigManager& operator=(const ConfigManager&);

protected:
	int _options;
};


//需要采集的信息，即需要打印的信息
struct PPNode
{
	string _filename;  //文件名
	string _function;  //函数名
	size_t _line;      //函数所在行号
	string _desc;      //对此的附加描述

	//构造
	PPNode(const char* filename, const char* function, size_t line, const char* desc)
		:_filename(filename)
		, _function(function)
		, _line(line)
		, _desc(desc)
	{}

	//需要比较，重载==
	bool operator==(const PPNode& node) const
	{
		if (_line == node._line && _function == node._function\
			&& _desc == node._desc)       //最先判断最有可能出现的  有助于效率
		{
			return true;
		}
		else
		{
			return false;
		}
	}
};

//统计耗费类
struct PPSection
{
	PPSection()
	:_costTime(0)
	, _callCount(0)
	, _ReCount(0)
	{}

	void Begin(int id); 
	void End(int id);

	// <线程id, 统计信息> 
	map<int, LongType> _beginTimeMap; //线程开始时间
	map<int, LongType> _costTimeMap;  //线程花费时间
	map<int, LongType> _callCountMap; //线程调用次数
	map<int, LongType> _refCountMap;  //线程中的循环引用计数问题

	LongType _beginTime; //开始系统时间
	LongType _costTime;  //减去结束时间 剩余代码段运行花费的时间
	LongType _callCount; //此函数被调用的次数
	LongType _ReCount;

	mutex _mtx; //线程同步 互斥锁
};

struct PPNodeHash
{
	static size_t BKDRHash(const char * str)
	{
		unsigned int seed = 131; // 31 131 1313 13131 131313
		unsigned int hash = 0;
		while (*str)
		{
			hash = hash * seed + (*str++);
		}
		return (hash & 0x7FFFFFFF);
	}

	size_t operator()(const PPNode& node) const
	{
		static string hash;
		hash = node._desc;
		hash += node._function;

		return BKDRHash(hash.c_str());  //重载()完成 描述和函数名的拼接 用哈希函数找映射点
	}
};


//解释下面的继承：在class出来以后，后边的东西就是一个类型，而单例的类型就给定为这个类型
class  PerformanceProfiler : public Singleton<PerformanceProfiler>  //会继承下来一个唯一的对象
{
	//时间为O(1)
	typedef unordered_map<PPNode, PPSection*, PPNodeHash> PP_MAP;  //<>中三个参数 均是类型
public:
	PPSection* CreatePPSection(const char* filename, const char* function, size_t line, const char* desc);

	void OutPut();

protected:
	void _OutPut(SaveAdapter& sa);
	friend class Singleton<PerformanceProfiler>;

	PerformanceProfiler(){}
	PerformanceProfiler(const PerformanceProfiler&);
	PerformanceProfiler& operator=(const PerformanceProfiler&);

protected:
	PP_MAP ppMap;    //容器 统计资源信息 哈希map
	mutex _mtex;
};

static int a = 0;

struct Report
{
	~Report()
	{
		cout << a++ << endl;
		PerformanceProfiler::GetInstance()->OutPut();
	}
};


static int GetTheadId()
{
#ifdef _WIN32
	return GetCurrentThreadId();
#else
	return thread_self();
#endif // _WIN32
}

	// 剖析单线程场景
#define PERFORMANCE_PROFILER_EE_ST_BEGIN(sign, desc) \
	PPSection* ps##sign = NULL;						 \
	ps##sign = PerformanceProfiler::GetInstance()->CreatePPSection(__FILE__, __FUNCTION__, __LINE__, desc); \
	ps##sign->Begin(-1);

#define PERFORMANCE_PROFILER_EE_ST_END(sign)	\
	if (ps##sign) ps##sign->End(-1);
