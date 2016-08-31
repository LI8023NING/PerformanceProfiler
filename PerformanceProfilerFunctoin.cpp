#include"PerformanceProfiler.h"

void PPSection::Begin(int id)
{
	// id == -1ʱ��ʾ���̳߳���
	if (id != -1)
	{
		lock_guard<mutex> lock(_mtex);  //�߳���

		// �ֱ�ͳ���̺߳��ܵĻ���ʱ��͵��ô���
		if (_refCountMap[id]++ == 0)  //idΪK
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
	vector<PP_MAP::iterator> vInfos;  //������ÿ��Ԫ�ض��ǵ����� 
	PP_MAP::iterator ppIt = ppMap.begin(); //����under_map
	while (ppIt != ppMap.end())
	{
		PPSection* section = ppIt->second;  //�ڶ������� ������ʱ�� ���ô�����
		map<int, LongType>::iterator timeIt;  //�߳��õ�����
		timeIt = section->_costTimeMap.begin();  //section->_costTimeMap��һ��map
		while (timeIt != section->_costTimeMap.end())
		{
			section->_costTime += timeIt->second; //��ʱ��
			section->_callCount += section->_callCountMap[timeIt->first]; //timeIt->first ��ӦID

			++timeIt;
		}
		vInfos.push_back(ppIt); //�ҵ�һ�� �����map�� ��֪���м����߳�
		++ppIt;
	}
	struct SortByCostTime
	{
		bool operator()(PP_MAP::iterator l, PP_MAP::iterator r) const
		{
			return l->second->_costTime > r->second->_costTime;
		}
	};
	// ������ʱ������
	sort(vInfos.begin(), vInfos.end(), SortByCostTime());

	int num = 1;
	//PP_MAP::iterator ppIt = _ppMap.begin();

	for (size_t i = 0; i < vInfos.size(); ++i)  //�̸߳���
	{
		ppIt = vInfos[i];  //��ȡ��һ���̵߳�����
		const PPNode& node = ppIt->first; //�����ṹ��
		PPSection* section = ppIt->second; 

		// node��Ϣ
		sa.Save("No.%d, Desc:%s\n", num++, node._desc.c_str());
		sa.Save("��һ��Filename:%s, Line:%d, Function:%s\n"
			, node._filename.c_str()
			, node._line
			, node._function.c_str());

		// section��Ϣ
		map<int, LongType>::iterator timeIt;
		timeIt = section->_costTimeMap.begin();
		while (timeIt != section->_costTimeMap.end())
		{
			int i = 1;
			int id = timeIt->first;
			sa.Save("   ��%d��ThreadId:%d, CostTime:%.2f s, CallCount:%lld\n"
				,i++
				, id
				, (double)timeIt->second / 1000
				, section->_callCountMap[id]);

			section->_costTime += timeIt->second;
			section->_callCount += section->_callCountMap[id];

			++timeIt;
		}

		sa.Save("������TotalCostTime:%.2f s, TotalCallCount:%lld, AverCostTime:%lld ms\n\n"
			, (double)section->_costTime / 1000
			, section->_callCount
			, section->_costTime / section->_callCount);

		++ppIt;
	}
}

void PerformanceProfiler::OutPut()
{
	//����ѡ�� Ҫ�����������û
	int option = ConfigManager::GetInstance()->GetOptions();  //zui]]]djfldsnfkdsfksdjfslfj
	if (option & SAVE_TO_CONSOLE)
	{
		ConsoleSaveAdapter Con;
		_OutPut(Con);
	}

	if (option & SAVE_TO_FILE)
	{
		FileSaveAdapter Fil("log.txt");    //��־�ļ���
		_OutPut(Fil);
	}
}

PPSection* PerformanceProfiler::CreatePPSection(const char* filename, const char* function, size_t line, const char* desc)
{
	PPNode node(filename, function, line, desc);
	PPSection* section = NULL;

	//RAII���Ƽ���
	lock_guard<mutex> lock(_mtex);    //RAII���� ����������������� �������� �Լ�����
	//lock_guard ���࣬������������ ���ڶ�������ʱ �Զ��ͷ���

	PP_MAP::iterator it = ppMap.find(node);   //��map��Ѱ��node
	if (it != ppMap.end())
	{
		section = it->second;   //�ҵ��ˣ�������ļ���Ϣ��sectionָ��
	}
	else
	{
		section = new PPSection;  //
		ppMap.insert(pair<PPNode, PPSection*>(node, section));
	}

	return section;
}