#include"PerformanceProfiler.h"

void PPSection::Begin(int id)
{
	// id == -1时表示多线程场景
	if (id != -1)
	{
		lock_guard<mutex> lock(_mtex);  //线程锁

		// 分别统计线程和总的花费时间和调用次数
		if (_refCountMap[id]++ == 0)  //id为K
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
		lock_guard<mutex> lock(_mtex);

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
	vector<PP_MAP::iterator> vInfos;  //数组中每个元素都是迭代器 
	PP_MAP::iterator ppIt = ppMap.begin(); //遍历under_map
	while (ppIt != ppMap.end())
	{
		PPSection* section = ppIt->second;  //第二各参数 ：花费时间 调用次数等
		map<int, LongType>::iterator timeIt;  //线程用迭代器
		timeIt = section->_costTimeMap.begin();  //section->_costTimeMap是一个map
		while (timeIt != section->_costTimeMap.end())
		{
			section->_costTime += timeIt->second; //村时间
			section->_callCount += section->_callCountMap[timeIt->first]; //timeIt->first 对应ID

			++timeIt;
		}
		vInfos.push_back(ppIt); //找到一个 让其进map。 就知道有几个线程
		++ppIt;
	}
	struct SortByCostTime
	{
		bool operator()(PP_MAP::iterator l, PP_MAP::iterator r) const
		{
			return l->second->_costTime > r->second->_costTime;
		}
	};
	// 按花费时间排序
	sort(vInfos.begin(), vInfos.end(), SortByCostTime());

	int num = 1;
	//PP_MAP::iterator ppIt = _ppMap.begin();

	for (size_t i = 0; i < vInfos.size(); ++i)  //线程个数
	{
		ppIt = vInfos[i];  //获取第一个线程迭代器
		const PPNode& node = ppIt->first; //两个结构体
		PPSection* section = ppIt->second; 

		// node信息
		sa.Save("No.%d, Desc:%s\n", num++, node._desc.c_str());
		sa.Save("【一】Filename:%s, Line:%d, Function:%s\n"
			, node._filename.c_str()
			, node._line
			, node._function.c_str());

		// section信息
		map<int, LongType>::iterator timeIt;
		timeIt = section->_costTimeMap.begin();
		while (timeIt != section->_costTimeMap.end())
		{
			int i = 1;
			int id = timeIt->first;
			sa.Save("   【%d】ThreadId:%d, CostTime:%.2f s, CallCount:%lld\n"
				,i++
				, id
				, (double)timeIt->second / 1000
				, section->_callCountMap[id]);

			section->_costTime += timeIt->second;
			section->_callCount += section->_callCountMap[id];

			++timeIt;
		}

		sa.Save("【二】TotalCostTime:%.2f s, TotalCallCount:%lld, AverCostTime:%lld ms\n\n"
			, (double)section->_costTime / 1000
			, section->_callCount
			, section->_costTime / section->_callCount);

		++ppIt;
	}
}

void PerformanceProfiler::OutPut()
{
	//加了选项 要看启动这个了没
	int option = ConfigManager::GetInstance()->GetOptions();  //zui]]]djfldsnfkdsfksdjfslfj
	if (option & SAVE_TO_CONSOLE)
	{
		ConsoleSaveAdapter Con;
		_OutPut(Con);
	}

	if (option & SAVE_TO_FILE)
	{
		FileSaveAdapter Fil("log.txt");    //日志文件名
		_OutPut(Fil);
	}
}

PPSection* PerformanceProfiler::CreatePPSection(const char* filename, const char* function, size_t line, const char* desc)
{
	PPNode node(filename, function, line, desc);
	PPSection* section = NULL;

	//RAII机制枷锁
	lock_guard<mutex> lock(_mtex);    //RAII机制 这里枷锁，函数结束 调用析构 自己解锁
	//lock_guard 是类，将锁放在其中 当期对象析构时 自动释放锁

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