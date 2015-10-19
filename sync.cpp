// sync.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>

#define _CRT_RAND_S
#include <stdlib.h>

void RandomDelaySimulation()
{
	const double constMaxDelayMS = 10000; // max delay in ms
	unsigned int randomNumber = 0;
	rand_s(&randomNumber);
	Sleep( MulDiv(randomNumber, constMaxDelayMS, UINT_MAX + 1) );
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

// ���������� NULL, ���� ������ stopSignal ��������� �� ������������� ���������,
// ���� ��������� �� ������, ������� � ���������� ��������� �������
Request* GetRequest(Stopper stopSignal)
{
	// FakeGetReque
	if (stopSignal.IsRaised())
		return NULL;

	return CreateNewRequest();
}

// ������������ ������, �� ������ �� �������, ��������� ��������� ��������, ����
// ������ stopSignal ��������� �� ������������� ���������
void ProcessRequest(Request* request, Stopper stopSignal)
{
	if (stopSignal.IsRaised())
		return;

	Sleep(15);

	if (stopSignal.IsRaised())
		return;

	Sleep(15);

	if (stopSignal.IsRaised())
		return;

	Sleep(15);
}

void DeleteRequest(Request* request)
{
	if (request)
		delete request;
}

int main()
{
	return 0;
}

