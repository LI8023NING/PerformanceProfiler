/************************************
 ** Auth: ����
 ** Date: 2016/8/27
 ** Desc: ����������
 ************************************/
#pragma once

#include<iostream>
#include<string>
#include<unordered_map>  //C++11�ײ��ϣ��
#include<windows.h>

#include <algorithm>  //STL�㷨
#include<map>
#include <thread>
#include <mutex>

#include<assert.h>
#include<time.h>          //clock() 

using namespace std;

//����������
#ifdef _WIN32
#include <windows.h>  //_WIN32�Դ��߳�
#else
#include <pthread.h>  //Linux���߳�
#endif // _WIN32

typedef long long LongType;

//����ģʽ 
//��֤һ����ֻ��һ��ʵ�������ṩһ����������ȫ�ַ��ʵ㡣
//���ȣ���Ҫ��֤һ����ֻ��һ��ʵ���������У�Ҫ����һ��ʵ�����ͱ��������Ĺ��캯����
//��ˣ�Ϊ�˷�ֹ���ⲿ������Ĺ��캯��������ʵ������Ҫ�����캯���ķ���Ȩ�ޱ��Ϊprotected��private��
//�����Ҫ�ṩҪ��ȫ�ַ��ʵ㣬����Ҫ�����ж���һ��static���������������ڲ�Ψһ�����ʵ����

//�����Ļ���  ����ģʽ
template<class T>
class Singleton
{
public:
	static T* GetInstance()  //��������������ڹ����Ψһʵ��
	{
		assert(_sInstance);
		return _sInstance;
	}

protected:
	static T* _sInstance;   //T��ʲô���ͣ���ô_sInstance����ʲô���͵Ķ��� ������
};

template<class T>
T* Singleton<T>::_sInstance = new T;  //�������Ա����ռ�

class SaveAdapter  //���������  ��־����δ�
{
public:
	virtual void Save(const char* format, ...) = 0;    //������  �ӿ���
}; 

//������
//�������̨
class ConsoleSaveAdapter : public SaveAdapter
{
public:
	virtual void Save(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		vfprintf(stdout, format,args);  //����ɱ����
		va_end(args);                    
	}

	~ConsoleSaveAdapter()
	{}
};

//�����ļ�
class FileSaveAdapter : public SaveAdapter
{
public:
	FileSaveAdapter(const char* filename)
	{
		fout = fopen(filename,"w");      //���������д���ļ�
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
		vfprintf(fout, format, args);  //����ɱ����
		va_end(args);
	}

protected:
	//��ֹ����  ��η�ֹ��
	//1.�Լ�����ӿڣ��Ͳ������ϵͳĬ�ϵĽӿ�
	//2.�����ڱ����У���粻�ɵ���
	FileSaveAdapter(FileSaveAdapter&);
	FileSaveAdapter& operator=(FileSaveAdapter&);

protected:
	FILE* fout;
};

//������ӽӿ�  ���籣�����ݵ����ݿ�
class SqlSaveAdapter :public SaveAdapter
{};

// ���ù���
// ��Ҫ������Щ���ܣ��ð�λ�����ظ����� δ��������˭����
enum ConfigOptions
{
	NONE = 0,
	PERFORMANCE_PROFILER = 1,  //�൱�ڿ��� ��֪�˹���Ҫ��Ҫ����
	SAVE_TO_CONSOLE = 2,       //�������̨����
	SAVE_TO_FILE = 4,          //����ļ�����
	SORT_BY_COSTTIME = 8,      //������ʱ������ѡ���
	SORT_BY_CALLCOUNT = 16,    //�����ô�������ѡ���
};

//ConfigManager �����ù���
class ConfigManager : public Singleton<ConfigManager>  //����ģʽ �̳�����Ψһ����
{
public:
	void SetOptions(int options)  //����ѡ��
	{
		_options = options;  //���õ�Ҳֻ�ǻ����ظ���λֵ
	}

	void AddOption(int option)  //��������ѡ����˵ 32���λ�������� �������Ͽ� 64��
	{
		_options |= option;
	}

	void DelOption(int option)  //ɾ
	{
		_options &= (~option);
	}

	int GetOptions()  //��ȡ
	{
		return _options;
	}

protected:
	friend class Singleton<ConfigManager>;  //��ߵ��������Է��ʱ��ຯ��

	ConfigManager()    //���춨λ����
		:_options(NONE)
	{}

	ConfigManager(const ConfigManager&);
	ConfigManager& operator=(const ConfigManager&);

protected:
	int _options;
};


//��Ҫ�ɼ�����Ϣ������Ҫ��ӡ����Ϣ
struct PPNode
{
	string _filename;  //�ļ���
	string _function;  //������
	size_t _line;      //���������к�
	string _desc;      //�Դ˵ĸ�������

	//����
	PPNode(const char* filename, const char* function, size_t line, const char* desc)
		:_filename(filename)
		, _function(function)
		, _line(line)
		, _desc(desc)
	{}

	//��Ҫ�Ƚϣ�����==
	bool operator==(const PPNode& node) const
	{
		if (_line == node._line && _function == node._function\
			&& _desc == node._desc)       //�����ж����п��ܳ��ֵ�  ������Ч��
		{
			return true;
		}
		else
		{
			return false;
		}
	}
};

//ͳ�ƺķ���
struct PPSection
{
	PPSection()
	:_costTime(0)
	, _callCount(0)
	, _ReCount(0)
	{}

	void Begin(int id); 
	void End(int id);

	// <�߳�id, ͳ����Ϣ> 
	map<int, LongType> _beginTimeMap; //�߳̿�ʼʱ��
	map<int, LongType> _costTimeMap;  //�̻߳���ʱ��
	map<int, LongType> _callCountMap; //�̵߳��ô���
	map<int, LongType> _refCountMap;  //�߳��е�ѭ�����ü�������

	LongType _beginTime; //��ʼϵͳʱ��
	LongType _costTime;  //��ȥ����ʱ�� ʣ���������л��ѵ�ʱ��
	LongType _callCount; //�˺��������õĴ���
	LongType _ReCount;

	mutex _mtx; //�߳�ͬ�� ������
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

		return BKDRHash(hash.c_str());  //����()��� �����ͺ�������ƴ�� �ù�ϣ������ӳ���
	}
};


//��������ļ̳У���class�����Ժ󣬺�ߵĶ�������һ�����ͣ������������;͸���Ϊ�������
class  PerformanceProfiler : public Singleton<PerformanceProfiler>  //��̳�����һ��Ψһ�Ķ���
{
	//ʱ��ΪO(1)
	typedef unordered_map<PPNode, PPSection*, PPNodeHash> PP_MAP;  //<>���������� ��������
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
	PP_MAP ppMap;    //���� ͳ����Դ��Ϣ ��ϣmap
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

	// �������̳߳���
#define PERFORMANCE_PROFILER_EE_ST_BEGIN(sign, desc) \
	PPSection* ps##sign = NULL;						 \
	ps##sign = PerformanceProfiler::GetInstance()->CreatePPSection(__FILE__, __FUNCTION__, __LINE__, desc); \
	ps##sign->Begin(-1);

#define PERFORMANCE_PROFILER_EE_ST_END(sign)	\
	if (ps##sign) ps##sign->End(-1);
