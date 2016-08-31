#include"PerformanceProfiler.h"

static Report report;  //ԭ������ ȫ�־�̬����������Ϊ����ñ������ļ� �������ͷ�ļ��� ���������� ���ļ����� �������� ������ �͵�����

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
LongType Fib(LongType n)
{
	PERFORMANCE_PROFILER_EE_BEGIN(in, "�ݹ�����");

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
	PERFORMANCE_PROFILER_EE_BEGIN(out, "�ݹ�����");

	Fib(28);

	PERFORMANCE_PROFILER_EE_END(out);
}

void Fun(size_t n)
{
	while (n--)
	{
		PERFORMANCE_PROFILER_EE_MT_BEGIN(network, "����m");
		Sleep(100);
		PERFORMANCE_PROFILER_EE_MT_END(network);

		PERFORMANCE_PROFILER_EE_MT_BEGIN(mid, "�м��߼�m");
		Sleep(200);
		PERFORMANCE_PROFILER_EE_MT_END(mid);

		PERFORMANCE_PROFILER_EE_MT_BEGIN(sql, "���ݿ�m");
		Sleep(300);
		PERFORMANCE_PROFILER_EE_MT_END(sql);
	}
}

// ���ݲ�׼ȷ
// �̰߳�ȫ
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