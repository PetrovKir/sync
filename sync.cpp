// sync.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <memory>
#include <mutex>

void RandomDelaySimulation()
{
	DWORD delay = 1000; // default 1000 ms if rand failed
	const double constMaxDelayMS = 10000; // max delay in ms

	unsigned int randomNumber = 0;
	errno_t err = rand_s(&randomNumber);
	if (err == 0)
		delay = (DWORD)((double)randomNumber / ((double)UINT_MAX + 1) * constMaxDelayMS);
	Sleep( delay );
}


class Stopper
{
	HANDLE event;

public:
	Stopper() { event = nullptr; }
	virtual ~Stopper() { if (event) ::CloseHandle(event); event = nullptr; }

	bool Create()
	{
		return (event = ::CreateEvent(NULL, TRUE, FALSE, NULL)) != NULL;		
	}
	bool IsRaised() const 
	{ 
		if (event) 
			return ::WaitForSingleObject(event, 0) == WAIT_OBJECT_0;
		return false;
	}
	void Raise()
	{
		if (event)
			SetEvent(event);
	}
	void Reset()
	{
		if (event)
			ResetEvent(event);
	}
};

class CriticalSectionLock
{
	LPCRITICAL_SECTION pCriticalSection;

public:
	CriticalSectionLock(LPCRITICAL_SECTION cs) : pCriticalSection(cs)
	{
		if (pCriticalSection)
			EnterCriticalSection(pCriticalSection);
	}
	virtual ~CriticalSectionLock()
	{
		if (pCriticalSection)
			LeaveCriticalSection(pCriticalSection);
	}
};

namespace Request
{
	class Request
	{
		int dataInt;
		LONG dataLong;
		std::string dataString;

	public:
		Request(int i, LONG l, const std::string& s) : dataInt(i), dataLong(l), dataString(s) { }
		void Dump() const {}

	};
	typedef Request* RequestPtr;

	Request* CreateNewRequest()
	{
		RandomDelaySimulation();

		static int i = 0;
		static LONG l = 0;
		std::string s = "Request";

		return new Request(i++, l++, s);
	}


	// возвращает NULL, если объект stopSignal указывает на необходимость остановки,
	// либо указатель на память, которую в дальнейшем требуется удалить
	Request* GetRequest(Stopper stopSignal)
	{
		// FakeGetReque
		if (stopSignal.IsRaised())
			return NULL;

		return CreateNewRequest();
	}

	// обрабатывает запрос, но память не удаляет, завершает обработку досрочно, если
	// объект stopSignal указывает на необходимость остановки
	void ProcessRequest(Request* request, Stopper stopSignal)
	{
		if (stopSignal.IsRaised())
			return;

		RandomDelaySimulation();

		if (stopSignal.IsRaised())
			return;

		RandomDelaySimulation();
	}

	void DeleteRequest(Request* request)
	{
		if (request)
			delete request;	
	}
}; 


class Processor
{
	Stopper stopper;

	std::queue<Request::RequestPtr> requests; // очередь с запросами
	HANDLE queueEvent;
	CRITICAL_SECTION criticalSection;

public:
	static DWORD WINAPI GetRequestThread(LPVOID lpParam)
	{
		Processor* pProcessor = reinterpret_cast<Processor*>(lpParam);
		if (!pProcessor)
			return S_FALSE;

		while (!pProcessor->stopper.IsRaised())
		{
			Request::Request* request = Request::GetRequest(pProcessor->stopper);
			if (request)
			{
				try
				{
					CriticalSectionLock lock(&pProcessor->criticalSection);
					pProcessor->requests.push(request);
				}
				catch (std::exception& e)
				{
										
				}
			}
		}
		return S_OK;
	}

	static DWORD WINAPI ProcessRequestThread(LPVOID lpParam)
	{
		Processor* pProcessor = reinterpret_cast<Processor*>(lpParam);
		if (!pProcessor)
			return S_FALSE;

		while (!pProcessor->stopper.IsRaised())
		{
			CriticalSectionLock lock(&pProcessor->criticalSection);
			while (pProcessor->requests.empty())
			{
				;
			}

			Request::Request* request = pProcessor->requests.front();
			pProcessor->requests.pop();

			Request::ProcessRequest(request, pProcessor->stopper);
			Request::DeleteRequest(request);
		}
		return S_OK;
	}

	Processor()
	{
		stopper.Create();
		queueEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		InitializeCriticalSection(&criticalSection);
	}
	virtual ~Processor()
	{
		while (!requests.empty())
		{
			Request::Request* request = requests.front();
			requests.pop();
			Request::DeleteRequest(request);
		}
		CloseHandle(queueEvent);
		DeleteCriticalSection(&criticalSection);
	}

	void Run(int requestThreadCount, int processThreadCount)
	{
		std::vector<DWORD> requestThreadIds;
		requestThreadIds.reserve(requestThreadCount);
		for (auto i = 0; i < requestThreadCount; i++)
			CreateThread(NULL, 0, GetRequestThread, this, 0, &requestThreadIds[i]);

		std::vector<DWORD> processThreadIds;
		processThreadIds.reserve(processThreadCount);
		for (auto i = 0; i < processThreadCount; i++)
			CreateThread(NULL, 0, ProcessRequestThread, this, 0, &processThreadIds[i]);
	}
};


int main()
{
	Processor processor;
	processor.Run(2, 3);
	return 0;
}

