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

	bool Create() { event = ::CreateEvent(NULL, TRUE, FALSE, NULL);  }
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
	std::queue<std::shared_ptr<Request::Request> > requests;
	Stopper stopper;
	mutable std::mutex mt;

public:
	static DWORD WINAPI GetRequestThread(LPVOID lpParam)
	{
		while (!stopper.IsRaised())
		{
			Request::Request* request = Request::GetRequest(stopper);
			if (request)
			{
				std::lock<std::mutex> queue_lock(mt);
				requests.push(request);
			}
		}
		return S_OK;
	}

	static DWORD WINAPI ProcessRequestThread(LPVOID lpParam)
	{
		while (!stopper.IsRaised())
		{
			Lock queue_lock();
			Request::Request* request = requests.front();
			requests.pop();
			Request::ProcessRequest(request, stopper);
		}
		return S_OK;
	}

	Processor() {}
	virtual ~Processor()
	{
		// TODO free here
		//for (auto it = requests)
		//Request::DeleteRequest(request);
	}

	void Run(int requestThreadCount, int processThreadCount)
	{
		std::vector<DWORD> requestThreadIds;
		requestThreadIds.reserve(requestThreadCount);
		for (auto i = 0; i < requestThreadCount; i++)
			CreateThread(NULL, 0, GetRequestThread, NULL, 0, &requestThreadIds[i]);

		std::vector<DWORD> processThreadIds;
		processThreadIds.reserve(processThreadCount);
		for (auto i = 0; i < processThreadCount; i++)
			CreateThread(NULL, 0, ProcessRequestThread, NULL, 0, &processThreadIds[i]);
	}
};


int main()
{
	Processor processor;
	processor.Run(2, 3);
	return 0;
}

