#include"PerformanceProfiler.h"

void PPSection::Begin(int id)
{
	// id == -1时表示多线程场景
	if (id != -1)
	{
		lock_guard<mutex> lock(_mtx);

		// 分别统计线程和总的花费时间和调用次数
		if (_refCountMap[id]++ == 0)
			_beginTimeMap[id] = clock();
	}
	else
	{
		if (_ReCount++ == 0)
			_beginTime = clock();
	}
}

void PPSection::End(int id)
{
	if (id != -1)
	{
		lock_guard<mutex> lock(_mtx);

		if (--_refCountMap[id] == 0)
			_costTimeMap[id] += clock() - _beginTimeMap[id];

		++_callCountMap[id];
	}
	else
	{
		if (--_ReCount == 0)
			_costTime += clock() - _beginTime;
		++_callCount;
	}
}


void PerformanceProfiler::_OutPut(SaveAdapter& sa)
{
	int num = 1;
	PP_MAP::iterator it = ppMap.begin();
	while (it != ppMap.end())
	{
		const PPNode& node = it->first;
		PPSection* section = it->second;  //pair中的两个参数  第一和第二个

		sa.Save("No.%d	Desc:%s\n", num++, node._desc.c_str());      //Save是处理可变参数
		sa.Save("【1】Filename:%s\n", node._filename.c_str());
		sa.Save("【2】Line:%d\n", node._line);
		sa.Save("【3】Function:%s\n", node._function.c_str());
		sa.Save("【4】CostTime:%.2f,CallCount : %lld\n\n", (double)section->_costTime / 1000, section->_callCount);

		++it;
	}
}

void PerformanceProfiler::OutPut()
{
	ConsoleSaveAdapter Con;
	_OutPut(Con);

	FileSaveAdapter Fil("log.txt");    //日志文件名
	_OutPut(Fil);
}

PPSection* PerformanceProfiler::CreatePPSection(const char* filename, const char* function, size_t line, const char* desc)
{
	PPNode node(filename, function, line, desc);
	PPSection* section = NULL;
	PP_MAP::iterator it = ppMap.find(node);   //在map中寻找node
	if (it != ppMap.end())
	{
		section = it->second;   //找到了，把相关文件信息让section指向
	}
	else
	{
		section = new PPSection;  //
		ppMap.insert(pair<PPNode, PPSection*>(node, section));
	}

	return section;
}