#include"PerformanceProfiler.h"

static Report report;  //ԭ������ �����������ͼ�ļ��� ���������� ���ļ����� �������� ������ �͵�����

//���Ե��߳�
void TestSingleThead()
{
	PERFORMANCE_PROFILER_EE_ST_BEGIN(network, "����");
	// ����
	Sleep(200);
	PERFORMANCE_PROFILER_EE_ST_END(network);

	PERFORMANCE_PROFILER_EE_ST_BEGIN(mid, "�м��߼�s");
	// �м��߼�
	Sleep(300);

	PERFORMANCE_PROFILER_EE_ST_END(mid);

	PERFORMANCE_PROFILER_EE_ST_BEGIN(sql, "���ݿ�s");
	// ���ݿ�
	Sleep(400);

	PERFORMANCE_PROFILER_EE_ST_END(sql);
}

//�еݹ�����������ü���
//LongType Fib(LongType n)
//{
//	PERFORMANCE_PROFILER_EE_BEGIN(in, "�ݹ�����");
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
//	PERFORMANCE_PROFILER_EE_BEGIN(out, "�ݹ�����");
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