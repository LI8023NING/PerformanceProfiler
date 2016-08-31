#include"PerformanceProfiler.h"

static Report report;  //原来错误 全局静态变量作用域为定义该变量的文件 定义放在头文件中 被包含两次 本文件结束 调用析构 包两次 就掉两次

//测试单线程
void TestSingleThead()
{
	PERFORMANCE_PROFILER_EE_ST_BEGIN(network, "网络");
	// 网络
	Sleep(200);
	PERFORMANCE_PROFILER_EE_ST_END(network);

	PERFORMANCE_PROFILER_EE_ST_BEGIN(mid, "中间逻辑s");
	// 中间逻辑
	Sleep(300);

	PERFORMANCE_PROFILER_EE_ST_END(mid);

	PERFORMANCE_PROFILER_EE_ST_BEGIN(sql, "数据库s");
	// 数据库
	Sleep(400);

	PERFORMANCE_PROFILER_EE_ST_END(sql);
}

//有递归的剖析：引用计数
LongType Fib(LongType n)
{
	PERFORMANCE_PROFILER_EE_BEGIN(in, "递归里面");

	LongType ret = n;
	if (n < 2)
	{
		ret = n;
	}
	else
	{
		ret = Fib(n - 1) + Fib(n - 2);
	}

	PERFORMANCE_PROFILER_EE_END(in);

	return ret;
}

void TestFib()
{
	PERFORMANCE_PROFILER_EE_BEGIN(out, "递归外面");

	Fib(28);

	PERFORMANCE_PROFILER_EE_END(out);
}

void Fun(size_t n)
{
	while (n--)
	{
		PERFORMANCE_PROFILER_EE_MT_BEGIN(network, "网络m");
		Sleep(100);
		PERFORMANCE_PROFILER_EE_MT_END(network);

		PERFORMANCE_PROFILER_EE_MT_BEGIN(mid, "中间逻辑m");
		Sleep(200);
		PERFORMANCE_PROFILER_EE_MT_END(mid);

		PERFORMANCE_PROFILER_EE_MT_BEGIN(sql, "数据库m");
		Sleep(300);
		PERFORMANCE_PROFILER_EE_MT_END(sql);
	}
}

// 数据不准确
// 线程安全
void TestMultiThread()
{
	thread t1(Fun, 5);
	thread t2(Fun, 4);
	thread t3(Fun, 3);

	t1.join();
	t2.join();
	t3.join();
}

int main()
{
	//Test1();
	//TestFib();
	//TestSingleThead();

	SET_CONFIG_OPTIONS(PERFORMANCE_PROFILER  //1
		| SAVE_TO_FILE       //4
		| SAVE_TO_CONSOLE    //2
		| SORT_BY_COSTTIME); //8

	TestMultiThread();

	return 0;
}