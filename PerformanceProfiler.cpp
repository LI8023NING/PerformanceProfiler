#include"PerformanceProfiler.h"

static Report report;  //原来错误 今天变量放在图文件中 被包含两次 本文件结束 调用析构 包两次 就掉两次

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
//LongType Fib(LongType n)
//{
//	PERFORMANCE_PROFILER_EE_BEGIN(in, "递归里面");
//
//	LongType ret = n;
//	if (n < 2)
//	{
//		ret = n;
//	}
//	else
//	{
//		ret = Fib(n - 1) + Fib(n - 2);
//	}
//
//	PERFORMANCE_PROFILER_EE_END(in);
//
//	return ret;
//}
//
//void TestFib()
//{
//	PERFORMANCE_PROFILER_EE_BEGIN(out, "递归外面");
//
//	Fib(28);
//
//	PERFORMANCE_PROFILER_EE_END(out);
//}

int main()
{
	//Test1();
	//TestFib();
	TestSingleThead();
	return 0;
}