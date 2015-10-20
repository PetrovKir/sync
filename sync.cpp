// sync.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

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

	bool IsRaised() const { return false; }
	void Raise() {}
};


class Request
{
	int dataInt;
	LONG dataLong;
	std::string dataString;

public:
	void Dump() const {}

};

Request* CreateNewRequest()
{
	RandomDelaySimulation();
	return new Request();
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

DWORD WINAPI GetRequestThread(LPVOID lpParam)
{
	return S_OK;
}

DWORD WINAPI ProcessRequestThread(LPVOID lpParam)
{
	return S_OK;
}

int main()
{
	const int requestThreadCount = 3;
	DWORD requestThreadId[requestThreadCount];
	for (int i = 0; i < requestThreadCount; i++)
		CreateThread(NULL, 0, GetRequestThread, NULL, 0, &requestThreadId[i]);

	const int processThreadCount = 2;
	DWORD processThreadId[processThreadCount];
	for (int i = 0; i < processThreadCount; i++)
		CreateThread(NULL, 0, ProcessRequestThread, NULL, 0, &processThreadId[i]);

	return 0;
}

